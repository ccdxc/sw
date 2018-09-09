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
 * ionic_memory.c --
 *
 * Implement all memory related functions
 */

#include "ionic_log.h"
#include "ionic_memory.h"


/*
 ******************************************************************************
 *
 * ionic_mem_pool_init --
 *
 *     Initialize a new Memory Pool
 *
 *  Parameters:
 *     name      - IN (name of this memory pool)
 *     module_id - IN (module ID that will be associated with this memory pool)
 *     parent    - IN (handle of the parent memory pool, VMK_MEMPOOL_INVALID
 *                    indicates no parent)
 *     type      - IN (type of this memory pool, could be leaf or parent)
 *     init_size - IN (min number of bytes reserved for this memory pool)
 *     max_size  - IN (max number of bytes this memory pool is able to provide)
 *     mem_pool  - OUT (handle of the memory pool which is going to be created)
 *
 *  Results:
 *     VMK_OK                   - Success, mem_pool contains valid handle.
 *     VMK_RESERVATION_GT_LIMIT - Reservation was larger than the limit in
 *                                the pool properties. The reservation should
 *                                always be less or equal to the limit.
 *     VMK_NO_RESOURCES         - The requested pool reservation was rejected
 *                                because the parent’s pool did not have the
 *                                resources to fulfill it. If parent is
 *                                VMK_MEMPOOL_INVALID, the entire system did
 *                                not have enough resources to fulfill the
 *                                resource reservation.
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_mem_pool_init(const char *const name,                       // IN
                    vmk_ModuleID module_id,                       // IN
                    vmk_MemPool parent,                           // IN
                    vmk_MemPoolType type,                         // IN
                    vmk_ByteCount init_size,                      // IN
                    vmk_ByteCount max_size,                       // IN
                    vmk_MemPool *const mem_pool)                  // OUT
{
        VMK_ReturnStatus status;
        vmk_uint32 resv_pages, max_pages;
        vmk_MemPoolProps mem_pool_props;

        VMK_ASSERT(name && mem_pool);

        status = vmk_NameInitialize(&mem_pool_props.name, name);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_NameInitialize() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        resv_pages = IONIC_BYTES_TO_PAGES(init_size);
        max_pages  = IONIC_BYTES_TO_PAGES(max_size);

        mem_pool_props.module                     = module_id;
        mem_pool_props.parentMemPool              = parent;
        mem_pool_props.memPoolType                = type;
        mem_pool_props.resourceProps.reservation  = resv_pages;
        mem_pool_props.resourceProps.limit        = max_pages;

        status = vmk_MemPoolCreate(&mem_pool_props, mem_pool);
        if (status != VMK_OK) {
                ionic_err("vmk_MemPoolCreate() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_mem_pool_destroy --
 *
 *     Destroy the given memory pool
 *
 *  Parameters:
 *     mem_pool - IN/OUT (memory pool to be destroyed)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_mem_pool_destroy(vmk_MemPool mem_pool)                      // IN
{
        vmk_MemPoolDestroy(mem_pool);
}


/*
 ******************************************************************************
 *
 * ionic_heap_init --
 *
 *     Initialize heap for given module
 *
 *  Parameters:
 *     name       - IN (name of this heap)
 *     module_id   - IN (module ID of the module creating this heap)
 *     mem_pool    - IN (memory pool to be consumed by this heap, if
 *                      the type of the heap is VMK_HEAP_TYPE_MEMPOOL)
 *     type       - IN (type of the heap)
 *     init_size   - IN (initial size of the heap in bytes)
 *     max_size    - IN (maximal size of the heap in bytes)
 *     contiguity - IN (physical contiguity allocated from this heap)
 *     heap_id     - IN/OUT (heap ID of created heap. In the failure
 *                          case, we get VMK_INVALID_HEAP_ID)
 *
 *  Results:
 *     VMK_OK        - Success, heap_id contains valid handle.
 *     VMK_NO_MEM    - The heap could not be allcoated due to memroy limitation.
 *     VMK_BAD_PARAM - Invalid combination of heap_props->initial and
 *                     heap_props->max was specified.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_heap_init(const char *const name,                           // IN
                vmk_ModuleID module_id,                           // IN
                vmk_MemPool mem_pool,                             // IN
                vmk_HeapType type,                                // IN
                vmk_ByteCountSmall init_size,                     // IN
                vmk_ByteCountSmall max_size,                      // IN
                vmk_MemPhysContiguity contiguity,                 // IN
                vmk_HeapID *const heap_id)                        // OUT
{
        VMK_ReturnStatus status;
        vmk_HeapCreateProps heap_props;

        VMK_ASSERT(name && heap_id);

        status = vmk_NameInitialize(&heap_props.name, name);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_NameInitialize() failed: %s",
                          vmk_StatusToString(status));
                return status;
        }

        heap_props.module              = module_id;
        heap_props.initial             = init_size;
        heap_props.max                 = max_size;
        heap_props.creationTimeoutMS   = VMK_TIMEOUT_NONBLOCKING;
        heap_props.type                = type;

        if (type == VMK_HEAP_TYPE_MEMPOOL) {
                heap_props.typeSpecific.memPool.memPool = mem_pool;
        }

        heap_props.typeSpecific.memPool.physContiguity   = contiguity;
        heap_props.typeSpecific.memPool.physRange        = VMK_PHYS_ADDR_ANY;

        status = vmk_HeapCreate(&heap_props, heap_id);
        if (status != VMK_OK) {
                ionic_err("vmk_HeapCreate() failed: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_heap_destroy --
 *
 *     Destroy the given heap
 *
 *  Parameters:
 *     heap_id - IN/OUT (heap to be destroyed)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_heap_destroy(vmk_HeapID heap_id)                            // IN
{
        vmk_HeapDestroy(heap_id);
}


/*
 ******************************************************************************
 *
 * ionic_heap_alloc --
 *
 *     Allocate memory from the given heap ID
 *
 *  Parameters:
 *     heap_id - IN (heap to allocate from)
 *     size    - IN (number of bytes to be allocated)
 *
 *  Results:
 *     Starting address of the allocated memory.
 *     Returns NULL if the allocation fails.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void *
ionic_heap_alloc(vmk_HeapID heap_id,                              // IN
                 vmk_ByteCountSmall size)                         // IN
{
        void *addr;

        addr  = vmk_HeapAlloc(heap_id, size);

        if (VMK_UNLIKELY(!addr)) {
                ionic_err("vmk_HeapAlloc() failed");
        }

        return addr;
}


/*
 ******************************************************************************
 *
 * ionic_heap_align --
 *
 *     Allocate aligned memory from the given heap ID
 *
 *  Parameters:
 *     heap_id  - IN (heap to allocate from)
 *     size    - IN (number of bytes to be allocated)
 *     align   - IN (alignment for this piece of memory)
 *
 *  Results:
 *     Starting address of the allocated memory.
 *     Returns NULL if the allocation failes.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void *
ionic_heap_align(vmk_HeapID heap_id,                              // IN
                 vmk_ByteCountSmall size,                         // IN
                 vmk_ByteCountSmall align)                        // IN
{
        void *addr;

        addr  = vmk_HeapAlign(heap_id, size, align);

        if (VMK_UNLIKELY(!addr)) {
                ionic_err("vmk_HeapAlign() failed");
        }

        return addr;
}


/*
 ******************************************************************************
 *
 * ionic_heap_zalloc --
 *
 *     Allocate and zero out the memory from the given heap ID
 *
 *  Parameters:
 *     heap_id - IN (heap to allocate from)
 *     size   - IN (number of bytes to be allocated)
 *
 *  Results:
 *     Starting address of the allocated memory.
 *     Returns NULL if the allocation fails.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void *
ionic_heap_zalloc(vmk_HeapID heap_id,                             // IN
                  vmk_ByteCountSmall size)                        // IN
{
        void *addr = ionic_heap_alloc(heap_id, size);

        if (VMK_LIKELY(addr)) {
                vmk_Memset(addr, 0, size);
        }

        return addr;
}


/*
 ******************************************************************************
 *
 * ionic_heap_zalign --
 *
 *     Allocate and zero out the aligned memory from the given heap ID
 *
 *  Parameters:
 *     heap_id  - IN (heap to allocate from)
 *     size     - IN (number of bytes to be allocated)
 *     align    - IN (alignment for this piece of memory)
 *
 *  Results:
 *     Starting address of the allocated memory.
 *     Returns NULL if the allocation failes.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void *
ionic_heap_zalign(vmk_HeapID heap_id,                             // IN
                  vmk_ByteCountSmall size,                        // IN
                  vmk_ByteCountSmall align)                       // IN
{
        void *addr = ionic_heap_align(heap_id, size, align);

        if (VMK_LIKELY(addr)) {
                vmk_Memset(addr, 0, size);
        }

        return addr;
}


/*
 ******************************************************************************
 *
 * ionic_heap_free --
 *
 *     Free memory resource
 *
 *  Parameters:
 *     heap_id - IN (heap to be freed)
 *     addr    - IN (starting address of the memory)
 *
 *  Results:
 *     Free addr and set it to be NULL.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

inline void
ionic_heap_free(vmk_HeapID heap_id,                               // IN
               void *addr)                                        // IN
{
        VMK_ASSERT(addr);

        if (VMK_LIKELY(addr)) {
                vmk_HeapFree(heap_id, addr);
                addr = NULL;
        }
}


