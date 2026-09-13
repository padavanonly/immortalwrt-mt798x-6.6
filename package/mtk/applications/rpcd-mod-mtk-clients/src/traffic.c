/* SPDX-License-Identifier: Apache-2.0 */
#include "mtk_clients.h"
/* Read-only conntrack accounting, including ECM/NSS synchronized counters. */
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
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <libubox/avl.h>
#include <libubox/uloop.h>
#include <libubus.h>

#define SAMPLE_MS 2000
#define WINDOW_SAMPLES 3
#define IDLE_MS 30000
#define MAX_CLIENTS 1024
#define MAX_FLOWS 32768

struct address { uint8_t family, bytes[16]; };
struct flow_key {
	struct address src, dst, reply_src;
	uint32_t id;
	uint16_t sport, dport, zone;
	uint8_t protocol;
};
struct flow {
	struct avl_node avl;
	struct flow_key key;
	uint64_t bytes[2];
	uint32_t generation;
	bool destroyed;
};
struct client {
	struct avl_node avl;
	struct address address;
	uint64_t bytes[2], rate[2], touched;
	uint64_t history[WINDOW_SAMPLES][2], duration[WINDOW_SAMPLES];
	unsigned int next_sample;
	bool warm, ready, missing;
};
static struct avl_tree clients, flows;
static struct uloop_fd ct_fd = { .fd = -1 };
static struct uloop_timeout timer;
static uint32_t sequence, generation;
static uint64_t dump_started, sampled, last_request;
static bool dumping, initialized;
static const char *failure = "warming_up";
static struct blob_buf result;

int mtk_traffic_init(struct ubus_context *ctx);

