//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __SDK_LIB_RTE_BITMAP_SLAB_OP_HPP__
#define __SDK_LIB_RTE_BITMAP_SLAB_OP_HPP__

#include <rte_bitmap.h>

#define RTE_BITMAP_START_SLAB_SCAN_POS 0xFFFFFFFF

//  Bitmap slab scan
// \param[in]  slab2 current slab for scan
// \param[in]  cur_pos current position of bit set in the slab, 0 based
// \param[out] next_pos next position where the next bit is set, 0 based
// \return     0 if no bit set in the slab starting current postion, 1 otherwise
static inline int
rte_bitmap_slab_scan (uint64_t slab2, uint32_t cur_pos, uint32_t *next_pos)
{
    //  completely un-scanned slab will have default current postion
    if (cur_pos == RTE_BITMAP_START_SLAB_SCAN_POS) {
        return (rte_bsf64_safe(slab2, next_pos));
    } else {
        slab2 = slab2 >> cur_pos >> 1;
        if (rte_bsf64_safe(slab2, next_pos)) {
            *next_pos += cur_pos + 1;
            return 1;
        }
    }
    return 0;
}

// Given the slab2 index and relative position of the bit within the slab,
// return the global position of the bit
// \param[in] index2 Slab2 index
// \param[in] pos Position of the bit within the slab
//  \return   global position of the bit
static inline uint32_t
rte_bitmap_get_global_bit_pos (uint32_t index2, uint32_t pos)
{
    return ((index2 << RTE_BITMAP_SLAB_BIT_SIZE_LOG2) + pos);
}

// Set the corresponding slab1 bit given a slab2 index
// \param[in] bmp bitmap
// \param[in] index2 Slab2 index
// This is pure internal function, shouldn't be called directly by app
static inline void
__rte_bitmap_set_slab1 (struct rte_bitmap *bmp, uint32_t index2)
{
    uint32_t index1, offset1;
    uint64_t *slab1;

    index1 = index2 >> RTE_BITMAP_CL_BIT_SIZE_LOG2;
    offset1 = (index2 >> (RTE_BITMAP_CL_BIT_SIZE_LOG2 -
                          RTE_BITMAP_SLAB_BIT_SIZE_LOG2))
                & RTE_BITMAP_SLAB_BIT_MASK;
    slab1 = bmp->array1 + index1;
    *slab1 |= 1llu << offset1;
}

static inline void
rte_bitmap_and (struct rte_bitmap *ibmp1, struct rte_bitmap *ibmp2,
                struct rte_bitmap *obmp)
{
    uint32_t index2;

    for (index2 = 0; index2 < ibmp1->array2_size; index2++) {
        obmp->array2[index2] = ibmp1->array2[index2] & ibmp2->array2[index2];

        if (obmp->array2[index2]) {
            __rte_bitmap_set_slab1(obmp, index2);
        }
    }
}

static inline void
rte_bitmap_or (struct rte_bitmap *ibmp1, struct rte_bitmap *ibmp2,
               struct rte_bitmap *obmp)
{
    uint32_t index2;

    for (index2 = 0; index2 < ibmp1->array2_size; index2++) {
        obmp->array2[index2] = ibmp1->array2[index2] | ibmp2->array2[index2];

        if (obmp->array2[index2]) {
            __rte_bitmap_set_slab1(obmp, index2);
        }
    }
}

// compare two indexes if present in same slab or not
// \param[in] index1 first index
// \param[in] index2 second index
// \return     true if array1 index is same for both
// <index1> and <index2>
static inline bool
rte_compare_indexes (uint32_t index1, uint32_t index2)
{
    uint32_t array1_index1, array1_index2;

    // If both <index1> and given <index2> has same array1 index
    // then return true otherwise false
    array1_index1 = index1 >> (RTE_BITMAP_SLAB_BIT_SIZE_LOG2 +
                               RTE_BITMAP_CL_BIT_SIZE_LOG2);
    array1_index2 = index2 >> (RTE_BITMAP_SLAB_BIT_SIZE_LOG2 +
                               RTE_BITMAP_CL_BIT_SIZE_LOG2);
    return (array1_index1 == array1_index2);
}

