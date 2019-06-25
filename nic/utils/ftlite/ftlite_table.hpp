//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_TABLE_HPP__
#define __FTLITE_TABLE_HPP__

#include <stdint.h>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "ftlite_indexer.hpp"
#include "ftlite_ipv4_structs.hpp"
#include "ftlite_ipv6_structs.hpp"
#include "ftlite_bucket.hpp"
#include "ftlite.hpp"
#include "nic/sdk/platform/capri/capri_tbl_rw.hpp"
#include <nic/sdk/platform/capri/capri_state.hpp>

using namespace sdk;
using namespace sdk::platform;
using namespace sdk::platform::capri;

namespace ftlite {
namespace internal {

typedef enum table_type_s {
    TABLE_TYPE_MAIN = 0,
    TABLE_TYPE_HINT = 1,
    TABLE_TYPE_MAX = 2,
} table_type_t;

class table_t {
private:
    string name_;
    uint64_t memva_; // Memory Virtual Address
    uint64_t mempa_; // Memory Physical Address
    uint32_t otid_; // Overflow Table ID
    uint32_t size_; // Table Size
    bucket_t *buckets_;
    indexer_t hints_;

private:
    template<class T>
    void memcopy(T* dst, T* src) {
        auto a = src[0], b = src[1], c = src[2], d = src[3]; 
        auto e = src[4], f = src[5], g = src[6], h = src[7];
        dst[0] = a, dst[1] = b, dst[2] = c, dst[3] = d;
        dst[4] = e, dst[5] = f, dst[6] = g, dst[7] = h;
    }

    template<class ETYPE, class WTYPE>
    inline __attribute__((always_inline))
    sdk_ret_t write_(ETYPE *entry, uint32_t index) {
        static thread_local char buff[1024];
        SDK_ASSERT(memva_);
        auto tblbase = (ETYPE *)memva_;
        memcopy<WTYPE>((WTYPE*)(tblbase+index), (WTYPE*)entry);
        entry->tostr(buff, sizeof(buff));
        FTLITE_TRACE_DEBUG("write: %s : index:%d entry:[ %s ]",
                           name_.c_str(), index, buff);
#ifdef HW
        SDK_ASSERT(mempa_);
        capri_hbm_table_entry_cache_invalidate(P4_TBL_CACHE_INGRESS,
                                               index*sizeof(ETYPE),
                                               1, mempa_);
#endif
        return SDK_RET_OK;
    }

    template<class ETYPE, class WTYPE>
    inline __attribute__((always_inline))
    sdk_ret_t read_(ETYPE *entry, uint32_t index) {
        SDK_ASSERT(memva_);
        auto tblbase = (ETYPE *)memva_;
        memcopy<WTYPE>((WTYPE*)entry, (WTYPE*)(tblbase+index));
        return SDK_RET_OK;
    }

public:
    uint32_t otid() {
        return otid_;
    }

    bucket_t& bucket(uint32_t bidx) {
        return buckets_[bidx];
    }

    sdk_ret_t init(string name, uint32_t table_id, bool oflow_table = false) {
        name_ = name;
        p4pd_table_properties_t tinfo;
        auto p4pdret = p4pd_table_properties_get(table_id, &tinfo);
        SDK_ASSERT_RETURN(p4pdret == P4PD_SUCCESS, SDK_RET_ERR);
        size_ = tinfo.tabledepth;
        memva_ = tinfo.base_mem_va;
        mempa_ = tinfo.base_mem_pa;
        SDK_ASSERT(size_);
        SDK_TRACE_DEBUG("TID:%d Size:%d BaseMemVa:%#lx BaseMemPa:%#lx",
                        table_id, size_, memva_, mempa_);

        buckets_ = (bucket_t*)SDK_CALLOC(SDK_MEM_ALLOC_FTLITE_TABLE_BUCKETS,
                                         sizeof(bucket_t) * size_);
        if (buckets_ == NULL) {
            return SDK_RET_OOM;
        }

        otid_ = tinfo.oflow_table_id;
        if (oflow_table) {
            auto ret = hints_.init(size_);
            SDK_ASSERT_RETURN(ret == SDK_RET_OK, ret);
        }
        
        return SDK_RET_OK;
    }

    inline __attribute__((always_inline))
    sdk_ret_t write(uint32_t ipv6, void *entry, uint32_t index) {
        if (ipv6) {
            return write_<ipv6_entry_t, uint64_t>((ipv6_entry_t*)entry, index);
        }
        return write_<ipv4_entry_t, uint32_t>((ipv4_entry_t*)entry, index);
    }

    inline __attribute__((always_inline))
    sdk_ret_t read(uint32_t ipv6, void *entry, uint32_t index) {
        if (ipv6) {
            return read_<ipv6_entry_t, uint64_t>((ipv6_entry_t*)entry, index);
        }
        return read_<ipv4_entry_t, uint32_t>((ipv4_entry_t*)entry, index);
    }

    sdk_ret_t alloc(uint32_t &index) {
        return hints_.alloc(index);
    }
};

struct state_t {
private:
    volatile uint32_t lock_;
public:
    table_t mtables[2];
    table_t htables[2];

    void lock() {
        while(__sync_lock_test_and_set(&lock_, 1));
    }

    void unlock() {
        __sync_lock_release(&lock_);
    }

    sdk_ret_t init(ftlite::init_params_t *ips) {
        sdk_ret_t ret = SDK_RET_OK;
        lock();

        if (ips->v4tid) {
            ret = mtables[0].init("Ipv4Main", ips->v4tid);
            SDK_ASSERT(ret == SDK_RET_OK);

            ret = htables[0].init("Ipv4Hint", mtables[0].otid(), true);
            SDK_ASSERT(ret == SDK_RET_OK);
        }

        if (ips->v6tid) {
            ret = mtables[1].init("Ipv6Main", ips->v6tid);
            SDK_ASSERT(ret == SDK_RET_OK);

            ret = htables[1].init("Ipv6Hint", mtables[1].otid(), true);
            SDK_ASSERT(ret == SDK_RET_OK);
        }
        unlock();
        return SDK_RET_OK;
    }
};

} // namespace internal
} // namespace ftlite

#endif // __FTLITE_TABLE_HPP__
