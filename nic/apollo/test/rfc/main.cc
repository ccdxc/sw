//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
// This file contains all rfc test cases
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/apollo/test/base/base.hpp"
#include "nic/apollo/api/impl/rfc/rte_bitmap_utils.hpp"

using std::cerr;
using std::cout;
using std::endl;

//----------------------------------------------------------------------------
// rfc test class
//----------------------------------------------------------------------------

class rfc_test : public ::pds_test_base {
protected:
    rfc_test() {}
    virtual ~rfc_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

//----------------------------------------------------------------------------
// rfc test cases implementation
//----------------------------------------------------------------------------

static inline void
rfc_test_rte_bitmap_get_validate (struct rte_bitmap *bmp,
                                  uint8_t num_bits, uint32_t *bits_pos)
{
    for (int idx = 0; idx < num_bits; idx++)
        ASSERT_TRUE(rte_bitmap_get(bmp, bits_pos[idx] != 0));
}

static inline void
rfc_test_rte_bitmap_set_init (struct rte_bitmap *bmp,
                              uint8_t num_bits, uint32_t *bits_pos)
{
    for (int idx = 0; idx < num_bits; idx++)
        rte_bitmap_set(bmp, bits_pos[idx]);
}

static inline void
rfc_test_rte_bitmap_or (struct rte_bitmap *ib1,
                        struct rte_bitmap *ib2,
                        struct rte_bitmap *ob)
{
    uint32_t bit_pos[16] = {
        0, 63, 127, 208, 32767, 32768, 50000, 50001,
        1, 64, 128, 421, 34312, 32333, 43213, 12232,
    };

    rfc_test_rte_bitmap_set_init(ib1, 8, bit_pos);
    rfc_test_rte_bitmap_set_init(ib2, 8, bit_pos+8);
    rte_bitmap_or(ib1, ib2, ob);
    rfc_test_rte_bitmap_get_validate(ob, 16, bit_pos);

    rte_bitmap_reset(ib1);
    rte_bitmap_reset(ib2);
    rte_bitmap_reset(ob);
}

static inline void
rfc_test_rte_bitmap_and (struct rte_bitmap *ib1,
                         struct rte_bitmap *ib2,
                         struct rte_bitmap *ob)
{
    uint32_t input_bit_pos[16] = {
        0, 1, 63, 64, 32767, 32768, 50000, 88323,
        0, 1, 0,  64, 32767, 32768, 50001, 43332,
    };
    uint32_t exp_bit_pos[5] = {
        0, 1, 064, 32767, 32768,
    };

    rfc_test_rte_bitmap_set_init(ib1, 8, input_bit_pos);
    rfc_test_rte_bitmap_set_init(ib2, 8, input_bit_pos+8);
    rte_bitmap_and(ib1, ib2, ob);
    rfc_test_rte_bitmap_get_validate(ob, 5, exp_bit_pos);

