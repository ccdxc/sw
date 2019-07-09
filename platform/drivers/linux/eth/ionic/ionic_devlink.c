// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/module.h>
#include <linux/netdevice.h>

#include "ionic.h"
#include "ionic_api.h"
#include "ionic_bus.h"
#include "ionic_lif.h"
#include "ionic_devlink.h"


/* We're only using the devlink dev info facility at this point,
 * which didn't show up until v5.1, so let's predicate this code
 * on one of the #defines that came with it.
 *
 * Using this facility also requires a devlink user program from
 * v5.1 or newer.
 *  Example:
 *	$ ./devlink -j -p dev info pci/0000:b6:00.0
 *	{
 *	    "info": {
 *		"pci/0000:b6:00.0": {
 *		    "driver": "ionic",
 *		    "serial_number": "FLM18420073",
 *		    "versions": {
 *			"fixed": {
 *			    "fw_version": "0.11.0-50",
 *			    "fw_status": "0x1",
 *			    "fw_heartbeat": "0x716ce",
 *			    "asic_type": "0x0",
 *			    "asic_rev": "0x0"
 *			}
 *		    }
 *		}
 *	    }
 *	}
 *
 * If/when we add devlink dev param support, we can predicate on
 * DEVLINK_PARAM_GENERIC instead.
 */
#ifdef DEVLINK_INFO_VERSION_GENERIC_BOARD_ID

struct ionic_devlink {
	struct ionic *ionic;
};

static int ionic_dl_info_get(struct devlink *dl, struct devlink_info_req *req,
			     struct netlink_ext_ack *extack)
{
	struct ionic *ionic = *(struct ionic **)devlink_priv(dl);
	struct ionic_dev *idev = &ionic->idev;
	char buf[16];
	u32 val;

	devlink_info_driver_name_put(req, DRV_NAME);

	devlink_info_version_fixed_put(req, "fw_version",
					    idev->dev_info.fw_version);

	val = ioread8(&idev->dev_info_regs->fw_status);
	snprintf(buf, sizeof(buf), "0x%x", val);
	devlink_info_version_fixed_put(req, "fw_status", buf);

	val = ioread32(&idev->dev_info_regs->fw_heartbeat);
	snprintf(buf, sizeof(buf), "0x%x", val);
	devlink_info_version_fixed_put(req, "fw_heartbeat", buf);

	snprintf(buf, sizeof(buf), "0x%x", idev->dev_info.asic_type);
	devlink_info_version_fixed_put(req, "asic_type", buf);

	snprintf(buf, sizeof(buf), "0x%x", idev->dev_info.asic_rev);
	devlink_info_version_fixed_put(req, "asic_rev", buf);

	devlink_info_serial_number_put(req, idev->dev_info.serial_num);

	return 0;
}

static const struct devlink_ops ionic_dl_ops = {
	.info_get	= ionic_dl_info_get,
};

int ionic_devlink_register(struct ionic *ionic)
{
	struct devlink *dl;
	struct ionic **ip;
	int err;

	dl = devlink_alloc(&ionic_dl_ops, sizeof(struct ionic *));
	if (!dl) {
		dev_warn(ionic->dev, "devlink_alloc failed");
		return -ENOMEM;
	}

	ip = (struct ionic **)devlink_priv(dl);
	*ip = ionic;
	ionic->dl = dl;

	err = devlink_register(dl, ionic->dev);
	if (err) {
		dev_warn(ionic->dev, "devlink_register failed: %d\n", err);
		goto err_dl_free;
	}

	return 0;

err_dl_unreg:
	devlink_unregister(dl);
err_dl_free:
	ionic->dl = NULL;
	devlink_free(dl);
	return err;
}

void ionic_devlink_unregister(struct ionic *ionic)
{
	if (!ionic->dl)
		return;

	devlink_unregister(ionic->dl);
	devlink_free(ionic->dl);
}
#endif
