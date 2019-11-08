/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef IONIC_API_H
#define IONIC_API_H

#include <linux/completion.h>
#include <linux/netdevice.h>
#include <linux/types.h>

#include <ionic_if.h>
#include <ionic_regs.h>

/** IONIC_API_VERSION - Version number of this interface.
 *
 * Any interface changes to this interface must also change the version.
 *
 * If netdev and other (eg, rdma) drivers are compiled from different sources,
 * they are compatible only if IONIC_API_VERSION is statically the same in both
 * sources.  Drivers must have matching values of IONIC_API_VERSION at compile
 * time, to be considered compatible at run time.
 *
 * This is a more strict check than just comparing the driver name in
 * ethtool_api->get_drvinfo().  The name may match, yet drivers may still be
 * incompatible if compiled from different sources, or if some other driver
 * happens to be called "ionic".
 */
#define IONIC_API_VERSION "5"

struct dentry;
struct ionic_lif;

/** struct ionic_devinfo - device information. */
struct ionic_devinfo {
	u8 asic_type;
	u8 asic_rev;
	char fw_version[IONIC_DEVINFO_FWVERS_BUFLEN + 1];
	char serial_num[IONIC_DEVINFO_SERIAL_BUFLEN + 1];
};

/** enum ionic_api_prsn - personalities that can be applied to the lif. */
enum ionic_api_prsn {
	IONIC_PRSN_RDMA,
};

/** get_netdev_ionic_lif - Get the lif if the netdev is ionic.
 * @netdev:		Net device to check.
 * @api_version:	IONIC_API_VERSION.
 * @prsn:		Personality to apply.
 *
 * This will return the opaque struct ionic_lif if and only if the netdev
 * was created by the ionic driver, if the api version matches as described
 * above for IONIC_API_VERSION, and if the personality can be applied to
 * the lif.
 *
 * Return: Ionic lif if the netdev is a compatible ionic device.
 */
struct ionic_lif *get_netdev_ionic_lif(struct net_device *netdev,
				       const char *api_version,
				       enum ionic_api_prsn prsn);

/** ionic_api_request_reset - request reset or disable the device or lif.
 * @lif:		Handle to lif.
 *
 * The reset will be carried out asynchronously.  If it succeeds, then the
 * callback specified in ionic_api_set_private() will be called.
 */
void ionic_api_request_reset(struct ionic_lif *lif);

/** ionic_api_get_private - Get private data associated with the lif.
 * @lif:		Handle to lif.
 * @prsn:		Personality to which the private data applies.
 *
 * Get the private data of some kind.  The private data may be, for example, an
 * instance of an rdma device for this lif.
 *
 * Return: private data or NULL.
 */
void *ionic_api_get_private(struct ionic_lif *lif, enum ionic_api_prsn prsn);

/** ionic_api_get_private - Set private data associated with the lif.
 * @lif:		Handle to lif.
 * @priv:		Private data or NULL.
 * @reset_cb:		Callback if device has been disabled or reset.
 * @prsn:		Personality to which the private data applies.
 *
 * Set the private data of some kind.  The private data may be, for example, an
 * instance of an rdma device for this lif.
 *
 * This will fail if private data is already set for that personality.
 *
 * Return: zero or negative error status.
 */
int ionic_api_set_private(struct ionic_lif *lif, void *priv,
			  void (*reset_cb)(void *priv),
			  enum ionic_api_prsn prsn);

/** ionic_api_get_debugfs - Get the debugfs dir (if any) for the lif.
 * @lif:		Handle to lif.
 *
 * Return: debugfs dir for the lif or NULL
 */
struct dentry *ionic_api_get_debugfs(struct ionic_lif *lif);

/** ionic_api_get_devinfo - Get device information.
 * @lif:		Handle to lif.
 *
 * Return: pointer to device information.
 */
const struct ionic_devinfo *ionic_api_get_devinfo(struct ionic_lif *lif);

/** ionic_api_get_identity - Get result of device identification.
 * @lif:		Handle to lif.
 * @lif_id:		This lif id.
 *
 * The dev member of the union is valid.
 *
 * Return: pointer to result of identification.
 */
const union lif_identity *ionic_api_get_identity(struct ionic_lif *lif,
						 int *lif_id);

