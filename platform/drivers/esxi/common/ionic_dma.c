/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_dma.c --
 *
 * Implement all DMA functions
 */

#include "ionic_log.h"
#include "ionic_memory.h"
#include "ionic_types.h"

/*
 ******************************************************************************
 *
 * ionic_dma_engine_create --
 *
 *    Create a DMA engine for given vmkDevice
 *
 *  Parameters:
 *     name              - IN (name of this DMA engine)
 *     module_id         - IN (module that associates with the DMA engine)
 *     device            - IN (device that this DMA engine is associated with)
 *     is_coherent       - IN (if true we consider this as a coherent DMA engine),
 *     sg_elem_size      - IN (max size of a SG array element)
 *     sg_elem_alignment - IN (all SG elements has to be aligned to this bytes)
 *     dma_engine        - OUT (return a vmk_DMAEngine handle)
 *
 *  Results:
 *     VMK_OK              - Success, dma_engine contains valid handle 
 *     VMK_NO_MODULE_HEAP  - The heap of this module is not set.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_dma_engine_create(const char *const name,                   // IN
                        vmk_ModuleID module_id,                   // IN
                        vmk_Device device,                        // IN
                        vmk_Bool is_coherent,                     // IN
                        vmk_ByteCountSmall sg_elem_size,          // IN
                        vmk_ByteCountSmall sg_elem_alignment,     // IN
                        vmk_DMAEngine *const dma_engine)          // IN/OUT
{
        VMK_ReturnStatus status;
        vmk_DMAEngineProps dma_engine_props;
        vmk_DMAConstraints dma_constraints;

        VMK_ASSERT(name && dma_engine);

        vmk_Memset(&dma_constraints, 0, sizeof(vmk_DMAConstraints));
        dma_constraints.addressMask       = VMK_ADDRESS_MASK_64BIT;
        dma_constraints.sgElemMaxSize     = sg_elem_size;
        dma_constraints.sgElemAlignment   = sg_elem_alignment;

        vmk_Memset(&dma_engine_props, 0, sizeof(vmk_DMAEngineProps));
        status = vmk_NameInitialize(&dma_engine_props.name, name);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_NameInitialize() failed: %s", vmk_StatusToString(status));
                *dma_engine = VMK_DMA_ENGINE_INVALID;
                return status;
        }
        dma_engine_props.module           = module_id;
        dma_engine_props.device           = device;
        dma_engine_props.constraints      = &dma_constraints;
        dma_engine_props.bounce           = NULL;

        if (is_coherent) {
                dma_engine_props.flags    = VMK_DMA_ENGINE_FLAGS_COHERENT;
        } else {
#if VSPHERE_VER >= VSPHERE_VERS(2020)
                dma_engine_props.flags    = VMK_DMA_ENGINE_FLAGS_NON_COHERENT;
#else
                dma_engine_props.flags    = VMK_DMA_ENGINE_FLAGS_NONE;
#endif
        }

        status = vmk_DMAEngineCreate(&dma_engine_props,
                                     dma_engine);
        if (status != VMK_OK) {
                ionic_err("vmk_DMAEngineCreate() failed: %s",
                          vmk_StatusToString(status));
                *dma_engine = VMK_DMA_ENGINE_INVALID;
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_dma_engine_destroy --
 *
 *    Destroy the given DMA engine
 *
 *  Parameters:
 *     dma_engine - IN (vmk_DMAEngine to be destroyed)
 *
 *  Results:
 *     VMK_OK - Success.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_dma_engine_destroy(vmk_DMAEngine dma_engine)                // IN
{
        VMK_ReturnStatus status;

        VMK_ASSERT(dma_engine != VMK_DMA_ENGINE_INVALID);

        status = vmk_DMAEngineDestroy(dma_engine);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_DMAEngineDestroy() failed: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_dma_map_ma
 *
 *     Map a DMA element
 *
 *  Parameters:
 *     dma_engine  - IN (DMA engine to use)
 *     dir         - IN (Direction of data flow for a DMA)
 *     ma          - IN (machine address to be mapped)
 *     length      - IN (size of the DMA mapping)
 *
 *  Results:
 *     DMA address returned if successful
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

vmk_IOA
ionic_dma_map_ma(vmk_DMAEngine dma_engine,                        // IN
                 vmk_DMADirection dir,                            // IN
                 vmk_MA ma,                                       // IN
                 vmk_ByteCountSmall length)                       // IN
{
        VMK_ReturnStatus status;
        vmk_SgElem in, out;
        vmk_DMAMapErrorInfo dma_err;

        VMK_ASSERT(ma);

        if (VMK_UNLIKELY(dma_engine == VMK_DMA_ENGINE_INVALID)) {
                ionic_err("DMA engine uninitialized");

                /* PSOD the system in beta build */
                VMK_ASSERT(0);

                return (vmk_IOA)NULL;
        }

        vmk_Memset(&in, 0, sizeof(in));
        in.addr   = ma;
        in.length = length;

        status = vmk_DMAMapElem(dma_engine,
                                dir,
                                &in,
                                VMK_TRUE,
                                &out,
                                &dma_err);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                if (status == VMK_NO_MEMORY) {
                        /*
                         * Not enough memory available to construct the mapping.
                         * This is not a fatal error - just return NULL
                         */
                        ionic_err("vmk_DMAMapElem() failed: not enough memory"
                                  " to map machine address 0x%lx (length %d)",
                                  ma, length);
                } else {
                        if (status == VMK_DMA_MAPPING_FAILED) {
                                ionic_err("Failed to map machine address"
                                          " 0x%lx (%d): %s.",
                                          ma, length,
                                          vmk_DMAMapErrorReasonToString(dma_err.reason));
                        } else {
                                ionic_err("Failed to map machine address"
                                          " 0x%lx (%d): %s.",
                                          ma, length, vmk_StatusToString(status));
                        }

                }

                return (vmk_IOA)NULL;
        }

        VMK_ASSERT(out.length == in.length);
        return out.ioAddr;
}


