// SPDX-License-Identifier: GPL-2.0
/*
 * xHCI host controller toolkit driver
 *
 * Copyright (C) 2021  MediaTek Inc.
 *
 *  Author: Zhanyong Wang <zhanyong.wang@mediatek.com>
 *          Shaocheng.Wang <shaocheng.wang@mediatek.com>
 *          Chunfeng.Yun <chunfeng.yun@mediatek.com>
 */

#include <linux/platform_device.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/kobject.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <dt-bindings/phy/phy.h>
#include <linux/mutex.h>
#include "xhci-mtk.h"
#include "xhci-mtk-test.h"
#include "xhci-mtk-unusual.h"

static int t_test_j(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_k(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_se0(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_packet(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_suspend(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_resume(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_test_get_device_descriptor(struct xhci_hcd_mtk *mtk,
				int argc, char **argv);
static int t_test_enumerate_bus(struct xhci_hcd_mtk *mtk,
				int argc, char **argv);
static int t_debug_port(struct xhci_hcd_mtk *mtk, int argc, char **argv);
static int t_power_u1u2(struct xhci_hcd_mtk *mtk, int argc, char **argv);

#define PORT_PLS_VALUE(p) ((p >> 5) & 0xf)
/* ip_xhci_cap register */
#define CAP_U3_PORT_NUM(p)	((p) & 0xff)
#define CAP_U2_PORT_NUM(p)	(((p) >> 8) & 0xff)

#define MAX_NAME_SIZE 32
#define MAX_ARG_SIZE 4

struct class_info {
        int class;
        char *class_name;
};

static const struct class_info clas_info[] = {
        /* max. 5 chars. per name string */
        {USB_CLASS_PER_INTERFACE,       ">ifc"},
        {USB_CLASS_AUDIO,               "audio"},
        {USB_CLASS_COMM,                "comm."},
        {USB_CLASS_HID,                 "HID"},
        {USB_CLASS_PHYSICAL,            "PID"},
        {USB_CLASS_STILL_IMAGE,         "still"},
        {USB_CLASS_PRINTER,             "print"},
        {USB_CLASS_MASS_STORAGE,        "stor."},
        {USB_CLASS_HUB,                 "hub"},
        {USB_CLASS_CDC_DATA,            "data"},
        {USB_CLASS_CSCID,               "scard"},
        {USB_CLASS_CONTENT_SEC,         "c-sec"},
        {USB_CLASS_VIDEO,               "video"},
        {USB_CLASS_WIRELESS_CONTROLLER, "wlcon"},
        {USB_CLASS_MISC,                "misc"},
        {USB_CLASS_APP_SPEC,            "app."},
        {USB_CLASS_VENDOR_SPEC,         "vend."},
        {-1,                            "unk."}         /* leave as last */
};

struct hqa_test_cmd {
	char name[MAX_NAME_SIZE];
	int (*cb_func)(struct xhci_hcd_mtk *mtk, int argc, char **argv);
	char *description;
};

struct hqa_test_cmd xhci_mtk_hqa_cmds[] = {
	{"test.j", &t_test_j, "Test_J"},
	{"test.k", &t_test_k, "Test_K"},
	{"test.se0", &t_test_se0, "Test_SE0_NAK"},
	{"test.packet", &t_test_packet, "Test_PACKET"},
	{"test.suspend", &t_test_suspend, "Port Suspend"},
	{"test.resume", &t_test_resume, "Port Resume"},
	{"test.enumbus", &t_test_enumerate_bus, "Enumerate Bus"},
	{"test.getdesc", &t_test_get_device_descriptor,
				"Get Device Descriptor"},
	{"test.debug", &t_debug_port, "debug Port infor"},
	{"pm.u1u2", &t_power_u1u2, "Port U1,U2"},
};

static const char *class_decode(const int class)
{
        int i;

        for (i = 0; clas_info[i].class != -1; i++)
                if (clas_info[i].class == class)
                        break;
        return clas_info[i].class_name;
}

/*
 * These bits are Read Only (RO) and should be saved and written to the
 * registers: 0, 3, 10:13, 30
 * connect status, over-current status, port speed, and device removable.
 * connect status and port speed are also sticky - meaning they're in
 * the AUX well and they aren't changed by a hot, warm, or cold reset.
 */
#define	XHCI_PORT_RO	((1<<0) | (1<<3) | (0xf<<10) | (1<<30))
/*
 * These bits are RW; writing a 0 clears the bit, writing a 1 sets the bit:
 * bits 5:8, 9, 14:15, 25:27
 * link state, port power, port indicator state, "wake on" enable state
 */
#define XHCI_PORT_RWS	((0xf<<5) | (1<<9) | (0x3<<14) | (0x7<<25))
/*
 * These bits are RW; writing a 1 sets the bit, writing a 0 has no effect:
 * bit 4 (port reset)
 */
#define	XHCI_PORT_RW1S	((1<<4))
/*
 * These bits are RW; writing a 1 clears the bit, writing a 0 has no effect:
 * bits 1, 17, 18, 19, 20, 21, 22, 23
 * port enable/disable, and
 * change bits: connect, PED, warm port reset changed (reserved zero for USB 2.0 ports),
 * over-current, reset, link state, and L1 change
 */
#define XHCI_PORT_RW1CS	((1<<1) | (0x7f<<17))
/*
 * Bit 16 is RW, and writing a '1' to it causes the link state control to be
 * latched in
 */
#define	XHCI_PORT_RW	((1<<16))
/*
 * These bits are Reserved Zero (RsvdZ) and zero should be written to them:
 * bits 2, 24, 28:31
 */
#define	XHCI_PORT_RZ	((1<<2) | (1<<24) | (0xf<<28))

/*
 * Given a port state, this function returns a value that would result in the
 * port being in the same state, if the value was written to the port status
 * control register.
 * Save Read Only (RO) bits and save read/write bits where
 * writing a 0 clears the bit and writing a 1 sets the bit (RWS).
 * For all other types (RW1S, RW1CS, RW, and RZ), writing a '0' has no effect.
 */
static u32 xhci_gki_port_state_to_neutral(u32 state)
{
	/* Save read-only status and port state */
	return (state & XHCI_PORT_RO) | (state & XHCI_PORT_RWS);
}

/*
 * xhci_handshake - spin reading hc until handshake completes or fails
 * @ptr: address of hc register to be read
 * @mask: bits to look at in result of read
 * @done: value of those bits when handshake succeeds
 * @usec: timeout in microseconds
 *
 * Returns negative errno, or zero on success
 *
 * Success happens when the "mask" bits have the specified value (hardware
 * handshake done).  There are two failure modes:  "usec" have passed (major
 * hardware flakeout), or the register reads as all-ones (hardware removed).
 */
static int xhci_gki_handshake(void __iomem *ptr, u32 mask, u32 done, u64 timeout_us)
{
	u32	result;
	int	ret;

	ret = readl_poll_timeout_atomic(ptr, result,
					(result & mask) == done ||
					result == U32_MAX,
					1, timeout_us);
	if (result == U32_MAX)		/* card removed */
		return -ENODEV;

	return ret;
}


/*
 * Disable interrupts and begin the xHCI halting process.
 */
static void xhci_gki_quiesce(struct xhci_hcd *xhci)
{
	u32 halted;
	u32 cmd;
	u32 mask;

	mask = ~(XHCI_IRQS);
	halted = readl(&xhci->op_regs->status) & STS_HALT;
	if (!halted)
		mask &= ~CMD_RUN;

	cmd = readl(&xhci->op_regs->command);
	cmd &= mask;
	writel(cmd, &xhci->op_regs->command);
}

/*
 * Force HC into halt state.
 *
 * Disable any IRQs and clear the run/stop bit.
 * HC will complete any current and actively pipelined transactions, and
 * should halt within 16 ms of the run/stop bit being cleared.
 * Read HC Halted bit in the status register to see when the HC is finished.
 */
static int xhci_gki_halt(struct xhci_hcd *xhci)
{
	int ret;
	xhci_gki_quiesce(xhci);

	ret = xhci_gki_handshake(&xhci->op_regs->status,
			STS_HALT, STS_HALT, XHCI_MAX_HALT_USEC);
	if (ret) {
		xhci_warn(xhci, "Host halt failed, %d\n", ret);
		return ret;
	}
	xhci->xhc_state |= XHCI_STATE_HALTED;
	xhci->cmd_ring_state = CMD_RING_STATE_STOPPED;
	return ret;
}

/*
 * usb_get_device_descriptor - (re)reads the device descriptor (usbcore)
 * @dev: the device whose device descriptor is being updated
 * @size: how much of the descriptor to read
 *
 * Context: task context, might sleep.
 *
 * Updates the copy of the device descriptor stored in the device structure,
 * which dedicates space for this purpose.
 *
 * Not exported, only for use by the core.  If drivers really want to read
 * the device descriptor directly, they can call usb_get_descriptor() with
 * type = USB_DT_DEVICE and index = 0.
 *
 * This call is synchronous, and may not be used in an interrupt context.
 *
 * Return: The number of bytes received on success, or else the status code
 * returned by the underlying usb_control_msg() call.
 */
static int usb_get_device_descriptor(struct usb_device *dev, unsigned int size)
{
	struct usb_device_descriptor *desc;
	int ret;

	if (size > sizeof(*desc))
		return -EINVAL;
	desc = kmalloc(sizeof(*desc), GFP_NOIO);
	if (!desc)
		return -ENOMEM;

	ret = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, size);
	if (ret >= 0)
		memcpy(&dev->descriptor, desc, size);
	kfree(desc);
	return ret;
}

int call_hqa_func(struct xhci_hcd_mtk *mtk, char *buf)
{
	struct hqa_test_cmd *hqa;
	struct usb_hcd *hcd = mtk->hcd;
	char *argv[MAX_ARG_SIZE];
	int argc;
	int i;

	argc = 0;
	do {
		argv[argc] = strsep(&buf, " ");
		xhci_err(hcd_to_xhci(hcd), "[%d] %s\r\n", argc, argv[argc]);
		argc++;
	} while (buf);

	for (i = 0; i < ARRAY_SIZE(xhci_mtk_hqa_cmds); i++) {
		hqa = &xhci_mtk_hqa_cmds[i];
		if ((!strcmp(hqa->name, argv[0])) && (hqa->cb_func != NULL))
			return hqa->cb_func(mtk, argc, argv);
	}

	return -1;
}

static int test_mode_enter(struct xhci_hcd_mtk *mtk,
				u32 port_id, u32 test_value)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 temp;

	if (mtk->test_mode == 0) {
		/* set the Run/Stop in USBCMD to 0 */
		addr = &xhci->op_regs->command;
		temp = readl(addr);
		temp &= ~CMD_RUN;
		writel(temp, addr);

		/*  wait for HCHalted */
		xhci_gki_halt(xhci);
	}

	addr = &xhci->op_regs->port_power_base +
			NUM_PORT_REGS * ((port_id - 1) & 0xff);
	temp = readl(addr);
	temp &= ~(0xf << 28);
	temp |= (test_value << 28);
	writel(temp, addr);
	mtk->test_mode = 1;

	return 0;
}

static int test_mode_exit(struct xhci_hcd_mtk *mtk)
{
	if (mtk->test_mode == 1)
		mtk->test_mode = 0;

	return 0;
}

static int t_test_j(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 1;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	force_preemphasis_disabled(mtk);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

static int t_test_k(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	force_preemphasis_disabled(mtk);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

static int t_test_se0(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 3;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%ld\n", __func__, port_id);
	force_preemphasis_disabled(mtk);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

static int t_test_packet(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	long port_id;
	u32 test_value;

	port_id = 2;
	test_value = 4;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%ld\n", __func__, port_id);
	test_mode_enter(mtk, port_id, test_value);

	return 0;
}

/* only for u3 ports, valid values are 1, 2, ...*/
static int t_power_u1u2(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 temp;
	int port_id;
	int retval = 0;
	int u_num = 1;
	int u1_val = 1;
	int u2_val = 0;

	port_id = 1; /* first u3port by default */

	if (argc > 1 && kstrtoint(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	if (argc > 2 && kstrtoint(argv[2], 10, &u_num))
		xhci_err(xhci, "mu3h %s get u_num failed\n", __func__);

	if (argc > 3 && kstrtoint(argv[3], 10, &u1_val))
		xhci_err(xhci, "mu3h %s get u1_val failed\n", __func__);

	if (argc > 4 && kstrtoint(argv[4], 10, &u2_val))
		xhci_err(xhci, "mu3h %s get u2_val failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d, u_num%d, u1_val%d, u2_val%d\n",
		__func__, (int)port_id, u_num, u1_val, u2_val);

	if (mtk->test_mode == 1) {
		xhci_err(xhci, "please suspend port first\n");
		return -1;
	}

	xhci_err(xhci, "%s: stop port polling\n", __func__);
	clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);
	del_timer_sync(&hcd->rh_timer);
	clear_bit(HCD_FLAG_POLL_RH, &xhci->shared_hcd->flags);
	del_timer_sync(&xhci->shared_hcd->rh_timer);
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
	clear_bit(HCD_FLAG_HW_ACCESSIBLE, &xhci->shared_hcd->flags);

	addr = &xhci->op_regs->port_power_base +
			NUM_PORT_REGS * ((port_id - 1) & 0xff);

	temp = readl(addr);
	if (u_num == 1) {
		temp &= ~PORT_U1_TIMEOUT_MASK;
		temp |= PORT_U1_TIMEOUT(u1_val);
	} else if (u_num == 2) {
		temp &= ~PORT_U2_TIMEOUT_MASK;
		temp |= PORT_U2_TIMEOUT(u2_val);
	} else if (u_num == 3) {
		temp &= ~(PORT_U1_TIMEOUT_MASK | PORT_U2_TIMEOUT_MASK);
		temp |= PORT_U1_TIMEOUT(u1_val) | PORT_U2_TIMEOUT(u2_val);
	}

	writel(temp, addr);

	return retval;
}

static void show_string(struct usb_device *udev, char *id, char *string)
{
	if (!string)
		return;
	dev_info(&udev->dev, "%s: %s\n", id, string);
}

static void announce_device(struct usb_device *udev)
{
	u16 bcdDevice = le16_to_cpu(udev->descriptor.bcdDevice);

	dev_info(&udev->dev,
		"New USB device found, idVendor=%04x, idProduct=%04x, bcdDevice=%2x.%02x\n",
		le16_to_cpu(udev->descriptor.idVendor),
		le16_to_cpu(udev->descriptor.idProduct),
		bcdDevice >> 8, bcdDevice & 0xff);
	dev_info(&udev->dev,
		"New USB device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
		udev->descriptor.iManufacturer,
		udev->descriptor.iProduct,
		udev->descriptor.iSerialNumber);
	show_string(udev, "Product", udev->product);
	show_string(udev, "Manufacturer", udev->manufacturer);
	show_string(udev, "SerialNumber", udev->serial);
}

static int t_debug_port(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *rhdev;
	struct usb_device *udev;
	long port_id;
    const struct usb_device_descriptor *desc;
    u16 bcdUSB;
    u16 bcdDevice;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	if (port_id <= mtk->num_u3_ports) {
		 hcd = xhci->shared_hcd;
	} else if (port_id <= mtk->num_u3_ports + mtk->num_u2_ports) {
		port_id -= mtk->num_u3_ports;
	} else {
		xhci_err(xhci, "mu3h %s get port-id override\n", __func__);
		return -EPERM;
	}

	rhdev = hcd->self.root_hub;
	udev = usb_hub_find_child(rhdev, port_id);
	if (udev == NULL) {
		xhci_err(xhci, "mu3h %s usb_hub_find_child(..., %i) failed\n", __func__, (int)port_id);
		return -EPERM;
	}

	dev_info(&udev->dev, "%s\n", usb_state_string(udev->state));
	if (udev && udev->state == USB_STATE_CONFIGURED) {
		announce_device(udev);
		desc = (const struct usb_device_descriptor *)&udev->descriptor;
                bcdUSB    = le16_to_cpu(desc->bcdUSB);
                bcdDevice = le16_to_cpu(desc->bcdDevice);

                dev_info(&udev->dev, "D:  Ver=%2x.%02x Cls=%02x(%-5s) Sub=%02x Prot=%02x MxPS=%2d #Cfgs=%3d\n",
                        bcdUSB >> 8, bcdUSB & 0xff,
                        desc->bDeviceClass,
                        class_decode(desc->bDeviceClass),
                        desc->bDeviceSubClass,
                        desc->bDeviceProtocol,
                        desc->bMaxPacketSize0,
                        desc->bNumConfigurations);

                dev_info(&udev->dev, "P:  Vendor=%04x ProdID=%04x Rev=%2x.%02x\n",
                        le16_to_cpu(desc->idVendor),
                        le16_to_cpu(desc->idProduct),
                        bcdDevice >> 8, bcdDevice & 0xff);
	}

	return 0;
}

static int t_test_suspend(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *rhdev = NULL;
	struct usb_device *udev = NULL;
	struct xhci_interrupter *ir;
	u32 __iomem *addr;
	u32 temp;
	long port_id;
	int port;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	if (port_id <= mtk->num_u3_ports) {
		hcd  = xhci->shared_hcd;
		port = (int)port_id;
		rhdev = hcd->self.root_hub;
		udev = usb_hub_find_child(rhdev, port);
		if (udev != NULL) {
			clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);
			del_timer_sync(&hcd->rh_timer);
			clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
		}
	} else if (port_id <= mtk->num_u3_ports + mtk->num_u2_ports) {
		port  = (int)(port_id - mtk->num_u3_ports);
		rhdev = hcd->self.root_hub;
		udev  = usb_hub_find_child(rhdev, port);
		if (udev != NULL) {
			clear_bit(HCD_FLAG_POLL_RH, &hcd->flags);
			del_timer_sync(&hcd->rh_timer);
			clear_bit(HCD_FLAG_HW_ACCESSIBLE, &hcd->flags);
		}
	} else {
		xhci_err(xhci, "mu3h %s get port-id override\n", __func__);
		return -EPERM;
	}

	xhci_err(xhci, "%s: stop port polling\n", __func__);
	if (mtk->test_mode == 1)
		test_mode_exit(mtk);

	if (udev != NULL) {
		usb_lock_device(udev);
		ir = xhci->interrupter;
		if (!hcd->msi_enabled) {		
			temp = readl(&ir->ir_set->irq_pending);
			writel(ER_IRQ_DISABLE(temp), &ir->ir_set->irq_pending);
		}

		/* set PLS = 3 */
		addr = &xhci->op_regs->port_status_base +
				NUM_PORT_REGS*((port_id - 1) & 0xff);

		temp = readl(addr);
		temp = xhci_gki_port_state_to_neutral(temp);
		temp = (temp & ~(0xf << 5));
		temp = (temp | (3 << 5) | PORT_LINK_STROBE);
		writel(temp, addr);
		xhci_gki_handshake(addr, (0xf << 5), (3 << 5), 30*1000);

		temp = readl(addr);
		if (PORT_PLS_VALUE(temp) != 3)
			xhci_err(xhci, "port not enter suspend state\n");
		else
			xhci_err(xhci, "port enter suspend state\n");
	}
	return 0;
}

static int t_test_resume(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *rhdev;
	struct usb_device *udev;
	u32 __iomem *addr;
	u32 temp;
	long port_id;
	int port;
	int retval = 0;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	if (port_id <= mtk->num_u3_ports) {
		hcd   = xhci->shared_hcd;
		port  = (int)port_id;
		rhdev = hcd->self.root_hub;
		udev  = usb_hub_find_child(rhdev, port);
	} else if (port_id <= mtk->num_u3_ports + mtk->num_u2_ports) {
		port  = (int)(port_id - mtk->num_u3_ports);
		rhdev = hcd->self.root_hub;
		udev = usb_hub_find_child(rhdev, port);
	} else {
		xhci_err(xhci, "mu3h %s get port-id override\n", __func__);
		return -EPERM;
	}

	if (mtk->test_mode == 1) {
		xhci_err(xhci, "please exit test_mode first\n");
		return -1;
	}

	if (udev != NULL) {
		if (!mutex_is_locked(&udev->dev.mutex)) {
			xhci_err(xhci, "please suspend port first\n");
			return -1;
		}

		addr = &xhci->op_regs->port_status_base +
				NUM_PORT_REGS * ((port_id - 1) & 0xff);

		temp = readl(addr);
		if (PORT_PLS_VALUE(temp) != 3) {
			xhci_err(xhci,
				"port not in suspend state, please suspend port first\n");
			retval = -1;
		} else {
			temp = xhci_gki_port_state_to_neutral(temp);
			temp = (temp & ~(0xf << 5));
			temp = (temp | (15 << 5) | PORT_LINK_STROBE);
			writel(temp, addr);
			mdelay(20);

			temp = readl(addr);
			temp = xhci_gki_port_state_to_neutral(temp);
			temp = (temp & ~(0xf << 5));
			temp = (temp | PORT_LINK_STROBE);
			writel(temp, addr);

			xhci_gki_handshake(addr, (0xf << 5), (0 << 5), 100*1000);
			temp = readl(addr);
			if (PORT_PLS_VALUE(temp) != 0) {
				xhci_err(xhci, "rusume fail,%x\n",
					PORT_PLS_VALUE(temp));
				retval = -1;
			} else {
				xhci_err(xhci, "port resume ok\n");
			}
		}

		usb_unlock_device(udev);
	}

	return retval;
}

static int t_test_enumerate_bus(struct xhci_hcd_mtk *mtk, int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *rhdev;
	struct usb_device *udev;
	long port_id;
	u32 retval;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	if (port_id <= mtk->num_u3_ports) {
		 hcd = xhci->shared_hcd;
	} else if (port_id <= mtk->num_u3_ports + mtk->num_u2_ports) {
		port_id -= mtk->num_u3_ports;
	} else {
		xhci_err(xhci, "mu3h %s get port-id override\n", __func__);
		return -EPERM;
	}

	if (mtk->test_mode == 1) {
		test_mode_exit(mtk);
		return 0;
	}

	rhdev = hcd->self.root_hub;
	udev = usb_hub_find_child(rhdev, port_id);
	if (udev != NULL) {
		retval = usb_reset_device(udev);
		if (retval) {
			xhci_err(xhci, "ERROR: enumerate bus fail!\n");
			return -1;
		}
	} else {
		xhci_err(xhci, "ERROR: Device does not exist!\n");
		return -1;
	}

	return 0;
}
static int t_test_get_device_descriptor(struct xhci_hcd_mtk *mtk,
				int argc, char **argv)
{
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	struct usb_device *rhdev;
	struct usb_device *udev;
	long port_id;
	u32 retval = 0;

	port_id = 2;

	if (argc > 1 && kstrtol(argv[1], 10, &port_id))
		xhci_err(xhci, "mu3h %s get port-id failed\n", __func__);

	xhci_err(xhci, "mu3h %s test port%d\n", __func__, (int)port_id);
	if (port_id <= mtk->num_u3_ports) {
		 hcd = xhci->shared_hcd;
	} else if (port_id <= mtk->num_u3_ports + mtk->num_u2_ports) {
		port_id -= mtk->num_u3_ports;
	} else {
		xhci_err(xhci, "mu3h %s get port-id override\n", __func__);
		return -EPERM;
	}

	if (mtk->test_mode == 1) {
		test_mode_exit(mtk);
		msleep(2000);
	}

	rhdev = hcd->self.root_hub;
	udev = usb_hub_find_child(rhdev, port_id);
	if (udev != NULL) {
		retval = usb_get_device_descriptor(udev, USB_DT_DEVICE_SIZE);
		if (retval != sizeof(udev->descriptor)) {
			xhci_err(xhci, "ERROR: get device descriptor fail!\n");
			return -1;
		}
	} else {
		xhci_err(xhci, "ERROR: Device does not exist!\n");
		return -1;
	}

	return 0;
}

static ssize_t hqa_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 val;
	u32 ports;
	int len = 0;
	struct hqa_test_cmd *hqa;
	int i;
	char   str[XHCI_MSG_MAX];

	len += sprintf(buf+len, "info:\n");
	len += sprintf(buf+len,
			"\techo -n item port-id > hqa\n");
	len += sprintf(buf+len,
			"\tport-id : based on number of usb3-port, e.g.\n");
	len += sprintf(buf+len,
			"\t\txHCI with 1 u3p, 2 u2p: 1st u2p-id is 2(1+1), 2nd is 3\n");
	len += sprintf(buf+len, "items:\n");

	for (i = 0; i < ARRAY_SIZE(xhci_mtk_hqa_cmds); i++) {
		hqa = &xhci_mtk_hqa_cmds[i];
		len += sprintf(buf+len,
				"\t%s: %s\n", hqa->name, hqa->description);
	}

	ports = mtk->num_u3_ports + mtk->num_u2_ports;
	for (i = 1; i <= ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS * ((i - 1) & 0xff);
		val = readl(addr);
		if (i <= mtk->num_u3_ports) {
			len += sprintf(buf + len, "USB30 Port%i: 0x%08X\n", i, val);
			len += sprintf(buf + len, "%s\n", xhci_mtk_decode_portsc(str, val));
		} else {
			len += sprintf(buf + len, "USB20 Port%i: 0x%08X\n", i, val);
			len += sprintf(buf + len, "%s\n", xhci_mtk_decode_portsc(str, val));

			addr = &xhci->op_regs->port_power_base +
				NUM_PORT_REGS * ((i - 1) & 0xff);
			val = readl(addr);
			len += sprintf(buf+len,
				"USB20 Port%i PORTMSC[31,28] 4b'0000: 0x%08X\n",
				i, val);
		}
	}

	return len;
}

static ssize_t hqa_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	int retval;

	retval = call_hqa_func(mtk, (char *)buf);
	if (retval < 0) {
		xhci_err(xhci, "mu3h cli fail\n");
		return -1;
	}

	return count;
}

static DEVICE_ATTR_RW(hqa);

static ssize_t usb3hqa_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	ssize_t cnt = 0;
	u32 __iomem *addr;
	u32 val;
	u32 i;
	int ports;
	char   str[XHCI_MSG_MAX];

	cnt += sprintf(buf + cnt, "usb3hqa usage:\n");
	cnt += sprintf(buf + cnt, "	echo [u3port] >usb3hqa\n");

	ports = mtk->num_u3_ports + mtk->num_u2_ports;
	for (i = 1; i <= ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS * ((i - 1) & 0xff);
		val = readl(addr);
		if (i <= mtk->num_u3_ports) {
			cnt += sprintf(buf + cnt, "USB30 Port%i: 0x%08X\n", i, val);
			cnt += sprintf(buf + cnt, "%s\n", xhci_mtk_decode_portsc(str, val));
		} else {
			cnt += sprintf(buf + cnt, "USB20 Port%i: 0x%08X\n", i, val);
			cnt += sprintf(buf + cnt, "%s\n", xhci_mtk_decode_portsc(str, val));
		}
	}

	if (mtk->hqa_pos) {
		cnt += sprintf(buf + cnt, "%s", mtk->hqa_buf);
		mtk->hqa_pos = 0;
	}

	return cnt;
}

static ssize_t
usb3hqa_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t n)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct xhci_hcd *xhci = hcd_to_xhci(hcd);
	u32 __iomem *addr;
	u32 val;
	int port;
	int words;

	mtk->hqa_pos = 0;
	memset(mtk->hqa_buf, 0, mtk->hqa_size);

	hqa_info(mtk, "usb3hqa: %s\n", buf);

	words = sscanf(buf, "%d", &port);
	if ((words != 1) ||
	    (port < 1 || port > mtk->num_u3_ports)) {
		hqa_info(mtk, "usb3hqa: param number:%i, port:%i (%i) failure\n",
			words, port, mtk->num_u3_ports);
		return -EINVAL;
	}

	addr = &xhci->op_regs->port_status_base +
		NUM_PORT_REGS * ((port - 1) & 0xff);
	val  = readl(addr);
	val &= ~(PORT_PLS_MASK);
	val |= (PORT_LINK_STROBE | XDEV_COMP_MODE);
	writel(val, addr);
	hqa_info(mtk, "usb3hqa: port%i: 0x%08X but 0x%08X\n",
		port, val, readl(addr));

	return n;
}
static DEVICE_ATTR_RW(usb3hqa);

static struct device_attribute *mu3h_hqa_attr_list[] = {
	&dev_attr_hqa,
	&dev_attr_usb3hqa,
#include "unusual-statement.h"
};

int hqa_create_attr(struct device *dev)
{
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);
	struct usb_hcd *hcd = mtk->hcd;
	struct mu3c_ippc_regs __iomem *ippc = mtk->ippc_regs;
	struct platform_device *device = to_platform_device(dev);
	int num = ARRAY_SIZE(mu3h_hqa_attr_list);
	int idx;
	int err = 0;
	u32 value;
	u32 addr = hcd->rsrc_start;
	u32 length;

	if (dev == NULL || mtk == NULL)
		return -EINVAL;

	mtk->hqa_size = HQA_PREFIX_SIZE;
	mtk->hqa_pos  = 0;
	mtk->hqa_buf = kzalloc(mtk->hqa_size, GFP_KERNEL);
	if (!mtk->hqa_buf)
		return -ENOMEM;

	if (!mtk->has_ippc && mtk->ippc_regs == NULL) {
		err = query_reg_addr(device, &addr, &length, "ippc");
		if (err)
			return -EINVAL;

		mtk->ippc_regs = ioremap(addr, length);
	}

	ippc  = mtk->ippc_regs;
	value = readl(&ippc->ip_xhci_cap);
	mtk->num_u3_ports = CAP_U3_PORT_NUM(value);
	mtk->num_u2_ports = CAP_U2_PORT_NUM(value);

	for (idx = 0; idx < num; idx++) {
		err = device_create_file(dev, mu3h_hqa_attr_list[idx]);
		if (err)
			break;
	}
	pm_runtime_forbid(dev);

	return err;
}

void hqa_remove_attr(struct device *dev)
{
	int idx;
	int num = ARRAY_SIZE(mu3h_hqa_attr_list);
	struct xhci_hcd_mtk *mtk = dev_get_drvdata(dev);

	for (idx = 0; idx < num; idx++)
		device_remove_file(dev, mu3h_hqa_attr_list[idx]);

	kfree(mtk->hqa_buf);
	mtk->hqa_size = 0;
	mtk->hqa_pos  = 0;
	if (!mtk->has_ippc && mtk->ippc_regs != NULL) {
		iounmap(mtk->ippc_regs);
		mtk->ippc_regs = NULL;
	}
}
