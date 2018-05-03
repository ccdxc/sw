#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/include/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/ipsec/ipsec.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/export/vrf_api.hpp"
#include "nic/hal/src/utils/utils.hpp"

namespace hal {
void *
ipsec_sa_encrypt_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipsec_sa_t *)entry)->sa_id);
}

uint32_t
ipsec_sa_encrypt_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(ipsec_sa_id_t)) % ht_size;
}

bool
ipsec_sa_encrypt_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(ipsec_sa_id_t *)key1 == *(ipsec_sa_id_t *)key2) {
        return true;
    }
    return false;
}

void *
ipsec_sa_encrypt_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipsec_sa_t *)entry)->hal_handle);
}

uint32_t
ipsec_sa_encrypt_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
ipsec_sa_encrypt_compare_handle_key_func (void *key1, void *key2)
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
validate_ipsec_sa_encrypt_create (IpsecSAEncrypt& spec, IpsecSAEncryptResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    auto kh = spec.key_or_handle();
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            IpsecSAEncryptKeyHandle::kCbId) {
        rsp->set_api_status(types::API_STATUS_IPSEC_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this IPSEC CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_ipsec_sa_to_db (ipsec_sa_t *ipsec)
{
    g_hal_state->ipseccb_id_ht()->insert(ipsec, &ipsec->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB create request
// TODO: if IPSEC CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
ipsec_saencrypt_create (IpsecSAEncrypt& spec, IpsecSAEncryptResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipsec_sa_t                *ipsec;
    pd::pd_ipsec_sa_create_args_t    pd_ipsec_sa_args;
    ep_t *sep, *dep;
    mac_addr_t *smac = NULL, *dmac = NULL;
    vrf_t   *vrf;
    vrf_id_t tid = 0;
    mac_addr_t smac1 = {0x0, 0xee, 0xff, 0x0, 0x0, 0x02};
    mac_addr_t dmac1 = {0x0, 0xee, 0xff, 0x0, 0x0, 0x03};

    // validate the request message
    ret = validate_ipsec_sa_encrypt_create(spec, rsp);

    ipsec = ipsec_sa_alloc_init();
    if (ipsec == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    ipsec->sa_id = spec.key_or_handle().cb_id();

    ipsec->iv = spec.iv();
    ipsec->iv_salt = spec.salt();
    ipsec->spi = spec.spi();
    //ipsec->key_index = spec.key_index();

    ip_addr_spec_to_ip_addr(&ipsec->tunnel_sip4, spec.local_gateway_ip());
    ip_addr_spec_to_ip_addr(&ipsec->tunnel_dip4, spec.remote_gateway_ip());

    vrf = vrf_get_infra_vrf();
    if (vrf) {
        tid = vrf->vrf_id;
        HAL_TRACE_DEBUG("infra_vrf success tid = {}", tid);
    }

    sep = find_ep_by_v4_key(tid, htonl(ipsec->tunnel_sip4.addr.v4_addr));
    if (sep) {
        smac = ep_get_mac_addr(sep);
        if (smac) {
            memcpy(ipsec->smac, smac, ETH_ADDR_LEN);
        }
    } else {
        memcpy(ipsec->smac, smac1, ETH_ADDR_LEN);
        HAL_TRACE_DEBUG("Src EP Lookup failed \n");
    }
    dep = find_ep_by_v4_key(tid, htonl(ipsec->tunnel_dip4.addr.v4_addr));
    if (dep) {
        dmac = ep_get_mac_addr(dep);
        if (dmac) {
            memcpy(ipsec->dmac, dmac, ETH_ADDR_LEN);
        }
    } else {
        memcpy(ipsec->dmac, dmac1, ETH_ADDR_LEN);
        HAL_TRACE_DEBUG("Dest EP Lookup failed\n");
    }

    ipsec->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_ipsec_sa_create_args_init(&pd_ipsec_sa_args);
    pd_ipsec_sa_args.ipsec_sa = ipsec;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_CREATE, (void *)&pd_ipsec_sa_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_CREATE,
                          (void *)&pd_ipsec_sa_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB decrypt create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_ipsec_sa_to_db(ipsec);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_ipsec_sa_status()->set_ipsec_sa_handle(ipsec->hal_handle);

    return HAL_RET_OK;

cleanup:

    ipsec_sa_free(ipsec);
    return ret;
}

//------------------------------------------------------------------------------
// process a IPSEC CB update request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_saencrypt_update (IpsecSAEncrypt& spec, IpsecSAEncryptResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipsec_sa_t*               ipsec;
    pd::pd_ipsec_sa_update_args_t    pd_ipsec_sa_args;
    ep_t *sep, *dep;
    mac_addr_t *smac = NULL, *dmac = NULL;
    vrf_t   *vrf;
    vrf_id_t tid;

    auto kh = spec.key_or_handle();

    ipsec = find_ipsec_sa_by_id(kh.cb_id());
    if (ipsec == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }

    pd::pd_ipsec_sa_update_args_init(&pd_ipsec_sa_args);
    pd_ipsec_sa_args.ipsec_sa = ipsec;

    ipsec->iv = spec.iv();
    ipsec->iv_salt = spec.salt();
    ipsec->spi = spec.spi();

    ip_addr_spec_to_ip_addr(&ipsec->tunnel_sip4, spec.local_gateway_ip());
    ip_addr_spec_to_ip_addr(&ipsec->tunnel_dip4, spec.remote_gateway_ip());

    vrf = vrf_get_infra_vrf();
    if (vrf) {
        tid = vrf->vrf_id;
        HAL_TRACE_DEBUG("infra_vrf success tid = {}", tid);
    } else {
    }
    sep = find_ep_by_v4_key(tid, htonl(ipsec->tunnel_sip4.addr.v4_addr));
    if (sep) {
        smac = ep_get_mac_addr(sep);
        if (smac) {
            memcpy(ipsec->smac, smac, ETH_ADDR_LEN);
        }
    } else {
        HAL_TRACE_DEBUG("Src EP Lookup failed \n");
    }
    dep = find_ep_by_v4_key(tid, htonl(ipsec->tunnel_dip4.addr.v4_addr));
    if (dep) {
        dmac = ep_get_mac_addr(dep);
        if (dmac) {
            memcpy(ipsec->dmac, dmac, ETH_ADDR_LEN);
        }
    } else {
        HAL_TRACE_DEBUG("Dest EP Lookup failed\n");
    }
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_UPDATE, (void *)&pd_ipsec_sa_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE,
                          (void *)&pd_ipsec_sa_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB get request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_saencrypt_get (IpsecSAEncryptGetRequest& req, IpsecSAEncryptGetResponseMsg *resp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipsec_sa_t                ripsec;
    ipsec_sa_t*               ipsec;
    pd::pd_ipsec_sa_get_args_t    pd_ipsec_sa_args;
    IpsecSAEncryptGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    ipsec = find_ipsec_sa_by_id(kh.cb_id());
    if (ipsec == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }

    ipsec_sa_init(&ripsec);
    ripsec.sa_id = ipsec->sa_id;
    pd::pd_ipsec_sa_get_args_init(&pd_ipsec_sa_args);
    pd_ipsec_sa_args.ipsec_sa = &ripsec;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_GET, (void *)&pd_ipsec_sa_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this IPSEC CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_cb_id(ripsec.sa_id);

    rsp->mutable_spec()->set_salt(ripsec.iv_salt);
    rsp->mutable_spec()->set_iv(ripsec.iv);
    rsp->mutable_spec()->set_spi(ripsec.spi);

    //rsp->mutable_spec()->set_tunnel_sip4(ripsec.tunnel_sip4);
    //rsp->mutable_spec()->set_tunnel_dip4(ripsec.tunnel_dip4);

    // fill operational state of this IPSEC CB
    rsp->mutable_status()->set_ipsec_sa_handle(ipsec->hal_handle);

    // fill stats of this IPSEC CB
    rsp->set_api_status(types::API_STATUS_OK);
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_GET,
                          (void *)&pd_ipsec_sa_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Decrypt IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_saencrypt_delete (ipsec::IpsecSAEncryptDeleteRequest& req, ipsec::IpsecSAEncryptDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipsec_sa_t*               ipsec;
    pd::pd_ipsec_sa_delete_args_t    pd_ipsec_sa_args;

    auto kh = req.key_or_handle();
    ipsec = find_ipsec_sa_by_id(kh.cb_id());
    if (ipsec == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_ipsec_sa_delete_args_init(&pd_ipsec_sa_args);
    pd_ipsec_sa_args.ipsec_sa = ipsec;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DELETE, (void *)&pd_ipsec_sa_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_DELETE,
                          (void *)&pd_ipsec_sa_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }


    // fill stats of this IPSEC CB
    rsp->add_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

}    // namespace hal
