/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_logical_dev_register.h] in the main directory of this source tree.
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
