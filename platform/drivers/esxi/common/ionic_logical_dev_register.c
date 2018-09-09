/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * ionic_logical_dev_register.c --
 *
 * Implement API to register logical device
 */

#include "ionic_logical_dev_register.h"
#include "ionic_log.h"

static vmk_DeviceOps ionic_device_ops = {
        .removeDevice = ionic_logical_dev_remove,
};


/*
 ******************************************************************************
 *
 * ionic_logical_dev_register --
 *
 *    Register logical devices. Should be called during scan.
 *
 *  Parameters:
 *     driver                  - IN     (Driver creating the device)
 *     parent_dev              - IN     (Parent of device being created)
 *     registering_driver_data - IN     (Opaque data set by registering driver
 *                                       for its private use)
 *     registration_data       - IN     (Opaque data set by registering driver for
 *                                       attaching driver)
 *     uplink_vmk_dev          - OUT    (logical uplink vmk device)
 *  Results:
 *     VMK_OK on success
 *     matching error code on failure
 *
 *  Side-effects:
 *     Logical devices supplied are registered under parent_dev
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_logical_dev_register(vmk_Driver driver,                     // IN
                           vmk_Device parent_dev,                 // IN
                           void *registering_driver_data,         // IN
                           void *registration_data,               // IN
                           vmk_Device *uplink_vmk_dev)            // OUT
{
        VMK_ReturnStatus status;
        vmk_Name bus_name;
        vmk_DeviceID device_id;
        vmk_DeviceProps device_props;

        status = vmk_NameInitialize(&bus_name,
                                    VMK_LOGICAL_BUS_NAME);
        if (status != VMK_OK) {
                ionic_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_BusTypeFind(&bus_name,
                                 &device_id.busType);
        if (status != VMK_OK) {
                ionic_err("vmk_BusTypeFind() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_LogicalCreateBusAddress(driver,
                                             parent_dev,
                                             0,
                                             &device_id.busAddress,
                                             &device_id.busAddressLen);
        if (status != VMK_OK) {
                ionic_err("vmk_LogicalCreateBusAddress() failed, status: %s",
                          vmk_StatusToString(status));
                goto bus_addr_create_err;
        }

        device_id.busIdentifier                = VMK_UPLINK_DEVICE_IDENTIFIER;
        device_id.busIdentifierLen             = sizeof(VMK_UPLINK_DEVICE_IDENTIFIER)-1; 
        device_props.deviceID                  = &device_id;
        device_props.deviceOps                 = &ionic_device_ops;
        device_props.registeringDriver         = driver;
        device_props.registeringDriverData.ptr = registering_driver_data;
        device_props.registrationData.ptr      = registration_data;

        status = vmk_DeviceRegister(&device_props,
                                    parent_dev,
                                    uplink_vmk_dev);
        if ((status != VMK_OK) && (status != VMK_EXISTS)) {
               ionic_err("vmk_DeviceRegister() failed, status: %s",
                         vmk_StatusToString(status));
               goto dev_reg_err;
        }

        ionic_info("Successfully registered logical device: %s, "
                   "logical port: %d", VMK_UPLINK_DEVICE_IDENTIFIER, 0);

        /* Bus id is no longer needed, feel free to free it */
        status = vmk_LogicalFreeBusAddress(driver,
                                           device_id.busAddress);
        if (status != VMK_OK) {
                ionic_err("vmk_LogicalFreeBusAddress() failed, status: %s",
                          vmk_StatusToString(status));
                goto bus_addr_create_err;
        }

        status = vmk_BusTypeRelease(device_id.busType);
        if (status != VMK_OK) {
                ionic_err("vmk_BusTypeRelease() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;

dev_reg_err:
        vmk_LogicalFreeBusAddress(driver,
                                  device_id.busAddress);

bus_addr_create_err:
        vmk_BusTypeRelease(device_id.busType);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_logical_dev_remove --
 *
 *     Driver callback to remove logical uplink device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_logical_dev_remove(vmk_Device logical_device)               // IN
{
        VMK_ReturnStatus status;

        status = vmk_DeviceUnregister(logical_device);

        if (status != VMK_OK) {
                ionic_err("vmk_DeviceUnregister() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}
