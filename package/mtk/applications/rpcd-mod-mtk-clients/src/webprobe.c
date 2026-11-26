/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Original-Author: coolsnowwolf <coolsnowwolf@gmail.com>
 * Upstream: https://github.com/coolsnowwolf/luci (openwrt-25.12)
 * Path: libs/rpcd-mod-luci/src/webprobe.c
 * Cherry-picked commit: 725ddc051fe44eac07cf39b70708e24bf1539bcc
 */
#include "mtk_clients.h"
/* Bounded, asynchronous TCP reachability checks for current DHCPv4 leases. */
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <libubox/avl.h>
#include <libubox/uloop.h>
#include <libubus.h>

#define WEB_MAX_CLIENTS 1024
#define WEB_CONCURRENCY 2
#define WEB_TIMEOUT_MS 750
#define WEB_INTERVAL_MS 300000
#define WEB_LEASE_REFRESH_MS 30000

static const uint16_t web_ports[] = { 80, 8080, 5666, 443, 4430, 5667 };
#define WEB_PORT_COUNT (sizeof(web_ports) / sizeof(web_ports[0]))

struct web_client {
	struct avl_node avl;
	struct in_addr address;
	unsigned char mac[6];
	uint64_t next_scan, checked;
	uint32_t generation;
	unsigned int next_port, pending, open_ports, checking;
	bool scanning, ready;
};
struct web_job {
	struct uloop_fd fd;
	struct uloop_timeout timeout;
	struct web_client *client;
	unsigned int port;
};
static struct avl_tree web_clients;
static struct web_job web_jobs[WEB_CONCURRENCY];
static struct uloop_timeout web_inventory, web_pump;
static uint32_t web_generation;
static bool web_initialized;
static struct blob_buf web_result;


static uint64_t web_now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int web_compare(const void *a, const void *b, void *priv)
{
	return memcmp(a, b, sizeof(struct in_addr));
}

static void web_close(struct web_job *job)
{
	uloop_timeout_cancel(&job->timeout);
	if (job->fd.fd >= 0) {
		uloop_fd_delete(&job->fd);
		close(job->fd.fd);
		job->fd.fd = -1;
	}
	job->client = NULL;
}

static void web_cancel_client(struct web_client *client)
{
	for (int i = 0; i < WEB_CONCURRENCY; i++)
		if (web_jobs[i].client == client)
			web_close(&web_jobs[i]);
	client->pending = 0;
	client->scanning = false;
}

static void web_finish(struct web_job *job, bool open)
{
	struct web_client *client = job->client;
	if (!client) return;
	if (open) client->checking |= 1U << job->port;
	client->pending--;
	web_close(job);
	if (client->next_port == WEB_PORT_COUNT && !client->pending) {
		client->open_ports = client->checking;
		client->ready = true;
		client->scanning = false;
		client->checked = web_now();
		client->next_scan = client->checked + WEB_INTERVAL_MS;
	}
	uloop_timeout_set(&web_pump, 250);
}

static void web_timeout(struct uloop_timeout *timeout)
{
	struct web_job *job = container_of(timeout, struct web_job, timeout);
	web_finish(job, false);
}

static void web_connected(struct uloop_fd *fd, unsigned int events)
{
	struct web_job *job = container_of(fd, struct web_job, fd);
	int error = 0;
	socklen_t len = sizeof(error);
	bool open = !getsockopt(fd->fd, SOL_SOCKET, SO_ERROR, &error, &len) && !error;
	web_finish(job, open);
}

static void web_start(struct web_job *job, struct web_client *client)
{
	job->client = client;
	job->port = client->next_port++;
	client->pending++;
	job->fd.fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	job->fd.cb = web_connected;
	job->timeout.cb = web_timeout;
	if (job->fd.fd < 0) { web_finish(job, false); return; }
	struct sockaddr_in remote = { .sin_family = AF_INET,
		.sin_addr = client->address, .sin_port = htons(web_ports[job->port]) };
	int ret = connect(job->fd.fd, (void *)&remote, sizeof(remote));
	if (!ret) { web_finish(job, true); return; }
	if (errno != EINPROGRESS || uloop_fd_add(&job->fd, ULOOP_WRITE) < 0) {
		web_finish(job, false);
		return;
	}
	uloop_timeout_set(&job->timeout, WEB_TIMEOUT_MS);
}

