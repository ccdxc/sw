/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_bitmap_utils.hpp
 *
 * @brief   bitmap library enhancements needed for RFC library
 */

#if !defined (__RFC_BITMAP_UTILS_HPP__)
#define __RFC_BITMAP_UTILS_HPP__

#include <rte_bitmap.h>

static inline void
rte_bitmap_and (struct rte_bitmap *ibmp1, struct rte_bitmap *ibmp2,
                struct rte_bitmap *obmp)
{
    uint32_t index;

    for (index = 0; index < ibmp1->array2_size; index++) {
        obmp->array2[index] = ibmp1->array2[index] & ibmp2->array2[index];
    }
}

static inline void
rte_bitmap_or (struct rte_bitmap *ibmp1, struct rte_bitmap *ibmp2,
               struct rte_bitmap *obmp)
{
    uint32_t index;

    for (index = 0; index < ibmp1->array2_size; index++) {
        obmp->array2[index] = ibmp1->array2[index] | ibmp2->array2[index];
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
