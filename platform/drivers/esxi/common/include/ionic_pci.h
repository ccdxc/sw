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
 * ionic_pci.h --
*
 * Definitions all PCI related functions and data structures
 */

#ifndef _IONIC_PCI_H_
#define _IONIC_PCI_H_

#include <vmkapi.h>

#define IONIC_PCI_CMD                     0x4
#define IONIC_PCI_CMD_MASTER              0x4

enum ionic_pci_device_flags {
   PENSANDO_IONIC_ETH_PF,
   PENSANDO_IONIC_ETH_VF,
   PENSANDO_IONIC_ETH_MGMT,
};

struct ionic_pci_device_entry {
   vmk_uint32 device_id;
   enum ionic_pci_device_flags flags;
   char desc[VMK_PCI_DEVICE_NAME_LENGTH];
};

VMK_ReturnStatus
ionic_pci_dev_entry_get(vmk_uint32 device_id,
                        const struct ionic_pci_device_entry *dev_tbl,
                        vmk_uint32 dev_table_len,
                        struct ionic_pci_device_entry *dev_entry);

VMK_ReturnStatus
ionic_pci_bus_master_enable(vmk_ModuleID module_id,
                            vmk_PCIDevice pci_device);

void
ionic_pci_bus_master_disable(vmk_ModuleID module_id,
                             vmk_PCIDevice pci_device);

inline vmk_uint32
ionic_readl_raw(const volatile vmk_VA addr);

inline vmk_uint16
ionic_readw_raw(const volatile vmk_VA addr);

inline vmk_uint8
ionic_readb_raw(const volatile vmk_VA addr);

inline void
ionic_writel_raw(vmk_uint32 value,
                 volatile vmk_VA addr);

inline void
ionic_writeq_raw(vmk_uint64 value,
                 volatile vmk_VA addr);

#endif /* End of _IONIC_PCI_H_ */
