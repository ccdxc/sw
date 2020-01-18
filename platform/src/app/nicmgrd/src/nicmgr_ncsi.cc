//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <sys/stat.h>
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"

#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "platform/src/lib/devapi_iris/devapi_mem.hpp"
#include "nicmgr_ncsi.hpp"

extern DeviceManager *devmgr;

namespace nicmgr {


sdk_ret_t
ncsi_ipc_vlan_filter (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    VlanFilterRequest *req = dynamic_cast<VlanFilterRequest *>(msg->msg);
    VlanFilterResponse *rsp = dynamic_cast<VlanFilterResponse *>(msg->rsp);

    NIC_LOG_DEBUG("NCSI Vlan filter: channel: {}, vlan: {}, oper: {}", 
                  req->channel(), req->vlan_id(), msg->oper);
    if (msg->oper == hal::NCSI_MSG_OPER_CREATE) {
        devmgr->DevApi()->swm_add_vlan(req->vlan_id(), req->channel());
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_del_vlan(req->vlan_id(), req->channel());
    }

    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

sdk_ret_t
ncsi_ipc_vlan_filter_get (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::set<channel_info_t *> channels_info;
    channel_info_t *cinfo;
    VlanFilterGetResponseMsg *rsp_msg = dynamic_cast<VlanFilterGetResponseMsg *>(msg->rsp);
    VlanFilterGetResponse *rsp;
    vlan_t vlan;

    NIC_LOG_DEBUG("NCSI Vlan filter get");

    devmgr->DevApi()->swm_get_channels_info(&channels_info);

    for (auto it = channels_info.cbegin(); it != channels_info.cend(); it) {
        cinfo = *it;
        for (auto it1 = cinfo->vlan_table.cbegin(); it1 != cinfo->vlan_table.cend(); it1++) {
            vlan = *it1;
            rsp = rsp_msg->add_response();
            rsp->mutable_request()->set_channel(cinfo->channel);
            rsp->mutable_request()->set_vlan_id(vlan);
            rsp->set_api_status(types::API_STATUS_OK);
            NIC_LOG_DEBUG("Channel: {}, Vlan: {}", cinfo->channel, vlan);
        }
        channels_info.erase(it++);
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO, cinfo);
    }
    return ret;
}

sdk_ret_t
ncsi_ipc_mac_filter (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    MacFilterRequest *req = dynamic_cast<MacFilterRequest *>(msg->msg);
    MacFilterResponse *rsp = dynamic_cast<MacFilterResponse *>(msg->rsp);

    NIC_LOG_DEBUG("NCSI Mac filter: channel: {}, mac: {}, oper: {}", 
                  req->channel(), mac2str(req->mac_addr()), msg->oper);
    if (msg->oper == hal::NCSI_MSG_OPER_CREATE) {
        devmgr->DevApi()->swm_add_mac(req->mac_addr(), req->channel());
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_del_mac(req->mac_addr(), req->channel());
    }

    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

sdk_ret_t
ncsi_ipc_mac_filter_get (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::set<channel_info_t *> channels_info;
    channel_info_t *cinfo;
    MacFilterGetResponseMsg *rsp_msg = dynamic_cast<MacFilterGetResponseMsg *>(msg->rsp);
    MacFilterGetResponse *rsp;
    mac_t mac;

    NIC_LOG_DEBUG("NCSI Mac filter get");

    devmgr->DevApi()->swm_get_channels_info(&channels_info);

    for (auto it = channels_info.cbegin(); it != channels_info.cend(); it) {
        cinfo = *it;
        for (auto it1 = cinfo->mac_table.cbegin(); it1 != cinfo->mac_table.cend(); it1++) {
            mac = *it1;
            rsp = rsp_msg->add_response();
            rsp->mutable_request()->set_channel(cinfo->channel);
            rsp->mutable_request()->set_mac_addr(mac);
            rsp->set_api_status(types::API_STATUS_OK);
            NIC_LOG_DEBUG("Channel: {}, Mac: {}", cinfo->channel, mac2str(mac));
        }
        channels_info.erase(it++);
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO, cinfo);
    }
    return ret;
}

sdk_ret_t
ncsi_ipc_bcast_filter (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    lif_bcast_filter_t bcast_filter = {0};
    BcastFilterRequest *req = dynamic_cast<BcastFilterRequest *>(msg->msg);
    BcastFilterResponse *rsp = dynamic_cast<BcastFilterResponse *>(msg->rsp);

    NIC_LOG_DEBUG("NCSI Bcast filter: channel: {}, oper: {}, arp: {}, "
                  "dhcp_client: {}, dhcp_server: {}, netbios: {}", 
                  req->channel(), msg->oper,
                  req->enable_arp(), req->enable_dhcp_client(),
                  req->enable_dhcp_server(), req->enable_netbios());
    if (msg->oper == hal::NCSI_MSG_OPER_CREATE ||
        msg->oper == hal::NCSI_MSG_OPER_UPDATE) {
        bcast_filter.arp = req->enable_arp();
        bcast_filter.dhcp_client = req->enable_dhcp_client();
        bcast_filter.dhcp_server = req->enable_dhcp_server();
        bcast_filter.netbios = req->enable_netbios();
        devmgr->DevApi()->swm_upd_bcast_filter(bcast_filter, req->channel());
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_upd_bcast_filter(bcast_filter, req->channel());
    }

    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

sdk_ret_t
ncsi_ipc_bcast_filter_get (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::set<channel_info_t *> channels_info;
    channel_info_t *cinfo;
    BcastFilterGetResponseMsg *rsp_msg = dynamic_cast<BcastFilterGetResponseMsg *>(msg->rsp);
    BcastFilterGetResponse *rsp;
    mac_t mac;

    NIC_LOG_DEBUG("NCSI Bcast filter get");

    devmgr->DevApi()->swm_get_channels_info(&channels_info);

    for (auto it = channels_info.cbegin(); it != channels_info.cend();) {
        cinfo = *it;
        rsp = rsp_msg->add_response();
        rsp->mutable_request()->set_channel(cinfo->channel);
        rsp->mutable_request()->set_enable_arp(cinfo->bcast_filter.arp);
        rsp->mutable_request()->set_enable_dhcp_client(cinfo->bcast_filter.dhcp_client);
        rsp->mutable_request()->set_enable_dhcp_server(cinfo->bcast_filter.dhcp_server);
        rsp->mutable_request()->set_enable_netbios(cinfo->bcast_filter.netbios);
        rsp->set_api_status(types::API_STATUS_OK);
        NIC_LOG_DEBUG("Channel: {}, arp: {}, dhcp_client: {}, dhcp_server: {}, netbios: {}", 
                      cinfo->channel, cinfo->bcast_filter.arp, cinfo->bcast_filter.dhcp_client,
                      cinfo->bcast_filter.dhcp_server, cinfo->bcast_filter.netbios);
        channels_info.erase(it++);
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO, cinfo);
    }
    return ret;
}

sdk_ret_t
ncsi_ipc_mcast_filter (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    lif_mcast_filter_t mcast_filter = {0};
    McastFilterRequest *req = dynamic_cast<McastFilterRequest *>(msg->msg);
    McastFilterResponse *rsp = dynamic_cast<McastFilterResponse *>(msg->rsp);

    NIC_LOG_DEBUG("NCSI Mcast filter: channel: {}, oper: {}, ipv6_neigh_adv: {}, "
                  "ipv6_router_adv: {}, dhcpv6_relay: {}, dhcpv6_mcast: {}, "
                  "ipv6_mld: {}, ipv6_neigh_sol: {}",
                  req->channel(), msg->oper,
                  req->enable_ipv6_neigh_adv(), req->enable_ipv6_router_adv(),
                  req->enable_dhcpv6_relay(), req->enable_dhcpv6_mcast(),
                  req->enable_ipv6_mld(), req->enable_ipv6_neigh_sol());
    if (msg->oper == hal::NCSI_MSG_OPER_CREATE ||
        msg->oper == hal::NCSI_MSG_OPER_UPDATE) {
        mcast_filter.ipv6_neigh_adv = req->enable_ipv6_neigh_adv();
        mcast_filter.ipv6_router_adv = req->enable_ipv6_router_adv();
        mcast_filter.dhcpv6_relay = req->enable_dhcpv6_relay();
        mcast_filter.dhcpv6_mcast = req->enable_dhcpv6_mcast();
        mcast_filter.ipv6_mld = req->enable_ipv6_mld();
        mcast_filter.ipv6_neigh_sol = req->enable_ipv6_neigh_sol();

        devmgr->DevApi()->swm_upd_mcast_filter(mcast_filter, req->channel());
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_upd_mcast_filter(mcast_filter, req->channel());
    }

    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

sdk_ret_t
ncsi_ipc_mcast_filter_get (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::set<channel_info_t *> channels_info;
    channel_info_t *cinfo;
    McastFilterGetResponseMsg *rsp_msg = dynamic_cast<McastFilterGetResponseMsg *>(msg->rsp);
    McastFilterGetResponse *rsp;
    mac_t mac;

    NIC_LOG_DEBUG("NCSI Mcast filter get");

    devmgr->DevApi()->swm_get_channels_info(&channels_info);

    for (auto it = channels_info.cbegin(); it != channels_info.cend();) {
        cinfo = *it;
        rsp = rsp_msg->add_response();
        rsp->mutable_request()->set_channel(cinfo->channel);
        rsp->mutable_request()->set_enable_ipv6_neigh_adv(cinfo->mcast_filter.ipv6_neigh_adv);
        rsp->mutable_request()->set_enable_ipv6_router_adv(cinfo->mcast_filter.ipv6_router_adv);
        rsp->mutable_request()->set_enable_dhcpv6_relay(cinfo->mcast_filter.dhcpv6_relay);
        rsp->mutable_request()->set_enable_dhcpv6_mcast(cinfo->mcast_filter.dhcpv6_mcast);
        rsp->mutable_request()->set_enable_ipv6_mld(cinfo->mcast_filter.ipv6_mld);
        rsp->mutable_request()->set_enable_ipv6_neigh_sol(cinfo->mcast_filter.ipv6_neigh_sol);
        rsp->set_api_status(types::API_STATUS_OK);
        NIC_LOG_DEBUG("Channel: {}, ipv6_neigh_adv: {}, ipv6_router_adv: {}, dhcpv6_relay: {}, "
                      "dhcpv6_mcast: {}, ipv6_mld: {}, ipv6_neigh_sol: {}", 
                      cinfo->channel, cinfo->mcast_filter.ipv6_neigh_adv, 
                      cinfo->mcast_filter.ipv6_router_adv, cinfo->mcast_filter.dhcpv6_relay, 
                      cinfo->mcast_filter.dhcpv6_mcast, cinfo->mcast_filter.ipv6_mld,
                      cinfo->mcast_filter.ipv6_neigh_sol);
        channels_info.erase(it++);
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO, cinfo);
    }
    return ret;
}

sdk_ret_t
ncsi_ipc_vlan_mode (ncsi_ipc_msg_t *msg)
{
    return SDK_RET_OK;
}

sdk_ret_t
ncsi_ipc_channel (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    ChannelRequest *req = dynamic_cast<ChannelRequest *>(msg->msg);
    ChannelResponse *rsp = dynamic_cast<ChannelResponse *>(msg->rsp);

    NIC_LOG_DEBUG("NCSI Channel config: channel: {}, oper: {}, tx_enable: {}, "
                  "rx_enable: {}", req->channel(), msg->oper,
                  req->tx_enable(), req->rx_enable());

    if (msg->oper == hal::NCSI_MSG_OPER_CREATE ||
        msg->oper == hal::NCSI_MSG_OPER_UPDATE) {
        if (req->tx_enable()) {
            devmgr->DevApi()->swm_enable_tx(req->channel());
        } else {
            devmgr->DevApi()->swm_disable_tx(req->channel());
        }
        if (req->rx_enable()) {
            devmgr->DevApi()->swm_enable_rx(req->channel());
        } else {
            devmgr->DevApi()->swm_disable_rx(req->channel());
        }
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        NIC_LOG_ERR("Channel should never be deleted.");
    }
    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

sdk_ret_t
ncsi_ipc_channel_get (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::set<channel_info_t *> channels_info;
    channel_info_t *cinfo;
    ChannelGetResponseMsg *rsp_msg = dynamic_cast<ChannelGetResponseMsg *>(msg->rsp);
    ChannelGetResponse *rsp;
    mac_t mac;

    NIC_LOG_DEBUG("NCSI Channel get");

    devmgr->DevApi()->swm_get_channels_info(&channels_info);

    for (auto it = channels_info.cbegin(); it != channels_info.cend();) {
        cinfo = *it;
        rsp = rsp_msg->add_response();
        rsp->mutable_request()->set_channel(cinfo->channel);
        rsp->mutable_request()->set_tx_enable(cinfo->tx_en);
        rsp->mutable_request()->set_rx_enable(cinfo->rx_en);
        rsp->set_api_status(types::API_STATUS_OK);
        NIC_LOG_DEBUG("Channel: {}, tx_en: {}, rx_en: {}",
                      cinfo->channel, cinfo->tx_en, cinfo->rx_en);
        channels_info.erase(it++);
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_SWM_CHANNEL_INFO, cinfo);
    }
    return ret;
}


void 
ncsi_ipc_handler_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    ncsi_ipc_msg_t *ncsi_msg = (ncsi_ipc_msg_t *)msg->data();
    ncsi_ipc_msg_t rsp_msg;

