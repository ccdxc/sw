//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of (service) tags
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/tag.hpp"
#include "nic/apollo/api/impl/artemis/tag_impl.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/lpm/lpm.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TAG_IMPL - tag datapath implementation
/// \ingroup PDS_TAG
/// \@{

tag_impl *
tag_impl::factory(pds_tag_spec_t *spec) {
    tag_impl    *impl;

    // TODO: move to slab later
    impl = (tag_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_TAG_IMPL,
                                          sizeof(tag_impl));
    new (impl) tag_impl();
    return impl;
}

void
tag_impl::destroy(tag_impl *impl) {
    impl->~tag_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_TAG_IMPL, impl);
}

// NOTE: reserve_resources() logic is same for both API_OP_CREATE and
//       API_OP_UPDATE as update doesn't reuse any of the existing resources
//       for this object
sdk_ret_t
tag_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t lpm_block_id;
    pds_tag_spec_t *spec = &obj_ctxt->api_params->tag_spec;

    // allocate free lpm slab for this tag table
    if (tag_impl_db()->alloc(spec->af, &lpm_block_id) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to allocate LPM block for tag %u",
                      spec->key.id);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    lpm_root_addr_ =
        tag_impl_db()->region_addr(spec->af) +
            tag_impl_db()->table_size(spec->af) * lpm_block_id;
    return SDK_RET_OK;
}

sdk_ret_t
tag_impl::release_resources(api_base *api_obj) {
    uint32_t lpm_block_id;
    tag_entry *tag = (tag_entry *)api_obj;

    // just free the lpm block we are using
    if (lpm_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        lpm_block_id =
            (lpm_root_addr_ - tag_impl_db()->region_addr(tag->af()))/
                tag_impl_db()->table_size(tag->af());
        tag_impl_db()->free(tag->af(), lpm_block_id);
    }
    return SDK_RET_OK;
}

sdk_ret_t
tag_impl::nuke_resources(api_base *api_obj) {
    // just free the lpm block we are using
    return this->release_resources(api_obj);
}

sdk_ret_t
tag_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
tag_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                        api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
