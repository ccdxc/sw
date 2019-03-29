/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_hash.h --
 *
 * Definitions for hash related data structures and functions 
 */

#ifndef _IONIC_HASH_H_
#define _IONIC_HASH_H_

#include <vmkapi.h>
#include "ionic_log.h"

inline VMK_ReturnStatus
ionic_hash_alloc(vmk_ModuleID module_id,
                 vmk_HeapID heap_id,
                 int key_type,
                 vmk_HashKeyFlags key_flags,
                 vmk_uint32 key_size,
                 vmk_uint32 num_entries,
                 void (*acquire)(vmk_HashValue value),
                 void (*release)(vmk_HashValue value),
                 vmk_HashTable *hdl);



#endif /* End of _IONIC_HASH_H_ */
