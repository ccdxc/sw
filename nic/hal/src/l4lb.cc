#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/vrf.hpp"
#include "nic/hal/src/l4lb.hpp"
#include "nic/include/eth.h"
#include <netinet/ether.h>
#include "nic/include/pd_api.hpp"

namespace hal {

void *
l4lb_get_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((l4lb_service_entry_t *)entry)->l4lb_key);
}

uint32_t
l4lb_compute_key_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(l4lb_key_t)) % ht_size;
}

bool
l4lb_compare_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(l4lb_key_t))) {
        return true;
    }
    return false;
}

void *
l4lb_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((l4lb_service_entry_t*)entry)->hal_handle);
}

uint32_t
l4lb_compute_handle_hash_func(void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
l4lb_compare_handle_key_func(void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an l4lb create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_l4lbservice_create (l4lb::L4LbServiceSpec& spec, l4lb::L4LbServiceResponse *rsp)
{
    if (!spec.has_meta() ||
        spec.meta().vrf_id() == HAL_VRF_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_VRF_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_L4LB_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // key-handle field set, but create requires key, not handle
    if (spec.key_or_handle().key_or_handle_case() !=
            L4LbServiceKeyHandle::kServiceKey) {
        rsp->set_api_status(types::API_STATUS_L4LB_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a l4lb service request
//------------------------------------------------------------------------------
//
hal_ret_t 
l4lbservice_create (l4lb::L4LbServiceSpec& spec, l4lb::L4LbServiceResponse *rsp) 
{
    hal_ret_t                   ret;
    vrf_id_t                    tid;
    vrf_t                       *vrf = NULL;
    l4lb_service_entry_t        *l4lb = NULL;
    pd::pd_l4lb_create_args_t   pd_l4lb_args = {0};

    HAL_TRACE_DEBUG("PI-L4LBService:{}: L4LB Create for vrf: {}", __FUNCTION__, 
                    spec.meta().vrf_id());


    ret = validate_l4lbservice_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the vrf information
    tid = spec.meta().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        goto end;
    }

    // allocate memory for the l4lb
    l4lb = (l4lb_service_entry_t *)g_hal_state->l4lb_slab()->alloc();
    if (l4lb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // copy the key
    ip_addr_spec_to_ip_addr(&l4lb->l4lb_key.service_ip, 
            spec.key_or_handle().service_key().service_ip_address());
    l4lb->l4lb_key.proto = spec.key_or_handle().service_key().ip_protocol();
    l4lb->l4lb_key.service_port = 
        spec.key_or_handle().service_key().service_port();

    l4lb->l4lb_key.vrf_id = tid;
    MAC_UINT64_TO_ADDR(l4lb->serv_mac_addr, spec.service_mac());
    l4lb->proxy_arp_en = spec.proxy_arp_enable();
    l4lb->sess_aff = spec.session_affinity();

    HAL_TRACE_DEBUG("PI-L4LBService:{}: {}", __FUNCTION__, l4lb_to_str(l4lb));

    // TODO: PD Call if needed
    // PD Call
    pd::pd_l4lb_create_args_init(&pd_l4lb_args);
    pd_l4lb_args.l4lb = l4lb;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_L4LB_CREATE, (void *)&pd_l4lb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD L4Lb create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // handle is allocated only after PD programming is SUCCESS
    l4lb->hal_handle = hal_alloc_handle();
    l4lb->hal_handle_ht_ctxt.reset();
    g_hal_state->l4lb_hal_handle_ht()->insert(l4lb, &l4lb->hal_handle_ht_ctxt);

    // add l4lb key into the hash table
    l4lb->l4lbkey_ht_ctxt.reset();
    g_hal_state->l4lb_ht()->insert(l4lb, &l4lb->l4lbkey_ht_ctxt);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_service_handle(l4lb->hal_handle);

end:

    return ret;
}
    
//------------------------------------------------------------------------------
// process a l4lb update request
//------------------------------------------------------------------------------
hal_ret_t
l4lbservice_update (l4lb::L4LbServiceSpec& spec, l4lb::L4LbServiceResponse *rsp)
{
    return HAL_RET_OK;
}

const char *
l4lb_to_str (l4lb_service_entry_t *l4lb)
{
    static thread_local char       ep_str[4][100];
    static thread_local uint8_t    ep_str_next = 0;
    char                           *buf;

    buf = ep_str[ep_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (l4lb) {
        snprintf(buf, 100, "ten_id:%lu, serv_ip:%s, proto:%d, serv_port:%d, serv_mac:%s",
                l4lb->l4lb_key.vrf_id, ipaddr2str(&l4lb->l4lb_key.service_ip),
                l4lb->l4lb_key.proto, l4lb->l4lb_key.service_port, 
                ether_ntoa((struct ether_addr*)(l4lb->serv_mac_addr)));
    }
    return buf;
}

}    // namespace hal