// find lowest set bit(LSB) in the indexer
// \param[in]  indxr rte bitmap indexer
// \param[out] slab  slab containing pos
// \param[out] pos   position of the LSB
// \return     1 if position found else 0
static inline uint32_t
rte_bitmap_leftmost_slab_scan (rte_bitmap *indxr, uint64_t *slab,
                              uint32_t *pos)
{
    uint64_t *slab1;
    uint32_t index1, index2, offset1, offset2 = 0;

    for (index1 = 0 ; index1 < indxr->array1_size; index1++) {
        slab1 = indxr->array1 + index1;
        offset1 = 0;
        if (rte_bsf64_safe(*slab1, &offset1)) {
            index2 = (((index1 << RTE_BITMAP_SLAB_BIT_SIZE_LOG2) + offset1)
                      << RTE_BITMAP_CL_SLAB_SIZE_LOG2);
            rte_prefetch0((void *)(indxr->array2 + index2));
            do {
                *slab = *(indxr->array2 + index2);
                offset2 = 0;
                if (rte_bsf64_safe(*slab, &offset2)) {
                    *pos = rte_bitmap_get_global_bit_pos(index2, offset2);
                    return 1;
                } else {
                    index2++;
                    rte_prefetch1((void *)(indxr->array2 + index2));
                }
            } while(index2 & RTE_BITMAP_CL_SLAB_MASK);
        }
    }
    return 0;
}

// clear block in the indexer
// param[in] indxr indexer
// param[in] start starting position of the block
// param[in] size  size of the block
// return    none
static inline void
rte_bitmap_clear_slabs (rte_bitmap *indxr, uint32_t start, uint32_t size)
{
    uint64_t *slab2, mask, *slab1;
    uint32_t index1, index2, offset2, offset1, block;

    while(size) {
        offset2 = start & RTE_BITMAP_SLAB_BIT_MASK;
        index2 = start >> RTE_BITMAP_SLAB_BIT_SIZE_LOG2;
        block = RTE_BITMAP_SLAB_BIT_SIZE - offset2;
        if (block > RTE_BITMAP_SLAB_BIT_MASK)
            mask = 0llu;
        else
            mask = (~0llu) >> block;
        if (size < block) {
            block = size;
            offset2 = RTE_BITMAP_SLAB_BIT_SIZE - (size + offset2);
            mask = mask | ~((~0llu) >> offset2);
        }
        slab2 = indxr->array2 + index2;
        *slab2 &= mask;
        index2 &= ~ RTE_BITMAP_CL_SLAB_MASK;
        slab2 = indxr->array2 + index2;
        if (!__rte_bitmap_line_not_empty(slab2)) {
            index1 = (start >> (RTE_BITMAP_SLAB_BIT_SIZE_LOG2 + RTE_BITMAP_CL_BIT_SIZE_LOG2));
            slab1 = indxr->array1 + index1;
            offset1 = (start >> RTE_BITMAP_CL_BIT_SIZE_LOG2) & RTE_BITMAP_SLAB_BIT_MASK;
            *slab1 &= ~(1llu << offset1);
        }
        start = start + block;
        size -= block;
    }
}

// set block in the indexer
// param[in] indxr indexer
// param[in] start starting position of the block
// param[in] size  size of the block
// return    none
static inline void
rte_bitmap_set_slabs (rte_bitmap *indxr, uint32_t start, uint32_t size)
{
    uint64_t *slab2, mask;
    uint32_t index2, offset2, block;

    while(size) {
        offset2 = start & RTE_BITMAP_SLAB_BIT_MASK;
        index2 = start >> RTE_BITMAP_SLAB_BIT_SIZE_LOG2;
        block = RTE_BITMAP_SLAB_BIT_SIZE - offset2;
        if (block > RTE_BITMAP_SLAB_BIT_MASK)
            mask = ~(0llu);
        else
            mask = ~((~1llu) >> block);
        if (size < block) {
            block = size;
            offset2 = RTE_BITMAP_SLAB_BIT_SIZE - (size + offset2);
            mask = mask & ((~1llu) >> offset2);
        }
        slab2 = indxr->array2 + index2;
        *slab2 |= mask;
        rte_bitmap_set_slab(indxr, start, *slab2);
        start = start + block;
        size -= block;
    }
}
#endif   // __SDK_LIB_RTE_BITMAP_SLAB_OP_HPP__

