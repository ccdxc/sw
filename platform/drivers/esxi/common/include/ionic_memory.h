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
 * ionic_memory.h --
 *
 * Implement all memory related functions
 */

#ifndef _IONIC_MEMORY_H_
#define _IONIC_MEMORY_H_

#include <vmkapi.h>

#define IONIC_BYTES_TO_PAGES(nBytes)                              \
   (((nBytes) + VMK_PAGE_SIZE - 1) >> VMK_PAGE_SHIFT)


VMK_ReturnStatus
ionic_mem_pool_init(const char *const name,
                    vmk_ModuleID module_id,
                    vmk_MemPool parent,
                    vmk_MemPoolType type,
                    vmk_ByteCount init_size,
                    vmk_ByteCount max_size,
                    vmk_MemPool *const memPool);

inline void
ionic_mem_pool_destroy(vmk_MemPool memPool);

VMK_ReturnStatus
ionic_heap_init(const char *const name,
                vmk_ModuleID module_id,
                vmk_MemPool memPool,
                vmk_HeapType type,
                vmk_ByteCountSmall init_size,
                vmk_ByteCountSmall max_size,
                vmk_MemPhysContiguity contiguity,
                vmk_HeapID *const heap_id);

inline void
ionic_heap_destroy(vmk_HeapID heap_id);

inline void *
ionic_heap_alloc(vmk_HeapID heap_id,
                 vmk_ByteCountSmall size);

inline void *
ionic_heap_align(vmk_HeapID heap_id,
                 vmk_ByteCountSmall size,
                 vmk_ByteCountSmall align);

inline void *
ionic_heap_zalloc(vmk_HeapID heap_id,
                  vmk_ByteCountSmall size);

inline void *
ionic_heap_zalign(vmk_HeapID heap_id,
                  vmk_ByteCountSmall size,
                  vmk_ByteCountSmall align);

inline void
ionic_heap_free(vmk_HeapID heap_id,
                void *addr);

#endif /* End of _IONIC_MEMORY_H_ */
