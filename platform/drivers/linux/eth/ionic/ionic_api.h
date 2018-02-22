#ifndef IONIC_API_H
#define IONIC_API_H

#include <linux/completion.h>
#include <linux/netdevice.h>
#include <linux/types.h>

#include "ionic_if.h"

/** IONIC_API_VERSION - Version number of this interface.
 *
 * Any interface changes to this interface must also change the version.
 */
#define IONIC_API_VERSION "1"

struct lif;

/** get_netdev_ionic_lif - Get the lif if the netdev is ionic.
 * @netdev:		Net device to check.
 * @api_version:	IONIC_API_VERSION.
 *
 * If netdev and other (eg, rdma) drivers are compiled from different sources,
 * they are compatible only if IONIC_API_VERSION is the same in both sources.
 * The compatibility is with exactly the driver module that exports the
 * symbols is_netdev_ionic, and get_netdev_ionic_api.  Only one driver may be
 * loaded exporting those symbols.  Compatible drivers must have the same
 * compile-time value of IONIC_API_VERSION.
 *
 * This is a more strict check than just comparing the driver name in
 * ethtool_api->get_drvinfo().  The name may match, yet drivers may still be
 * incompatible if compiled from different sources.
 *
 * Return: Ionic lif if the netdev is a compatible ionic device.
 */
struct lif *get_netdev_ionic_lif(struct net_device *netdev,
				 const char *api_version);

/** ionic_api_get_intr - Reserve a device iterrupt index.
 * @lif:		Handle to lif.
 * @irq:		OS interrupt number returned.
 *
 * Reserve an interrupt index, and indicate the irq number for that index.
 *
 * Return: interrupt index or negative error status.
 */
int ionic_api_get_intr(struct lif *lif, int *irq);

/** ionic_api_put_intr - Release a device interrupt index.
 * @lif:		Handle to lif.
 * @intr:		Interrupt index.
 *
 * Mark the interrupt index unused, so that it can be reserved again.
 */
void ionic_api_put_intr(struct lif *lif, int intr);

/** ionic_api_get_dbpages - Get doorbell page information for kernel space.
 * @lif:		Handle to lif.
 * @dbpage:		One ioremapped doorbell page for use in kernel space.
 * @phys_dbpage_base:	Phys base address of doorbell pages for the device.
 *
 * The doorbell id and dbpage are special.  Most doorbell ids and pages are for
 * use by user space.  The id and page returned here refer to the one reserved
 * for use in kernel space.
 *
 * Return: kernel doorbell id or negative error status.
 */
int ionic_api_get_dbpages(struct lif *lif,
			  u64 __iomem **dbpage,
			  phys_addr_t *phys_dbpage_base);

/** ionic_api_get_dbid - Reserve a doorbell id.
 * @lif:		Handle to lif.
 *
 * Reserve an doorbell id.  This corresponds with exactly one doorbell page at
 * an offset from the doorbell page base address, that can be mapped into a
 * user space process.
 *
 * Return: doorbell id or negative error status.
 */
int ionic_api_get_dbid(struct lif *lif);

/** ionic_api_put_dbid - Release a doorbell id.
 * @lif:		Handle to lif.
 * @dbid:		Doorbell id.
 *
 * Mark the doorbell id unused, so that it can be reserved again.
 */
void ionic_api_put_dbid(struct lif *lif, int dbid);

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
	struct completion	work;
	struct admin_cmd	cmd;
	struct admin_comp	comp;

#ifndef ADMINQ
	void			*side_data;
	size_t			side_data_len;
#endif
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
int ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);

#endif /* IONIC_API_H */
