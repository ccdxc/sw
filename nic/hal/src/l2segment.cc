#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <l2segment.hpp>
#include <l2segment_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {

void *
l2seg_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((l2seg_t *)entry)->seg_id);
}

uint32_t
l2seg_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(l2seg_id_t)) % ht_size;
}

bool
l2seg_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(l2seg_id_t *)key1 == *(l2seg_id_t *)key2) {
        return true;
    }
    return false;
}

void *
l2seg_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((l2seg_t *)entry)->hal_handle);
}

uint32_t
l2seg_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
l2seg_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming L2 segment create request
// TODO:
// 1. check if L2 segment exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_l2segment_create (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    if (!spec.has_meta() ||
        spec.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            L2SegmentKeyHandle::kSegmentId) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // must have either access and/or fabric encap set
    if (!spec.has_access_encap() && !spec.has_fabric_encap()) {
        rsp->set_api_status(types::API_STATUS_ENCAP_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this L2 segment in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_l2seg_to_db (tenant_t *tenant, l2seg_t *l2seg)
{
    HAL_SPINLOCK_LOCK(&tenant->slock);
    utils::dllist_add(&tenant->l2seg_list_head, &l2seg->tenant_l2seg_lentry);
    HAL_SPINLOCK_UNLOCK(&tenant->slock);

    g_hal_state->l2seg_hal_handle_ht()->insert(l2seg,
                                               &l2seg->hal_handle_ht_ctxt);
    g_hal_state->l2seg_id_ht()->insert(l2seg, &l2seg->ht_ctxt);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a L2 segment create request
// TODO: if L2 segment exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
l2segment_create (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    hal_ret_t              ret;
    tenant_t               *tenant;
    l2seg_t                *l2seg;
    tenant_id_t            tid;
    pd::pd_l2seg_args_t    pd_l2seg_args;


    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-L2Seg:{}: L2Seg Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().segment_id());

    // validate the request message
    ret = validate_l2segment_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("PI-L2Seg:{}: Validation Failed. ret: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    // fetch the tenant
    tid = spec.meta().tenant_id();
    tenant = find_tenant_by_id(tid);
    if (tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("PI-L2Seg:{}: Fetch Tenant Id:{} Failed. ret: {}",
                      __FUNCTION__, tid, ret);
        goto end;
    }

    // instantiate the L2 segment
    l2seg = l2seg_alloc_init();
    if (l2seg == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("PI-L2Seg:{}: Out of Memory. ret: {}",
                      __FUNCTION__, ret);
        goto end;
    }
    l2seg->tenant_id = tid;
    l2seg->seg_id = spec.key_or_handle().segment_id();
    l2seg->segment_type = spec.segment_type();
    l2seg->mcast_fwd_policy = spec.mcast_fwd_policy();
    l2seg->bcast_fwd_policy = spec.bcast_fwd_policy();
    if (spec.has_access_encap()) {
        l2seg->access_encap.type = spec.access_encap().encap_type();
        l2seg->access_encap.val = spec.access_encap().encap_value();
    }
    if (spec.has_fabric_encap()) {
        l2seg->fabric_encap.type = spec.fabric_encap().encap_type();
        l2seg->fabric_encap.val = spec.fabric_encap().encap_value();
    }
    l2seg->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming, if any
    pd::pd_l2seg_args_init(&pd_l2seg_args);
    pd_l2seg_args.tenant = tenant;
    pd_l2seg_args.l2seg = l2seg;
    ret = pd::pd_l2seg_create(&pd_l2seg_args);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        HAL_TRACE_ERR("PI-L2Seg:{}: PD programming fail.  ret: {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // add this L2 segment to our db
    ret = add_l2seg_to_db(tenant, l2seg);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_l2segment_status()->set_l2segment_handle(l2seg->hal_handle);

end:

    if (ret != HAL_RET_OK && l2seg != NULL) {
        l2seg_free(l2seg);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a L2 segment update request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_update (L2SegmentSpec& spec, L2SegmentResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a L2 segment get request
//------------------------------------------------------------------------------
hal_ret_t
l2segment_get (L2SegmentGetRequest& req, L2SegmentGetResponse *rsp)
{
    l2seg_t    *l2seg;

    if (!req.has_meta() || req.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    auto kh = req.key_or_handle();

    if (kh.key_or_handle_case() == l2segment::L2SegmentKeyHandle::kSegmentId) {
        l2seg = find_l2seg_by_id(kh.segment_id());
    } else if (kh.key_or_handle_case() == l2segment::L2SegmentKeyHandle::kL2SegmentHandle) {
        l2seg = find_l2seg_by_handle(kh.l2segment_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (l2seg == NULL) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    // fill config spec of this L2 segment
    rsp->mutable_spec()->mutable_meta()->set_tenant_id(l2seg->tenant_id);
    rsp->mutable_spec()->mutable_key_or_handle()->set_segment_id(l2seg->seg_id);
    rsp->mutable_spec()->set_segment_type(l2seg->segment_type);
    rsp->mutable_spec()->set_mcast_fwd_policy(l2seg->mcast_fwd_policy);
    rsp->mutable_spec()->set_bcast_fwd_policy(l2seg->bcast_fwd_policy);
    rsp->mutable_spec()->mutable_access_encap()->set_encap_type(l2seg->access_encap.type);
    rsp->mutable_spec()->mutable_access_encap()->set_encap_value(l2seg->access_encap.val);
    rsp->mutable_spec()->mutable_fabric_encap()->set_encap_type(l2seg->fabric_encap.type);
    rsp->mutable_spec()->mutable_fabric_encap()->set_encap_value(l2seg->fabric_encap.val);

    // fill operational state of this L2 segment
    rsp->mutable_status()->set_l2segment_handle(l2seg->hal_handle);

    // fill stats of this L2 segment
    rsp->mutable_stats()->set_num_endpoints(l2seg->num_ep);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
