// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_STATE_PD_HPP__
#define __HAL_STATE_PD_HPP__

#include "sdk/indexer.hpp"
#include "sdk/slab.hpp"
#include "sdk/ht.hpp"
#include "sdk/directmap.hpp"
#include "sdk/hash.hpp"
#include "sdk/tcam.hpp"
#include "nic/hal/pd/utils/flow/flow.hpp"
#include "nic/gen/include/p4pd_table.h"
#include "nic/gen/include/common_txdma_actions_p4pd_table.h"
#include "nic/gen/include/common_rxdma_actions_p4pd_table.h"
#include "nic/gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "nic/gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "nic/gen/gft/include/p4pd.h"
#include "nic/utils/bm_allocator/bm_allocator.hpp"

using sdk::lib::slab;

namespace hal {
namespace pd {

#define HAL_MAX_HW_LIFS         1025        

extern class hal_state_pd    *g_hal_state_pd;

class hal_state_pd {
public:
    static hal_state_pd *factory(void);
    static void destroy(hal_state_pd *state);

    slab *get_slab(hal_slab_t slab_id);


    hal_ret_t init_tables(void);
    hal_ret_t p4plus_rxdma_init_tables(void);
    hal_ret_t p4plus_txdma_init_tables(void);


    // get APIs for LIF related state
    slab *lif_pd_slab(void) const { return lif_pd_slab_; }
    indexer *lif_hwid_idxr(void) const { return lif_hwid_idxr_; }

    // get APIs for Uplinkif  related state
    slab *uplinkif_pd_slab(void) const { return uplinkif_pd_slab_; }

    // get APIs for enicif  related state
    slab *enicif_pd_slab(void) const { return enicif_pd_slab_; }

    // get APIs for TXS scheduler related state
    BMAllocator *txs_scheduler_map_idxr(void) { return txs_scheduler_map_idxr_; }


    directmap *dm_table(uint32_t tid) const {
        if ((tid < P4TBL_ID_INDEX_MIN) || (tid > P4TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return dm_tables_[tid - P4TBL_ID_INDEX_MIN];
    }

    sdk_hash *hash_tcam_table(uint32_t tid) const {
        if ((tid < P4TBL_ID_HASH_OTCAM_MIN) ||
            (tid > P4TBL_ID_HASH_OTCAM_MAX)) {
            return NULL;
        }
        return hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN];
    }

    tcam *tcam_table(uint32_t tid) const {
        if ((tid < P4TBL_ID_TCAM_MIN) || (tid > P4TBL_ID_TCAM_MAX)) {
            return NULL;
        }
        return tcam_tables_[tid - P4TBL_ID_TCAM_MIN];
    }

    Flow *flow_table(void) const {
        return flow_table_;
    }

    directmap *p4plus_rxdma_dm_table(uint32_t tid) const {
        if ((tid < P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN) ||
            (tid > P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return p4plus_rxdma_dm_tables_[tid - P4_COMMON_RXDMA_ACTIONS_TBL_ID_INDEX_MIN];
    }

    directmap *p4plus_txdma_dm_table(uint32_t tid) const {
        if ((tid < P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN) ||
            (tid > P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return p4plus_txdma_dm_tables_[tid - P4_COMMON_TXDMA_ACTIONS_TBL_ID_INDEX_MIN];
    }

private:
    bool init(void);
    hal_state_pd();
    ~hal_state_pd();

private:
    directmap    **dm_tables_;
    sdk_hash     **hash_tcam_tables_;
    tcam         **tcam_tables_;
    Flow         *flow_table_;
    directmap    **p4plus_rxdma_dm_tables_;
    directmap    **p4plus_txdma_dm_tables_;

    // LIF related state
    struct {
        slab       *lif_pd_slab_;
        indexer    *lif_hwid_idxr_;         // Used even by Uplink IF/PCs
    } __PACK__;

    // Uplink IF related state
    struct {
        slab       *uplinkif_pd_slab_;
    } __PACK__;

    // Enic IF related state
    struct {
        slab       *enicif_pd_slab_;
    } __PACK__;

    // TXS scheduler related state
    struct {
        BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;
};

hal_ret_t delay_delete_to_slab(hal_slab_t slab_id, void *elem);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_STATE_PD_HPP__