static uint64_t now_ms(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int address_cmp(const void *a, const void *b, void *priv)
{
	return memcmp(a, b, sizeof(struct address));
}

static int flow_cmp(const void *a, const void *b, void *priv)
{
	return memcmp(a, b, sizeof(struct flow_key));
}

static bool parse_address(const char *text, struct address *a)
{
	memset(a, 0, sizeof(*a));
	if (inet_pton(AF_INET, text, a->bytes) == 1) {
		a->family = AF_INET;
		return true;
	}
	if (inet_pton(AF_INET6, text, a->bytes) == 1) {
		a->family = AF_INET6;
		return true;
	}
	return false;
}

static void clear_flows(void)
{
	struct flow *f, *tmp;
	avl_for_each_element_safe(&flows, f, avl, tmp) {
		avl_delete(&flows, &f->avl);
		free(f);
	}
}

static void reset_sampler(const char *reason)
{
	struct client *c;
	if (ct_fd.fd >= 0) {
		uloop_fd_delete(&ct_fd);
		close(ct_fd.fd);
		ct_fd.fd = -1;
	}
	clear_flows();
	avl_for_each_element(&clients, c, avl) {
		c->warm = c->ready = false;
		c->bytes[0] = c->bytes[1] = 0;
		memset(c->history, 0, sizeof(c->history));
		memset(c->duration, 0, sizeof(c->duration));
		c->next_sample = 0;
	}
	dumping = false;
	sampled = 0;
	failure = reason;
}

/* All attributes come from the kernel, but validate lengths before reading. */
static struct nlattr *attr_find(void *data, size_t len, unsigned int type)
{
	struct nlattr *a = data;
	while (len >= sizeof(*a) && a->nla_len >= sizeof(*a) && a->nla_len <= len) {
		if ((a->nla_type & NLA_TYPE_MASK) == type)
			return a;
		size_t step = NLA_ALIGN(a->nla_len);
		if (step > len)
			break;
		len -= step;
		a = (void *)((char *)a + step);
	}
	return NULL;
}

static struct nlattr *nested(struct nlattr *a, unsigned int type)
{
	return a ? attr_find((char *)a + NLA_HDRLEN, a->nla_len - NLA_HDRLEN, type) : NULL;
}

static bool attr_copy(struct nlattr *a, void *out, size_t len)
{
	if (!a || a->nla_len < NLA_HDRLEN + len)
		return false;
	memcpy(out, (char *)a + NLA_HDRLEN, len);
	return true;
}

static bool tuple_address(struct nlattr *tuple, unsigned int family, bool source, struct address *addr)
{
	struct nlattr *ip = nested(tuple, CTA_TUPLE_IP);
	unsigned int type = family == AF_INET ? (source ? CTA_IP_V4_SRC : CTA_IP_V4_DST) :
		(source ? CTA_IP_V6_SRC : CTA_IP_V6_DST);
	addr->family = family;
	return attr_copy(nested(ip, type), addr->bytes, family == AF_INET ? 4 : 16);
}

static bool counter_value(struct nlattr *a, uint64_t *value)
{
	unsigned char bytes[8];
	if (!attr_copy(nested(a, CTA_COUNTERS_BYTES), bytes, sizeof(bytes)))
		return false;
	*value = 0;
	for (int i = 0; i < 8; i++)
		*value = (*value << 8) | bytes[i];
	return true;
}

static void add_bytes(struct client *c, uint64_t up, uint64_t down)
{
	if (c && c->warm) {
		c->bytes[0] += up;
		c->bytes[1] += down;
	}
}

static bool process_flow(struct nlmsghdr *nlh)
{
	struct nfgenmsg *msg = NLMSG_DATA(nlh);
	struct flow_key key = {0};
	struct flow *f;
	struct client *src, *dst;
	uint64_t bytes[2], delta[2] = {0};
	bool destroyed = (nlh->nlmsg_type & 0xff) == IPCTNL_MSG_CT_DELETE;
	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*msg)) ||
	    (msg->nfgen_family != AF_INET && msg->nfgen_family != AF_INET6))
		return true;
	void *data = (char *)msg + NLMSG_ALIGN(sizeof(*msg));
	size_t len = nlh->nlmsg_len - NLMSG_LENGTH(sizeof(*msg));
	struct nlattr *orig = attr_find(data, len, CTA_TUPLE_ORIG);
	struct nlattr *reply = attr_find(data, len, CTA_TUPLE_REPLY);
	struct nlattr *proto = nested(orig, CTA_TUPLE_PROTO);
	if (!tuple_address(orig, msg->nfgen_family, true, &key.src) ||
	    !tuple_address(orig, msg->nfgen_family, false, &key.dst) ||
	    !tuple_address(reply, msg->nfgen_family, true, &key.reply_src) ||
	    !attr_copy(nested(proto, CTA_PROTO_NUM), &key.protocol, 1) ||
	    !attr_copy(attr_find(data, len, CTA_ID), &key.id, 4))
		return true;
	attr_copy(nested(proto, CTA_PROTO_SRC_PORT), &key.sport, 2);
	attr_copy(nested(proto, CTA_PROTO_DST_PORT), &key.dport, 2);
	attr_copy(attr_find(data, len, CTA_ZONE), &key.zone, 2);
	src = avl_find_element(&clients, &key.src, src, avl);
	/* reply.src identifies the internal destination of a DNAT connection. */
	dst = avl_find_element(&clients, &key.reply_src, dst, avl);
	if (!src && !dst)
		return true;
	if (!counter_value(attr_find(data, len, CTA_COUNTERS_ORIG), &bytes[0]) ||
	    !counter_value(attr_find(data, len, CTA_COUNTERS_REPLY), &bytes[1])) {
		if (src) src->missing = true;
		if (dst) dst->missing = true;
		return true;
	}
	f = avl_find_element(&flows, &key, f, avl);
	if (!f) {
		if (flows.count >= MAX_FLOWS)
			return false;
		f = calloc(1, sizeof(*f));
		if (!f)
			return false;
		f->key = key;
		f->avl.key = &f->key;
		avl_insert(&flows, &f->avl);
	}
	/* A dump record can be queued before a destroy notification is delivered.
	 * Keep tombstones for two rounds to avoid counting that old record twice. */
	if (f->destroyed)
		return true;
	for (int i = 0; i < 2; i++) {
		/* A counter reset starts a new baseline; never subtract unsigned values. */
		if (bytes[i] >= f->bytes[i])
			delta[i] = bytes[i] - f->bytes[i];
		f->bytes[i] = bytes[i];
	}
	add_bytes(src, delta[0], delta[1]);
	if (dst != src)
		add_bytes(dst, delta[1], delta[0]);
	f->generation = generation;
	if (destroyed)
		f->destroyed = true;
	return true;
}

/* Weight by actual elapsed time, not an average of rounded sample rates. */
static void update_rate(struct client *c, uint64_t elapsed)
{
	if (!c->ready) {
		memset(c->history, 0, sizeof(c->history));
		memset(c->duration, 0, sizeof(c->duration));
		c->next_sample = 0;
	} else {
		memcpy(c->history[c->next_sample], c->bytes, sizeof(c->bytes));
		c->duration[c->next_sample] = elapsed;
		c->next_sample = (c->next_sample + 1) % WINDOW_SAMPLES;
	}
	uint64_t duration = 0, bytes[2] = {0};
	for (int j = 0; j < WINDOW_SAMPLES; j++) {
		duration += c->duration[j];
		bytes[0] += c->history[j][0];
		bytes[1] += c->history[j][1];
	}
	for (int i = 0; i < 2; i++) {
		c->rate[i] = duration ? (uint64_t)((double)bytes[i] * 1000 / duration) : 0;
		c->bytes[i] = 0;
	}
}

