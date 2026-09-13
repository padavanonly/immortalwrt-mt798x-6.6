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
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <libubox/avl.h>
#include <libubox/uloop.h>
#include <libubus.h>

#define SAMPLE_MS 2000
#define WINDOW_SAMPLES 3
#define ADDRESS_IDLE_MS (24ULL * 60 * 60 * 1000)
#define MAX_HOSTS 4096
#ifndef STATE_FILE
#define STATE_FILE "/tmp/luci-client-traffic.state"
#endif
#define MAX_CLIENTS 8192
#define MAX_REQUEST 1024
#define MAX_FLOWS 32768

struct address { uint8_t family, bytes[16]; };
struct flow_key {
	struct address src, dst, reply_src;
	uint32_t id;
	uint16_t sport, dport, zone;
	uint8_t protocol;
};
struct host {
	struct avl_node avl;
	unsigned char mac[6];
	uint64_t total[2];
	bool available;
};
struct flow {
	struct avl_node avl;
	struct flow_key key;
	uint64_t bytes[2];
	unsigned char owner[2][6];
	uint32_t generation;
	bool destroyed;
};
struct client {
	struct avl_node avl;
	struct address address;
	unsigned char mac[6];
	uint64_t bytes[2], rate[2], touched;
	uint64_t history[WINDOW_SAMPLES][2], duration[WINDOW_SAMPLES];
	unsigned int next_sample;
	bool warm, ready, missing;
};
static struct avl_tree clients, flows, hosts;
static struct uloop_fd ct_fd = { .fd = -1 };
static struct uloop_timeout timer;
static uint32_t sequence, generation;
static uint64_t dump_started, sampled, started;
static bool incomplete;
static bool dumping, initialized;
static const char *failure = "warming_up";
static struct blob_buf result;

static void save_state(void);
static void load_state(void);

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

static int mac_cmp(const void *a, const void *b, void *priv)
{
	return memcmp(a, b, 6);
}

static struct host *get_host(const unsigned char *mac, bool create)
{
	static const unsigned char zero[6];
	struct host *h;
	if (!memcmp(mac, zero, 6) || (mac[0] & 1))
		return NULL;
	h = avl_find_element(&hosts, mac, h, avl);
	if (!h && create) {
		if (hosts.count >= MAX_HOSTS || !(h = calloc(1, sizeof(*h)))) {
			incomplete = true;
			return NULL;
		}
		memcpy(h->mac, mac, 6);
		h->avl.key = h->mac;
		avl_insert(&hosts, &h->avl);
	}
	return h;
}

static void bind_client(int family, const void *bytes, const unsigned char *mac)
{
	struct address a = { .family = family };
	struct client *c;
	if ((family != AF_INET && family != AF_INET6) || !get_host(mac, true))
		return;
	memcpy(a.bytes, bytes, family == AF_INET ? 4 : 16);
	c = avl_find_element(&clients, &a, c, avl);
	if (!c) {
		if (clients.count >= MAX_CLIENTS || !(c = calloc(1, sizeof(*c)))) {
			incomplete = true;
			return;
		}
		c->address = a;
		c->avl.key = &c->address;
		avl_insert(&clients, &c->avl);
	}
	if (memcmp(c->mac, mac, 6)) {
		memcpy(c->mac, mac, 6);
		c->warm = c->ready = false;
		memset(c->bytes, 0, sizeof(c->bytes));
		memset(c->history, 0, sizeof(c->history));
		memset(c->duration, 0, sizeof(c->duration));
		c->next_sample = 0;
	}
	c->touched = now_ms();
}

/* Apply one final binding per address: a stale neighbour and a current DHCP
 * lease must not reset the same client's rate baseline twice per scan. */
struct binding { struct avl_node avl; struct address address; unsigned char mac[6]; };
static struct avl_tree bindings;

static void stage_binding(int family, const void *bytes, const unsigned char *mac)
{
	struct address address = { .family = family };
	struct binding *b;
	if (family != AF_INET && family != AF_INET6) return;
	memcpy(address.bytes, bytes, family == AF_INET ? 4 : 16);
	b = avl_find_element(&bindings, &address, b, avl);
	if (!b) {
		if (bindings.count >= MAX_CLIENTS || !(b = calloc(1, sizeof(*b)))) {
			incomplete = true;
			return;
		}
		b->address = address;
		b->avl.key = &b->address;
		avl_insert(&bindings, &b->avl);
	}
	memcpy(b->mac, mac, 6);
}