/** ionic_api_get_intr - Reserve a device iterrupt index.
 * @lif:		Handle to lif.
 * @irq:		OS interrupt number returned.
 *
 * Reserve an interrupt index, and indicate the irq number for that index.
 *
 * Return: interrupt index or negative error status.
 */
int ionic_api_get_intr(struct ionic_lif *lif, int *irq);

/** ionic_api_put_intr - Release a device interrupt index.
 * @lif:		Handle to lif.
 * @intr:		Interrupt index.
 *
 * Mark the interrupt index unused, so that it can be reserved again.
 */
void ionic_api_put_intr(struct ionic_lif *lif, int intr);

/** ionic_api_get_cmb - Reserve cmb pages.
 * @lif:		Handle to lif.
 * @pgid:		First page index.
 * @pgaddr:		First page bus addr (contiguous).
 * @order:		Log base two number of pages (PAGE_SIZE).
 *
 * Reserve cmb pages.
 *
 * Return: zero or negative error status.
 */
int ionic_api_get_cmb(struct ionic_lif *lif, u32 *pgid, phys_addr_t *pgaddr,
		      int order);

/** ionic_api_put_cmb - Release cmb pages.
 * @lif:		Handle to lif.
 * @pgid:		First page index.
 * @order:		Log base two number of pages (PAGE_SIZE).
 *
 * Release cmb pages.
 */
void ionic_api_put_cmb(struct ionic_lif *lif, u32 pgid, int order);

/** ionic_api_kernel_dbpage - Get mapped dorbell page for use in kernel space.
 * @lif:		Handle to lif.
 * @intr_ctrl:		Interrupt control registers.
 * @dbid:		Doorbell id for use in kernel space.
 * @dbpage:		One ioremapped doorbell page for use in kernel space.
 *
 * This also provides mapped interrupt control registers.
 *
 * The id and page returned here refer to the doorbell page reserved for use in
 * kernel space for this lif.  For user space, use ionic_api_get_dbid to
 * allocate a doorbell id for exclusive use by a process.
 */
void ionic_api_kernel_dbpage(struct ionic_lif *lif,
			     struct ionic_intr __iomem **intr_ctrl,
			     u32 *dbid, u64 __iomem **dbpage);

/** ionic_api_get_dbid - Reserve a doorbell id.
 * @lif:		Handle to lif.
 * @dbid:		Doorbell id.
 * @addr:		Phys address of doorbell page.
 *
 * Reserve a doorbell id.  This corresponds with exactly one doorbell page at
 * an offset from the doorbell page base address, that can be mapped into a
 * user space process.
 *
 * Return: zero on success or negative error status.
 */
int ionic_api_get_dbid(struct ionic_lif *lif, u32 *dbid, phys_addr_t *addr);

/** ionic_api_put_dbid - Release a doorbell id.
 * @lif:		Handle to lif.
 * @dbid:		Doorbell id.
 *
 * Mark the doorbell id unused, so that it can be reserved again.
 */
void ionic_api_put_dbid(struct ionic_lif *lif, int dbid);

/** ionic_admin_ctx - Admin command context.
 * @work:		Work completion wait queue element.
 * @cmd:		Admin command (64B) to be copied to the queue.
 * @comp:		Admin completion (16B) copied from the queue.
 *
 * @side_data:		Additional data to be copied to the doorbell page,
 *			  if the command is issued as a dev cmd.
 * @side_data_len:	Length of additional data to be copied.
 *
 * TODO:
 * The side_data and side_data_len are temporary and will be removed.  For now,
 * they are used when admin commands referring to side-band data are posted as
 * dev commands instead.  Only single-indirect side-band data is supported.
 * Only 2K of data is supported, because first half of page is for registers.
 */
struct ionic_admin_ctx {
	struct completion work;
	union adminq_cmd cmd;
	union adminq_comp comp;
};

/** ionic_api_adminq_post - Post an admin command.
 * @lif:		Handle to lif.
 * @cmd_ctx:		Api admin command context.
 *
 * Post the command to an admin queue in the ethernet driver.  If this command
 * succeeds, then the command has been posted, but that does not indicate a
 * completion.  If this command returns success, then the completion callback
 * will eventually be called.
 *
 * Return: zero or negative error status.
 */
int ionic_api_adminq_post(struct ionic_lif *lif, struct ionic_admin_ctx *ctx);

#endif /* IONIC_API_H */
