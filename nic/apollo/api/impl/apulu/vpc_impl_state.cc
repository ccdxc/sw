//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl_state.hpp"
#include "gen/p4gen/apulu/include/p4pd.h"
#include "nic/apollo/core/trace.hpp"

using sdk::table::sdk_table_factory_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE - vpc database functionality
/// \ingroup PDS_VPC
/// @{

vpc_impl_state::vpc_impl_state(pds_state *state) {
    p4pd_table_properties_t tinfo;
    sdk_table_factory_params_t tparams;

    p4pd_global_table_properties_get(P4TBL_ID_VPC, &tinfo);
    // create indexer for vpc hw id allocation and reserve 0th entry
    vpc_idxr_ = rte_indexer::factory(tinfo.tabledepth, false, true);
    SDK_ASSERT(vpc_idxr_ != NULL);

    // instantiate P4 tables for bookkeeping
    bzero(&tparams, sizeof(tparams));
    tparams.entry_trace_en = false;
    tparams.table_id = P4TBL_ID_VNI;
    vni_tbl_ = slhash::factory(&tparams);
    SDK_ASSERT(vni_tbl_ != NULL);

    // allocate tag/class state
    tag_state_ =
        (vpc_impl_tag_state_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_VPC_IMPL_TAG_STATE,
                                           sizeof(vpc_impl_tag_state_t));
    SDK_ASSERT(tag_state_ != NULL);
    new (tag_state_) vpc_impl_tag_state_t();
    // create classid indexer for local mappings
    tag_state_->class_id_idxr_ =
        rte_indexer::factory(PDS_MAX_CLASS_ID_PER_VPC, false, false);
    SDK_ASSERT(tag_state_->class_id_idxr_ != NULL);
    // set the reserved classid aside
    tag_state_->class_id_idxr_->alloc(PDS_IMPL_RSVD_MAPPING_CLASS_ID);
#if 0
    // allocate tag/class state
    tag_state_ =
        (vpc_impl_tag_state_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_VPC_IMPL_TAG_STATE,
                                           sizeof(vpc_impl_tag_state_t));
    SDK_ASSERT(tag_state_ != NULL);
    new (tag_state_) vpc_impl_tag_state_t();
    // create classid indexer for local mappings
    tag_state_->local_mapping_class_id_idxr_ =
        rte_indexer::factory(PDS_MAX_CLASS_ID_PER_VPC, false, false);
    SDK_ASSERT(tag_state_->local_mapping_class_id_idxr_ != NULL);
    // set the reserved classid aside
    tag_state_->local_mapping_class_id_idxr_->alloc(PDS_IMPL_RSVD_MAPPING_CLASS_ID);
    // create classid indexer for remote mappings
    tag_state_->remote_mapping_class_id_idxr_ =
        rte_indexer::factory(PDS_MAX_CLASS_ID_PER_VPC, false, false);
    SDK_ASSERT(tag_state_->remote_mapping_class_id_idxr_ != NULL);
    // set the reserved classid aside
    tag_state_->remote_mapping_class_id_idxr_->alloc(PDS_IMPL_RSVD_MAPPING_CLASS_ID);
#endif

    // create ht for vpc id to key mapping
    impl_ht_ =
        ht::factory(PDS_MAX_VPC >> 2, vpc_impl::key_get, sizeof(uint16_t));
    SDK_ASSERT(impl_ht_ != NULL);
}

vpc_impl_state::~vpc_impl_state() {
    rte_indexer::destroy(vpc_idxr_);
    slhash::destroy(vni_tbl_);
    rte_indexer::destroy(tag_state_->class_id_idxr_);
#if 0
    rte_indexer::destroy(tag_state_->local_mapping_class_id_idxr_);
    rte_indexer::destroy(tag_state_->remote_mapping_class_id_idxr_);
#endif
    SDK_FREE(PDS_MEM_ALLOC_ID_VPC_IMPL_TAG_STATE, tag_state_);
    ht::destroy(impl_ht_);
}

vpc_impl *
vpc_impl_state::alloc(void) {
    return (vpc_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_VPC_IMPL, sizeof(vpc_impl));
}