static void web_run(struct uloop_timeout *timeout)
{
	uint64_t now = web_now();
	struct web_client *client;
	for (int i = 0; i < WEB_CONCURRENCY; i++) {
		if (web_jobs[i].client) continue;
		struct web_client *selected = NULL;
		avl_for_each_element(&web_clients, client, avl) {
			if ((client->scanning && client->next_port >= WEB_PORT_COUNT) ||
			    (!client->scanning && now < client->next_scan))
				continue;
			if (!selected || client->next_scan < selected->next_scan)
				selected = client;
		}
		if (selected) {
			if (!selected->scanning) {
				selected->scanning = true;
				selected->next_port = selected->pending = selected->checking = 0;
			}
			web_start(&web_jobs[i], selected);
		}
	}
	/* A completed connect wakes the queue. When idle, sleep until the next
	 * due client instead of polling the cache every second. */
	bool free_slot = false;
	for (int i = 0; i < WEB_CONCURRENCY; i++)
		if (!web_jobs[i].client) free_slot = true;
	if (free_slot) {
		uint64_t delay = WEB_INTERVAL_MS;
		avl_for_each_element(&web_clients, client, avl) {
			if (client->scanning && client->next_port >= WEB_PORT_COUNT) continue;
			uint64_t due = client->scanning || client->next_scan <= now ? 250 : client->next_scan - now;
			if (due < delay) delay = due;
		}
		uloop_timeout_set(&web_pump, delay < 250 ? 250 : delay);
	}
}

static void web_add_client(const struct in_addr *address, const unsigned char *mac)
{
	uint32_t ip = ntohl(address->s_addr);
	struct web_client *client;
	/* Targets come only from local lease files, never from RPC input. */
	if (!ip || (ip >> 24) == 127 || (ip >> 24) == 0 || ip >= 0xe0000000U)
		return;
	client = avl_find_element(&web_clients, address, client, avl);
	if (!client) {
		if (web_clients.count >= WEB_MAX_CLIENTS || !(client = calloc(1, sizeof(*client))))
			return;
		client->address = *address;
		client->avl.key = &client->address;
		avl_insert(&web_clients, &client->avl);
	}
	if (memcmp(client->mac, mac, 6)) {
		web_cancel_client(client);
		memcpy(client->mac, mac, 6);
		client->open_ports = client->checking = 0;
		client->ready = false;
		client->checked = client->next_scan = 0;
	}
	client->generation = web_generation;
}

static void web_refresh(struct uloop_timeout *timeout)
{
	struct web_client *client, *next;
	web_generation++;
	mtk_dhcp4_clients(web_add_client);
	avl_for_each_element_safe(&web_clients, client, avl, next) {
		if (client->generation != web_generation) {
			web_cancel_client(client);
			avl_delete(&web_clients, &client->avl);
			free(client);
		}
	}
	uloop_timeout_set(&web_pump, 250);
	uloop_timeout_set(&web_inventory, WEB_LEASE_REFRESH_MS);
}

static int web_get(struct ubus_context *ctx, struct ubus_object *object,
                  struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct web_client *client;
	uint64_t now = web_now();
	blob_buf_init(&web_result, 0);
	blobmsg_add_u32(&web_result, "interval_ms", WEB_INTERVAL_MS);
	void *clients = blobmsg_open_table(&web_result, "clients");
	avl_for_each_element(&web_clients, client, avl) {
		char ip[INET_ADDRSTRLEN], mac[18];
		inet_ntop(AF_INET, &client->address, ip, sizeof(ip));
		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
		         client->mac[0], client->mac[1], client->mac[2], client->mac[3], client->mac[4], client->mac[5]);
		void *entry = blobmsg_open_table(&web_result, ip);
		blobmsg_add_string(&web_result, "mac", mac);
		blobmsg_add_u8(&web_result, "ready", client->ready);
		blobmsg_add_u32(&web_result, "age_ms", client->ready ? now - client->checked : 0);
		void *ports = blobmsg_open_array(&web_result, "ports");
		if (client->ready && now - client->checked < 2 * WEB_INTERVAL_MS)
			for (int i = 0; i < WEB_PORT_COUNT; i++)
				if (client->open_ports & (1U << i))
					blobmsg_add_u32(&web_result, NULL, web_ports[i]);
		blobmsg_close_array(&web_result, ports);
		blobmsg_close_table(&web_result, entry);
	}
	blobmsg_close_table(&web_result, clients);
	ubus_send_reply(ctx, req, web_result.head);
	return 0;
}

int mtk_webprobe_init(struct ubus_context *ctx)
{
	static const struct ubus_method methods[] = { UBUS_METHOD_NOARG("get", web_get) };
	static struct ubus_object_type type = UBUS_OBJECT_TYPE("luci-client-web", methods);
	static struct ubus_object object = { .name = "luci.client-web", .type = &type,
		.methods = methods, .n_methods = 1 };
	int ret = ubus_add_object(ctx, &object);
	if (!ret && !web_initialized) {
		avl_init(&web_clients, web_compare, false, NULL);
		for (int i = 0; i < WEB_CONCURRENCY; i++) web_jobs[i].fd.fd = -1;
		web_inventory.cb = web_refresh;
		web_pump.cb = web_run;
		web_initialized = true;
		uloop_timeout_set(&web_inventory, 5000);
	}
	return ret;
}
