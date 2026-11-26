/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Client discovery helpers and rpcd plugin entry for rpcd-mod-mtk-clients.
 * Derived from rpcd-mod-luci/src/luci.c:
 *   Copyright (C) 2019 Jo-Philipp Wich <jo@mein.io>
 *   Original-Author (traffic/webprobe discovery hooks): coolsnowwolf <coolsnowwolf@gmail.com>
 *   Upstream: https://github.com/coolsnowwolf/luci (openwrt-25.12)
 *   Cherry-picked commits:
 *     8ac42d9a355f89ccd7414e17f65a6bff5ce62d95
 *     725ddc051fe44eac07cf39b70708e24bf1539bcc
 */

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/ether.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <linux/rtnetlink.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <netlink/socket.h>

#include <libubox/avl-cmp.h>
#include <libubox/avl.h>
#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <uci.h>
#include <rpcd/plugin.h>

#include "mtk_clients.h"

struct discover_context {
	struct avl_tree avl;
	int pending;
};

struct host_hint {
	struct avl_node avl;
	struct avl_tree ipaddrs;
	struct avl_tree ip6addrs;
};

#define HOST_HINT_PRIO_IGNORE -1
#define HOST_HINT_PRIO_NL     10

struct host_hint_addr {
	struct avl_node avl;
	int af;
	int prio;
	union {
		struct in_addr in;
		struct in6_addr in6;
	} addr;
};

static bool
ea_empty(struct ether_addr *ea)
{
	struct ether_addr empty = { 0 };

	return !memcmp(ea, &empty, sizeof(empty));
}

static char *
ea2str(struct ether_addr *ea)
{
	static char mac[18];

	if (!ea)
		return NULL;

	snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
	         ea->ether_addr_octet[0], ea->ether_addr_octet[1],
	         ea->ether_addr_octet[2], ea->ether_addr_octet[3],
	         ea->ether_addr_octet[4], ea->ether_addr_octet[5]);

	return mac;
}

static struct ether_addr *
duid2ea(const char *duid)
{
	static struct ether_addr ea;
	const char *p = NULL;
	size_t len;

	if (!duid)
		return NULL;

	for (len = 0; duid[len]; len++)
		if (!isxdigit((unsigned char)duid[len]))
			return NULL;

#define hex(x) \
	(((x) <= '9') ? ((x) - '0') : \
		(((x) <= 'F') ? ((x) - 'A' + 10) : \
			((x) - 'a' + 10)))

	switch (len) {
	case 28:
		if (!strncmp(duid, "00010001", 8))
			p = duid + 16;
		break;
	case 20:
		if (!strncmp(duid, "00030001", 8))
			p = duid + 8;
		break;
	case 12:
		p = duid;
		break;
	}

	if (!p)
		return NULL;

	ea.ether_addr_octet[0] = hex(p[0]) * 16 + hex(p[1]);
	ea.ether_addr_octet[1] = hex(p[2]) * 16 + hex(p[3]);
	ea.ether_addr_octet[2] = hex(p[4]) * 16 + hex(p[5]);
	ea.ether_addr_octet[3] = hex(p[6]) * 16 + hex(p[7]);
	ea.ether_addr_octet[4] = hex(p[8]) * 16 + hex(p[9]);
	ea.ether_addr_octet[5] = hex(p[10]) * 16 + hex(p[11]);

#undef hex
	return &ea;
}

static void strip_colon(char *str)
{
	char *pr = str, *pw = str;

	while (*pr) {
		*pw = *pr++;
		pw += (*pw != ':');
	}
	*pw = '\0';
}

static struct {
	time_t now;
	size_t num, off;
	struct {
		FILE *fh;
		bool odhcpd;
	} *files;
} lease_state = { };

struct lease_entry {
	int af, n_addr;
	char buf[512];
	int32_t expire;
	struct ether_addr mac;
	char *iface;
	char *hostname;
	char *duid;
	char *iaid;
	union {
		struct in_addr in;
		struct in6_addr in6;
	} addr[10];
	uint8_t mask;
};

static bool
add_leasefile(const char *path, bool is_odhcpd)
{
	void *ptr;
	FILE *fh;

	fh = fopen(path, "r");
	if (!fh)
		return false;

	ptr = realloc(lease_state.files, sizeof(*lease_state.files) * (lease_state.num + 1));
	if (!ptr) {
		fclose(fh);
		return false;
	}

	lease_state.files = ptr;
	lease_state.files[lease_state.num].fh = fh;
	lease_state.files[lease_state.num].odhcpd = is_odhcpd;
	lease_state.num++;
	return true;
}

