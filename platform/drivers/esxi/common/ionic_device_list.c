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
 * ionic_device_list.c --
 *
 * Implement functions for  maintaining device list(be used by mgmt interface)
 */

#include "ionic_device_list.h"
#include "ionic_locks.h"
#include "ionic_log.h"


/*
 ******************************************************************************
 *
 * ionic_device_list_init --
 *
 *     Initialize a new device list.
 *
 *
 *  Parameters:
 *     module_id    - IN (Driver module ID)
 *     heap_id      - IN (Driver module heap ID)
 *     cb           - IN (drive specific get sbdf function callback)
 *     device_list  - IN/OUT (device list pointer)
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
ionic_device_list_init(vmk_ModuleID module_id,                    // IN
                       vmk_HeapID heap_id,                        // IN
                       ionic_get_sbdf cb,                         // IN
                       struct ionic_device_list *device_list)     // IN/OUT
{  
        VMK_ReturnStatus status;
        vmk_HashProperties hash_props;

        device_list->get_sbdf_cb = cb;

        status = ionic_binary_sema_create(heap_id,
                                          "device list lock",
                                          &device_list->lock);
        if (status != VMK_OK) {
                ionic_err("ionic_binary_sema_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        vmk_Memset(&hash_props, 0, sizeof(vmk_HashProperties));

        hash_props.moduleID  = module_id;
        hash_props.heapID    = heap_id;
        hash_props.keyType   = VMK_HASH_KEY_TYPE_STR;
        hash_props.keyFlags  = VMK_HASH_KEY_FLAGS_LOCAL_COPY;
        hash_props.keySize   = VMK_MISC_NAME_MAX;
        hash_props.nbEntries = IONIC_DEVICE_LIST_SIZE_HINT;
        hash_props.acquire   = NULL;
        hash_props.release   = NULL;

        status = vmk_HashAlloc(&hash_props,
                               &device_list->list);
        if (status != VMK_OK) {
                ionic_err("vmk_HashAlloc() failed, status: %s",
                          vmk_StatusToString(status));
                ionic_sema_destroy(&device_list->lock);
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_device_list_destroy --
 *
 *     Destroy the current device list
 *     We must flush the list before destroying it.
 *
 *  Parameters:
 *     device_list  - IN (device list pointer)
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
ionic_device_list_destroy(struct ionic_device_list *device_list)  // IN
{
        VMK_ReturnStatus status, status1;

        status = vmk_HashDeleteAll(device_list->list);
        if (status != VMK_OK) {
                ionic_err("vmk_HashDeleteAll() failed, status: %s",
                          vmk_StatusToString(status));
        }

        status1 = vmk_HashRelease(device_list->list);
        if (status1 != VMK_OK) {
                ionic_err("vmk_HashRelease() failed, status: %s",
                          vmk_StatusToString(status1));
        }

        ionic_sema_destroy(&device_list->lock);

        device_list->list = NULL;

        return (status && status1);
}


/*
 ******************************************************************************
 *
 * ionic_device_list_flush --
 *
 *     Delete every entry in the device list.
 *
 *  Parameters:
 *     device_list  - IN (device list pointer)
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
ionic_device_list_flush(struct ionic_device_list *device_list)               //IN
{
        VMK_ReturnStatus status;

        vmk_SemaLock(&device_list->lock);
        status = vmk_HashDeleteAll(device_list->list);
        vmk_SemaUnlock(&device_list->lock);

        if (status != VMK_OK) {
                ionic_err("vmk_HashKeyDelete() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_device_list_add --
 *
 *    Insert a new entry to the device list.
 *
 *  Parameters:
 *     device_name   - IN (a key that represent the name of the device)
 *     data         - IN (driver priv data)
 *     device_list    - IN/OUT (device list pointer)
 *
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
ionic_device_list_add(vmk_Name device_name,                       // IN
                      vmk_AddrCookie data,                        // IN
                      struct ionic_device_list *device_list)      // IN/OUT
{
        VMK_ReturnStatus status;

        if (!device_list->list) {
                ionic_err("ionic_device_list is not initialized");
                return VMK_NOT_INITIALIZED;
        }

        vmk_SemaLock(&device_list->lock);
        status = vmk_HashKeyInsert(device_list->list,
                                   (vmk_HashKey) device_name.string,
                                   (vmk_HashValue) data.ptr);
        vmk_SemaUnlock(&device_list->lock);

        if (status != VMK_OK) {
                ionic_err("vmk_HashKeyInsert() failed, status %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_device_list_remove --
 *
 *    Delete an existing entry from the device list.
 *
 *  Parameters:
 *     device_name    - IN  (a key that represents the name of the device)
 *     device_list    - IN/OUT (device list pointer)
 *     data           - IN (driver priv data)
 *
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
ionic_device_list_remove(vmk_Name device_name,                    // IN
                         struct ionic_device_list *device_list,   // IN/OUT
                         vmk_AddrCookie data)                     // IN
{
        VMK_ReturnStatus status;

        if (!device_list->list) {
                ionic_err("ionic_en_device_list is not initialized");
                return VMK_NOT_INITIALIZED;
        }

        vmk_SemaLock(&device_list->lock);
        status = vmk_HashKeyDelete(device_list->list,
                                   (vmk_HashKey) device_name.string,
                                   (vmk_HashValue *) data.ptr);
        vmk_SemaUnlock(&device_list->lock);

        if (status != VMK_OK) {
                ionic_err("vmk_HashKeyDelete() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_device_list_get --
 *
 *    Retrieve an existing entry from the device list.
 *
 *  Parameters:
 *     device_name       - IN (a key that represents the name of the device)
 *     device_list       - IN/OUT (device list pointer)
 *     data              - IN (driver priv data)
 *
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
ionic_device_list_get(vmk_Name device_name,                       // IN
                      struct ionic_device_list *device_list,      // IN/OUT
                      vmk_AddrCookie data)                        // IN
{
        VMK_ReturnStatus status;

        if (!device_list->list) {
                ionic_err("ionic_en_device_list is not initialized");
                return VMK_NOT_INITIALIZED;
        }

        vmk_SemaLock(&device_list->lock);
        status = vmk_HashKeyFind(device_list->list,
                                 (vmk_HashKey) device_name.string,
                                 (vmk_HashValue *) data.ptr);
        vmk_SemaUnlock(&device_list->lock);

        if (status != VMK_OK) {
                ionic_err("vmk_HashKeyFind() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}
