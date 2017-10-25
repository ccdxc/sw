#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/ipseccb.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/include/pd_api.hpp"
#include "utils.hpp"

namespace hal {
void *
ipseccb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipseccb_t *)entry)->cb_id);
}

uint32_t
ipseccb_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(ipseccb_id_t)) % ht_size;
}

bool
ipseccb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(ipseccb_id_t *)key1 == *(ipseccb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
ipseccb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipseccb_t *)entry)->hal_handle);
}

uint32_t
ipseccb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
ipseccb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming IPSECCB create request
// TODO:
// 1. check if IPSECCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_ipseccb_create (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            IpsecCbKeyHandle::kIpseccbId) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this IPSEC CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_ipseccb_to_db (ipseccb_t *ipseccb)
{
    g_hal_state->ipseccb_hal_handle_ht()->insert(ipseccb,
                                               &ipseccb->hal_handle_ht_ctxt);
    g_hal_state->ipseccb_id_ht()->insert(ipseccb, &ipseccb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB create request
// TODO: if IPSEC CB exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_create (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipseccb_t                *ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;
    ep_t *sep, *dep;
    mac_addr_t *smac = NULL, *dmac = NULL;
    l2seg_t *infra_seg;
    hal::tenant_id_t tid;

    // validate the request message
    ret = validate_ipseccb_create(spec, rsp);
    
    ipseccb = ipseccb_alloc_init();
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    ipseccb->cb_id = spec.key_or_handle().ipseccb_id();

    ipseccb->iv_size = spec.iv_size();
    ipseccb->icv_size = spec.icv_size();
    ipseccb->block_size = spec.block_size();
    ipseccb->iv = spec.iv();
    ipseccb->iv_salt = spec.iv_salt();
    ipseccb->esn_hi = spec.esn_hi();
    ipseccb->esn_lo = spec.esn_lo();
    ipseccb->spi = spec.spi();
    ipseccb->key_index = spec.key_index();
    ipseccb->barco_enc_cmd = spec.barco_enc_cmd();

    ipseccb->tunnel_sip4 = spec.tunnel_sip4();
    ipseccb->tunnel_dip4 = spec.tunnel_dip4();

    infra_seg = l2seg_get_infra_l2seg();
    if (infra_seg) {
        tid = hal::tenant_lookup_by_handle(infra_seg->tenant_handle)->tenant_id;
        HAL_TRACE_DEBUG("infra_seg success tid = {}", tid);
    } else {
        tid = 0;
    }
    sep = find_ep_by_v4_key(tid, htonl(spec.tunnel_sip4()));
    if (sep) {
        smac = ep_get_mac_addr(sep);
        if (smac) {
            memcpy(ipseccb->smac, smac, ETH_ADDR_LEN);
        } 
    } else {
        HAL_TRACE_DEBUG("Src EP Lookup failed \n");
    }
    dep = find_ep_by_v4_key(tid, htonl(spec.tunnel_dip4()));
    if (dep) {
        dmac = ep_get_mac_addr(dep);
        if (dmac) {
            memcpy(ipseccb->dmac, dmac, ETH_ADDR_LEN);
        } 
    } else {
        HAL_TRACE_DEBUG("Dest EP Lookup failed\n");
    }
    

    ipseccb->is_v6 = spec.is_v6();
    ip_addr_spec_to_ip_addr(&ipseccb->sip6, spec.sip6()); 
    ip_addr_spec_to_ip_addr(&ipseccb->dip6, spec.dip6());
     
    HAL_TRACE_DEBUG("SIP6 : {}  DIP6: {} \n", ipaddr2str(&ipseccb->sip6), ipaddr2str(&ipseccb->dip6)); 
     
    ipseccb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    ret = pd::pd_ipseccb_create(&pd_ipseccb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }
    ret = pd::pd_ipseccb_decrypt_create(&pd_ipseccb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB decrypt create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_ipseccb_to_db(ipseccb);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_ipseccb_status()->set_ipseccb_handle(ipseccb->hal_handle);
    return HAL_RET_OK;

cleanup:

    ipseccb_free(ipseccb);
    return ret;
}

//------------------------------------------------------------------------------
// process a IPSEC CB update request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_update (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;
    ep_t *sep, *dep;
    mac_addr_t *smac = NULL, *dmac = NULL;
    l2seg_t *infra_seg;
    hal::tenant_id_t tid;

    auto kh = spec.key_or_handle();

    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }
 
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    
    ipseccb->iv_size = spec.iv_size();
    ipseccb->icv_size = spec.icv_size();
    ipseccb->block_size = spec.block_size();
    ipseccb->iv = spec.iv();
    ipseccb->iv_salt = spec.iv_salt();
    ipseccb->esn_hi = spec.esn_hi();
    ipseccb->esn_lo = spec.esn_lo();
    ipseccb->spi = spec.spi();
    ipseccb->key_index = spec.key_index();
    ipseccb->barco_enc_cmd = spec.barco_enc_cmd();

    ipseccb->tunnel_sip4 = spec.tunnel_sip4();
    ipseccb->tunnel_dip4 = spec.tunnel_dip4();
    
    ipseccb->is_v6 = spec.is_v6();
    ip_addr_spec_to_ip_addr(&ipseccb->sip6, spec.sip6()); 
    ip_addr_spec_to_ip_addr(&ipseccb->dip6, spec.dip6());
     
    HAL_TRACE_DEBUG("SIP6 : {}  DIP6: {}\n", ipaddr2str(&ipseccb->sip6), ipaddr2str(&ipseccb->dip6)); 
    infra_seg = l2seg_get_infra_l2seg();
    if (infra_seg) {
        tid = hal::tenant_lookup_by_handle(infra_seg->tenant_handle)->tenant_id;
        HAL_TRACE_DEBUG("infra_seg success tid = {}", tid);
    } else {
        tid = 0;
    }
    sep = find_ep_by_v4_key(tid, (spec.tunnel_sip4()));
    if (sep) {
        smac = ep_get_mac_addr(sep);
        if (smac) {
            memcpy(ipseccb->smac, smac, ETH_ADDR_LEN);
        } 
    } else {
        HAL_TRACE_DEBUG("Src EP Lookup failed \n");
    }
    dep = find_ep_by_v4_key(tid, (spec.tunnel_dip4()));
    if (dep) {
        dmac = ep_get_mac_addr(dep);
        if (dmac) {
            memcpy(ipseccb->dmac, dmac, ETH_ADDR_LEN);
        } 
    } else {
        HAL_TRACE_DEBUG("Dest EP Lookup failed\n");
    }
    ret = pd::pd_ipseccb_update(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    ret = pd::pd_ipseccb_decrypt_update(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    
    rsp->set_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB get request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_get (IpsecCbGetRequest& req, IpsecCbGetResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    ipseccb_t                ripseccb;
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;

    auto kh = req.key_or_handle();

    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }
    
    ipseccb_init(&ripseccb);
    ripseccb.cb_id = ipseccb->cb_id;
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = &ripseccb;
    
    ret = pd::pd_ipseccb_get(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this IPSEC CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_ipseccb_id(ripseccb.cb_id);

    //rsp->mutable_spec()->set_pi(ripseccb.pi);
    //rsp->mutable_spec()->set_ci(ripseccb.ci);


    rsp->mutable_spec()->set_iv_size(ripseccb.iv_size);
    rsp->mutable_spec()->set_icv_size(ripseccb.icv_size);
    rsp->mutable_spec()->set_block_size(ripseccb.block_size);
    rsp->mutable_spec()->set_iv_salt(ripseccb.iv_salt);
    rsp->mutable_spec()->set_iv(ripseccb.iv);
    rsp->mutable_spec()->set_key_index(ripseccb.key_index);
    rsp->mutable_spec()->set_esn_hi(ripseccb.esn_hi);
    rsp->mutable_spec()->set_esn_lo(ripseccb.esn_lo);
    rsp->mutable_spec()->set_spi(ripseccb.spi);
    rsp->mutable_spec()->set_barco_enc_cmd(ripseccb.barco_enc_cmd);
    
    rsp->mutable_spec()->set_tunnel_sip4(ripseccb.tunnel_sip4);
    rsp->mutable_spec()->set_tunnel_dip4(ripseccb.tunnel_dip4);

    rsp->mutable_spec()->set_pi(ripseccb.pi);
    rsp->mutable_spec()->set_ci(ripseccb.ci);
    // fill operational state of this IPSEC CB
    rsp->mutable_status()->set_ipseccb_handle(ipseccb->hal_handle);

    // fill stats of this IPSEC CB
    rsp->set_api_status(types::API_STATUS_OK);
    ret = pd::pd_ipseccb_decrypt_get(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Decrypt IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    rsp->mutable_spec()->set_expected_seq_no(ripseccb.expected_seq_no);
    rsp->mutable_spec()->set_seq_no_bmp(ripseccb.seq_no_bmp);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_delete (ipseccb::IpsecCbDeleteRequest& req, ipseccb::IpsecCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_args_t    pd_ipseccb_args;

    auto kh = req.key_or_handle();
    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }
 
    pd::pd_ipseccb_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    
    ret = pd::pd_ipseccb_delete(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    ret = pd::pd_ipseccb_decrypt_delete(&pd_ipseccb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_IPSEC_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    
    // fill stats of this IPSEC CB
    rsp->add_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

}    // namespace hal
