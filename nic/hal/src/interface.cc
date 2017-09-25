#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <interface_svc.hpp>
#include <interface.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <l2segment.pb.h>
#include <lif_manager.hpp>
#include <utils.hpp>
#include <oif_list_api.hpp>
#include <if_utils.hpp>
#include <rdma.hpp>

using hal::pd::pd_if_args_t;

namespace hal {

#if 0
void *
lif_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((lif_t *)entry)->lif_id);
}

uint32_t
lif_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(lif_id_t)) % ht_size;
}

bool
lif_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(lif_id_t *)key1 == *(lif_id_t *)key2) {
        return true;
    }
    return false;
}

void *
lif_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((lif_t *)entry)->hal_handle);
}

uint32_t
lif_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
lif_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}
#endif

void *
if_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((if_t *)entry)->if_id);
}

uint32_t
if_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(if_id_t)) % ht_size;
}

bool
if_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(if_id_t *)key1 == *(if_id_t *)key2) {
        return true;
    }
    return false;
}

void *
if_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((if_t *)entry)->hw_id);
}

uint32_t
if_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(if_hw_id_t)) % ht_size;
}

bool
if_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(if_hw_id_t *)key1 == *(if_hw_id_t *)key2) {
        return true;
    }
    return false;
}

void *
if_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((if_t *)entry)->hal_handle);
}

uint32_t
if_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
if_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

#if 0
//------------------------------------------------------------------------------
// insert this lif in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_lif_to_db (lif_t *lif)
{
    g_hal_state->lif_id_ht()->insert(lif, &lif->ht_ctxt);
    g_hal_state->lif_hal_handle_ht()->insert(lif, &lif->hal_handle_ht_ctxt);

    return HAL_RET_OK;
}
#endif

//------------------------------------------------------------------------------
// insert this interface in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_if_to_db (if_t *hal_if)
{
    g_hal_state->if_id_ht()->insert(hal_if, &hal_if->ht_ctxt);
    g_hal_state->if_hwid_ht()->insert(hal_if, &hal_if->hw_ht_ctxt);
    g_hal_state->if_hal_handle_ht()->insert(hal_if,
                                            &hal_if->hal_handle_ht_ctxt);

    return HAL_RET_OK;
}

