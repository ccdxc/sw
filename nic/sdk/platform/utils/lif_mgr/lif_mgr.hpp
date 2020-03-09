//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __LIF_MGR_HPP__
#define __LIF_MGR_HPP__
#include "include/sdk/lock.hpp"
#include "include/sdk/base.hpp"
#include "lib/indexer/indexer.hpp"
#include "asic/pd/pd.hpp"
namespace sdk {
namespace platform {
namespace utils {

typedef std::map<uint32_t, lif_qstate_t*> lif_map_t;
typedef std::map<uint32_t, uint32_t> alloc_size_map_t;
using sdk::lib::indexer;

class lif_mgr {
private:
    uint32_t num_lifs_;
    uint64_t hbm_base_;
    indexer *indexer_;
    indexer *hbm_indexer_;
    lif_map_t lifs_;
    alloc_size_map_t allocation_sizes_; // Track mem alloc. for free
    class mpartition *mp_;
    sdk_spinlock_t slock_;

private:
    sdk_ret_t init_(uint32_t num_lifs, mpartition *mp, const char *kHBMLabel);
    lif_qstate_t *get_lif_qstate_(uint32_t lif_id);

public:
    static lif_mgr *factory(uint32_t num_lifs, mpartition *mp,
                            const char *kHBMLabel);
    static void destroy(lif_mgr *lm);

    lif_mgr() {
        num_lifs_ = 0;
        hbm_base_ = 0;
        indexer_ = NULL;
        hbm_indexer_ = NULL;
        SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }
    ~lif_mgr() {
        SDK_SPINLOCK_DESTROY(&slock_);
    }

    sdk_ret_t alloc_id(uint32_t *idx, uint32_t count);
    sdk_ret_t reserve_id(uint32_t start, uint32_t count);
    sdk_ret_t free_id(uint32_t start, uint32_t count);
    sdk_ret_t init(lif_qstate_t *qstate);
    sdk_ret_t remove(uint32_t lif_id);
    sdk_ret_t clear_qstate(uint32_t lif_id);
    sdk_ret_t write_qstate(uint32_t lif_id, uint32_t type, uint32_t qid,
                           uint8_t *buf, uint32_t bufsize);
    sdk_ret_t read_qstate(uint32_t lif_id, uint32_t type, uint32_t qid,
                          uint8_t *buf, uint32_t bufsize);
    sdk_ret_t read_qstate_map(uint32_t lif_id, lif_qstate_t *qstate);
    sdk_ret_t enable(uint32_t lif_id);
    sdk_ret_t disable(uint32_t lif_id);
    lif_qstate_t *get_lif_qstate(uint32_t lif_id);
    int64_t get_lif_qstate_addr(uint32_t lif_id, uint32_t type, uint32_t qid);
    int64_t get_lif_qstate_base_addr(uint32_t lif_id, uint32_t type);

    mpartition *get_mpartition() {
        return mp_;
    }
    static sdk_ret_t lifs_reset(uint32_t start_lif, uint32_t end_lif);

};


// Start Traces
#define LIF_MGR_API_START_LOCK()                                        \
    SDK_SPINLOCK_LOCK(&slock_);

#define LIF_MGR_API_START_LIF_ID_TRACE_LOCK()                           \
    SDK_TRACE_DEBUG("lif_mgr id: %u", lif_id);                          \
    SDK_SPINLOCK_LOCK(&slock_);

#define LIF_MGR_API_START_LIF_ID_TRACE()                                \
    SDK_TRACE_DEBUG("lif_mgr id: %u", lif_id);


// End Traces
#define LIF_MGR_API_END_TRACE_UNLOCK()                                  \
    SDK_TRACE_DEBUG("lif_mgr status: %u ", ret);                        \
    SDK_SPINLOCK_UNLOCK(&slock_);

#define LIF_MGR_API_END_LIF_ID_TRACE_UNLOCK()                           \
    SDK_TRACE_DEBUG("lif_mgr id: %u status: %u", lif_id, ret);          \
    SDK_SPINLOCK_UNLOCK(&slock_);

#define LIF_MGR_API_END_LIF_ID_TRACE()                                  \
    SDK_TRACE_DEBUG("lif_mgr id: %u status: %u", lif_id, ret);

#define LIF_MGR_API_END_UNLOCK()                                        \
    SDK_SPINLOCK_UNLOCK(&slock_);

} // namespace utils
} // namespace platform
} // namespace sdk

using sdk::platform::utils::lif_mgr;

#endif  // __LIF_MGR_HPP__
