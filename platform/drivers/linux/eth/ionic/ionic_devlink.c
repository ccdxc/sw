// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/module.h>
#include <linux/netdevice.h>

#include "ionic.h"
#include "ionic_api.h"
#include "ionic_bus.h"
#include "ionic_lif.h"
#include "ionic_devlink.h"

#ifdef IONIC_DEVLINK
static int ionic_dl_info_get(struct devlink *dl, struct devlink_info_req *req,
			     struct netlink_ext_ack *extack)
{
	struct ionic *ionic = devlink_priv(dl);
	struct ionic_dev *idev = &ionic->idev;
	char buf[16];
	u32 val;

	devlink_info_driver_name_put(req, DRV_NAME);

	devlink_info_version_running_put(req, "fw_version",
					 idev->dev_info.fw_version);

	val = ioread32(&idev->dev_info_regs->fw_heartbeat);
	snprintf(buf, sizeof(buf), "0x%x", val);
	devlink_info_version_running_put(req, "fw_heartbeat", buf);

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

struct ionic *ionic_devlink_alloc(struct device *dev)
{
	struct devlink *dl;
	struct ionic *ionic;

	dl = devlink_alloc(&ionic_dl_ops, sizeof(struct ionic));
	if (!dl) {
		dev_warn(dev, "devlink_alloc failed");
		return NULL;
	}

	ionic = devlink_priv(dl);
	ionic->dl = dl;

	return ionic;
}

void ionic_devlink_free(struct ionic *ionic)
{
	devlink_free(ionic->dl);
}

int ionic_devlink_register(struct ionic *ionic)
{
	int err;

	err = devlink_register(ionic->dl, ionic->dev);
	if (err)
		dev_warn(ionic->dev, "devlink_register failed: %d\n", err);

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(5,2,0) )
	devlink_port_attrs_set(&ionic->dl_port, DEVLINK_PORT_FLAVOUR_PHYSICAL,
			       0, false, 0);
#else
	devlink_port_attrs_set(&ionic->dl_port, DEVLINK_PORT_FLAVOUR_PHYSICAL,
			       0, false, 0, NULL, 0);
#endif
	err = devlink_port_register(ionic->dl, &ionic->dl_port, 0);
	if (err)
		dev_err(ionic->dev, "devlink_port_register failed: %d\n", err);
	else
		devlink_port_type_eth_set(&ionic->dl_port,
					  ionic->master_lif->netdev);

	return err;
}

void ionic_devlink_unregister(struct ionic *ionic)
{
	if (!ionic || !ionic->dl)
		return;

	devlink_port_unregister(&ionic->dl_port);
	devlink_unregister(ionic->dl);
}
#endif /* IONIC_DEVLINK */
