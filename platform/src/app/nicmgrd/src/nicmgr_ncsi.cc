//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <sys/stat.h>
#include "nic/include/trace.hpp"
#include "nic/include/base.hpp"

#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "nicmgr_ncsi.hpp"

extern DeviceManager *devmgr;

namespace nicmgr {


sdk_ret_t
ncsi_ipc_vlan_filter (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    VlanFilterRequest *req = dynamic_cast<VlanFilterRequest *>(msg->msg);
    VlanFilterResponse *rsp = dynamic_cast<VlanFilterResponse *>(msg->rsp);

    if (msg->oper == hal::NCSI_MSG_OPER_CREATE) {
        devmgr->DevApi()->swm_add_vlan(req->vlan_id());
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_del_vlan(req->vlan_id());
    }

    return ret;
}

sdk_ret_t
ncsi_ipc_mac_filter (ncsi_ipc_msg_t *msg)
{
    sdk_ret_t ret = SDK_RET_OK;
    MacFilterRequest *req = dynamic_cast<MacFilterRequest *>(msg->msg);
    MacFilterResponse *rsp = dynamic_cast<MacFilterResponse *>(msg->rsp);

    if (msg->oper == hal::NCSI_MSG_OPER_CREATE) {
        devmgr->DevApi()->swm_add_mac(req->mac_addr());
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_del_mac(req->mac_addr());
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

    if (msg->oper == hal::NCSI_MSG_OPER_CREATE ||
        msg->oper == hal::NCSI_MSG_OPER_UPDATE) {
        bcast_filter.arp = req->enable_arp();
        bcast_filter.dhcp_client = req->enable_dhcp_client();
        bcast_filter.dhcp_server = req->enable_dhcp_server();
        bcast_filter.netbios = req->enable_netbios();
        devmgr->DevApi()->swm_upd_bcast_filter(bcast_filter);
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_upd_bcast_filter(bcast_filter);
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

    if (msg->oper == hal::NCSI_MSG_OPER_CREATE ||
        msg->oper == hal::NCSI_MSG_OPER_UPDATE) {
        mcast_filter.ipv6_neigh_adv = req->enable_ipv6_neigh_adv();
        mcast_filter.ipv6_router_adv = req->enable_ipv6_router_adv();
        mcast_filter.dhcpv6_relay = req->enable_dhcpv6_relay();
        mcast_filter.dhcpv6_mcast = req->enable_dhcpv6_mcast();
        mcast_filter.ipv6_mld = req->enable_ipv6_mld();
        mcast_filter.ipv6_neigh_sol = req->enable_ipv6_neigh_sol();

        devmgr->DevApi()->swm_upd_mcast_filter(mcast_filter);
    } else if (msg->oper == hal::NCSI_MSG_OPER_DELETE) {
        devmgr->DevApi()->swm_upd_mcast_filter(mcast_filter);
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
    return SDK_RET_OK;
}


void 
ncsi_ipc_handler_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    ncsi_ipc_msg_t *ncsi_msg = (ncsi_ipc_msg_t *)msg->data();

    NIC_LOG_DEBUG("Got NCSI message: {} ", ncsi_msg->msg_id);

    switch(ncsi_msg->msg_id) {
    case hal::NCSI_MSG_VLAN_FILTER:
        ret = ncsi_ipc_vlan_filter(ncsi_msg);
        break;
    case hal::NCSI_MSG_MAC_FILTER:
        ret = ncsi_ipc_mac_filter(ncsi_msg);
        break;
    case hal::NCSI_MSG_BCAST_FILTER:
        ret = ncsi_ipc_bcast_filter(ncsi_msg);
        break;
    case hal::NCSI_MSG_MCAST_FILTER:
        ret = ncsi_ipc_mcast_filter(ncsi_msg);
        break;
    case hal::NCSI_MSG_VLAN_MODE:
        ret = ncsi_ipc_vlan_mode(ncsi_msg);
        break;
    case hal::NCSI_MSG_CHANNEL:
        ret = ncsi_ipc_channel(ncsi_msg);
        break;
    default:
        NIC_LOG_ERR("Invalid message id");
    }

}

void
nicmgr_ncsi_ipc_init (void)
{
    sdk::ipc::reg_request_handler(event_id_t::EVENT_ID_NCSI,
                                  ncsi_ipc_handler_cb, NULL);
}

} // namespace nicmgr
