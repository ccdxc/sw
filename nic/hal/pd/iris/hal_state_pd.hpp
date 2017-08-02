#ifndef __HAL_STATE_PD_HPP__
#define __HAL_STATE_PD_HPP__

#include <indexer.hpp>
#include <slab.hpp>
#include <ht.hpp>
#include <directmap.hpp>
#include <hash.hpp>
#include <tcam.hpp>
#include <flow.hpp>
#include <met.hpp>
#include <p4pd.h>

using hal::utils::indexer;
using hal::utils::slab;
using hal::utils::ht;
using hal::pd::utils::DirectMap;
using hal::pd::utils::Hash;
using hal::pd::utils::Tcam;
using hal::pd::utils::Flow;
using hal::pd::utils::Met;

namespace hal {
namespace pd {

// LIF HW ID Space for SB LIFs, Uplink Ifs/PCs
#define HAL_MAX_HW_LIFS     1025        
#define HAL_MAX_UPLINK_IFS  16

#define HAL_RW_TABLE_SIZE 4096

//-----------------------------------------------------------------------------
// class hal_state_pd
//  
//  - slab memory instances for all pd objects.
//  - indexers needed for pd
//-----------------------------------------------------------------------------
class hal_state_pd {
public:
    static hal_state_pd *factory(void);
    ~hal_state_pd();

    // get APIs for tenant related state
    slab *tenant_slab(void) const { return tenant_slab_; }
    indexer *tenant_hwid_idxr(void) const { return tenant_hwid_idxr_; }
    ht *tenant_hwid_ht(void) const { return tenant_hwid_ht_; }

    // get APIs for security related state
    indexer *nwsec_profile_hwid_idxr(void) const { return nwsec_profile_hwid_idxr_; }

    // get APIs for L2 segment related state
    slab *l2seg_slab(void) const { return l2seg_slab_; }
    indexer *l2seg_hwid_idxr(void) const { return l2seg_hwid_idxr_; }
    ht *l2seg_hwid_ht(void) const { return l2seg_hwid_ht_; }

    // get APIs for LIF related state
    slab *lif_pd_slab(void) const { return lif_pd_slab_; }
    indexer *lif_hwid_idxr(void) const { return lif_hwid_idxr_; }

    // get APIs for Uplinkif  related state
    slab *uplinkif_pd_slab(void) const { return uplinkif_pd_slab_; }

    // get APIs for enicif  related state
    slab *enicif_pd_slab(void) const { return enicif_pd_slab_; }

    // get APIs for LIF related state
    slab *ep_pd_slab(void) const { return ep_pd_slab_; }
    slab *ep_pd_ip_entry_slab(void) const { return ep_pd_ip_entry_slab_; }
    indexer *rw_table_idxr(void) const { return rw_table_idxr_; }

    // get APIs for nwsec related state
    slab *nwsec_pd_slab(void) const { return nwsec_pd_slab_; }

    // get APIs for session related state
    slab *session_slab(void) const { return session_slab_; }

    hal_ret_t init_tables(void);
    DirectMap *dm_table(p4pd_table_id tid) const {
        if ((tid < P4TBL_ID_INDEX_MIN) || (tid > P4TBL_ID_INDEX_MAX)) {
            return NULL;
        }
        return dm_tables_[tid - P4TBL_ID_INDEX_MIN];
    }

    Hash *hash_tcam_table(p4pd_table_id tid) const {
        if ((tid < P4TBL_ID_HASH_OTCAM_MIN) ||
            (tid > P4TBL_ID_HASH_OTCAM_MAX)) {
            return NULL;
        }
        return hash_tcam_tables_[tid - P4TBL_ID_HASH_OTCAM_MIN];
    }

    Tcam *tcam_table(p4pd_table_id tid) const {
        if ((tid < P4TBL_ID_TCAM_MIN) || (tid > P4TBL_ID_TCAM_MAX)) {
            return NULL;
        }
        return tcam_tables_[tid - P4TBL_ID_TCAM_MIN];
    }

    Flow *flow_table(void) const {
        return flow_table_;
    }

    Met *met_table(void) const {
        return met_table_;
    }

private:
    bool init(void);
    hal_state_pd();

    // tenant related state
    struct {
        slab       *tenant_slab_;
        indexer    *tenant_hwid_idxr_;
        ht         *tenant_hwid_ht_;
    } __PACK__;

    struct {
        indexer    *nwsec_profile_hwid_idxr_;
    } __PACK__;

    // l2seg related state
    struct {
        slab       *l2seg_slab_;
        indexer    *l2seg_hwid_idxr_;
        ht         *l2seg_hwid_ht_;
    } __PACK__;

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

    // EP related state
    struct {
        slab       *ep_pd_slab_;
        slab       *ep_pd_ip_entry_slab_;
        indexer    *rw_table_idxr_;
    } __PACK__;

    // nwsec related state
    struct {
        slab       *nwsec_pd_slab_;
    } __PACK__;

    // session related state
    struct {
        slab       *session_slab_;
    } __PACK__;

    DirectMap    **dm_tables_;
    Hash         **hash_tcam_tables_;
    Tcam         **tcam_tables_;
    Flow         *flow_table_;
    Met          *met_table_;
};

extern class hal_state_pd    *g_hal_state_pd;

} // namespace pd
} // namespace hal

#endif    // __HAL_STATE_PD_HPP__

