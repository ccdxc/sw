//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// service mapping datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/service_impl.hpp"
#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/p4/include/defines.h"
//#include "gen/p4gen/artemis/include/p4pd.h"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_SERVICE_IMPL_STATE - service mapping database functionality
/// \ingroup PDS_SERVICE
/// \@{

svc_mapping_impl_state::svc_mapping_impl_state(pds_state *state) {
    // create a slab for mapping impl entries
    svc_mapping_impl_slab_ = slab::factory("svc-mapping-impl",
                                           PDS_SLAB_ID_SVC_MAPPING_IMPL,
                                           sizeof(svc_mapping_impl), 128,
                                           true, true);
    SDK_ASSERT(svc_mapping_impl_slab_!= NULL);
}

svc_mapping_impl_state::~svc_mapping_impl_state() {
    slab::destroy(svc_mapping_impl_slab_);
}

svc_mapping_impl *
svc_mapping_impl_state::alloc(void) {
    return ((svc_mapping_impl *)svc_mapping_impl_slab_->alloc());
}

void
svc_mapping_impl_state::free(svc_mapping_impl *impl) {
    svc_mapping_impl_slab_->free(impl);
}

sdk_ret_t
svc_mapping_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl_state::table_stats(debug::table_stats_get_cb_t cb,
                                    void *ctxt) {
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