    rte_bitmap_reset(ib1);
    rte_bitmap_reset(ib2);
    rte_bitmap_reset(ob);
}

/// \brief Test rte_bitmap AND & OR operations used by RFC
TEST_F(rfc_test, test_and_or_oper) {
    uint8_t *i1, *i2, *o;
    struct rte_bitmap *ib1, *ib2, *ob;
    uint32_t bmem_size, bbits;

    bbits = RTE_BITMAP_CL_BIT_SIZE*128;
    bmem_size = rte_bitmap_get_memory_footprint(bbits);

    posix_memalign((void **)&i1, RTE_CACHE_LINE_SIZE, bmem_size);
    posix_memalign((void **)&i2, RTE_CACHE_LINE_SIZE, bmem_size);
    posix_memalign((void **)&o, RTE_CACHE_LINE_SIZE, bmem_size);

    ib1 = rte_bitmap_init(bbits, i1, bmem_size);
    ib2 = rte_bitmap_init(bbits, i2, bmem_size);
    ob = rte_bitmap_init(bbits, o, bmem_size);

    // test AND operation with one cache line size equivalent bits
    rfc_test_rte_bitmap_and(ib1, ib2, ob);
    rfc_test_rte_bitmap_or(ib1, ib2, ob);
}

static inline void
rfc_test_rte_bitmap_scan (struct rte_bitmap *bmp, uint8_t num_bit_pos,
                          uint32_t *bit_pos)
{
    uint64_t slab = 0;
    uint32_t pos, new_pos, start_pos, index;
    uint32_t *out_bit_pos = (uint32_t *) calloc(num_bit_pos, sizeof(uint32_t));

    rte_bitmap_reset(bmp);
    pos = new_pos = start_pos = index = 0;

    rfc_test_rte_bitmap_set_init(bmp, num_bit_pos, bit_pos);

    // remember starting position to break automatic wrap around
    ASSERT_TRUE(rte_bitmap_scan(bmp, &start_pos, &slab) != 0);

    // test scan operation
    do  {
        pos = RTE_BITMAP_START_SLAB_SCAN_POS;

        while (rte_bitmap_slab_scan(slab, pos, &new_pos) != 0) {
            out_bit_pos[index] = rte_bitmap_get_global_bit_pos(
                bmp->index2 - 1,  // scan went pass through the current slab
                new_pos);
            pos = new_pos;
            index++;
        }
        rte_bitmap_scan(bmp, &pos, &slab);
    } while (pos != start_pos);

    ASSERT_TRUE(memcmp(out_bit_pos, bit_pos,
                       sizeof(bit_pos[0])*num_bit_pos) == 0);
    free(out_bit_pos);
}

/// \brief Test rte_bitmap scan operations used by RFC
TEST_F(rfc_test, test_scan_oper) {
    uint8_t *mem;
    struct rte_bitmap *bmp;
    uint32_t bmem_size, bbits;
    uint32_t bit_pos[512];

    bbits = RTE_BITMAP_CL_BIT_SIZE*128;
    bmem_size = rte_bitmap_get_memory_footprint(bbits);

    posix_memalign((void **)&mem, RTE_CACHE_LINE_SIZE, bmem_size);
    bmp = rte_bitmap_init(bbits, mem, bmem_size);

    // test with only one bit (first) in first slab
    bit_pos[0] = 0;
    rfc_test_rte_bitmap_scan(bmp, 1, bit_pos);

    // test with only one bit (some where in the middle) in first slab
    bit_pos[0] = 34;
    rfc_test_rte_bitmap_scan(bmp, 1, bit_pos);

    // test with only one bit (last bit) in first slab
    bit_pos[0] = 63;
    rfc_test_rte_bitmap_scan(bmp, 1, bit_pos);

    // test with only one bit (first bit) in second slab
    bit_pos[0] = 64;
    rfc_test_rte_bitmap_scan(bmp, 1, bit_pos);

    // test with only one bit (some where in the middle) in second slab
    bit_pos[0] = 84;
    rfc_test_rte_bitmap_scan(bmp, 1, bit_pos);

    // test with only one bit (last bit) in first slab
    bit_pos[0] = 127;
    rfc_test_rte_bitmap_scan(bmp, 1, bit_pos);

    // test with all combo of bits (above) together in the first two slabs
    bit_pos[0] = 0;   bit_pos[1] = 34; bit_pos[2] = 63;
    bit_pos[3] = 64;  bit_pos[4] = 84; bit_pos[5] = 127;
    rfc_test_rte_bitmap_scan(bmp, 6, bit_pos);

    // test with all bits set in the first two slabs
    for (int idx = 0; idx < 128; idx++)
        bit_pos[idx] = idx;
    rfc_test_rte_bitmap_scan(bmp, 128, bit_pos);

    // leave one slab empty between two completely filled slabs
    for (int idx = 0; idx < 64; idx++)
        bit_pos[idx] = idx;
    for (int idx = 128; idx < 192; idx++)
        bit_pos[idx] = idx;
    rfc_test_rte_bitmap_scan(bmp, 128, bit_pos);

    // scatter randomly
    bit_pos[0] = 1032;   bit_pos[1] = 3342; bit_pos[2] = 4332;
    bit_pos[3] = 10000;  bit_pos[4] = 16433; bit_pos[5] = 20000;
    rfc_test_rte_bitmap_scan(bmp, 6, bit_pos);
}

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
