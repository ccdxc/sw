//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/include/fte.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/ipsec/ipseccb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/vrf_api.hpp"
#include "nic/hal/src/utils/utils.hpp"

namespace hal {
void *
ipseccb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((ipseccb_t *)entry)->cb_id);
}

uint32_t
ipseccb_key_size ()
{
    return sizeof(ipseccb_id_t);
}

void *
ipseccb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((ipseccb_t *)entry)->hal_handle);
}

uint32_t
ipseccb_handle_key_size ()
{
    return sizeof(hal_handle_t);
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
    g_hal_state->ipseccb_id_ht()->insert(ipseccb, &ipseccb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB create request
// TODO: if IPSEC CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_create (IpsecCbSpec& spec, IpsecCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipseccb_t                *ipseccb;
    pd::pd_ipseccb_create_args_t    pd_ipseccb_args;
    pd::pd_ipseccb_decrypt_create_args_t dec_args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    ep_t *sep, *dep;
    mac_addr_t *smac = NULL, *dmac = NULL;
    vrf_t   *vrf;
    vrf_id_t tid = 0;
    mac_addr_t smac1 = {0x0, 0xee, 0xff, 0x0, 0x0, 0x02};
    mac_addr_t dmac1 = {0x0, 0xee, 0xff, 0x0, 0x0, 0x03};

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
    ipseccb->new_spi = spec.new_spi();
    ipseccb->key_index = spec.key_index();
    ipseccb->new_key_index = spec.new_key_index();
    ipseccb->barco_enc_cmd = spec.barco_enc_cmd();

    ipseccb->tunnel_sip4 = spec.tunnel_sip4();
    ipseccb->tunnel_dip4 = spec.tunnel_dip4();

    vrf = vrf_get_infra_vrf();
    if (vrf) {
        tid = vrf->vrf_id;
        HAL_TRACE_DEBUG("infra_vrf success tid = {}", tid);
    }

    sep = find_ep_by_v4_key(tid, htonl(spec.tunnel_sip4()));
    if (sep) {
        smac = ep_get_mac_addr(sep);
        if (smac) {
            memcpy(ipseccb->smac, smac, ETH_ADDR_LEN);
        }
    } else {
        memcpy(ipseccb->smac, smac1, ETH_ADDR_LEN);
        HAL_TRACE_DEBUG("Src EP Lookup failed \n");
    }
    dep = find_ep_by_v4_key(tid, htonl(spec.tunnel_dip4()));
    if (dep) {
        dmac = ep_get_mac_addr(dep);
        if (dmac) {
            memcpy(ipseccb->dmac, dmac, ETH_ADDR_LEN);
        }
    } else {
        memcpy(ipseccb->dmac, dmac1, ETH_ADDR_LEN);
        HAL_TRACE_DEBUG("Dest EP Lookup failed\n");
    }

    ipseccb->vrf_vlan = (int16_t)spec.vrf_vlan();
    ipseccb->is_v6 = spec.is_v6();
    ipseccb->is_nat_t = spec.is_nat_t();
    ipseccb->is_random = spec.is_random();
    ipseccb->extra_pad = spec.extra_pad();
    ip_addr_spec_to_ip_addr(&ipseccb->sip6, spec.sip6());
    ip_addr_spec_to_ip_addr(&ipseccb->dip6, spec.dip6());

    HAL_TRACE_DEBUG("SIP6 : {}  DIP6: {} \n", ipaddr2str(&ipseccb->sip6), ipaddr2str(&ipseccb->dip6));

    ipseccb->hal_handle = hal_alloc_handle();

    uint64_t sess_hdl = 0;
    SessionSpec sess_spec;
    SessionResponse sess_rsp;
    ::google::protobuf::uint32  ip1 = ipseccb->tunnel_sip4;
    ::google::protobuf::uint32  ip2 = ipseccb->tunnel_dip4;

    // allocate all PD resources and finish programming
    pd::pd_ipseccb_create_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;
    pd_func_args.pd_ipseccb_create = &pd_ipseccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }
    dec_args.ipseccb = ipseccb;
    pd_func_args.pd_ipseccb_decrypt_create = &dec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSEC CB decrypt create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_ipseccb_to_db(ipseccb);
    SDK_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_ipseccb_status()->set_ipseccb_handle(ipseccb->hal_handle);

    sess_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip1);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip2);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->set_ip_proto(types::IPPROTO_ESP);
    sess_spec.mutable_initiator_flow()->mutable_flow_key()->mutable_v4_key()->mutable_esp()->set_spi(0);
    sess_spec.mutable_initiator_flow()->mutable_flow_data()->mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);


    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_sip(ip2);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_dip(ip1);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->set_ip_proto(types::IPPROTO_ESP);
    sess_spec.mutable_responder_flow()->mutable_flow_key()->mutable_v4_key()->mutable_esp()->set_spi(0);
    sess_spec.mutable_responder_flow()->mutable_flow_data()->mutable_flow_info()->set_flow_action(session::FLOW_ACTION_ALLOW);
    ret = fte::session_create(sess_spec, &sess_rsp);

    sess_hdl = sess_rsp.mutable_status()->session_handle();
    HAL_TRACE_DEBUG("Session Handle: {}", sess_hdl);

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
    pd::pd_ipseccb_update_args_t    pd_ipseccb_args;
    pd::pd_ipseccb_decrypt_update_args_t   dec_args;
    ep_t *sep, *dep;
    mac_addr_t *smac = NULL, *dmac = NULL;
    vrf_t   *vrf;
    vrf_id_t tid;
    pd::pd_func_args_t pd_func_args = {0};

    auto kh = spec.key_or_handle();

    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }

    pd::pd_ipseccb_update_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;

    ipseccb->iv_size = spec.iv_size();
    ipseccb->icv_size = spec.icv_size();
    ipseccb->block_size = spec.block_size();
    ipseccb->iv = spec.iv();
    ipseccb->iv_salt = spec.iv_salt();
    ipseccb->esn_hi = spec.esn_hi();
    ipseccb->esn_lo = spec.esn_lo();
    ipseccb->spi = spec.spi();
    ipseccb->new_spi = spec.new_spi();
    ipseccb->key_index = spec.key_index();
    ipseccb->new_key_index = spec.new_key_index();
    ipseccb->barco_enc_cmd = spec.barco_enc_cmd();

    ipseccb->tunnel_sip4 = spec.tunnel_sip4();
    ipseccb->tunnel_dip4 = spec.tunnel_dip4();

    ipseccb->vrf_vlan = (uint16_t)spec.vrf_vlan();
    ipseccb->is_v6 = spec.is_v6();
    ipseccb->is_nat_t = spec.is_nat_t();
    ipseccb->is_random = spec.is_random();
    ipseccb->extra_pad = spec.extra_pad();
    ip_addr_spec_to_ip_addr(&ipseccb->sip6, spec.sip6());
    ip_addr_spec_to_ip_addr(&ipseccb->dip6, spec.dip6());

    HAL_TRACE_DEBUG("SIP6 : {}  DIP6: {}\n", ipaddr2str(&ipseccb->sip6), ipaddr2str(&ipseccb->dip6));
    vrf = vrf_get_infra_vrf();
    if (vrf) {
        tid = vrf->vrf_id;
        HAL_TRACE_DEBUG("infra_vrf success tid = {}", tid);
    } else {
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
    pd_func_args.pd_ipseccb_update = &pd_ipseccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    dec_args.ipseccb = ipseccb;
    pd_func_args.pd_ipseccb_decrypt_update = &dec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_UPDATE, &pd_func_args);
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
ipseccb_get (IpsecCbGetRequest& req, IpsecCbGetResponseMsg *resp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipseccb_t                ripseccb;
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_get_args_t    pd_ipseccb_args;
    pd::pd_ipseccb_decrypt_get_args_t  dec_args;
    pd::pd_func_args_t          pd_func_args = {0};
    IpsecCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_IPSEC_CB_NOT_FOUND;
    }

    ipseccb_init(&ripseccb);
    ripseccb.cb_id = ipseccb->cb_id;
    pd::pd_ipseccb_get_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = &ripseccb;
    dec_args.ipseccb = &ripseccb;

    pd_func_args.pd_ipseccb_get = &pd_ipseccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this IPSEC CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_ipseccb_id(ripseccb.cb_id);

    rsp->mutable_spec()->set_iv_size(ripseccb.iv_size);
    rsp->mutable_spec()->set_icv_size(ripseccb.icv_size);
    rsp->mutable_spec()->set_block_size(ripseccb.block_size);
    rsp->mutable_spec()->set_iv_salt(ripseccb.iv_salt);
    rsp->mutable_spec()->set_iv(ripseccb.iv);
    rsp->mutable_spec()->set_key_index(ripseccb.key_index);
    rsp->mutable_spec()->set_new_key_index(ripseccb.new_key_index);
    rsp->mutable_spec()->set_esn_hi(ripseccb.esn_hi);
    rsp->mutable_spec()->set_esn_lo(ripseccb.esn_lo);
    rsp->mutable_spec()->set_spi(ripseccb.spi);
    rsp->mutable_spec()->set_new_spi(ripseccb.new_spi);
    rsp->mutable_spec()->set_barco_enc_cmd(ripseccb.barco_enc_cmd);

    rsp->mutable_spec()->set_tunnel_sip4(ripseccb.tunnel_sip4);
    rsp->mutable_spec()->set_tunnel_dip4(ripseccb.tunnel_dip4);

    rsp->mutable_spec()->set_pi(ripseccb.pi);
    rsp->mutable_spec()->set_ci(ripseccb.ci);
    rsp->mutable_spec()->set_is_v6(ripseccb.is_v6);
    rsp->mutable_spec()->set_is_nat_t(ripseccb.is_nat_t);
    rsp->mutable_spec()->set_is_random(ripseccb.is_random);
    rsp->mutable_spec()->set_extra_pad(ripseccb.extra_pad);
    rsp->mutable_spec()->set_vrf_vlan(ripseccb.vrf_vlan);

    // fill operational state of this IPSEC CB
    rsp->mutable_status()->set_ipseccb_handle(ipseccb->hal_handle);

    // fill stats of this IPSEC CB
    rsp->set_api_status(types::API_STATUS_OK);
    pd_func_args.pd_ipseccb_decrypt_get = &dec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD Decrypt IPSECCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    rsp->mutable_spec()->set_expected_seq_no(ripseccb.expected_seq_no);
    rsp->mutable_spec()->set_seq_no_bmp(ripseccb.seq_no_bmp);
    rsp->mutable_spec()->set_last_replay_seq_no(ripseccb.last_replay_seq_no);
    // fillup stats
    rsp->mutable_spec()->set_rx_pkts(ripseccb.h2n_rx_pkts+ripseccb.n2h_rx_pkts);
    rsp->mutable_spec()->set_rx_bytes(ripseccb.h2n_rx_bytes+ripseccb.n2h_rx_bytes);
    rsp->mutable_spec()->set_rx_drops(ripseccb.h2n_rx_drops+ripseccb.n2h_rx_drops);
    rsp->mutable_spec()->set_tx_pkts(ripseccb.h2n_tx_pkts+ripseccb.n2h_tx_pkts);
    rsp->mutable_spec()->set_tx_bytes(ripseccb.h2n_tx_bytes+ripseccb.n2h_tx_bytes);
    rsp->mutable_spec()->set_tx_drops(ripseccb.h2n_tx_drops+ripseccb.n2h_tx_drops);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
ipseccb_delete (ipsec::IpsecCbDeleteRequest& req, ipsec::IpsecCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    ipseccb_t*               ipseccb;
    pd::pd_ipseccb_delete_args_t    pd_ipseccb_args;
    pd::pd_ipseccb_decrypt_delete_args_t dec_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    ipseccb = find_ipseccb_by_id(kh.ipseccb_id());
    if (ipseccb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_ipseccb_delete_args_init(&pd_ipseccb_args);
    pd_ipseccb_args.ipseccb = ipseccb;

    pd_func_args.pd_ipseccb_delete = &pd_ipseccb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD IPSECCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    dec_args.ipseccb = ipseccb;
    pd_func_args.pd_ipseccb_decrypt_delete = &dec_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_IPSECCB_DECRYPT_DELETE, &pd_func_args);
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
