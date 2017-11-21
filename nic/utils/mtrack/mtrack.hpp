// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_MTRACK_HPP__
#define __HAL_MTRACK_HPP__

#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace utils {

// info stored by memory tracker per allocation id
typedef struct mtrack_info_s {
    uint32_t    num_allocs;
    uint32_t    num_frees;
} __PACK__ mtrack_info_t;
typedef std::map<uint32_t, mtrack_info_t *> mtrack_map_t;
typedef std::map<uint32_t, mtrack_info_t *>::iterator mtrack_map_it_t;

class mem_mgr {
public:
    mem_mgr() : mtrack_map_() {
        HAL_SPINLOCK_INIT(&mtrack_map_slock_, PTHREAD_PROCESS_PRIVATE);
        enabled_ = true;
    }

    ~mem_mgr() {
        mtrack_map_it_t    it;

        for (it = mtrack_map_.begin(); it != mtrack_map_.end(); ) {
            free(it->second);
            mtrack_map_.erase(it++);
        }
    }

    void enable(void) { enabled_ = true; }
    bool enabled(void) const { return enabled_; }
    void disable(void) { enabled_ = false; }

    void *mtrack_alloc(uint32_t alloc_id, bool zero,
                       uint32_t size, const char *func, uint32_t line) {
        mtrack_info_t       *mtrack_info;
        mtrack_map_it_t     it;
        void                *mem;
        bool                free_mem = false;

        mem = zero ? calloc(1, size) : malloc(size);
        if (!mem) {
            return NULL;
        }

        // if mem tracking is not enabled, nothing more to do
        if (!enabled_) {
            return mem;
        }

        HAL_SPINLOCK_LOCK(&mtrack_map_slock_);
        it = mtrack_map_.find(alloc_id);
        if (it != mtrack_map_.end()) {
            it->second->num_allocs++;
        } else {
            mtrack_info = (mtrack_info_t *)calloc(1, sizeof(mtrack_info_t));
            if (mtrack_info == NULL) {
                free_mem = true;
            } else {
                mtrack_info->num_allocs = 1;
                mtrack_map_[alloc_id] = mtrack_info;
            }
        }
        HAL_SPINLOCK_UNLOCK(&mtrack_map_slock_);

        if (free_mem) {
            free(mem);
            return NULL;
        }
        return mem;
    }

    void mtrack_free(uint32_t alloc_id, void *ptr,
                     const char *func, uint32_t line) {
        bool                free_mem = false;
        mtrack_info_t       *mtrack_info;
        mtrack_map_it_t     it;

        if (!ptr) {
            return;
        }
        ::free(ptr);

        // if mem tracking is not enabled, nothing more to do
        if (!enabled_) {
            return;
        }

        HAL_SPINLOCK_LOCK(&mtrack_map_slock_);
        it = mtrack_map_.find(alloc_id);
        if (it != mtrack_map_.end()) {
            it->second->num_frees++;
            if (it->second->num_frees == it->second->num_allocs) {
                // we can free this state now
                free_mem = true;
                mtrack_info = it->second;
                mtrack_map_.erase(it);
            }
        }
        HAL_SPINLOCK_UNLOCK(&mtrack_map_slock_);

        if (free_mem) {
            free(mtrack_info);
        }

        return;
    }

    typedef bool (*walk_cb_t)(void *ctxt, uint32_t alloc_id, mtrack_info_t *minfo);
    void walk(void *ctxt, walk_cb_t walk_cb) {
        mtrack_map_it_t     it;

        HAL_SPINLOCK_LOCK(&mtrack_map_slock_);
        for (it = mtrack_map_.begin(); it != mtrack_map_.end(); ) {
            if (!walk_cb(ctxt, it->first, it->second)) {
                // if callback returns true, stop iterating
                break;
            }
        }
        HAL_SPINLOCK_UNLOCK(&mtrack_map_slock_);
    }

private:
    mtrack_map_t      mtrack_map_;
    hal_spinlock_t    mtrack_map_slock_;
    bool              enabled_;
};

extern mem_mgr g_hal_mem_mgr;

}    // namespace utils
}    // namespace hal

#endif    // __HAL_MTRACK_HPP__

