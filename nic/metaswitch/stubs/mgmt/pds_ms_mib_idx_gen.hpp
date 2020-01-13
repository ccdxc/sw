//---------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// Mgmt stub MS MIB index generator utilities
//--------------------------------------------------------------

#ifndef __PDS_MS_MIB_IDX_GEN_HPP__
#define __PDS_MS_MIB_IDX_GEN_HPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"

namespace pds_ms {

enum mib_idx_gen_tbl_t {
    MIB_IDX_GEN_TBL_START = 0,
    MIB_IDX_GEN_TBL_VRF,
    MIB_IDX_GEN_TBL_BD,
    MIB_IDX_GEN_TBL_MAX
}; 

using mib_idx_t = uint32_t;

// Each instance of this class owns a new Metaswitch MIB table internal index.
// The index is allocated using rte_indexer in pds_ms::mgmt_state_t
class mib_idx_gen_guard_t {
public:
    mib_idx_gen_guard_t(mib_idx_gen_tbl_t tbl_type);
    ~mib_idx_gen_guard_t(void); // Frees the index back to rte_indexer
    mib_idx_t idx(void) {return mib_idx_;}

private:  
    mib_idx_t mib_idx_; 
    mib_idx_gen_tbl_t tbl_type_;
};

class mib_idx_gen_indexer_t {
public:
    mib_idx_gen_indexer_t(void);
    ~mib_idx_gen_indexer_t(void);
    sdk_ret_t alloc(mib_idx_gen_tbl_t tbl_type,
                    mib_idx_t* mib_idx) {
        return mib_indexers_[tbl_type]->alloc(mib_idx);
    }

    sdk_ret_t free(mib_idx_gen_tbl_t tbl_type,
                   mib_idx_t mib_idx) {
        return mib_indexers_[tbl_type]->free(mib_idx);
    }
private:
    sdk::lib::rte_indexer *mib_indexers_[MIB_IDX_GEN_TBL_MAX]
        = {nullptr};
};

} // End namespace

#endif
