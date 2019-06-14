//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_TABLE_HPP__
#define __FTLITE_TABLE_HPP__

#include <stdint.h>
#include "include/sdk/base.hpp"
#include "ftlite_indexer.hpp"
#include "ftlite_bucket.hpp"
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include <nic/sdk/platform/capri/capri_state.hpp>

using namespace sdk;
using namespace sdk::platform;
using namespace sdk::platform::capri;

namespace ftlite {
namespace internal {

typedef enum ipaf_s {
    IPAF_IPV6 = 0,
    IPAF_IPV4 = 1,
    IPAF_MAX = 2,
} ipaf_t;

typedef enum table_type_s {
    TABLE_TYPE_MAIN = 0,
    TABLE_TYPE_HINT = 1,
    TABLE_TYPE_MAX = 2,
} table_type_t;

class table_t {
private:
    uint64_t memva; // Memory Virtual Address
    uint64_t mempa; // Memory Physical Address
    uint32_t otid; // Overflow Table ID
    uint32_t size; // Table Size
    bucket_t *buckets;
    indexer_t hints;

private:
    template<class T>
    void memcopy(T* dst, T* src) {
        auto a = src[0], b = src[1], c = src[2], d = src[3]; 
        auto e = src[4], f = src[5], g = src[6], h = src[7];
        dst[0] = a, dst[1] = b, dst[2] = c, dst[3] = d;
        dst[4] = e, dst[5] = f, dst[6] = g, dst[7] = h;
    }

public:
    uint32_t oflow_table_id() {
        return otid;
    }

    bucket_t& bucket(uint32_t bidx) {
        return buckets[bidx];
    }

    sdk_ret_t init(uint32_t table_id) {
        p4pd_table_properties_t tinfo;
        auto p4pdret = p4pd_table_properties_get(table_id, &tinfo);
        SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);
        size = tinfo.tabledepth;
        memva = tinfo.base_mem_va;
        mempa = tinfo.base_mem_pa;
        SDK_ASSERT(size && memva && mempa);
        SDK_TRACE_DEBUG("TID:%d Size:%d BaseMemVa:%#lx BaseMemPa:%#lx",
                        table_id, size, memva, mempa);

        buckets = (bucket_t*)SDK_CALLOC(SDK_MEM_ALLOC_FTLITE_TABLE_BUCKETS,
                                        sizeof(bucket_t) * size);
        if (buckets == NULL) {
            return SDK_RET_OOM;
        }

        otid = tinfo.oflow_table_id;
        if (otid != 0) {
            auto ret = hints.init(size);
            SDK_ASSERT_RETURN(ret == SDK_RET_OK, ret);
        }
        
        return SDK_RET_OK;
    }

    inline __attribute__((always_inline))
    sdk_ret_t write(ftlite_ipv4_entry_t *entry, uint32_t index) {
        auto tblbase = (ftlite_ipv4_entry_t *)memva;
        memcopy<uint32_t>((uint32_t*)(tblbase+index), (uint32_t*)entry);
        capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS,
                                    index*sizeof(ftlite_ipv4_entry_t),
                                    1, mempa);
        return SDK_RET_OK;
    }

    inline __attribute__((always_inline))
    sdk_ret_t write(ftlite_ipv6_entry_t *entry, uint32_t index) {
        auto tblbase = (ftlite_ipv6_entry_t *)memva;
        memcopy<uint64_t>((uint64_t*)(tblbase+index), (uint64_t*)entry);
        capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS,
                                    index*sizeof(ftlite_ipv6_entry_t),
                                    1, mempa);
        return SDK_RET_OK;
    }

    sdk_ret_t alloc(uint32_t &index) {
        SDK_ASSERT(otid);
        return hints.alloc(index);
    }
};

struct state_t {
private:
    volatile uint32_t lock_;
public:
    table_t main_tables[IPAF_MAX];
    table_t hint_tables[IPAF_MAX];

    void lock() {
        while(__sync_lock_test_and_set(&lock_, 1));
    }

    void unlock() {
        __sync_lock_release(&lock_);
    }
};

} // namespace internal
} // namespace ftlite

#endif // __FTLITE_TABLE_HPP__