static bool
find_leasefiles(struct uci_context *uci, bool is_odhcpd)
{
	struct uci_ptr ptr = { .package = "dhcp" };
	struct uci_package *pkg = NULL;
	struct uci_section *s;
	struct uci_element *e;
	bool found = false;

	pkg = uci_lookup_package(uci, ptr.package);
	if (!pkg) {
		uci_load(uci, ptr.package, &pkg);
		if (!pkg)
			return false;
	}

	uci_foreach_element(&pkg->sections, e) {
		s = uci_to_section(e);
		if (strcmp(s->type, is_odhcpd ? "odhcpd" : "dnsmasq"))
			continue;

		ptr.flags = 0;
		ptr.section = s->e.name;
		ptr.s = NULL;
		ptr.option = "leasefile";
		ptr.o = NULL;

		if (uci_lookup_ptr(uci, &ptr, NULL, true) || ptr.o == NULL)
			continue;
		if (ptr.o->type != UCI_TYPE_STRING)
			continue;
		if (add_leasefile(ptr.o->v.string, is_odhcpd))
			found = true;
	}

	return found;
}

static void
lease_close(void)
{
	while (lease_state.num > 0)
		fclose(lease_state.files[--lease_state.num].fh);

	free(lease_state.files);
	lease_state.files = NULL;
	lease_state.num = 0;
	lease_state.off = 0;
}

static void
lease_open(void)
{
	struct uci_context *uci;

	lease_close();
	uci = uci_alloc_context();
	if (!uci)
		return;

	lease_state.now = time(NULL);
	if (!find_leasefiles(uci, false))
		add_leasefile("/tmp/dhcp.leases", false);
	if (!find_leasefiles(uci, true))
		add_leasefile("/tmp/odhcpd.leases", true);

	uci_free_context(uci);
}

static struct lease_entry *
lease_next(void)
{
	static struct lease_entry e;
	struct ether_addr *ea;
	char *p;
	long n;

	while (lease_state.off < lease_state.num) {
		memset(&e, 0, sizeof(e));

		while (fgets(e.buf, sizeof(e.buf), lease_state.files[lease_state.off].fh)) {
			ea = NULL;

			if (lease_state.files[lease_state.off].odhcpd) {
				p = strtok(e.buf, " \t\n");
				if (!p || strcmp(p, "#"))
					continue;
				e.iface = strtok(NULL, " \t\n");
				if (!e.iface)
					continue;
				e.duid = strtok(NULL, " \t\n");
				if (!e.duid)
					continue;
				e.iaid = strtok(NULL, " \t\n");
				if (!e.iaid)
					continue;

				if (!strcmp(e.iaid, "ipv4")) {
					e.af = AF_INET;
					e.mask = 32;
					ea = ether_aton(e.duid);
					e.duid = NULL;
					e.iaid = NULL;
				} else {
					e.af = AF_INET6;
					e.mask = 128;
				}

				e.hostname = strtok(NULL, " \t\n");
				if (!e.hostname)
					continue;
				p = strtok(NULL, " \t\n");
				if (!p)
					continue;

				n = strtol(p, NULL, 10);
				if (n > lease_state.now)
					e.expire = n - lease_state.now;
				else if (n >= 0)
					e.expire = 0;
				else
					e.expire = -1;

				strtok(NULL, " \t\n");
				p = strtok(NULL, " \t\n");
				if (!p)
					continue;
				n = atoi(p);
				if (n != 0)
					e.mask = n;

				for (e.n_addr = 0, p = strtok(NULL, "/ \t\n");
				     e.n_addr < ARRAY_SIZE(e.addr) && p != NULL;
				     p = strtok(NULL, "/ \t\n")) {
					if (inet_pton(e.af, p, &e.addr[e.n_addr].in6))
						e.n_addr++;
				}

				if (!ea)
					ea = duid2ea(e.duid);
				if (ea)
					e.mac = *ea;
				if (!strcmp(e.hostname, "-"))
					e.hostname = NULL;
				if (e.duid && !strcmp(e.duid, "-"))
					e.duid = NULL;
			} else {
				p = strtok(e.buf, " \t\n");
				if (!p)
					continue;
				n = strtol(p, NULL, 10);
				if (n > lease_state.now)
					e.expire = n - lease_state.now;
				else if (n > 0)
					e.expire = 0;
				else
					e.expire = -1;

				p = strtok(NULL, " \t\n");
				if (!p)
					continue;
				ea = ether_aton(p);

				p = strtok(NULL, " \t\n");
				if (p && inet_pton(AF_INET6, p, &e.addr[0].in6)) {
					e.af = AF_INET6;
					e.mask = 128;
					e.n_addr = 1;
				} else if (p && inet_pton(AF_INET, p, &e.addr[0].in)) {
					e.af = AF_INET;
					e.mask = 32;
					e.n_addr = 1;
				} else {
					continue;
				}

				if (!ea && e.af != AF_INET6)
					continue;

				e.hostname = strtok(NULL, " \t\n");
				e.duid = strtok(NULL, " \t\n");
				e.iaid = NULL;
				if (!e.hostname || !e.duid)
					continue;
				if (!strcmp(e.hostname, "*"))
					e.hostname = NULL;

				if (e.af == AF_INET && strlen(e.duid) > 15 && !strncmp(e.duid, "ff:", 3)) {
					e.duid[14] = '\0';
					e.iaid = &e.duid[3];
					strip_colon(e.iaid);
					e.duid = &e.duid[15];
					strip_colon(e.duid);
				} else if (e.af == AF_INET && strlen(e.duid) == 20 && !strncmp(e.duid, "01:", 3)) {
					if (!ea)
						ea = ether_aton(&e.duid[3]);
					e.duid = NULL;
				} else if (!strcmp(e.duid, "*")) {
					e.duid = NULL;
				} else {
					strip_colon(e.duid);
				}

				if (!ea && e.duid)
					ea = duid2ea(e.duid);
				if (ea)
					e.mac = *ea;
			}

			return &e;
		}

		lease_state.off++;
	}

	return NULL;
}

