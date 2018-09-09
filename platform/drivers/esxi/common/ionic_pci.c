/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * i under the terms of the GNU General Public License as published by
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
 * ionic_pci.c --
 *
 * Implement all PCI related functions
 */

#include "ionic_pci.h"
#include "ionic_log.h"

/*
 ******************************************************************************
 *
 * ionic_pci_dev_entry_get --
 *
 *    Get device entry in device table
 *
 *  Parameters:
 *     device_id      - IN  (device ID that we are looing for)
 *     dev_table      - IN  (device table to go through)
 *     dev_tbl_len    - IN  (number of elements within the device table)
 *     dev_entry      - OUT (dev_entry matches to this given device ID)
 *
 *  Results:
 *     Return VMK_OK if device ID is found, dev_entry contains valid info.
 *     Return VMK_NOT_FOUND if device ID is not found
 *
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */


VMK_ReturnStatus
ionic_pci_dev_entry_get(vmk_uint32 device_id,                          // IN
                        const struct ionic_pci_device_entry *dev_tbl,  // IN
                        vmk_uint32 dev_tbl_len,                        // IN
                        struct ionic_pci_device_entry *dev_entry)      // OUT
{
        VMK_ReturnStatus status = VMK_NOT_FOUND;
        vmk_uint32 i;

        VMK_ASSERT(dev_tbl && dev_tbl_len && dev_entry);

        for (i = 0; i < dev_tbl_len; i++) {
                if (dev_tbl[i].device_id == device_id) {
                        vmk_Memcpy(dev_entry,
                                   &dev_tbl[i],
                                   sizeof(struct ionic_pci_device_entry));
                        status = VMK_OK;
                        break;
                }
        }

        return status;
}

/*
 ******************************************************************************
 *
 * ionic_pci_bus_master_enable --
 *
 *    Enable bus mastering for PCI device
 *
 *  Parameters:
 *     module_id  - IN (module ID)
 *     pci_device - IN (PCI device handle)
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
ionic_pci_bus_master_enable(vmk_ModuleID module_id,               // IN
                            vmk_PCIDevice pci_device)             // IN
{
        VMK_ReturnStatus status;
        vmk_uint32 cmd;

        status = vmk_PCIReadConfig(module_id,
                                   pci_device,
                                   VMK_PCI_CONFIG_ACCESS_16,
                                   IONIC_PCI_CMD,
                                   &cmd);

        if (status != VMK_OK) {
                ionic_err("vmk_PCIReadConfig() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        /*
         * Check if already bus master
         */

        if (cmd & IONIC_PCI_CMD_MASTER) {
                return status;
        }

        cmd |= IONIC_PCI_CMD_MASTER;

        status = vmk_PCIWriteConfig(module_id,
                                    pci_device,
                                    VMK_PCI_CONFIG_ACCESS_16,
                                    IONIC_PCI_CMD,
                                    cmd);

        if (status != VMK_OK) {
                ionic_err("vmk_PCIWriteConfig() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_pci_bus_master_disable --
 *
 *    Disable bus mastering for PCI device
 *
 *  Parameters:
 *     module_id  - IN (module ID)
 *     pci_device - IN (PCI device handle)
 *
 *  Results:
 *     PCI device bus mastering is turned off
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_pci_bus_master_disable(vmk_ModuleID module_id,              // IN
                             vmk_PCIDevice pci_device)            // IN
{
        VMK_ReturnStatus status;
        vmk_uint32 cmd;

        status = vmk_PCIReadConfig(module_id,
                                   pci_device,
                                   VMK_PCI_CONFIG_ACCESS_16,
                                   IONIC_PCI_CMD,
                                   &cmd);

        if (status != VMK_OK) {
                ionic_err("vmk_PCIReadConfig() failed, status:  %s",
                          vmk_StatusToString(status));
                return;
        }

        /*
         * Check if has already been disabled bus master
         */

        if (!(cmd & IONIC_PCI_CMD_MASTER)) {
                return;
        }

        cmd &= ~IONIC_PCI_CMD_MASTER;

        status = vmk_PCIWriteConfig(module_id,
                                    pci_device,
                                    VMK_PCI_CONFIG_ACCESS_16,
                                    IONIC_PCI_CMD,
                                    cmd);

        if (status != VMK_OK) {
                ionic_err("vmk_PCIWriteConfig() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return;
}


/*
 ******************************************************************************
 *
 * ionic_readl_raw --
 *
 *    Read unsigned 32 bit integer in native endianness - no byteswapping.
 *    Implement semantics of __raw_readl() Linux function.
 *    Access memory in native endianness.
 *
 *  Parameters:
 *     addr   - IN (virtual address)
 *
 *  Results:
 *     Return unsigned 32-bit integer read in native endian
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_uint32
ionic_readl_raw(const volatile vmk_VA addr)
{
        return *(const volatile vmk_uint32 *)addr;
}


/*
 ******************************************************************************
 *
 * ionic_readw_raw --
 *
 *    Read unsigned 16 bit integer in native endianness - no byteswapping.
 *    Implement semantics of __raw_readw() Linux function.
 *    Access memory in native endianness.
 *
 *  Parameters:
 *     addr   - IN (virtual address)
 *
 *  Results:
 *     Return unsigned 16-bit integer read in native endian
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_uint16
ionic_readw_raw(const volatile vmk_VA addr)
{
        return *(const volatile vmk_uint16 *)addr;
}


/*
 ******************************************************************************
 *
 * ionic_readb_raw --
 *
 *    Read unsigned 8 bit integer in native endianness - no byteswapping.
 *    Implement semantics of __raw_readb() Linux function.
 *    Access memory in native endianness.
 *
 *  Parameters:
 *     addr   - IN (virtual address)
 *
 *  Results:
 *     Return unsigned 8-bit integer read
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline vmk_uint8
ionic_readb_raw(const volatile vmk_VA addr)
{
        return *(const volatile vmk_uint8 *)addr;
}


/*
 ******************************************************************************
 *
 * ionic_writel_raw --
 *
 *    Write unsigned 32 bit integer in native endianness - no byteswapping.
 *    Implement semantics of __raw_writel() Linux function.
 *
 *  Parameters:
 *     value  - IN (unsigned 32 bit integer to write)
 *     addr   - IN (virtual address)
 *
 *  Results:
 *     @value is written to address @addr
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_writel_raw(vmk_uint32 value,
                 volatile vmk_VA addr)
{
        *(volatile vmk_uint32 *)addr = value;
}


/*
 ******************************************************************************
 *
 * ionic_writeq_raw --
 *
 *    Write unsigned 64 bit integer in native endianness - no byteswapping.
 *    Implement semantics of __raw_writeq() Linux function.
 *
 *  Parameters:
 *     value  - IN (unsigned 64 bit integer to write)
 *     addr   - IN (virtual address)
 *
 *  Results:
 *     @value is written to address @addr
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_writeq_raw(vmk_uint64 value,
                 volatile vmk_VA addr)
{
        *(volatile vmk_uint64 *)addr = value;
}