    NIC_LOG_DEBUG("--------------  NCSI message: {}, oper: {} -------------", 
                  ncsi_msg->msg_id, 
                  ncsi_msg->oper);

    switch(ncsi_msg->msg_id) {
    case hal::NCSI_MSG_VLAN_FILTER:
        if (ncsi_msg->oper == hal::NCSI_MSG_OPER_CREATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_DELETE) { 
            ret = ncsi_ipc_vlan_filter(ncsi_msg);
        } else if (ncsi_msg->oper == hal::NCSI_MSG_OPER_GET) {
            ret = ncsi_ipc_vlan_filter_get(ncsi_msg);
        }
        break;
    case hal::NCSI_MSG_MAC_FILTER:
        if (ncsi_msg->oper == hal::NCSI_MSG_OPER_CREATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_DELETE) { 
            ret = ncsi_ipc_mac_filter(ncsi_msg);
        } else if (ncsi_msg->oper == hal::NCSI_MSG_OPER_GET) {
            ret = ncsi_ipc_mac_filter_get(ncsi_msg);
        }
        break;
    case hal::NCSI_MSG_BCAST_FILTER:
        if (ncsi_msg->oper == hal::NCSI_MSG_OPER_CREATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_UPDATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_DELETE) { 
            ret = ncsi_ipc_bcast_filter(ncsi_msg);
        } else {
            ret = ncsi_ipc_bcast_filter_get(ncsi_msg);
        }
        break;
    case hal::NCSI_MSG_MCAST_FILTER:
        if (ncsi_msg->oper == hal::NCSI_MSG_OPER_CREATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_UPDATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_DELETE) { 
            ret = ncsi_ipc_mcast_filter(ncsi_msg);
        } else {
            ret = ncsi_ipc_mcast_filter_get(ncsi_msg);
        }
        break;
    case hal::NCSI_MSG_VLAN_MODE:
        ret = ncsi_ipc_vlan_mode(ncsi_msg);
        break;
    case hal::NCSI_MSG_CHANNEL:
        if (ncsi_msg->oper == hal::NCSI_MSG_OPER_CREATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_UPDATE ||
            ncsi_msg->oper == hal::NCSI_MSG_OPER_DELETE) { 
            ret = ncsi_ipc_channel(ncsi_msg);
        } else {
            ret = ncsi_ipc_channel_get(ncsi_msg);
        }
        break;
    default:
        NIC_LOG_ERR("Invalid message id");
    }

    rsp_msg.msg_id = ncsi_msg->msg_id;
    rsp_msg.rsp_ret = ret;

    NIC_LOG_DEBUG("Nicmgr response for NCSI message to hal: msg_id: {}, ret: {}",
                  rsp_msg.msg_id, rsp_msg.rsp_ret);

    sdk::ipc::respond(msg, &rsp_msg, sizeof(rsp_msg));
}

void
nicmgr_ncsi_ipc_init (void)
{
    sdk::ipc::reg_request_handler(event_id_t::EVENT_ID_NCSI,
                                  ncsi_ipc_handler_cb, NULL);
}

} // namespace nicmgr
