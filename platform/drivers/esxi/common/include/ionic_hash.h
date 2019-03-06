/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_hash.h] in the main directory of this source tree.
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
