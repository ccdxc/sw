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
 * ionic_interrupt.c --
 *
 * Implement interrupt related APIs 
 */

#include "ionic_utilities.h"
#include "ionic_memory.h"
#include "ionic_locks.h"
#include "ionic_log.h"
#include "ionic_interrupt.h"

/*
 ******************************************************************************
 *
 * ionic_dummy_msix_iar --
 *
 *    Dummy MSIX Interrupt acknowledge routine (IAR)
 *
 *
 *  Parameters:
 *     handler_data - IN (The info provided during the cookie registration)
 *     intr_cookie  - IN (interrupt cookie that this interrupt is associated with)
 *
 *  Results:
 *     Interrupt is handled and VMK_OK is returned.
 *
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_dummy_msix_iar(void *handler_data,                          // IN
                     vmk_IntrCookie intr_cookie)                  // IN
{
        return VMK_OK;
}


/*
 ******************************************************************************
 *
 * ionic_int_register --
 *
 *    Helper function to register a interrupt.
 *
 *  Parameters:
 *     module_id    - IN (module that this interrupt belongs)
 *     vmk_device   - IN (vmkernel device associates with this interrupt)
 *     intr_cookie  - IN (Interrupt cookie to register)
 *     name         - IN (Name of the device registering the interrupt)
 *     handler      - IN (Interrupt handler function)
 *     handler_data - IN (The client data that provided during the cookie
 *                        registration)
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
ionic_int_register(vmk_ModuleID module_id,                        // IN
                   vmk_Device vmk_device,                         // IN
                   vmk_IntrCookie intr_cookie,                    // IN
                   const char *name,                              // IN
                   vmk_IntrHandler handler,                       // IN
                   void *handler_data)                            // IN
{
        VMK_ReturnStatus status;
        vmk_IntrProps intr_props;

        vmk_Memset(&intr_props, 0, sizeof(vmk_IntrProps));

        intr_props.device    = vmk_device;
        /* device interrupts contribute to system wide entropy pool */
        intr_props.attrs     = VMK_INTR_ATTRS_ENTROPY_SOURCE;

        status = vmk_NameInitialize(&intr_props.deviceName, name);
        if (status != VMK_OK) {
                ionic_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        intr_props.acknowledgeInterrupt   = ionic_dummy_msix_iar;
        intr_props.handler                = handler;
        intr_props.handlerData            = handler_data;

        status = vmk_IntrRegister(module_id,
                                  intr_cookie,
                                  &intr_props);
        if (status != VMK_OK) {
                ionic_err("vmk_IntrRegister() for interrupt cookie "
                          "0x%x \"%s\" failed, status: %s",
                          intr_cookie, name, vmk_StatusToString(status));
                return status;
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_int_unregister --
 *
 *    Unregister the given interrupt.
 *
 *  Parameters:
 *     module_id    - IN (ID of the calling module)
 *     intr_cookie  - IN (Interrupt cookie to unregister)
 *     handler_data - IN (The info provided during the cookie registration)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_int_unregister(vmk_ModuleID module_id,                      // IN
                     vmk_IntrCookie intr_cookie,                  // IN
                     void *handler_data)                          // IN
{
        VMK_ReturnStatus status;

        status = vmk_IntrUnregister(module_id,
                                    intr_cookie,
                                    handler_data);
        if (status != VMK_OK) {
                ionic_err("vmk_IntrUnregister() for interrupt cookie "
                          "0x%x failed, status: %s",
                          intr_cookie, vmk_StatusToString(status));
        }
}


/*
 ******************************************************************************
 *
 * ionic_int_alloc --
 *
 *    Allocate interrupt cookies.
 *
 *  Parameters:
 *     module_id         - IN (ID of the calling module)
 *     heap_id           - IN (ID of heap to be used for allocations)
 *     pci_device        - IN (PCI device handle)
 *     int_type          - IN (Interrupt type)
 *     desired           - IN (Number of interrupts desired)
 *     required          - IN (Number of interrupts required)
 *     intr_cookie_array - OUT (Array of interrupts allocated)
 *     granted           - OUT (Number of interrupts allocated)
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
ionic_int_alloc(vmk_ModuleID module_id,                           // IN
                vmk_HeapID heap_id,                               // IN
                vmk_PCIDevice pci_device,                         // IN
                vmk_PCIInterruptType int_type,                    // IN
                vmk_uint32 desired,                               // IN
                vmk_uint32 required,                              // IN
                vmk_IntrCookie *intr_cookie_array,                // OUT
                vmk_uint32 *granted)                              // OUT
{
        VMK_ReturnStatus status;
        vmk_uint16 *msix_index_array;
        int i;

        if (int_type == VMK_PCI_INTERRUPT_TYPE_MSIX) {

                msix_index_array = ionic_heap_alloc(heap_id,
                                                    desired *
                                                    sizeof(vmk_uint16));

                if (VMK_UNLIKELY(!msix_index_array)) {
                        return VMK_NO_MEMORY;
                }

                /*
                 * Specify the index into MSIX table for the desired interrupts
                 */

                for (i = 0; i < desired; ++i) {
                        msix_index_array[i] = i;
                }

        } else {
                msix_index_array = NULL;
        }

        status = vmk_PCIAllocIntrCookie(module_id,
                                        pci_device,
                                        int_type,
                                        desired,
                                        required,
                                        msix_index_array,
                                        intr_cookie_array,
                                        granted);

        if (int_type == VMK_PCI_INTERRUPT_TYPE_MSIX) {
                ionic_heap_free(heap_id, msix_index_array);
        }

        if (status != VMK_OK) {
                ionic_err("vmk_PCIAllocIntrCookie() failed. status: %s",
                          vmk_StatusToString(status));
        }


        return status;
}


/*
 ******************************************************************************
 *
 * ionic_int_free --
 *
 *    Free interrupt cookies.
 *
 *  Parameters:
 *     module_id         - IN (ID of the calling module)
 *     heap_id           - IN (ID of heap to be used for allocations)
 *     pci_device        - IN (PCI device handle)
 *     intr_cookie_array - OUT (Array of interrupts to be freeed)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_int_free(vmk_ModuleID module_id,                           // IN
               vmk_HeapID heap_id,                               // IN
               vmk_PCIDevice pci_device,                         // IN
               vmk_IntrCookie *intr_cookie_array)                // IN
{
        VMK_ReturnStatus status;

        VMK_ASSERT(intr_cookie_array);

        status = vmk_PCIFreeIntrCookie(module_id,
                                       pci_device);

	if (status != VMK_OK) {
		ionic_err("vmk_PCIFreeIntrCookie() failed, status: %s",
			  vmk_StatusToString(status));
	}
	
        ionic_heap_free(heap_id,
			intr_cookie_array);
}
