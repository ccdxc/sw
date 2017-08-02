#include <base.h>
#include <hal.hpp>
#include <hal_state.hpp>
#include <interface_svc.hpp>
#include <interface.hpp>
#include <pd.hpp>
#include <pd_api.hpp>
#include <l2segment.pb.h>

namespace hal {

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
lif_get_handle_key_func(void *entry)
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
if_get_hw_key_func(void *entry)
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
if_get_handle_key_func(void *entry)
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

//------------------------------------------------------------------------------
// process a interface create request
// TODO: if interface already exists, treat it as modify
//------------------------------------------------------------------------------
hal_ret_t
lif_create (LifSpec& spec, LifResponse *rsp)
{
    hal_ret_t            ret;
    lif_t                *lif = NULL;
    pd::pd_lif_args_t    pd_lif_args;

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
    lif->allmulti = spec.allmulti();

    // allocate all PD resources and finish programming, if any
    pd::pd_lif_args_init(&pd_lif_args);
    pd_lif_args.lif = lif;
    ret = pd::pd_lif_create(&pd_lif_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD lif create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this lif to the db
    add_lif_to_db(lif);
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_lif_status(lif->admin_status);
    rsp->mutable_status()->set_lif_handle(lif->hal_handle);

end:

    if (ret != HAL_RET_OK && lif != NULL) {
        lif_free(lif);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// Fetch If from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static if_t *
fetch_if_ifl2seg(InterfaceL2SegmentSpec& spec)
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
// Fetch L2 Segment from InterfaceL2SegmentSpec
//------------------------------------------------------------------------------
static l2seg_t *
fetch_l2seg_ifl2seg(InterfaceL2SegmentSpec& spec)
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
// validate l2 segment on uplink
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

    // Uplink key/hanle is must
    if (!spec.has_if_key_handle()) {
        rsp->set_api_status(types::API_STATUS_INTERFACE_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // Uplink has to exist
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

#if 0
    // LIF key/handle is a must
    if (!spec.has_lif_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_IF_LIF_INFO_NOT_INVALID);
        return HAL_RET_INVALID_ARG;
    }
#endif

    // interface type must be valid
    if_type = spec.type();
    if (if_type == intf::IF_TYPE_NONE) {
        rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (if_type == intf::IF_TYPE_ENIC) {
        // fetch the tenant information
        tid = spec.meta().tenant_id();
        tenant = find_tenant_by_id(tid);
        if (tenant == NULL) {
            HAL_TRACE_ERR("PI-IF:{}: Valid. failed. TenId: {} Err: {} ApiStatus: {}",
                    __FUNCTION__, tid, HAL_RET_INVALID_ARG, rsp->api_status());
            rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
            return HAL_RET_INVALID_ARG;
        }

        // make sure ENIC info is provided
        if (!spec.has_if_enic_info()) {
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

        // Check if lif is provided
        if (!spec.if_enic_info().has_lif_key_or_handle()) {
            rsp->set_api_status(types::API_STATUS_IF_ENIC_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }

    } else if (if_type == intf::IF_TYPE_UPLINK) {
        // Uplink specific validation
        if (!spec.has_if_uplink_info()) {
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_UPLINK_PC) {
        // Uplink PC specific validation
        if (!spec.has_if_uplink_pc_info()) {
            rsp->set_api_status(types::API_STATUS_IF_INFO_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    } else if (if_type == intf::IF_TYPE_TUNNEL) {
        // Tunnel specification validation
        if (!spec.has_if_tunnel_info()) {
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

//------------------------------------------------------------------------------
// process a interface create request
// TODO: if interface already exists, treat it as modify
//------------------------------------------------------------------------------
hal_ret_t
interface_create (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    int                 i;
    hal_ret_t           ret = HAL_RET_OK;
    if_t                *hal_if = NULL;
    l2seg_id_t          l2seg_id;
    l2seg_t             *l2seg;
    indexer::status     rs;
    pd::pd_if_args_t    pd_if_args;


    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-IF:{}: If Create for id {} Type: {} EnicType: {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id(), spec.type(), 
                    (spec.type() == intf::IF_TYPE_ENIC) ? 
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
            ret = enicif_create(spec, rsp, hal_if);
            if (ret != HAL_RET_OK) {
                goto end;
            }
            break;

        case intf::IF_TYPE_UPLINK:
            ret = uplinkif_create(spec, rsp, hal_if);
            if (ret != HAL_RET_OK) {
                goto end;
            }
            break;

        case intf::IF_TYPE_UPLINK_PC:
            hal_if->uplink_pc_num = spec.if_uplink_pc_info().uplink_pc_num();
            hal_if->uplink_port_num = HAL_PORT_INVALID;
            hal_if->native_l2seg = spec.if_uplink_pc_info().native_l2segment_id();
            hal_if->vlans = bitmap::factory(4096);
            if (hal_if->vlans == NULL) {
                ret = HAL_RET_OOM;
                rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
                goto end;
            }
            for (i = 0; i < spec.if_uplink_pc_info().l2segment_id_size(); i++) {
                l2seg_id = spec.if_uplink_pc_info().l2segment_id(i);
                l2seg = find_l2seg_by_id(l2seg_id);
                if (l2seg == NULL) {
                    ret = HAL_RET_L2SEG_NOT_FOUND;
                    rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
                    goto end;
                }
                if (l2seg->access_encap.type == types::ENCAP_TYPE_DOT1Q) {
                    hal_if->vlans->set(l2seg->access_encap.val);
                }
            }
            break;

        case intf::IF_TYPE_TUNNEL:
            HAL_TRACE_DEBUG("Tunnel create: NOT IMPLEMENTED. Returning success.");
            hal_if->hal_handle = hal_alloc_handle();
            // prepare the response
            rsp->set_api_status(types::API_STATUS_OK);
            rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
            rsp->mutable_status()->set_if_handle(hal_if->hal_handle);
            break;

        default:
            ret = HAL_RET_INVALID_ARG;
            rsp->set_api_status(types::API_STATUS_OK);
            rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
            rsp->set_api_status(types::API_STATUS_IF_TYPE_INVALID);
            goto end;
    }

    // Bharat: Why do we need this? TODO: Clean this up
    hal_if->hal_handle = hal_alloc_handle();
    rs = g_hal_state->if_hwid_idxr()->alloc((uint32_t *)&hal_if->hw_id);
    if (rs != indexer::SUCCESS) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_RESOURCE);
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

#if 0
    if ((hal_if->if_type == intf::IF_TYPE_UPLINK) ||
        (hal_if->if_type == intf::IF_TYPE_UPLINK_PC)) {
        for (i = 0; i < spec.l2_segment_id_size(); i++) {
            process_if_l2seg_up_on_uplink(hal_if, spec.l2_segment_id(i));
        }
    }
#endif

    // allocate all PD resources and finish programming, if any
    pd::pd_if_args_init(&pd_if_args);
    pd_if_args.intf = hal_if;
    ret = pd::pd_if_create(&pd_if_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD interface create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this interface to the db
    add_if_to_db(hal_if);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_if_status(hal_if->if_admin_status);
    rsp->mutable_status()->set_if_handle(hal_if->hal_handle);

end:

    if (ret != HAL_RET_OK && hal_if != NULL) {
        if_free(hal_if);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a interface update request
//------------------------------------------------------------------------------
hal_ret_t
interface_update (InterfaceSpec& spec, InterfaceResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a interface get request
//------------------------------------------------------------------------------
hal_ret_t
interface_get (InterfaceGetRequest& spec, InterfaceGetResponse *rsp)
{
    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// add l2segment on Uplink
//------------------------------------------------------------------------------
hal_ret_t add_l2seg_on_uplink(InterfaceL2SegmentSpec& spec,
                              InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    if_t                        *hal_if = NULL;
    l2seg_t                     *l2seg = NULL;
    pd::pd_l2seg_uplink_args_t  pd_l2seg_uplink_args;

    // Validate if l2seg and uplink exists
    ret = validate_l2seg_on_uplink(spec, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // Fetch L2 Segment and Uplink If
    hal_if = fetch_if_ifl2seg(spec);
    l2seg = fetch_l2seg_ifl2seg(spec);
    HAL_ASSERT((hal_if != NULL) && (l2seg != NULL));


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

cleanup:
    return ret;
}


//------------------------------------------------------------------------------
// del l2segment to Uplink
//------------------------------------------------------------------------------
hal_ret_t del_l2seg_on_uplink(InterfaceL2SegmentSpec& spec,
                              InterfaceL2SegmentResponse *rsp)
{
    hal_ret_t               ret = HAL_RET_OK;

    return ret;
}


//------------------------------------------------------------------------------
// Enic If Create 
//------------------------------------------------------------------------------
hal_ret_t enicif_create(InterfaceSpec& spec, InterfaceResponse *rsp, 
                        if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    // l2seg_id_t          l2seg_id;
    // l2seg_t             *l2seg;
    lif_t               *lif;

    HAL_TRACE_DEBUG("PI-Enicif:{}: Enicif Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    // lif for enicif ... rsp is updated within the call
    ret = get_lif_hdl_for_enicif(spec, rsp, hal_if);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Enicif:{}: Unable to find the lif handle Err: {}", ret);
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

// end:
    return ret;
}

//------------------------------------------------------------------------------
// Uplink If Create 
//------------------------------------------------------------------------------
hal_ret_t uplinkif_create(InterfaceSpec& spec, InterfaceResponse *rsp, 
                        if_t *hal_if)
{
    hal_ret_t           ret = HAL_RET_OK;
    l2seg_id_t          l2seg_id;
    l2seg_t             *l2seg;

    HAL_TRACE_DEBUG("PI-Uplinkif:{}: Uplinkif Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().interface_id());

    // TODO: for a member port, we can have valid pc#
    hal_if->uplink_port_num = spec.if_uplink_info().port_num();
    hal_if->uplink_pc_num = HAL_PC_INVALID;
    hal_if->native_l2seg = spec.if_uplink_info().native_l2segment_id();
    hal_if->vlans = bitmap::factory(4096);
    if (hal_if->vlans == NULL) {
        ret = HAL_RET_OOM;
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        HAL_TRACE_ERR("PI-Uplinkif:{}: Out of Memory Err: {}", ret);
        goto end;
    }
    for (int i = 0; i < spec.if_uplink_info().l2segment_id_size(); i++) {
        l2seg_id = spec.if_uplink_info().l2segment_id(i);
        l2seg = find_l2seg_by_id(l2seg_id);
        if (l2seg == NULL) {
            ret = HAL_RET_L2SEG_NOT_FOUND;
            rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
            HAL_TRACE_ERR("PI-Uplinkif:{}: L2Seg id {} not found Err: {}", 
                          l2seg_id, ret);
            goto end;
        }
        if (l2seg->access_encap.type == types::ENCAP_TYPE_DOT1Q) {
            hal_if->vlans->set(l2seg->access_encap.val);
        }
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Get lif handle
//------------------------------------------------------------------------------
hal_ret_t get_lif_hdl_for_enicif(InterfaceSpec& spec, InterfaceResponse *rsp, 
                                 if_t *hal_if)
{
    lif_id_t            lif_id;
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

    if(lif == NULL) {
        rsp->set_api_status(types::API_STATUS_LIF_NOT_FOUND);
         ret = HAL_RET_LIF_NOT_FOUND;
         goto end;
    } else {
        hal_if->lif_handle = lif->hal_handle;
    }

end:
    return ret;
}


}    // namespace hal