static void finish_dump(void)
{
	uint64_t now = now_ms(), elapsed = now - sampled;
	struct client *c;
	struct flow *f, *tmp;
	avl_for_each_element(&clients, c, avl) {
		c->ready = c->warm && sampled && elapsed && !c->missing;
		update_rate(c, elapsed);
		c->warm = true;
		c->missing = false;
	}
	avl_for_each_element_safe(&flows, f, avl, tmp) {
		if ((uint32_t)(generation - f->generation) > 1) {
			avl_delete(&flows, &f->avl);
			free(f);
		}
	}
	sampled = now;
	dumping = false;
	failure = NULL;
}

static void receive_conntrack(struct uloop_fd *fd, unsigned int events)
{
	/* Bound each callback so a large table cannot starve other rpcd requests. */
	for (int batch = 0; batch < 32; batch++) {
		char buf[65536];
		struct sockaddr_nl sender;
		struct iovec iov = { .iov_base = buf, .iov_len = sizeof(buf) };
		struct msghdr msg = { .msg_name = &sender, .msg_namelen = sizeof(sender),
			.msg_iov = &iov, .msg_iovlen = 1 };
		ssize_t len = recvmsg(fd->fd, &msg, MSG_DONTWAIT);
		if (len < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				return;
			reset_sampler("netlink_overrun");
			return;
		}
		if (!len || (msg.msg_flags & MSG_TRUNC)) {
			reset_sampler("truncated_dump");
			return;
		}
		if (sender.nl_pid)
			continue;
		struct nlmsghdr *nlh;
		for (nlh = (void *)buf; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
			bool response = dumping && nlh->nlmsg_seq == sequence;
			if (response && (nlh->nlmsg_flags & NLM_F_DUMP_INTR)) {
				reset_sampler("interrupted_dump");
				return;
			}
			if (nlh->nlmsg_type == NLMSG_ERROR) {
				reset_sampler("netlink_error");
				return;
			}
			if (nlh->nlmsg_type == NLMSG_DONE) {
				if (response) finish_dump();
				continue;
			}
			if ((nlh->nlmsg_type >> 8) != NFNL_SUBSYS_CTNETLINK)
				continue;
			if (response || (nlh->nlmsg_type & 0xff) == IPCTNL_MSG_CT_DELETE) {
				if (!process_flow(nlh)) {
					reset_sampler("flow_limit");
					return;
				}
			}
		}
	}
}

static bool open_conntrack(void)
{
	struct sockaddr_nl addr = { .nl_family = AF_NETLINK,
		.nl_groups = 1 << (NFNLGRP_CONNTRACK_DESTROY - 1) };
	int size = 1024 * 1024;
	ct_fd.fd = socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, NETLINK_NETFILTER);
	if (ct_fd.fd < 0)
		return false;
	setsockopt(ct_fd.fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
	if (bind(ct_fd.fd, (void *)&addr, sizeof(addr)) < 0) {
		close(ct_fd.fd);
		ct_fd.fd = -1;
		return false;
	}
	ct_fd.cb = receive_conntrack;
	if (uloop_fd_add(&ct_fd, ULOOP_READ) < 0) {
		close(ct_fd.fd);
		ct_fd.fd = -1;
		return false;
	}
	return true;
}

