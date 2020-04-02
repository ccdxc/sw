//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Bitmap library enhancements needed for RFC library
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_RFC_RTE_BITMAP_UTILS_HPP__
#define __INCLUDE_RFC_RTE_BITMAP_UTILS_HPP__

#include <iostream>
#include <iomanip>
#include <sstream>
#include <rte_bitmap.h>
#include "nic/sdk/include/sdk/base.hpp"

#define RTE_BITMAP_START_SLAB_SCAN_POS 0xFFFF

// Bitmap slab scan
//
// \param[in] slab2 current slab for scan
// \param[in] cur_pos current position of bit set in the slab, 0 based
// \param[out] next_pos next position where the next bit is set, 0 based
//
// \return 0 if no bit set in the slab starting current postion, 1 otherwise
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
//
// \param[in] index2 Slab2 index
// \param[in] pos Position of the bit within the slab
//
//  \return Global position of the bit
static inline uint32_t
rte_bitmap_get_global_bit_pos (uint32_t index2, uint32_t pos)
{
    return ((index2 << RTE_BITMAP_SLAB_BIT_SIZE_LOG2) + pos);
}

// Set the corresponding slab1 bit given a slab2 index
//
// \param[in] bmp bitmap
// \param[in] index2 Slab2 index
//
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

static inline uint64_t
rte_bitmap_inline_xor (struct rte_bitmap *bmp)
{
    uint32_t    index2;
    uint64_t    value;

    if (bmp->array2_size > 0) {
        value = bmp->array2[0];
    }

    for (index2 = 1; index2 < bmp->array2_size; index2++) {
        value = value ^ bmp->array2[index2];
    }
    return value;
}

static inline bool
rte_bitmap_eq (struct rte_bitmap *ibmp1, struct rte_bitmap *ibmp2)
{
    uint32_t    index;

    for (index = 0; index < ibmp1->array2_size; index++) {
        if (ibmp1->array2[index] != ibmp2->array2[index]) {
            return false;
        }
    }
    return true;
}

static inline void
rte_bitmap2str (rte_bitmap *bmap, std::stringstream& a1ss,
                std::stringstream& a2ss)
{
    std::ios::fmtflags f(a1ss.flags());
    for (uint32_t index = 0; index < bmap->array1_size; index++) {
        a1ss << std::setfill('0') << std::setw(8) << std::hex
             << bmap->array1[index] << " ";
    }
    a1ss.flags(f);
    f = a2ss.flags();
    for (uint32_t index = 0; index < bmap->array2_size; index++) {
        a2ss << std::setfill('0') << std::setw(8) << std::hex
             << bmap->array2[index] << " ";
    }
    a2ss.flags(f);
}

struct rte_bitmap_hasher {
    size_t operator() (rte_bitmap *bmap) const {
        return rte_bitmap_inline_xor(bmap);
    }
};

struct rte_bitmap_equal_to {
    bool operator() (rte_bitmap *bmap1, rte_bitmap *bmap2) const {
        return rte_bitmap_eq(bmap1, bmap2);
    }
};

#endif    /** __RFC_BITMAP_UTILS_HPP__ */
