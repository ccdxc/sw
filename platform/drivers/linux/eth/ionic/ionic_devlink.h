/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_DEVLINK_H_
#define _IONIC_DEVLINK_H_

#include <net/devlink.h>

#ifdef DEVLINK_INFO_VERSION_GENERIC_BOARD_ID
int ionic_devlink_register(struct ionic *ionic);
void ionic_devlink_unregister(struct ionic *ionic);
#else
#define ionic_devlink_register(x)    0
#define ionic_devlink_unregister(x)
#endif

#endif /* _IONIC_DEVLINK_H_ */
