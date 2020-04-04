/*
 * Copyright 2017 Pensando Systems, Inc.  All rights reserved.
 */

#ifndef _PCI_IDS_H_
#define _PCI_IDS_H_

#define DRV_NAME			"ionic"
#define DRV_DESCRIPTION		"Pensando Ethernet NIC Driver"
#define DRV_VERSION_STR		"0.14.0-53"
#define DRV_PRODUCT         "Pensando Systems Ethernet"
//#define DRV_VERSION_ID		014053

// TODO: register these with the official include/linux/pci_ids.h
#define PCI_VENDOR_ID_PENSANDO			0x1dd8

#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF	0x1002
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF	0x1003
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT	0x1004

#define IONIC_SUBDEV_ID_NAPLES_25		0x4000
#define IONIC_SUBDEV_ID_NAPLES_100_4	0x4001
#define IONIC_SUBDEV_ID_NAPLES_100_8	0x4002

#endif /* _PCI_IDS_H_ */