static int
host_hint_addr_avl_cmp(const void *k1, const void *k2, void *ptr)
{
	const struct host_hint_addr *a1 = k1;
	const struct host_hint_addr *a2 = k2;

	if (a1->prio != a2->prio &&
	    a1->prio != HOST_HINT_PRIO_IGNORE &&
	    a2->prio != HOST_HINT_PRIO_IGNORE)
		return a1->prio < a2->prio ? 1 : -1;

	if (a1->af != a2->af)
		return a1->af < a2->af ? -1 : 1;

	return memcmp(&a1->addr, &a2->addr, sizeof(a1->addr));
}

static int
nl_cb_done(struct nl_msg *msg, void *arg)
{
	struct discover_context *dctx = arg;
	dctx->pending = 0;
	return NL_STOP;
}

static int
nl_cb_error(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
	struct discover_context *dctx = arg;
	dctx->pending = 0;
	return NL_STOP;
}

static struct host_hint *
discover_get_host_hint(struct discover_context *dctx, struct ether_addr *ea)
{
	struct host_hint *hint;
	char *p, *mac;

	if (!ea)
		return NULL;

	mac = ea2str(ea);
	hint = avl_find_element(&dctx->avl, mac, hint, avl);
	if (!hint) {
		hint = calloc_a(sizeof(*hint), &p, strlen(mac) + 1);
		if (!hint)
			return NULL;
		hint->avl.key = strcpy(p, mac);
		avl_init(&hint->ipaddrs, host_hint_addr_avl_cmp, false, NULL);
		avl_init(&hint->ip6addrs, host_hint_addr_avl_cmp, false, NULL);
		avl_insert(&dctx->avl, &hint->avl);
	}
	return hint;
}

static void
discover_add_host_hint_addr(struct host_hint *hint, int af, int prio, void *addr)
{
	struct host_hint_addr e, *a;
	struct avl_tree *addrs = af == AF_INET ? &hint->ipaddrs : &hint->ip6addrs;

	if (!addr)
		return;

	memset(&e, 0, sizeof(e));
	e.af = af;
	e.prio = HOST_HINT_PRIO_IGNORE;
	if (af == AF_INET)
		memcpy(&e.addr.in, addr, sizeof(e.addr.in));
	else
		memcpy(&e.addr.in6, addr, sizeof(e.addr.in6));

	a = avl_find_element(addrs, &e, a, avl);
	if (a) {
		if (prio <= a->prio)
			return;
		avl_delete(addrs, &a->avl);
		a->prio = prio;
		avl_insert(addrs, &a->avl);
		return;
	}

	a = calloc(1, sizeof(*a));
	if (!a)
		return;

	memcpy(a, &e, sizeof(*a));
	a->prio = prio;
	a->avl.key = a;
	avl_insert(addrs, &a->avl);
}

