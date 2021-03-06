/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_hash.c --
 *
 * Implement hash related functions
 */

#include "ionic_log.h"

/*
 *******************************************************************************
 *
 *   ionic_hash_alloc
 *
 *      Allocate a new hash table
 *
 *  Parameters:
 *    module_id   - IN (module ID that requests the hash table)
 *    heap_id     - IN (heap ID to use for allocation)
 *    key_type    - IN (type of key to be used)
 *    key_flags   - IN (flags relating to the keys)
 *    key_size    - IN (key size)
 *    num_entries - IN (estimated number of hash bucket entries)
 *    acquire     - IN (function to acquire a reference to a value)
 *    release     - IN (function to release a reference to a value)
 *    hdl         - OUT (Handle for the new hash table)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 *******************************************************************************
 */

inline VMK_ReturnStatus
ionic_hash_alloc(vmk_ModuleID module_id,                          // IN
                 vmk_HeapID heap_id,                              // IN
                 int key_type,                                    // IN
                 vmk_HashKeyFlags key_flags,                      // IN
                 vmk_uint32 key_size,                             // IN
                 vmk_uint32 num_entries,                          // IN
                 void (*acquire)(vmk_HashValue value),            // IN
                 void (*release)(vmk_HashValue value),            // IN
                 vmk_HashTable *hdl)                              // OUT
{
        VMK_ReturnStatus status;
        vmk_HashProperties hash_props;

        hash_props.moduleID     = module_id,
        hash_props.heapID       = heap_id,
        hash_props.keyType      = key_type,
        hash_props.keyFlags     = key_flags,
        hash_props.keySize      = key_size,
        hash_props.nbEntries    = num_entries,
        hash_props.acquire      = acquire,
        hash_props.release      = release,

        status = vmk_HashAlloc(&hash_props, hdl);

        if (status != VMK_OK) {
                ionic_err("vmk_HashAlloc() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


