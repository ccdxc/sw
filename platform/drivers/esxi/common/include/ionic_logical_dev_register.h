/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_logical_dev_register.h --
 *
 * Defination of API to register logical device
 */

#ifndef _IONIC_LOGICAL_DEV_REGISTER_H_
#define _IONIC_LOGICAL_DEV_REGISTER_H_

#include <vmkapi.h>

VMK_ReturnStatus
ionic_logical_dev_register(vmk_Driver driver,
                           vmk_Device parent_dev,
                           void *registering_driver_data,
                           void *registration_data,
                           vmk_Device *uplink_vmk_dev);

VMK_ReturnStatus
ionic_logical_dev_remove(vmk_Device logical_device);

#endif /* End of _IONIC_LOGICAL_DEV_REGISTER_H_ */