static void sample_timer(struct uloop_timeout *t)
{
	uint64_t now = now_ms();
	struct client *c, *tmp;
	if (now - last_request > IDLE_MS) {
		reset_sampler("idle");
		avl_for_each_element_safe(&clients, c, avl, tmp) {
			avl_delete(&clients, &c->avl);
			free(c);
		}
		return;
	}
	uloop_timeout_set(&timer, SAMPLE_MS);
	if (dumping) {
		if (now - dump_started > 10000)
			reset_sampler("dump_timeout");
		return;
	}
	FILE *acct = fopen("/proc/sys/net/netfilter/nf_conntrack_acct", "r");
	int enabled = 0;
	if (acct) { enabled = fgetc(acct) == '1'; fclose(acct); }
	if (!enabled) {
		reset_sampler("accounting_disabled");
		return;
	}
	if (ct_fd.fd < 0 && !open_conntrack()) {
		failure = "netlink_unavailable";
		return;
	}
	struct { struct nlmsghdr nlh; struct nfgenmsg nfg; } request = {
		.nlh = { .nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg)),
			.nlmsg_type = (NFNL_SUBSYS_CTNETLINK << 8) | IPCTNL_MSG_CT_GET,
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP, .nlmsg_seq = ++sequence },
		.nfg = { .nfgen_family = AF_UNSPEC, .version = NFNETLINK_V0 }
	};
	struct sockaddr_nl kernel = { .nl_family = AF_NETLINK };
	/* Never use CT_GET_CTRZERO: other accounting consumers keep their counters. */
	if (sendto(ct_fd.fd, &request, request.nlh.nlmsg_len, MSG_DONTWAIT,
	           (void *)&kernel, sizeof(kernel)) < 0) {
		reset_sampler("send_failed");
		return;
	}
	generation++;
	dumping = true;
	dump_started = now;
}

static const struct blobmsg_policy policy[] = {
	{ .name = "addresses", .type = BLOBMSG_TYPE_ARRAY }
};

static int get_rates(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[1], *a;
	struct address address;
	struct client *c, *tmp;
	uint64_t now = now_ms();
	int rem, count = 0;
	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;
	blobmsg_parse(policy, 1, tb, blob_data(msg), blob_len(msg));
	if (!tb[0])
		return UBUS_STATUS_INVALID_ARGUMENT;
	blobmsg_for_each_attr(a, tb[0], rem) {
		if (++count > MAX_CLIENTS || blobmsg_type(a) != BLOBMSG_TYPE_STRING ||
		    !parse_address(blobmsg_get_string(a), &address))
			return UBUS_STATUS_INVALID_ARGUMENT;
	}
	avl_for_each_element_safe(&clients, c, avl, tmp) {
		if (now - c->touched > IDLE_MS) {
			avl_delete(&clients, &c->avl);
			free(c);
		}
	}
	blobmsg_for_each_attr(a, tb[0], rem) {
		parse_address(blobmsg_get_string(a), &address);
		c = avl_find_element(&clients, &address, c, avl);
		if (!c) {
			if (clients.count >= MAX_CLIENTS)
				return UBUS_STATUS_NOT_SUPPORTED;
			c = calloc(1, sizeof(*c));
			if (!c)
				return UBUS_STATUS_UNKNOWN_ERROR;
			c->address = address;
			c->avl.key = &c->address;
			avl_insert(&clients, &c->avl);
		}
		c->touched = now;
	}
	last_request = now;
	if (!timer.pending)
		uloop_timeout_set(&timer, 1);
	blob_buf_init(&result, 0);
	blobmsg_add_string(&result, "source", "conntrack");
	blobmsg_add_u32(&result, "interval_ms", SAMPLE_MS);
	blobmsg_add_u32(&result, "window_ms", SAMPLE_MS * WINDOW_SAMPLES);
	blobmsg_add_u32(&result, "age_ms", sampled ? now - sampled : 0);
	if (failure)
		blobmsg_add_string(&result, "status", failure);
	void *table = blobmsg_open_table(&result, "rates");
	blobmsg_for_each_attr(a, tb[0], rem) {
		parse_address(blobmsg_get_string(a), &address);
		c = avl_find_element(&clients, &address, c, avl);
		void *entry = blobmsg_open_table(&result, blobmsg_get_string(a));
		bool ready = c && c->ready && sampled && now - sampled < 3 * SAMPLE_MS;
		blobmsg_add_u8(&result, "ready", ready);
		if (ready) {
			blobmsg_add_u64(&result, "upload", c->rate[0]);
			blobmsg_add_u64(&result, "download", c->rate[1]);
		}
		blobmsg_close_table(&result, entry);
	}
	blobmsg_close_table(&result, table);
	ubus_send_reply(ctx, req, result.head);
	return 0;
}

int mtk_traffic_init(struct ubus_context *ctx)
{
	static const struct ubus_method methods[] = { UBUS_METHOD("get", get_rates, policy) };
	static struct ubus_object_type type = UBUS_OBJECT_TYPE("luci-client-rates", methods);
	static struct ubus_object obj = { .name = "luci.client-rates", .type = &type,
		.methods = methods, .n_methods = 1 };
	if (!initialized) {
		avl_init(&clients, address_cmp, false, NULL);
		avl_init(&flows, flow_cmp, false, NULL);
		timer.cb = sample_timer;
		initialized = true;
	}
	return ubus_add_object(ctx, &obj);
}
