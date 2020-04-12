/*
* Copyright (c) 2019, Pensando Systems Inc.
*/

#include "grpc_ipc.h"
#include "delphi_ipc.h"

using std::chrono::seconds;

#define NCSI_LINK_SPEED_10G     (0x8)
#define NCSI_LINK_SPEED_25G     (0xA)
#define NCSI_LINK_SPEED_40G     (0xB)
#define NCSI_LINK_SPEED_50G     (0xC)
#define NCSI_LINK_SPEED_100G     (0xD)

#define HAL_GRPC_API_TIMEOUT 25

#define SET_TIMEOUT()                                                       \
    uint8_t timeout = HAL_GRPC_API_TIMEOUT;                                 \
    std::chrono::system_clock::time_point deadline =                        \
    std::chrono::system_clock::now() + seconds(timeout);                \
    context.set_deadline(deadline);

#define NCSI_CREATE_API(obj_api, obj_class)                             \
    Status                                                                  \
    grpc_ipc::obj_api ## _create (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        SET_TIMEOUT();                                                      \
        status = ncsi_stub_->obj_class ## Create(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

#define NCSI_UPDATE_API(obj_api, obj_class)                             \
    Status                                                                  \
    grpc_ipc::obj_api ## _update (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        SET_TIMEOUT();                                                      \
        status = ncsi_stub_->obj_class ## Update(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

#define NCSI_DELETE_API(obj_api, obj_class)                             \
    Status                                                                  \
    grpc_ipc::obj_api ## _delete (obj_class ## RequestMsg& req_msg,    \
                                       obj_class ## ResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        SET_TIMEOUT();                                                      \
        status = ncsi_stub_->obj_class ## Delete(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

#define NCSI_GET_API(obj_api, obj_class)                                \
    Status                                                                  \
    grpc_ipc::obj_api ## _get (obj_class ## GetRequestMsg& req_msg,    \
                                    obj_class ## GetResponseMsg& rsp_msg)   \
    {                                                                       \
        grpc::ClientContext         context;                                \
        grpc::Status                status;                                 \
        SET_TIMEOUT();                                                      \
        status = ncsi_stub_->obj_class ## Get(&context, req_msg, &rsp_msg);   \
        return status;                                                      \
    }

NCSI_CREATE_API(vlan, VlanFilter);
NCSI_DELETE_API(vlan, VlanFilter);

NCSI_CREATE_API(mac_filter, MacFilter);
NCSI_DELETE_API(mac_filter, MacFilter);

NCSI_CREATE_API(vlan_mode, VlanMode);
NCSI_UPDATE_API(vlan_mode, VlanMode);

NCSI_CREATE_API(channel_state, Channel);
NCSI_UPDATE_API(channel_state, Channel);
NCSI_GET_API(channel_state, Channel);

NCSI_CREATE_API(bcast_filters, BcastFilter);
NCSI_UPDATE_API(bcast_filters, BcastFilter);
NCSI_GET_API(bcast_filters, BcastFilter);

NCSI_CREATE_API(mcast_filters, McastFilter);
NCSI_UPDATE_API(mcast_filters, McastFilter);
NCSI_GET_API(mcast_filters, McastFilter);

static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;
    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

sdk_ret_t
grpc_ipc::connect_hal(void)
{
    grpc::ChannelArguments channel_args;
    std::string svc_url;

    if (getenv("HAL_SOCK_PATH")) {
        svc_url = std::string("unix:") + std::getenv("HAL_SOCK_PATH") + "halsock";
    } else if (getenv("HAL_GRPC_PORT")) {
        svc_url = std::string("localhost:") + getenv("HAL_GRPC_PORT");
    } else {
        svc_url = std::string("localhost:50054");
    }

    channel_args.SetInt(GRPC_ARG_INITIAL_RECONNECT_BACKOFF_MS, 100); // Time bet 1st & 2nd attempts
    channel_args.SetInt(GRPC_ARG_MIN_RECONNECT_BACKOFF_MS, 100); // Min time bet subseq. attemps;
    channel = grpc::CreateCustomChannel(svc_url, grpc::InsecureChannelCredentials(),
                                        channel_args);

    SDK_TRACE_INFO("Connecting to HAL at: %s", svc_url.c_str());
    auto state = channel->GetState(true);
    while (state != GRPC_CHANNEL_READY) {
        // Wait for State change or deadline
        channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
        state = channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
    }
    SDK_TRACE_INFO("Connected to HAL at: %s", svc_url.c_str());

    ncsi_stub_ = ncsi::Ncsi::NewStub(channel);
    port_stub_ = port::Port::NewStub(channel);

    return SDK_RET_OK;
}