static void discover_clients(void)
{
	struct binding *b, *next;
	avl_init(&bindings, address_cmp, false, NULL);
	mtk_traffic_discover(stage_binding);
	avl_for_each_element_safe(&bindings, b, avl, next) {
		bind_client(b->address.family, b->address.bytes, b->mac);
		avl_delete(&bindings, &b->avl);
		free(b);
	}
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
	incomplete = true;
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

/* A complete checkpoint lives in tmpfs, never on flash. Keep flow baselines
 * together with totals so an rpcd reload cannot count live connections twice. */
struct state_header {
	char magic[8], boot[40];
	uint64_t started;
	uint32_t hosts, clients, flows, generation, incomplete;
};
struct host_record { unsigned char mac[6]; uint64_t total[2]; bool available; };
struct client_record { struct address address; unsigned char mac[6]; uint64_t touched; };
struct flow_record {
	struct flow_key key;
	uint64_t bytes[2];
	unsigned char owner[2][6];
	uint32_t generation;
	bool destroyed;
};

static void boot_id(char *out, size_t len)
{
	FILE *f = fopen("/proc/sys/kernel/random/boot_id", "r");
	if (f) { if (!fgets(out, len, f)) out[0] = 0; fclose(f); }
}

static void save_state(void)
{
	struct state_header header = { .magic = "LCTRAF1", .started = started,
		.hosts = hosts.count, .clients = clients.count, .flows = flows.count,
		.generation = generation, .incomplete = incomplete };
	struct host *h;
	struct client *c;
	struct flow *flow;
	bool ok = true;
	boot_id(header.boot, sizeof(header.boot));
	if (!header.boot[0]) return;
	int fd = open(STATE_FILE ".new", O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW | O_CLOEXEC, 0600);
	if (fd < 0) return;
	FILE *f = fdopen(fd, "wb");
	if (!f) { close(fd); return; }
	ok = fwrite(&header, sizeof(header), 1, f) == 1;
	avl_for_each_element(&hosts, h, avl) {
		struct host_record r = {0};
		memcpy(r.mac, h->mac, 6);
		memcpy(r.total, h->total, sizeof(r.total));
		r.available = h->available;
		ok &= fwrite(&r, sizeof(r), 1, f) == 1;
	}
	avl_for_each_element(&clients, c, avl) {
		struct client_record r = { .address = c->address, .touched = c->touched };
		memcpy(r.mac, c->mac, 6);
		ok &= fwrite(&r, sizeof(r), 1, f) == 1;
	}
	avl_for_each_element(&flows, flow, avl) {
		struct flow_record r = { .key = flow->key, .generation = flow->generation,
			.destroyed = flow->destroyed };
		memcpy(r.bytes, flow->bytes, sizeof(r.bytes));
		memcpy(r.owner, flow->owner, sizeof(r.owner));
		ok &= fwrite(&r, sizeof(r), 1, f) == 1;
	}
	if (fclose(f)) ok = false;
	if (ok) rename(STATE_FILE ".new", STATE_FILE);
	else unlink(STATE_FILE ".new");
}

static void load_state(void)
{
	struct state_header header;
	char boot[40] = {0};
	struct stat st;
	int fd = open(STATE_FILE, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0) return;
	FILE *f = fdopen(fd, "rb");
	if (!f) { close(fd); return; }
	boot_id(boot, sizeof(boot));
	if (fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_uid != geteuid() ||
	    fread(&header, sizeof(header), 1, f) != 1 ||
	    memcmp(header.magic, "LCTRAF1", 8) || !boot[0] || memcmp(header.boot, boot, sizeof(boot)) ||
	    header.hosts > MAX_HOSTS || header.clients > MAX_CLIENTS || header.flows > MAX_FLOWS ||
	    header.started > now_ms() || st.st_size != sizeof(header) +
	    header.hosts * sizeof(struct host_record) + header.clients * sizeof(struct client_record) +
	    header.flows * sizeof(struct flow_record)) goto out;
	for (uint32_t i = 0; i < header.hosts; i++) {
		struct host_record r;
		struct host *h;
		if (fread(&r, sizeof(r), 1, f) != 1 || !(h = get_host(r.mac, true))) goto fail;
		memcpy(h->total, r.total, sizeof(h->total));
		h->available = r.available;
	}
	for (uint32_t i = 0; i < header.clients; i++) {
		struct client_record r;
		struct client *c;
		if (fread(&r, sizeof(r), 1, f) != 1) goto fail;
		bind_client(r.address.family, r.address.bytes, r.mac);
		c = avl_find_element(&clients, &r.address, c, avl);
		if (!c) goto fail;
		c->touched = r.touched;
	}
	for (uint32_t i = 0; i < header.flows; i++) {
		struct flow_record r;
		struct flow *flow;
		if (fread(&r, sizeof(r), 1, f) != 1 || !(flow = calloc(1, sizeof(*flow)))) goto fail;
		flow->key = r.key;
		flow->avl.key = &flow->key;
		memcpy(flow->bytes, r.bytes, sizeof(flow->bytes));
		memcpy(flow->owner, r.owner, sizeof(flow->owner));
		flow->generation = r.generation;
		flow->destroyed = r.destroyed;
		if (avl_insert(&flows, &flow->avl)) { free(flow); goto fail; }
	}
	started = header.started;
	generation = header.generation;
	/* Connections ending while rpcd was stopped cannot be recovered. */
	incomplete = true;
	goto out;
fail:;
	struct client *c, *cn;
	struct host *h, *hn;
	clear_flows();
	avl_for_each_element_safe(&clients, c, avl, cn) { avl_delete(&clients, &c->avl); free(c); }
	avl_for_each_element_safe(&hosts, h, avl, hn) { avl_delete(&hosts, &h->avl); free(h); }
	incomplete = true;
out:
	fclose(f);
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

static void add_bytes(struct client *c, const unsigned char *owner, uint64_t up, uint64_t down, bool count)
{
	struct host *h = get_host(owner, false);
	if (h) h->available = true;
	if (count && h) {
		h->total[0] += up;
		h->total[1] += down;
	}
	if (c && c->warm && !memcmp(c->mac, owner, 6)) {
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
	bool known;
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
	f = avl_find_element(&flows, &key, f, avl);
	known = f != NULL;
	if (!src && !dst && !f)
		return true;
	if (!counter_value(attr_find(data, len, CTA_COUNTERS_ORIG), &bytes[0]) ||
	    !counter_value(attr_find(data, len, CTA_COUNTERS_REPLY), &bytes[1])) {
		if (src) src->missing = true;
		if (dst) dst->missing = true;
		incomplete = true;
		return true;
	}
	if (!f) {
		if (flows.count >= MAX_FLOWS)
			return false;
		f = calloc(1, sizeof(*f));
		if (!f)
			return false;
		f->key = key;
		if (src) memcpy(f->owner[0], src->mac, 6);
		if (dst) memcpy(f->owner[1], dst->mac, 6);
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
	add_bytes(src, f->owner[0], delta[0], delta[1], known || (src && src->warm));
	if (memcmp(f->owner[0], f->owner[1], 6))
		add_bytes(dst, f->owner[1], delta[1], delta[0], known || (dst && dst->warm));
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
	save_state();
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
					/* Drop baselines only at capacity, allowing a later smaller dump
					 * to recover. Cold clients will establish a fresh baseline. */
					clear_flows();
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
	avl_for_each_element_safe(&clients, c, avl, tmp) {
		if (now - c->touched > ADDRESS_IDLE_MS) {
			avl_delete(&clients, &c->avl);
			free(c);
		}
	}
	uloop_timeout_set(&timer, SAMPLE_MS);
	if (dumping) {
		if (now - dump_started > 10000)
			reset_sampler("dump_timeout");
		return;
	}
	discover_clients();
	FILE *acct = fopen("/proc/sys/net/netfilter/nf_conntrack_acct", "r");
	int enabled = 0;
	if (acct) { enabled = fgetc(acct) == '1'; fclose(acct); }
	if (!enabled) {
		reset_sampler("accounting_disabled");
		uloop_timeout_set(&timer, 10000);
		return;
	}
	if (ct_fd.fd < 0 && !open_conntrack()) {
		failure = "netlink_unavailable";
		uloop_timeout_set(&timer, 10000);
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
	struct client *c;
	uint64_t now = now_ms();
	int rem, count = 0;
	if (!msg)
		return UBUS_STATUS_INVALID_ARGUMENT;
	blobmsg_parse(policy, 1, tb, blob_data(msg), blob_len(msg));
	if (!tb[0])
		return UBUS_STATUS_INVALID_ARGUMENT;
	blobmsg_for_each_attr(a, tb[0], rem) {
		if (++count > MAX_REQUEST || blobmsg_type(a) != BLOBMSG_TYPE_STRING ||
		    !parse_address(blobmsg_get_string(a), &address))
			return UBUS_STATUS_INVALID_ARGUMENT;
	}
	blob_buf_init(&result, 0);
	blobmsg_add_string(&result, "source", "conntrack");
	blobmsg_add_u64(&result, "started_ms", started);
	blobmsg_add_u8(&result, "incomplete", incomplete);
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
		struct host *h = c ? get_host(c->mac, false) : NULL;
		if (h) {
			char mac[18];
			snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
			         h->mac[0], h->mac[1], h->mac[2], h->mac[3], h->mac[4], h->mac[5]);
			blobmsg_add_string(&result, "mac", mac);
		}
		if (ready) {
			blobmsg_add_u64(&result, "upload", c->rate[0]);
			blobmsg_add_u64(&result, "download", c->rate[1]);
		}
		blobmsg_close_table(&result, entry);
	}
	blobmsg_close_table(&result, table);
	void *totals = blobmsg_open_table(&result, "totals");
	struct host *host;
	avl_for_each_element(&hosts, host, avl) {
		if (!host->available && !sampled) continue;
		char mac[18];
		snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
		         host->mac[0], host->mac[1], host->mac[2], host->mac[3], host->mac[4], host->mac[5]);
		blobmsg_add_u64(&result, mac, host->total[0] + host->total[1]);
	}
	blobmsg_close_table(&result, totals);

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
		avl_init(&hosts, mac_cmp, false, NULL);
		started = now_ms();
		load_state();
		timer.cb = sample_timer;
		initialized = true;
		uloop_timeout_set(&timer, 1);
	}
	return ubus_add_object(ctx, &obj);
}
