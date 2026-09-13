/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MTK_CLIENTS_H
#define MTK_CLIENTS_H

#include <netinet/in.h>
#include <libubus.h>

int mtk_traffic_init(struct ubus_context *ctx);
int mtk_webprobe_init(struct ubus_context *ctx);
void mtk_traffic_discover(void (*add)(int, const void *, const unsigned char *));
void mtk_dhcp4_clients(void (*add)(const struct in_addr *, const unsigned char *));

#endif