void
vpc_impl_state::free(vpc_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_VPC_IMPL, impl);
}

sdk_ret_t
vpc_impl_state::table_transaction_begin(void) {
    vni_tbl_->txn_start();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_transaction_end(void) {
    vni_tbl_->txn_end();
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    pds_table_stats_t stats;
    p4pd_table_properties_t tinfo;

    memset(&stats, 0, sizeof(pds_table_stats_t));
    p4pd_global_table_properties_get(P4TBL_ID_VNI_OTCAM, &tinfo);
    stats.table_name = tinfo.tablename;
    vni_tbl_->stats_get(&stats.api_stats, &stats.table_stats);
    cb(&stats, ctxt);

    return SDK_RET_OK;
}

vpc_impl *
vpc_impl_state::find(uint16_t hw_id) {
    return (vpc_impl *)impl_ht_->lookup(&hw_id);
}

sdk_ret_t
vpc_impl_state::insert(uint16_t hw_id, vpc_impl *impl) {
    impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::update(uint16_t hw_id, vpc_impl *impl) {
    if (impl_ht_->remove(&hw_id)) {
        return impl_ht_->insert_with_key(&hw_id, impl, impl->ht_ctxt());
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
vpc_impl_state::remove(uint16_t hw_id) {
    vpc_impl *vpc = NULL;

    vpc = (vpc_impl *)impl_ht_->remove(&hw_id);
    if (!vpc) {
        PDS_TRACE_ERR("Failed to find vpc impl for hw id %u", hw_id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::alloc_class_id(uint32_t tag, bool local, uint32_t *class_id) {
    sdk_ret_t ret;
#if 0
    rte_indexer *class_idxr;
    tag2class_map_t *tag2class_map;
    class2tag_map_t *class2tag_map;

    if (local) {
        tag2class_map = &tag_state_->local_tag2class_map_;
        class2tag_map = &tag_state_->local_class2tag_map_;
        class_idxr = tag_state_->local_mapping_class_id_idxr_;
    } else {
        tag2class_map = &tag_state_->remote_tag2class_map_;
        class2tag_map = &tag_state_->remote_class2tag_map_;
        class_idxr = tag_state_->remote_mapping_class_id_idxr_;
    }

    if (tag2class_map->find(tag) != tag2class_map->end()) {
        // tag to class id mapping exists
        (*tag2class_map)[tag].refcount++;
        *class_id = (*tag2class_map)[tag].class_id;
    } else {
        // try to allocate new class id
        ret = class_idxr->alloc(class_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to allocate class id for tag %u, err %u",
                          tag, ret);
            return ret;
        }
        // add tag -> class id mapping
        (*tag2class_map)[tag].class_id = *class_id;
        (*tag2class_map)[tag].refcount = 1;
        // add class id -> tag mapping
        (*class2tag_map)[*class_id] = tag;
    }
#endif
    if (tag_state_->tag2class_map_.find(tag) !=
            tag_state_->tag2class_map_.end()) {
        // tag to class id mapping exists
        tag_state_->tag2class_map_[tag].refcount++;
        *class_id = tag_state_->tag2class_map_[tag].class_id;
    } else {
        // try to allocate new class id
        ret = tag_state_->class_id_idxr_->alloc(class_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to allocate class id for tag %u, err %u",
                          tag, ret);
            return ret;
        }
        // add tag -> class id mapping
        tag_state_->tag2class_map_[tag].class_id = *class_id;
        tag_state_->tag2class_map_[tag].refcount = 1;
        // add class id -> tag mapping
        tag_state_->class2tag_map_[*class_id] = tag;
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::release_class_id(uint32_t class_id, bool local) {
    uint32_t tag;
    sdk_ret_t ret;
#if 0
    rte_indexer *class_idxr;
    tag2class_map_t *tag2class_map;
    class2tag_map_t *class2tag_map;

    if (local) {
        tag2class_map = &tag_state_->local_tag2class_map_;
        class2tag_map = &tag_state_->local_class2tag_map_;
        class_idxr = tag_state_->local_mapping_class_id_idxr_;
    } else {
        tag2class_map = &tag_state_->remote_tag2class_map_;
        class2tag_map = &tag_state_->remote_class2tag_map_;
        class_idxr = tag_state_->remote_mapping_class_id_idxr_;
    }

    if (class2tag_map->find(class_id) != class2tag_map->end()) {
        tag = (*class2tag_map)[class_id];
        if (unlikely(tag2class_map->find(tag) == tag2class_map->end())) {
            PDS_TRACE_ERR("tag2class lookup failed tag %u, class id %u",
                          tag, class_id);
            // go ahead and release the classid and class2tag map entry
            class2tag_map->erase(class_id);
            class_idxr->free(class_id);
        } else {
            (*tag2class_map)[tag].refcount--;
            if ((*tag2class_map)[tag].refcount == 0) {
                class2tag_map->erase(class_id);
                tag2class_map->erase(tag);
                class_idxr->free(class_id);
            }
        }
    } else {
        // handle it gracefully
        PDS_TRACE_ERR("class id %u not in use", class_id);
    }
#endif

    if (tag_state_->class2tag_map_.find(class_id) !=
            tag_state_->class2tag_map_.end()) {
        tag = tag_state_->class2tag_map_[class_id];
        if (unlikely(tag_state_->tag2class_map_.find(tag) ==
                         tag_state_->tag2class_map_.end())) {
            PDS_TRACE_ERR("tag2class lookup failed tag %u, class id %u",
                          tag, class_id);
            // go ahead and release the classid and class2tag map entry
            tag_state_->class2tag_map_.erase(class_id);
            tag_state_->class_id_idxr_->free(class_id);
        } else {
            tag_state_->tag2class_map_[tag].refcount--;
            if (tag_state_->tag2class_map_[tag].refcount == 0) {
                tag_state_->class2tag_map_.erase(class_id);
                tag_state_->tag2class_map_.erase(tag);
                tag_state_->class_id_idxr_->free(class_id);
            }
        }
    } else {
        // handle it gracefully
        PDS_TRACE_ERR("class id %u not in use", class_id);
    }
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::release_tag(uint32_t tag, bool local) {
    sdk_ret_t ret;
    uint32_t class_id;
#if 0
    rte_indexer *class_idxr;
    tag2class_map_t *tag2class_map;
    class2tag_map_t *class2tag_map;

    if (local) {
        tag2class_map = &tag_state_->local_tag2class_map_;
        class2tag_map = &tag_state_->local_class2tag_map_;
        class_idxr = tag_state_->local_mapping_class_id_idxr_;
    } else {
        tag2class_map = &tag_state_->remote_tag2class_map_;
        class2tag_map = &tag_state_->remote_class2tag_map_;
        class_idxr = tag_state_->remote_mapping_class_id_idxr_;
    }

    if (tag2class_map->find(tag) != tag2class_map->end()) {
        (*tag2class_map)[tag].refcount--;
        if ((*tag2class_map)[tag].refcount == 0) {
            // cleanup the tag <-> class id mappings
            class_id = (*tag2class_map)[tag].class_id;
            tag2class_map->erase(tag);
            class2tag_map->erase(class_id);
            // free back the class id
            class_idxr->free(class_id);
        }
    } else {
        // handle it gracefully
        PDS_TRACE_ERR("Tag %u has no class id allocated", tag);
    }
#endif
    if (tag_state_->tag2class_map_.find(tag) !=
            tag_state_->tag2class_map_.end()) {
        tag_state_->tag2class_map_[tag].refcount--;
        if (tag_state_->tag2class_map_[tag].refcount == 0) {
            // cleanup the tag <-> class id mappings
            class_id = tag_state_->tag2class_map_[tag].class_id;
            tag_state_->tag2class_map_.erase(tag);
            tag_state_->class2tag_map_.erase(class_id);
            // free back the class id
            tag_state_->class_id_idxr_->free(class_id);
        }
    } else {
        // handle it gracefully
        PDS_TRACE_ERR("Tag %u has no class id allocated", tag);
    }
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