static int
nl_cb_dump_neigh(struct nl_msg *msg, void *arg)
{
	struct discover_context *dctx = arg;
	struct ether_addr *mac;
	void *dst;
	struct nlmsghdr *hdr = nlmsg_hdr(msg);
	struct ndmsg *nd = NLMSG_DATA(hdr);
	struct nlattr *tb[NDA_MAX + 1];
	struct host_hint *hint;

	dctx->pending = !!(hdr->nlmsg_flags & NLM_F_MULTI);

	if (hdr->nlmsg_type != RTM_NEWNEIGH ||
	    (nd->ndm_family != AF_INET && nd->ndm_family != AF_INET6))
		return NL_SKIP;

	if (!(nd->ndm_state & (0xFF & ~NUD_NOARP)))
		return NL_SKIP;

	nlmsg_parse(hdr, sizeof(*nd), tb, NDA_MAX, NULL);
	mac = tb[NDA_LLADDR] ? RTA_DATA(tb[NDA_LLADDR]) : NULL;
	dst = tb[NDA_DST] ? RTA_DATA(tb[NDA_DST]) : NULL;
	if (!mac || !dst)
		return NL_SKIP;

	hint = discover_get_host_hint(dctx, mac);
	if (!hint)
		return NL_SKIP;

	discover_add_host_hint_addr(hint, nd->ndm_family, HOST_HINT_PRIO_NL, dst);
	return NL_SKIP;
}

static void
discover_host_hints_nl(struct discover_context *dctx)
{
	struct nl_sock *sock = NULL;
	struct nl_msg *msg = NULL;
	struct nl_cb *cb = NULL;
	struct ndmsg ndm = { };

	sock = nl_socket_alloc();
	if (!sock)
		goto out;
	if (nl_connect(sock, NETLINK_ROUTE))
		goto out;
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto out;
	msg = nlmsg_alloc_simple(RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP);
	if (!msg)
		goto out;

	nlmsg_append(msg, &ndm, sizeof(ndm), 0);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, nl_cb_dump_neigh, dctx);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, nl_cb_done, dctx);
	nl_cb_err(cb, NL_CB_CUSTOM, nl_cb_error, dctx);

	dctx->pending = 1;
	nl_send_auto_complete(sock, msg);
	while (dctx->pending)
		nl_recvmsgs(sock, cb);

out:
	if (sock)
		nl_socket_free(sock);
	if (cb)
		nl_cb_put(cb);
	if (msg)
		nlmsg_free(msg);
}

void
mtk_dhcp4_clients(void (*add)(const struct in_addr *, const unsigned char *))
{
	struct lease_entry *lease;

	lease_open();
	while ((lease = lease_next()) != NULL) {
		if (lease->af == AF_INET && lease->expire != 0 && !ea_empty(&lease->mac))
			add(&lease->addr[0].in, lease->mac.ether_addr_octet);
	}
	lease_close();
}

/* Discover accounting identities without DNS lookups or browser requests. */
void
mtk_traffic_discover(void (*add)(int, const void *, const unsigned char *))
{
	struct discover_context dctx = { 0 };
	struct host_hint *hint, *next;
	struct host_hint_addr *addr, *anext;
	struct lease_entry *lease;

	avl_init(&dctx.avl, avl_strcmp, false, NULL);
	discover_host_hints_nl(&dctx);
	avl_for_each_element_safe(&dctx.avl, hint, avl, next) {
		struct ether_addr *mac = ether_aton(hint->avl.key);
		avl_for_each_element_safe(&hint->ipaddrs, addr, avl, anext) {
			if (mac)
				add(AF_INET, &addr->addr.in, mac->ether_addr_octet);
			avl_delete(&hint->ipaddrs, &addr->avl);
			free(addr);
		}
		avl_for_each_element_safe(&hint->ip6addrs, addr, avl, anext) {
			if (mac)
				add(AF_INET6, &addr->addr.in6, mac->ether_addr_octet);
			avl_delete(&hint->ip6addrs, &addr->avl);
			free(addr);
		}
		avl_delete(&dctx.avl, &hint->avl);
		free(hint);
	}

	/* Current leases take precedence over stale neighbour cache entries. */
	lease_open();
	while ((lease = lease_next()) != NULL) {
		if (lease->expire == 0 || ea_empty(&lease->mac))
			continue;
		for (int i = 0; i < lease->n_addr; i++)
			add(lease->af, &lease->addr[i], lease->mac.ether_addr_octet);
	}
	lease_close();
}

static int
mtk_clients_plugin_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	int ret = mtk_traffic_init(ctx);
	if (!ret)
		ret = mtk_webprobe_init(ctx);
	return ret;
}

struct rpc_plugin rpc_plugin = {
	.init = mtk_clients_plugin_init
};