/*
 ******************************************************************************
 *
 * ionic_dma_map_va
 *
 *     Map a DMA element
 *
 *  Parameters:
 *     dma_engine  - IN (DMA engine to use)
 *     dir         - IN (Direction of data flow for a DMA)
 *     va          - IN (virtual address to be mapped)
 *     length      - IN (size of the DMA mapping)
 *
 *  Results:
 *     DMA address returned if successful
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

vmk_IOA
ionic_dma_map_va(vmk_DMAEngine dma_engine,                        // IN
                 vmk_DMADirection dir,                            // IN
                 vmk_VA va,                                       // IN
                 vmk_ByteCountSmall length)                       // IN
{
        VMK_ReturnStatus status;
        vmk_MA ma;

        VMK_ASSERT(va);

        status = vmk_VA2MA(va,
                           length,
                           &ma);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_VA2MA() failed, status: %s",
                          vmk_StatusToString(status));
                return (vmk_IOA)NULL;
        }

        return ionic_dma_map_ma(dma_engine,
                                dir,
                                ma,
                                length);
}


/*
 ******************************************************************************
 *
 * ionic_dma_unmap
 *
 *     Unmap a DMA element
 *
 *  Parameters:
 *     dma_engine  - IN (DMA engine to use)
 *     dir         - IN (Direction of data flow for a DMA)
 *     length      - IN (size of the DMA mapping)
 *     ioa         - IN (DMA address to unmap)
 *
 *  Results:
 *     VMK_ReturnStatus
 *
 *  Side effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_dma_unmap(vmk_DMAEngine dma_engine,                         // IN
                vmk_DMADirection dir,                             // IN
                vmk_ByteCountSmall length,                        // IN
                vmk_IOA ioa)                                      // IN
{
        VMK_ReturnStatus status;
        vmk_SgElem unmap;

        VMK_ASSERT(ioa);

        vmk_Memset(&unmap, 0, sizeof(unmap));
        unmap.ioAddr = ioa;
        unmap.length = length;

        status = vmk_DMAUnmapElem(dma_engine, dir, &unmap);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_DMAUnmapElem() failed, status: %s",
                          vmk_StatusToString(status));
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_dma_alloc_align --
 *
 *    Allocate (aligned to a required size) and map DMA memory
 *
 *  Parameters:
 *     heap_id    - IN (heap to allocate buffer from)
 *     dma_engine - IN (DMA engine to use)
 *     size       - IN (size of the allocation)
 *     align      - IN (alignment of the allocation)
 *     ioa        - OUT (DMA address returned if successful)
 *
 *  Results:
 *     Return virtual address on success or NULL on failure
 *     ioa will contain DMA address
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void *
ionic_dma_alloc_align(vmk_HeapID heap_id,                         // IN
                      vmk_DMAEngine dma_engine,                   // IN
                      vmk_uint32 size,                            // IN
                      vmk_uint32 align,                           // IN
                      vmk_IOA *ioa)                               // OUT
{
        void *va;

        VMK_ASSERT(ioa);

        va = ionic_heap_align(heap_id,
                              size,
                              align);
        if (VMK_UNLIKELY(!va)) {
                return va;
        }

        *ioa = ionic_dma_map_va(dma_engine,
                                VMK_DMA_DIRECTION_BIDIRECTIONAL,
                                (vmk_VA)va,
                                size);
        if (VMK_UNLIKELY(!*ioa)) {
                ionic_err("ionic_dma_map_va failed");
                goto map_va_err;
        }

        return va;

map_va_err:
        ionic_heap_free(heap_id, va);

        return NULL;
}


/*
 ******************************************************************************
 *
 * ionic_dma_zalloc_align --
 *
 *    Allocate (aligned to a required size), clear and map DMA memory
 *
 *  Parameters:
 *     heap_id    - IN (heap to allocate buffer from)
 *     dma_engine - IN (DMA engine to use)
 *     size       - IN (size of the allocation)
 *     align      - IN (alignment of the allocation)
 *     ioa        - OUT (DMA address returned if successful)
 *
 *  Results:
 *     Return virtual address on success or NULL on failure
 *     ioa will contain DMA address
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void *
ionic_dma_zalloc_align(vmk_HeapID heap_id,                        // IN
                       vmk_DMAEngine dma_engine,                  // IN
                       vmk_uint32 size,                           // IN
                       vmk_uint32 align,                          // IN
                       vmk_IOA *ioa)                              // OUT
{
        void *va;

        va = ionic_dma_alloc_align(heap_id,
                                   dma_engine,
                                   size,
                                   align,
                                   ioa);
        if (VMK_LIKELY(va)) {
                vmk_Memset(va, 0, size);
        }

        return va;
}


/*
 ******************************************************************************
 *
 * ionic_dma_free --
 *
 *    Free and unmap DMA memory
 *
 *  Parameters:
 *     heap_id    - IN (heap to use)
 *     dma_engine - IN (DMA engine to use)
 *     size       - IN (size to free / unmap, should be equal to size of allocation)
 *     va         - IN (virtual address of the memory)
 *     ioa        - IN (DMA address of the memory)
 *
 *  Results:
 *     DMA memory described by @ioa is unmapped and virtual address @va freed
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_dma_free(vmk_HeapID heap_id,                                // IN
               vmk_DMAEngine dma_engine,                          // IN
               vmk_uint32 size,                                   // IN
               void *va,                                          // IN
               vmk_IOA ioa)                                       // IN
{
        VMK_ReturnStatus status;

        VMK_ASSERT(va || !size);

        if (!va) {
                return;
        }

        status = ionic_dma_unmap(dma_engine,
                                 VMK_DMA_DIRECTION_BIDIRECTIONAL,
                                 size,
                                 ioa);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("vmk_DMAUnmapElem() failed, status: %s",
                          vmk_StatusToString(status));
        }

        ionic_heap_free(heap_id, va);
}

