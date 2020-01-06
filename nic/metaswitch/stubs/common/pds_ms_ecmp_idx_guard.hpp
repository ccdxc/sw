//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS HAL Overlay ECMP Table Index guard class
//---------------------------------------------------------------

#ifndef __PDS_MS_ECMP_IDX_GUARD_HPP__
#define __PDS_MS_ECMP_IDX_GUARD_HPP__

#include "nic/metaswitch/stubs/common/pds_ms_defs.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_slab_object.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/sdk/lib/slab/slab.hpp"

namespace pds_ms {

// Each instance of this class owns a new PDS HAL ECMP table entry index.
// The index is allocated using rte_indexer in pds_ms::state_t
class ecmp_idx_guard_t : public slab_obj_t<ecmp_idx_guard_t> {
public:
    ecmp_idx_guard_t(void); // Allocates a new index
    ~ecmp_idx_guard_t(void); // Frees the index back to rte_indexer
    pds_nexthop_group_id_t idx(void) {return ecmp_idx_;}

private:  
    pds_nexthop_group_id_t ecmp_idx_; 
    uint32_t   filler; // SLAB requires at least 8 bytes
};

void ecmp_idx_guard_slab_init (slab_uptr_t slabs[], sdk::lib::slab_id_t slab_id);

}

#endif