#if 0
//------------------------------------------------------------------------------
// init lif specific queue state, if any
//------------------------------------------------------------------------------
hal_ret_t
lif_qstate_init (LifSpec& spec, uint32_t hw_lif_id, lif_t *lif)
{
    LIFQStateParams    qs_params = { 0 };
    int32_t            ec = 0;

    for (int i = 0; i < spec.lif_qstate_map_size(); i++) {
        const auto &ent = spec.lif_qstate_map(i);
        if (ent.type_num() >= kNumQTypes) {
            HAL_TRACE_ERR("Invalid type num in LifSpec : {}", ent.type_num());
            return HAL_RET_INVALID_ARG;
        }
        if (ent.size() > 7 || ent.entries() > 24) {
            HAL_TRACE_ERR("Invalid entry in LifSpec : size={} entries={}",
                          ent.size(), ent.entries());
            return HAL_RET_INVALID_ARG;
        }
        qs_params.type[ent.type_num()].size = ent.size();
        qs_params.type[ent.type_num()].entries = ent.entries();

        if (ent.purpose() > intf::LifQPurpose_MAX) {
            HAL_TRACE_ERR("Invalid entry in LifSpec : purpose={}", ent.purpose());
            return HAL_RET_INVALID_ARG;
        }
        lif->qtypes[ent.purpose()] = ent.type_num();
    }

    // make sure that when you are creating with hw_lif_id the lif is alloced
    // already, otherwise this call may return an error
    if ((ec = g_lif_manager->InitLIFQState(hw_lif_id, &qs_params)) < 0) {
        HAL_TRACE_ERR("Failed to initialize LIFQState: err_code : {}", ec);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

static hal_ret_t
lif_fwd_create (LifSpec& spec, LifResponse *rsp, lif_t *lif, 
                lif_hal_info_t *lif_hal_info)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd::pd_lif_args_t    pd_lif_args;

    HAL_TRACE_DEBUG("PI-LIF:{}: P4 Processing for lif id {}", __FUNCTION__, 
                    spec.key_or_handle().lif_id());

    // allocate all PD resources and finish programming, if any
    pd::pd_lif_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    if (lif_hal_info) {
        pd_lif_args.with_hw_lif_id = lif_hal_info->with_hw_lif_id;
        pd_lif_args.hw_lif_id = lif_hal_info->hw_lif_id;
    }
    ret = pd::pd_lif_create(&pd_lif_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD lif create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

end:

    return ret;

}

//------------------------------------------------------------------------------
// process lif create
//------------------------------------------------------------------------------
hal_ret_t
lif_create (LifSpec& spec, LifResponse *rsp, lif_hal_info_t *lif_hal_info)
{
    hal_ret_t          ret = HAL_RET_OK;
    lif_t              *lif = NULL;
    uint32_t           hw_lif_id = 0;
    std::unique_ptr<uint8_t[]> buf;
    lif_hal_info_t       lif_info;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-LIF:{}: Lif Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().lif_id());

    // make sure key-handle field is set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("PI-LIF:{}: Lif didn't have key or handle. Err: {}", 
                      __FUNCTION__, HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_LIF_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // check if lif id is set in key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            intf::LifKeyHandle::kLifId) {
        HAL_TRACE_ERR("PI-LIF:{}: Lif didn't have Id. Err: {}", 
                      __FUNCTION__, HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_LIF_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // allocate and initialize interface instance
    lif = lif_alloc_init();
    if (lif == NULL) {
        HAL_TRACE_ERR("PI-LIF:{}: Out of Memory. Err: {}", 
                      __FUNCTION__, HAL_RET_OOM);
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // consume the config
    lif->lif_id = spec.key_or_handle().lif_id();
    MAC_UINT64_TO_ADDR(lif->mac_addr, spec.mac_addr());
    lif->admin_status = spec.admin_status();
    lif->port_num = spec.port_num();
    lif->hal_handle = hal_alloc_handle();
    lif->vlan_strip_en = spec.vlan_strip_en();
    //lif->allmulti = spec.allmulti();
    lif->enable_rdma = spec.enable_rdma();

    // allocate a hw lif id
    if (lif_hal_info && lif_hal_info->with_hw_lif_id) {
        hw_lif_id = lif_hal_info->hw_lif_id;
    } else {
        hw_lif_id = g_lif_manager->LIFRangeAlloc(-1, 1);
        if (((int32_t)hw_lif_id) < 0) {
            HAL_TRACE_ERR("Failed to allocate lif, hw_lif_id : {}", hw_lif_id);
            ret = HAL_RET_NO_RESOURCE;
            goto end;
        }
        lif_hal_info = &lif_info;
        lif_hal_info->with_hw_lif_id = 1;
        lif_hal_info->hw_lif_id = hw_lif_id;
    }

    // init queues
    ret = lif_qstate_init(spec, hw_lif_id, lif);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // create the lif now
    ret = lif_fwd_create(spec, rsp, lif, lif_hal_info);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("P4 Lif create failure, err : {}", ret);
        ret = HAL_RET_ERR;
        goto end;
    }


    // Initialize QState
    for (int i = 0; i < spec.lif_qstate_size(); i++) {
        const auto &req = spec.lif_qstate(i);

        const uint8_t *state = (const uint8_t *)req.queue_state().c_str();
        if (req.has_label()) {
            uint8_t off;
            int ret = g_lif_manager->GetPCOffset(req.label().handle().c_str(),
                req.label().prog_name().c_str(),
                req.label().label().c_str(), &off);
            if (ret < 0) {
                ret = HAL_RET_ERR;
                goto end;
            }
            buf.reset(new uint8_t[req.queue_state().size()]);
            bcopy(req.queue_state().c_str(), buf.get(), req.queue_state().size());
            buf.get()[0] = off;
            state = buf.get();
        }

        int ret = g_lif_manager->WriteQState(hw_lif_id, req.type_num(),
                                             req.qid(), state,
                                             req.queue_state().size());
        if (ret < 0) {
            HAL_TRACE_ERR("Failed to set LIFQState : {}", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
    }

    // For rdma enabled Lifs, call RDMA specific init (allocates KT, PT, etc)
    if (lif->enable_rdma) {
        ret = rdma_lif_init(spec, hw_lif_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("P4 Lif create failure, err : {}", ret);
            ret = HAL_RET_ERR;
            goto end;
        }
    }


    ret = HAL_RET_OK;

    // add this lif to the db
    add_lif_to_db(lif);
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->set_hw_lif_id(hw_lif_id);
    rsp->mutable_status()->set_lif_status(lif->admin_status);
    rsp->mutable_status()->set_lif_handle(lif->hal_handle);

    // Return LifQstate addresses
    intf::LifQState *entry;
    for (int type_num = 0; type_num < spec.lif_qstate_map_size(); type_num++) {
        entry = rsp->add_qstate();
        entry->set_type_num(type_num);
        entry->set_addr(g_lif_manager->GetLIFQStateAddr(hw_lif_id, type_num, 0));
    }

end:

    if ((ret != HAL_RET_OK) && lif != NULL) {
        lif_free(lif);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}
#endif
//------------------------------------------------------------------------------
// fetch if from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static if_t *
fetch_if_ifl2seg (InterfaceL2SegmentSpec& spec)
{
    if (spec.if_key_handle().key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId) {
        return find_if_by_id(spec.if_key_handle().interface_id());
    } else {
        return find_if_by_handle(spec.if_key_handle().if_handle());
    }

    return NULL;
}

//------------------------------------------------------------------------------
// get if from either id or handle
//------------------------------------------------------------------------------
if_t *
if_lookup (const intf::InterfaceKeyHandle& key_handle)
{
    if (key_handle.key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId) {
        return find_if_by_id(key_handle.interface_id());
    }
    if (key_handle.key_or_handle_case() == 
            intf::InterfaceKeyHandle::kIfHandle) {
        return find_if_by_handle(key_handle.if_handle());
    }

    return NULL;

}

//------------------------------------------------------------------------------
// fetch L2 segment from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static l2seg_t *
fetch_l2seg_ifl2seg (InterfaceL2SegmentSpec& spec)
{

    if (spec.l2segment_key_or_handle().key_or_handle_case() ==
            l2segment::L2SegmentKeyHandle::kSegmentId) {
        return find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id());
    } else {
        return find_l2seg_by_handle(spec.l2segment_key_or_handle().l2segment_handle());
    }

    return NULL;
}

//------------------------------------------------------------------------------
// validate L2 segment on uplink
//------------------------------------------------------------------------------
static hal_ret_t
validate_l2seg_on_uplink (InterfaceL2SegmentSpec& spec, 
                          InterfaceL2SegmentResponse *rsp)
{
    // L2 segment key/handle is must
    if (!spec.has_l2segment_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // L2 segment has to exist
    if (spec.l2segment_key_or_handle().key_or_handle_case() == 
            l2segment::L2SegmentKeyHandle::kSegmentId && 
            !find_l2seg_by_id(spec.l2segment_key_or_handle().segment_id())) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.l2segment_key_or_handle().key_or_handle_case() == 
            l2segment::L2SegmentKeyHandle::kL2SegmentHandle &&
            !find_l2seg_by_handle(spec.l2segment_key_or_handle().l2segment_handle())) {
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // uplink key/hanle is must
    if (!spec.has_if_key_handle()) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // uplink has to exist
    if (spec.if_key_handle().key_or_handle_case() == 
            intf::InterfaceKeyHandle::kInterfaceId && 
            !find_if_by_id(spec.if_key_handle().interface_id())) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }
    if (spec.if_key_handle().key_or_handle_case() == 
            intf::InterfaceKeyHandle::kIfHandle &&
            !find_if_by_handle(spec.if_key_handle().if_handle())) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("Validation Passed");
    return HAL_RET_OK;
}



//------------------------------------------------------------------------------
// validate an incoming interface create request
// TODO:
// 1. check if interface exists already
// 2. every interface has a lif id ?? (may be not tunnel ??)
//------------------------------------------------------------------------------
static hal_ret_t
validate_interface_create (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    intf::IfType    if_type;
    tenant_id_t     tid;
    tenant_t        *tenant = NULL;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            InterfaceKeyHandle::kInterfaceId) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // interface type must be valid
    if_type = spec.type();
    if (if_type == intf::IF_TYPE_NONE) {
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (if_type == intf::IF_TYPE_ENIC) {
        // fetch the tenant information
        tid = spec.meta().tenant_id();
        tenant = tenant_lookup_by_id(tid);
        if (tenant == NULL) {
            HAL_TRACE_ERR("PI-IF:{}: Valid. failed. TenId: {} Err: {} "
                          "ApiStatus: {}", __FUNCTION__, tid,
                          HAL_RET_INVALID_ARG, rsp->api_status());
            rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
            return HAL_RET_INVALID_ARG;
        }

        // make sure ENIC info is provided
        if (!spec.has_if_enic_info()) {
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

        // check if lif is provided
        if (!spec.if_enic_info().has_lif_key_or_handle()) {
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

    } else if (if_type == intf::IF_TYPE_UPLINK) {
        // uplink specific validation
        if (!spec.has_if_uplink_info()) {
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_UPLINK_PC) {
        // uplink PC specific validation
        if (!spec.has_if_uplink_pc_info()) {
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_TUNNEL) {
        // tunnel specification validation
        if (!spec.has_if_tunnel_info()) {
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_CPU) {
        // CPU specific validation
        if (!spec.has_if_cpu_info()) {
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        // TODO: Tunnels etc.
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a interface create request
// TODO: if interface already exists, treat it as modify
//------------------------------------------------------------------------------
hal_ret_t
interface_create (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    l2seg_t         *l2seg = NULL;
    if_t            *hal_if = NULL;
    pd_if_args_t    pd_if_args;

    HAL_TRACE_DEBUG("----------------------- Interface API Start ------------");
    HAL_TRACE_DEBUG("PI-IF:{}: If Create for id {} Type: {} EnicType: {}",
                    __FUNCTION__, spec.key_or_handle().interface_id(),
                    spec.type(), (spec.type() == intf::IF_TYPE_ENIC) ?
                    spec.if_enic_info().enic_type() : intf::IF_ENIC_TYPE_NONE);

    // do basic validations on interface
    ret = validate_interface_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-IF:{}: Validation failed. Err: {} ApiStatus: {}",
                      __FUNCTION__, ret, rsp->api_status());
        goto end;
    }

    // allocate and initialize interface instance
    hal_if = if_alloc_init();
    if (hal_if == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // consume the config
    hal_if->if_id = spec.key_or_handle().interface_id();
    hal_if->if_type = spec.type();
    hal_if->if_admin_status = spec.admin_status();
    hal_if->if_op_status = intf::IF_STATUS_NONE;      // TODO: set this later !!

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        ret = enic_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        l2seg = find_l2seg_by_id(hal_if->l2seg_id);
        break;

    case intf::IF_TYPE_UPLINK:
        ret = uplink_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_uplink_to_l2seg() call
        break;

    case intf::IF_TYPE_UPLINK_PC:
        ret = uplink_pc_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }

        // Will be added to broadcast list through add_uplink_to_l2seg() call
        break;

    case intf::IF_TYPE_TUNNEL:
        ret = tunnel_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_CPU:
        ret = cpu_if_create(spec, rsp, hal_if);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;
    default:
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        goto end;
    }
    hal_if->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming, if any
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;
    ret = pd::pd_if_create(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD interface create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // If its enic, add to l2seg
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        ret = l2seg_add_if(l2seg, hal_if);
        HAL_ABORT(ret == HAL_RET_OK);
    }

    // add this interface to the db
    add_if_to_db(hal_if);

    if (l2seg) {
        // Add the new interface to the broadcast list of the associated l2seg. This applies to enicifs only.
        // Its here because the multicast oif call requires the pi_if to have been created fully.
        oif_t  oif;
        oif.if_id = hal_if->if_id;
        oif.l2_seg_id = l2seg->seg_id;
        ret = oif_list_add_oif(l2seg->bcast_oif_list, &oif);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Add oif to oif_list failed, err : {}", ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
    rsp->mutable_status()->set_if_handle(hal_if->hal_handle);

end:

    if (ret != HAL_RET_OK && hal_if != NULL) {
        if_free(hal_if);
    }
    HAL_TRACE_DEBUG("----------------------- Interface API End --------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a interface update request
//------------------------------------------------------------------------------
hal_ret_t
interface_update (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    if_t                *hal_if = NULL;
    pd_if_args_t        pd_if_args;

    HAL_TRACE_DEBUG("----------------------- Interface API Start ------------");

    hal_if = if_lookup(spec.key_or_handle());
    if (!hal_if) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("PI-IF:{}: If Update Fail", __FUNCTION__);
        goto end;
    }

    HAL_TRACE_DEBUG("PI-IF:{}: If Update for id {} Type: {} EnicType: {}",
                    __FUNCTION__, hal_if->if_id, hal_if->if_type,
                    hal_if->enic_type);

    // allocate pd args
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;

    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
        break;

    case intf::IF_TYPE_UPLINK:
        ret = uplink_if_update(spec, rsp, hal_if, &pd_if_args);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_UPLINK_PC:
        ret = uplink_pc_update(spec, rsp, hal_if, &pd_if_args);
        if (ret != HAL_RET_OK) {
            goto end;
        }
        break;

    case intf::IF_TYPE_TUNNEL:
        break;

    default:
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        goto end;
    }

    // PD If Update
    ret = pd::pd_if_update(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD interface update failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
    rsp->mutable_status()->set_if_handle(hal_if->hal_handle);

end:

    HAL_TRACE_DEBUG("----------------- Interface API End --------------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a interface get request
//------------------------------------------------------------------------------
hal_ret_t
interface_get (InterfaceGetRequest& req, InterfaceGetResponse *rsp)
{
    if_t             *hal_if;
    InterfaceSpec    *spec;

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    hal_if = if_lookup(req.key_or_handle());
    if (!hal_if) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        return HAL_RET_INVALID_ARG;
    }

    // fill in the config spec of this interface
    spec = rsp->mutable_spec();
    spec->mutable_key_or_handle()->set_interface_id(hal_if->if_id);
    spec->set_type(hal_if->if_type);
    spec->set_admin_status(hal_if->if_admin_status);
    switch (hal_if->if_type) {
    case intf::IF_TYPE_ENIC:
    {
        auto enic_if_info = spec->mutable_if_enic_info();
        enic_if_info->set_enic_type(hal_if->enic_type);
        enic_if_info->mutable_lif_key_or_handle()->set_lif_id(hal_if->if_id);
        enic_if_info->set_l2segment_id(hal_if->l2seg_id);
        enic_if_info->set_mac_address(MAC_TO_UINT64(hal_if->mac_addr));
        enic_if_info->set_encap_vlan_id(hal_if->encap_vlan);
    }
        break;

    case intf::IF_TYPE_UPLINK:
    {
        auto uplink_if_info = spec->mutable_if_uplink_info();
        uplink_if_info->set_port_num(hal_if->uplink_port_num);
        uplink_if_info->set_native_l2segment_id(hal_if->native_l2seg);
        // TODO: is this populated today ?
        //uplink_if_info->set_l2segment_id();
        // TODO: don't see this info populated in if today
        //uplink_if_info->set_rx_traffic_class_info();
    }
        break;

    case intf::IF_TYPE_UPLINK_PC:
    {
        auto uplink_pc_info = spec->mutable_if_uplink_pc_info();
        uplink_pc_info->set_uplink_pc_num(hal_if->uplink_pc_num);
        uplink_pc_info->set_native_l2segment_id(hal_if->native_l2seg);
        // TODO: is this populated today ???
        //uplink_pc_info->set_l2segment_id();
        // TODO: don't see this info populated in if_t today !!!
        //uplink_pc_info->set_rx_traffic_class_info();
        // TODO: not available in if_t today !!!
        //uplink_pc_info->set_member_if_handle();
    }
        break;

    case intf::IF_TYPE_TUNNEL:
    {
        auto tunnel_if_info = spec->mutable_if_tunnel_info();
        tunnel_if_info->set_encap_type(hal_if->encap_type);
        if (hal_if->encap_type == intf::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
            auto vxlan_info = tunnel_if_info->mutable_vxlan_info();
            ip_addr_to_spec(vxlan_info->mutable_local_tep(),
                            &hal_if->vxlan_ltep);
            ip_addr_to_spec(vxlan_info->mutable_remote_tep(),
                            &hal_if->vxlan_rtep);
        }
    }
        break;

    case intf::IF_TYPE_CPU:     // TODO: why is this exposed in API or only GET is supported ?
        break;

    default:
        break;
    }
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// add l2segment on Uplink
//------------------------------------------------------------------------------
hal_ret_t
add_l2seg_on_uplink (InterfaceL2SegmentSpec& spec,
                     InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    l2seg_t                     *l2seg = NULL;
    oif_t                       oif;
    pd::pd_l2seg_uplink_args_t  pd_l2seg_uplink_args;

    HAL_TRACE_DEBUG("---------------- Interface API Start -------------------");
    // Validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    HAL_ASSERT((hal_if != NULL) && (l2seg != NULL));

    HAL_TRACE_DEBUG("PI-AddL2SegUplink:{}: if_id:{} l2seg_id:{}", __FUNCTION__, 
            hal_if->if_id, l2seg->seg_id);

    // PD Call
    pd::pd_l2seg_uplinkif_args_init(&pd_l2seg_uplink_args);
    pd_l2seg_uplink_args.l2seg  = l2seg;
    pd_l2seg_uplink_args.intf   = hal_if;
    ret = pd::pd_add_l2seg_uplink(&pd_l2seg_uplink_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Add L2 segment on Uplink If failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // Update PI structures - Add vlan to Uplink and vice-versa
    if (l2seg->fabric_encap.type == types::ENCAP_TYPE_DOT1Q) {
        hal_if->vlans->set(l2seg->fabric_encap.val);
    }

    // Add Uplink in l2seg
    ret = l2seg_add_if(l2seg, hal_if);
    HAL_ABORT(ret == HAL_RET_OK);


    // Add the uplink to the broadcast list of the l2seg
    oif.if_id = hal_if->if_id;
    oif.l2_seg_id = l2seg->seg_id;
    ret = oif_list_add_oif(l2seg->bcast_oif_list, &oif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Add oif to oif_list failed, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

cleanup:

    HAL_TRACE_DEBUG("----------------------- Interface API End --------------");
    return ret;
}

//------------------------------------------------------------------------------
// del l2segment to Uplink
//------------------------------------------------------------------------------
hal_ret_t
del_l2seg_on_uplink (InterfaceL2SegmentSpec& spec,
                     InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// CPU If Create 
//------------------------------------------------------------------------------
hal_ret_t
cpu_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
               if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    lif_t               *lif;

    HAL_TRACE_DEBUG("PI-CPUif:{}: CPUif Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    ret = get_lif_handle_for_cpu_if(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-CPUif:{}: Unable to find the lif handle Err: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    lif = find_lif_by_handle(hal_if->lif_handle);
    HAL_TRACE_DEBUG("PI-CPUif:{}: if_id:{} lif_id:{}", __FUNCTION__, 
            hal_if->if_id, lif->lif_id);

    return ret;
}



//------------------------------------------------------------------------------
// Enic If Create 
//------------------------------------------------------------------------------
hal_ret_t
enic_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    // l2seg_id_t          l2seg_id;
    // l2seg_t             *l2seg;
    lif_t               *lif;

    HAL_TRACE_DEBUG("PI-Enicif:{}: Enicif Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    // lif for enic_if ... rsp is updated within the call
    ret = get_lif_handle_for_enic_if(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Enicif:{}: Unable to find the lif handle Err: {}",
                      __FUNCTION__, ret);
        return ret;
    }

    auto if_enic_info = spec.if_enic_info();
    hal_if->enic_type = if_enic_info.enic_type();
    hal_if->tid = spec.meta().tenant_id();
    lif = find_lif_by_handle(hal_if->lif_handle);

    if (hal_if->enic_type == intf::IF_ENIC_TYPE_USEG || 
            hal_if->enic_type == intf::IF_ENIC_TYPE_PVLAN ||
            hal_if->enic_type == intf::IF_ENIC_TYPE_DIRECT) {
        hal_if->l2seg_id = if_enic_info.l2segment_id();
        MAC_UINT64_TO_ADDR(hal_if->mac_addr,
                if_enic_info.mac_address());
        hal_if->encap_vlan = if_enic_info.encap_vlan_id();

        HAL_TRACE_DEBUG("{}: Enic-If: L2_seg: {}, Encap: {}, Mac: {} lif: {}", 
                        __FUNCTION__,
                        hal_if->l2seg_id,
                        hal_if->encap_vlan,
                        macaddr2str(hal_if->mac_addr),
                        lif->lif_id);

#if 0
        // Bharat: Revisit if we need to support bitmap in create
        // populate VLANs that are up on this interface, if any
        hal_if->vlan_bmap = bitmap::factory(4096);
        if (hal_if->vlan_bmap == NULL) {
            ret = HAL_RET_OOM;
            rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
            goto end;
        }
        for (int i = 0; i < if_enic_info.direct_info().l2segment_id_size(); i++) {
            l2seg_id = if_enic_info.direct_info().l2segment_id(i);
            l2seg = find_l2seg_by_id(l2seg_id);
            if (l2seg == NULL) {
                ret = HAL_RET_L2SEG_NOT_FOUND;
                rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
                goto end;
            }
            if (l2seg->access_encap.type == types::ENCAP_TYPE_DOT1Q) {
                hal_if->vlan_bmap->set(l2seg->access_encap.val);
            }
        }
#endif
    } else {
        HAL_TRACE_ERR("{}: Invalid Enic Type: {}", __FUNCTION__, 
                        hal_if->enic_type);
        rsp->set_api_status(types::API_STATUS_IF_ENIC_TYPE_INVALID);
        ret = HAL_RET_INVALID_ARG;

    }

    return ret;
}


//------------------------------------------------------------------------------
// Uplink If Update
//------------------------------------------------------------------------------
hal_ret_t
uplink_if_update (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if, void *if_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_if_args_t        *pd_if_args = (pd_if_args_t *)if_args;

    HAL_TRACE_DEBUG("PI-Uplinkif:{}: Update for if_id:{}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    HAL_ASSERT_RETURN(if_args != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->native_l2seg != spec.if_uplink_info().native_l2segment_id()) {
        hal_if->native_l2seg = spec.if_uplink_info().native_l2segment_id();
        pd_if_args->native_l2_seg_upd = true;
        HAL_TRACE_DEBUG("PI-Uplinkif:{}: Updating native_l2seg_id:{}", 
                __FUNCTION__, hal_if->native_l2seg);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Uplink PC Update
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_update (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if, void *if_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_if_args_t        *pd_if_args = (pd_if_args_t *)if_args;

    HAL_TRACE_DEBUG("PI-UplinkPC:{}: Update for if_id:{}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    HAL_ASSERT_RETURN(if_args != NULL, HAL_RET_INVALID_ARG);

    if (hal_if->native_l2seg !=
        spec.if_uplink_pc_info().native_l2segment_id()) {
        hal_if->native_l2seg = spec.if_uplink_pc_info().native_l2segment_id();
        pd_if_args->native_l2_seg_upd = true;
        HAL_TRACE_DEBUG("PI-UplinkPC:{}: Updating native_l2seg_id:{}", 
                __FUNCTION__, hal_if->native_l2seg);
    }

    return ret;
}

//------------------------------------------------------------------------------
// Uplink If Create 
//------------------------------------------------------------------------------
hal_ret_t
uplink_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("PI-Uplinkif:{}: Uplinkif Create for id {}, "
                    "native_l2seg_id : {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id(),
                    spec.if_uplink_info().native_l2segment_id());

    // TODO: for a member port, we can have valid pc#
    ret = pltfm_get_port_from_front_port_num(spec.if_uplink_info().port_num(),
                                             &hal_if->uplink_port_num);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, HAL_RET_INVALID_ARG);

    hal_if->uplink_pc_num = HAL_PC_INVALID;
    hal_if->native_l2seg = spec.if_uplink_info().native_l2segment_id();
    hal_if->vlans = bitmap::factory(4096);
    if (hal_if->vlans == NULL) {
        ret = HAL_RET_OOM;
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        HAL_TRACE_ERR("PI-Uplinkif:{}: Out of Memory Err: {}", ret);
        goto end;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// Uplink PC If Create 
//------------------------------------------------------------------------------
hal_ret_t
uplink_pc_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if)
{
    hal_ret_t    ret = HAL_RET_OK;
    uint64_t     mbr_if_handle = 0;
    if_t         *mbr_if = NULL;

    HAL_TRACE_DEBUG("PI-UplinkPC:{}: UplinkPC Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    hal_if->uplink_port_num = HAL_PORT_INVALID;
    hal_if->uplink_pc_num = spec.if_uplink_pc_info().uplink_pc_num();
    hal_if->native_l2seg = spec.if_uplink_pc_info().native_l2segment_id();
    hal_if->vlans = bitmap::factory(4096);
    if (hal_if->vlans == NULL) {
        ret = HAL_RET_OOM;
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        HAL_TRACE_ERR("PI-UplinkPC:{}: Out of Memory Err: {}", ret);
        goto end;
    }

    HAL_TRACE_DEBUG("PI-UplinkPC:{}: Adding {} no. of members", __FUNCTION__,
                    spec.if_uplink_pc_info().member_if_handle_size());
    // Walk through member uplinks
    utils::dllist_reset(&hal_if->mbr_if_list_head);
    for (int i = 0; i < spec.if_uplink_pc_info().member_if_handle_size(); i++) {
        mbr_if_handle = spec.if_uplink_pc_info().member_if_handle(i);
        mbr_if = find_if_by_handle(mbr_if_handle);
        HAL_ASSERT_RETURN(mbr_if != NULL, HAL_RET_INVALID_ARG);
        HAL_TRACE_DEBUG("PI-UplinkPC:{}: Trying to add mbr_if_id:{}",
                __FUNCTION__, mbr_if->if_id);
        if (mbr_if->if_type != intf::IF_TYPE_UPLINK) {
            HAL_TRACE_ERR("PI-UplinkPC:{}: Unable to add non-uplinkif. "
                          "Skipping if id: {}", __FUNCTION__, mbr_if->if_id);
            continue;
        }
        // add uplink if to list
        utils::dllist_add(&hal_if->mbr_if_list_head, &mbr_if->pc_lentry);
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// Tunnel If Create 
//------------------------------------------------------------------------------
hal_ret_t
tunnel_if_create (InterfaceSpec& spec, InterfaceResponse *rsp, 
                  if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("PI-TunnelIF:{}: TunnelIF Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());
    hal_if->tid = spec.meta().tenant_id();
    auto if_tunnel_info = spec.if_tunnel_info();
    hal_if->encap_type = if_tunnel_info.encap_type();
    /* Both Local TEP And remote TEP have to v4 or v6 */
    if ((if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
        !if_tunnel_info.vxlan_info().remote_tep().v4_addr()) || 
        (!if_tunnel_info.vxlan_info().local_tep().v4_addr() &&
          if_tunnel_info.vxlan_info().remote_tep().v4_addr())) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_OK);
        rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
        rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
    }
    if (hal_if->encap_type ==
            intf::IfTunnelEncapType::IF_TUNNEL_ENCAP_TYPE_VXLAN) {
        ip_addr_spec_to_ip_addr(&hal_if->vxlan_ltep,
                                if_tunnel_info.vxlan_info().local_tep());
        ip_addr_spec_to_ip_addr(&hal_if->vxlan_rtep,
                                if_tunnel_info.vxlan_info().remote_tep());
    }
    return ret;
}

//------------------------------------------------------------------------------
// Get lif handle
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_for_enic_if (InterfaceSpec& spec, InterfaceResponse *rsp, 
                            if_t *hal_if)
{
    lif_id_t            lif_id = 0;
    hal_handle_t        lif_handle = 0;
    lif_t               *lif = NULL;
    hal_ret_t           ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_enic_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == intf::LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("PI-Enicif:{}: LIF handle not found for ID:{} HDL:{}",
                      __FUNCTION__, lif_id, lif_handle);
        rsp->set_api_status(types::API_STATUS_LIF_NOT_FOUND);
         ret = HAL_RET_LIF_NOT_FOUND;
         goto end;
    } else {
        hal_if->lif_handle = lif->hal_handle;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// Get lif handle for CPU If
//------------------------------------------------------------------------------
hal_ret_t
get_lif_handle_for_cpu_if (InterfaceSpec& spec, InterfaceResponse *rsp, 
                          if_t *hal_if)
{
    lif_id_t        lif_id = 0;
    hal_handle_t    lif_handle = 0;
    lif_t           *lif = NULL;
    hal_ret_t       ret = HAL_RET_OK;

    // fetch the lif associated with this interface
    auto lif_kh = spec.if_cpu_info().lif_key_or_handle();
    if (lif_kh.key_or_handle_case() == intf::LifKeyHandle::kLifId) {
        lif_id = lif_kh.lif_id();
        lif = find_lif_by_id(lif_id);
    } else {
        lif_handle = lif_kh.lif_handle();
        lif = find_lif_by_handle(lif_handle);
    }

    if (lif == NULL) {
        HAL_TRACE_ERR("PI-CPUif:{}: LIF handle not found for ID:{} HDL:{}",
                      __FUNCTION__, lif_id, lif_handle);
        rsp->set_api_status(types::API_STATUS_LIF_NOT_FOUND);
         ret = HAL_RET_LIF_NOT_FOUND;
         goto end;
    } else {
        hal_if->lif_handle = lif->hal_handle;
    }

end:

    return ret;
}

#if 0
// TODO: following APIs need to be cleaned up.. they dont use types.APIStatus
//       and give out return codes that user has no way to interpret !!!
void
LifGetQState (const intf::QStateGetReq &req, intf::QStateGetResp *rsp)
{
    std::unique_ptr<char[]> buf(new char[kMaxQStateSize]);
    uint32_t size_to_copy = std::min(kMaxQStateSize, req.ret_data_size());
    int64_t ret = g_lif_manager->GetLIFQStateAddr(
        req.lif_handle(), req.type_num(), req.qid());

    if (ret < 0) {
        rsp->set_error_code(0 - (int)ret);
        return;
    }
    rsp->set_q_addr((uint64_t)ret);
    int ret2 = g_lif_manager->ReadQState(req.lif_handle(), req.type_num(),
                                         req.qid(), (uint8_t *)buf.get(),
                                         kMaxQStateSize);
    if (ret2 < 0) {
        rsp->set_error_code(0 - ret2);
        return;
    }
    rsp->set_error_code(0);
    if (size_to_copy == 0)
        return;
    rsp->set_queue_state(std::string(buf.get(), size_to_copy));
}

void
LifSetQState (const intf::QStateSetReq &req, intf::QStateSetResp *rsp)
{
    if (req.queue_state().size() == 0) {
        rsp->set_error_code(EINVAL);
        return;
    }
    std::unique_ptr<uint8_t[]> buf;
    const uint8_t *state = (const uint8_t *)req.queue_state().c_str();
    if (req.has_label()) {
        uint8_t off;
        int ret = g_lif_manager->GetPCOffset(req.label().handle().c_str(),
            req.label().prog_name().c_str(),
            req.label().label().c_str(), &off);
        if (ret < 0) {
            rsp->set_error_code(0 - ret);
            return;
        }
        buf.reset(new uint8_t[req.queue_state().size()]);
        bcopy(req.queue_state().c_str(), buf.get(), req.queue_state().size());
        buf.get()[0] = off;
        state = buf.get();
    }

    int ret = g_lif_manager->WriteQState(req.lif_handle(), req.type_num(),
                                         req.qid(), state,
                                         req.queue_state().size());
    rsp->set_error_code(0 - ret);
}

#endif
hal_ret_t
if_handle_nwsec_update (l2seg_t *l2seg, if_t *hal_if, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_if_nwsec_upd_args_t  args;

    HAL_TRACE_DEBUG("{}: if_id: {}", __FUNCTION__, hal_if->if_id);
    pd::pd_if_nwsec_upd_args_init(&args);
    args.l2seg = l2seg;
    args.intf = hal_if;
    args.nwsec_prof = nwsec_prof;

    ret = pd::pd_if_nwsec_update(&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{}: PD call for nwsec update on if failed. ret: {}", 
                __FUNCTION__, ret);
        goto end;
    }

end:
    return ret;
}





}    // namespace hal