#if 0
Status
grpc_ipc::vlan_create (VlanFilterRequestMsg& req_msg, VlanFilterResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    SET_TIMEOUT();
    status = ncsi_stub_->VlanFilterCreate(&context, req_msg, &rsp_msg);

    return status;
}

Status
grpc_ipc::vlan_delete (VlanFilterRequestMsg& req_msg, VlanFilterResponseMsg& rsp_msg)
{
    grpc::ClientContext         context;
    grpc::Status                status;

    SET_TIMEOUT();
    status = ncsi_stub_->VlanFilterDelete(&context, req_msg, &rsp_msg);

    return status;
}

#endif

int grpc_ipc::PostMsg(struct VlanFilterMsg& vlan_filter)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    VlanFilterRequest* req;
    VlanFilterResponse rsp;
    VlanFilterRequestMsg req_msg;
    VlanFilterResponseMsg rsp_msg;

    SDK_TRACE_INFO("vlan %s on ncsi channel: 0x%x, vlan_id: 0x%x", 
            vlan_filter.enable ? "create" : "remove", vlan_filter.port, vlan_filter.vlan_id);
    req = req_msg.add_request();

    req->set_id(vlan_filter.filter_id);
    req->set_vlan_id(vlan_filter.vlan_id);
    req->set_channel(vlan_filter.port);

    if (vlan_filter.enable) {
        status = vlan_create(req_msg, rsp_msg);
    }
    else {
        status = vlan_delete(req_msg, rsp_msg);
    }

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
        } 
        else if (vlan_filter.enable && rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            SDK_TRACE_ERR("vlan filter already exists with vlan_id: 0x%x", 
                    vlan_filter.vlan_id);
            ret = SDK_RET_ERR;
            goto end;
        } else {
            SDK_TRACE_ERR("Failed to %s vlan filter for vlan_id: 0x%x. err: 0x%x",
                    vlan_filter.enable ? "create":"delete",
                    vlan_filter.vlan_id, rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to %s vlan filter for vlan_id: 0x%x. err: 0x%x err_msg: %s",
                vlan_filter.enable ? "create":"delete", 
                vlan_filter.vlan_id, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct VlanModeMsg& vlan_mode)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    VlanModeRequest* req;
    VlanModeResponse rsp;
    VlanModeRequestMsg req_msg;
    VlanModeResponseMsg rsp_msg;

    SDK_TRACE_INFO("vlan mode on ncsi channel: 0x%x, vlan_enable: 0x%x, vlan_mode: 0x%x", 
            vlan_mode.port, vlan_mode.enable, vlan_mode.mode);
    req = req_msg.add_request();

    //req->set_id(vlan_mode.filter_id);
    req->set_enable(vlan_mode.enable);
    req->set_mode(vlan_mode.mode);
    req->set_channel(vlan_mode.port);

    status = vlan_mode_update(req_msg, rsp_msg);

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
        } 
        else if (vlan_mode.enable && rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            SDK_TRACE_ERR("vlan mode already exists with mode: 0x%x", vlan_mode.mode);
            ret = SDK_RET_ERR;
            goto end;
        } else {
            SDK_TRACE_ERR("Failed to update vlan mode: 0x%x. err: 0x%x",
                    vlan_mode.mode, rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to update vlan mode: 0x%x. err: 0x%x err_msg: %s",
                vlan_mode.mode, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct MacFilterMsg& mac_filter)
{
    uint64_t mac_addr = 0;
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    MacFilterRequest* req;
    MacFilterResponse rsp;
    MacFilterRequestMsg req_msg;
    MacFilterResponseMsg rsp_msg;

    memcpy(&mac_addr, mac_filter.mac_addr, sizeof(mac_filter.mac_addr));
    memrev((uint8_t* )&mac_addr, 6);

    SDK_TRACE_INFO("mac filter %s on ncsi channel: 0x%x, mac_addr: 0x%llx", 
            mac_filter.enable ? "create" : "remove", mac_filter.port, mac_addr);
    req = req_msg.add_request();

    req->set_id(mac_filter.filter_id);
    req->set_mac_addr(mac_addr);
    req->set_channel(mac_filter.port);

    if (mac_filter.enable) {
        status = mac_filter_create(req_msg, rsp_msg);
    }
    else {
        status = mac_filter_delete(req_msg, rsp_msg);
    }

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() == types::API_STATUS_OK) {
        } 
        else if (mac_filter.enable && rsp.api_status() == types::API_STATUS_EXISTS_ALREADY) {
            SDK_TRACE_ERR("mac filter already exists with mac_addr: 0x%llx", 
                    mac_addr);
            ret = SDK_RET_ERR;
            goto end;
        } else {
            SDK_TRACE_ERR("Failed to %s mac filter for mac_addr: 0x%llx. err: 0x%x",
                    mac_filter.enable ? "create":"delete",
                    mac_addr, rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to %s mac filter for mac_addr: 0x%llx. err: 0x%x err_msg: %s",
                mac_filter.enable ? "create":"delete", mac_addr, 
                status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct SetLinkMsg& set_link_msg)
{
    return 0;
}

int grpc_ipc::PostMsg(struct EnableChanMsg& enable_ch)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    ChannelRequest* req;
    ChannelResponse rsp;

    ChannelRequestMsg req_msg;
    ChannelResponseMsg rsp_msg;
    ChannelGetResponse get_rsp;
    ChannelGetRequestMsg get_req_msg;
    ChannelGetResponseMsg get_rsp_msg;

    SDK_TRACE_INFO("channel enable on ncsi channel: 0x%x", 
            enable_ch.port);
    auto get_req = get_req_msg.add_request();

    get_req->set_channel(enable_ch.port);
    status = channel_state_get(get_req_msg, get_rsp_msg);

    if (!status.ok()) {
        SDK_TRACE_ERR("Failed to get channel state for channel: %d. err: {}, err_msg: {}",
                enable_ch.port, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    get_rsp = get_rsp_msg.response(0);
    if (get_rsp.api_status() != types::API_STATUS_OK) {
        SDK_TRACE_ERR("Failed: API status: 0x%x channel: 0x%x", 
                get_rsp.api_status(), enable_ch.port);
        ret = SDK_RET_ERR;
        goto end;
    }

    req = req_msg.add_request();
    req->CopyFrom(get_rsp.request());

    //req->set_tx_enable(get_rsp.tx_enable());
    req->set_rx_enable(enable_ch.enable);
    //req->set_reset(get_rsp.tx_enable());
    //req->set_channel(get_rsp.tx_enable());

    status = channel_state_update(req_msg, rsp_msg);

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            SDK_TRACE_ERR("Failed to update channel state for err: 0x%x",
                    rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to update channel state. err: 0x%x err_msg: %s",
                status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct ResetChanMsg& reset_ch)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    ChannelRequest* req;
    ChannelResponse rsp;

    ChannelRequestMsg req_msg;
    ChannelResponseMsg rsp_msg;
    ChannelGetResponse get_rsp;
    ChannelGetRequestMsg get_req_msg;
    ChannelGetResponseMsg get_rsp_msg;

    SDK_TRACE_INFO("reset channel on ncsi channel: 0x%x", 
            reset_ch.port);
    auto get_req = get_req_msg.add_request();

    get_req->set_channel(reset_ch.port);
    status = channel_state_get(get_req_msg, get_rsp_msg);

    if (!status.ok()) {
        SDK_TRACE_ERR("Failed to get channel state for channel: %d. err: {}, err_msg: {}",
                reset_ch.port, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    get_rsp = get_rsp_msg.response(0);
    if (get_rsp.api_status() != types::API_STATUS_OK) {
        SDK_TRACE_ERR("Failed: API status: 0x%x channel: 0x%x", 
                get_rsp.api_status(), reset_ch.port);
        ret = SDK_RET_ERR;
        goto end;
    }

    req = req_msg.add_request();
    req->CopyFrom(get_rsp.request());

    req->set_reset(reset_ch.reset);

    status = channel_state_update(req_msg, rsp_msg);

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            SDK_TRACE_ERR("Failed to update channel state for err: 0x%x",
                    rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to update channel state. err: 0x%x err_msg: %s",
                status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct EnableChanTxMsg& enable_ch_tx)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    ChannelRequest* req;
    ChannelResponse rsp;

    ChannelRequestMsg req_msg;
    ChannelResponseMsg rsp_msg;
    ChannelGetResponse get_rsp;
    ChannelGetRequestMsg get_req_msg;
    ChannelGetResponseMsg get_rsp_msg;

    SDK_TRACE_INFO("enable channel tx on ncsi channel: 0x%x", 
            enable_ch_tx.port);
    auto get_req = get_req_msg.add_request();

    get_req->set_channel(enable_ch_tx.port);
    status = channel_state_get(get_req_msg, get_rsp_msg);

    if (!status.ok()) {
        SDK_TRACE_ERR("Failed to get channel state for channel: %d. err: {}, err_msg: {}",
                enable_ch_tx.port, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    get_rsp = get_rsp_msg.response(0);
    if (get_rsp.api_status() != types::API_STATUS_OK) {
        SDK_TRACE_ERR("Failed: API status: 0x%x channel: 0x%x", 
                get_rsp.api_status(), enable_ch_tx.port);
        ret = SDK_RET_ERR;
        goto end;
    }

    req = req_msg.add_request();
    req->CopyFrom(get_rsp.request());

    req->set_tx_enable(enable_ch_tx.enable);

    status = channel_state_update(req_msg, rsp_msg);

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            SDK_TRACE_ERR("Failed to update channel state for err: 0x%x",
                    rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to update channel state. err: 0x%x err_msg: %s",
                status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct EnableBcastFilterMsg& bcast_filter)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    BcastFilterRequest* req;
    BcastFilterResponse rsp;

    BcastFilterRequestMsg req_msg;
    BcastFilterResponseMsg rsp_msg;
    BcastFilterGetResponse get_rsp;
    BcastFilterGetRequestMsg get_req_msg;
    BcastFilterGetResponseMsg get_rsp_msg;

    SDK_TRACE_INFO("Update Bcast filters on ncsi channel: 0x%x", 
            bcast_filter.port);
    auto get_req = get_req_msg.add_request();

    get_req->set_channel(bcast_filter.port);
    status = bcast_filters_get(get_req_msg, get_rsp_msg);

    if (!status.ok()) {
        SDK_TRACE_ERR("Failed to get broadcast filters for channel: %d. err: {}, err_msg: {}",
                bcast_filter.port, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    get_rsp = get_rsp_msg.response(0);
    if (get_rsp.api_status() != types::API_STATUS_OK) {
        SDK_TRACE_ERR("Failed: API status: 0x%x channel: 0x%x", 
                get_rsp.api_status(), bcast_filter.port);
        ret = SDK_RET_ERR;
        goto end;
    }

    req = req_msg.add_request();
    req->CopyFrom(get_rsp.request());

    req->set_enable_arp(bcast_filter.enable_arp);
    req->set_enable_dhcp_client(bcast_filter.enable_dhcp_client);
    req->set_enable_dhcp_server(bcast_filter.enable_dhcp_server);
    req->set_enable_netbios(bcast_filter.enable_netbios);

    status = bcast_filters_update(req_msg, rsp_msg);

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            SDK_TRACE_ERR("Failed to update channel state for err: 0x%x",
                    rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to update broadcast filters. err: 0x%x err_msg: %s",
                status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

int grpc_ipc::PostMsg(struct EnableGlobalMcastFilterMsg& mcast_filter)
{
    grpc::Status status;
    sdk_ret_t ret = SDK_RET_OK;
    McastFilterRequest* req;
    McastFilterResponse rsp;

    McastFilterRequestMsg req_msg;
    McastFilterResponseMsg rsp_msg;
    McastFilterGetResponse get_rsp;
    McastFilterGetRequestMsg get_req_msg;
    McastFilterGetResponseMsg get_rsp_msg;

    SDK_TRACE_INFO("Update Mcast filters on ncsi channel: 0x%x", 
            mcast_filter.port);
    auto get_req = get_req_msg.add_request();

    get_req->set_channel(mcast_filter.port);
    status = mcast_filters_get(get_req_msg, get_rsp_msg);

    if (!status.ok()) {
        SDK_TRACE_ERR("Failed to get multicast filters for channel: %d. err: {}, err_msg: {}",
                mcast_filter.port, status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    get_rsp = get_rsp_msg.response(0);
    if (get_rsp.api_status() != types::API_STATUS_OK) {
        SDK_TRACE_ERR("Failed: API status: 0x%x channel: 0x%x", 
                get_rsp.api_status(), mcast_filter.port);
        ret = SDK_RET_ERR;
        goto end;
    }

    req = req_msg.add_request();
    req->CopyFrom(get_rsp.request());

    req->set_enable_ipv6_neigh_adv(mcast_filter.enable_ipv6_neigh_adv);
    req->set_enable_ipv6_router_adv(mcast_filter.enable_ipv6_router_adv);
    req->set_enable_dhcpv6_relay(mcast_filter.enable_dhcpv6_relay);
    req->set_enable_dhcpv6_mcast(mcast_filter.enable_dhcpv6_mcast);
    req->set_enable_ipv6_mld(mcast_filter.enable_ipv6_mld);
    req->set_enable_ipv6_neigh_sol(mcast_filter.enable_ipv6_neigh_sol);

    status = mcast_filters_update(req_msg, rsp_msg);

    if (status.ok()) {
        rsp = rsp_msg.response(0);
        if (rsp.api_status() != types::API_STATUS_OK) {
            SDK_TRACE_ERR("Failed to update channel state for err: 0x%x",
                    rsp.api_status());
            ret = SDK_RET_ERR;
            goto end;
        }
    } else {
        SDK_TRACE_ERR("Failed to update multicast filters. err: 0x%x err_msg: %s",
                status.error_code(), status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

end:
    return ret;
}

static bool port_handle_api_status(types::ApiStatus api_status,
        uint32_t port_id) {
    switch(api_status) {
        case types::API_STATUS_OK:
            return true;

        case types::API_STATUS_NOT_FOUND:
            return false;

        default:
            SDK_TRACE_ERR("Unknown erro while getting Port Status over gRPC");
            return false;
    }

    return true;
}

int grpc_ipc::GetLinkStatus(uint32_t port, bool& link_status, uint8_t& link_speed)
{
    //g_link_event_handler->get_link_status();
    PortGetRequest      *req;
    PortGetRequestMsg   req_msg;
    PortGetResponseMsg  rsp_msg;
    grpc::ClientContext context;
    Status              status;
    link_status = false;
    link_speed = 0;

    if (port == 0)
        port = 0x11010001;
    else if (port == 1)
        port = 0x11020001;
    else {
        SDK_TRACE_ERR("Invalid port number: %d\n", port);
        return -1;
    }

    req = req_msg.add_request();
    req->mutable_key_or_handle()->set_port_id(port);

    // port get
    status = port_stub_->PortGet(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port) == true) {
            SDK_TRACE_DEBUG("Port oper status: %x",
                rsp_msg.response(0).status().link_status().oper_state());

            if (rsp_msg.response(0).status().link_status().oper_state() == 
                    port::PORT_OPER_STATUS_UP) {
                link_status = true;

                switch(rsp_msg.response(0).spec().port_speed()) {
                    case port::PORT_SPEED_10G:
                        link_speed = NCSI_LINK_SPEED_10G; //0x8
                        break;
                    case port::PORT_SPEED_25G:
                        link_speed = NCSI_LINK_SPEED_25G; //0xA;
                        break;
                    case port::PORT_SPEED_40G:
                        link_speed = NCSI_LINK_SPEED_40G; //0xB;
                        break;
                    case port::PORT_SPEED_50G:
                        link_speed = NCSI_LINK_SPEED_50G; //0xC;
                        break;
                    case port::PORT_SPEED_100G:
                        link_speed = NCSI_LINK_SPEED_100G; //0xD;
                        break;
                    default:
                        link_speed = 0;

                }// end of switch
            }
            else {
                link_status = false;
            }

        } else {
            SDK_TRACE_ERR("Port Get failed for port %d api_status: %x", 
                    port, rsp_msg.response(0).api_status());
        }

        return 0;
    }
    SDK_TRACE_ERR("Port Get failed for port %d", port);
    return -1;
}
