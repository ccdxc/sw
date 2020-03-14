//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "port.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "hal_grpc.hpp"
#include "devapi_mem.hpp"
#include "nic/sdk/lib/utils/port_utils.hpp"

namespace iris {

std::map<uint32_t, devapi_port*> devapi_port::port_db_;

devapi_port *
devapi_port::factory(uint32_t port_num)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_port *port = NULL;

    api_trace("port create");

    if (port_db_.find(port_num) != port_db_.end()) {
        NIC_LOG_WARN("Duplicate create of port with port_num: {}",
                     port_num);
        return NULL;
    }

    NIC_LOG_DEBUG("port_num: {}", port_num);

    mem = (devapi_port *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_PORT,
                                  sizeof(devapi_port));
    if (mem) {
        port = new (mem) devapi_port();
        ret = port->init_(port_num);
        if (ret != SDK_RET_OK) {
            port->~devapi_port();
            DEVAPI_FREE(DEVAPI_MEM_ALLOC_PORT, mem);
            port = NULL;
            goto end;
        }
    }

    // Store in DB
    port_db_[port_num] = port;

end:
    return port;
}

sdk_ret_t
devapi_port::init_(uint32_t port_num)
{
    port_num_ = port_num;

    return SDK_RET_OK;
}

void
devapi_port::destroy(devapi_port *port)
{
    if (!port) {
        return;
    }
    api_trace("port delete");

    port_db_.erase(port->get_port_num());

    port->~devapi_port();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_PORT, port);
}

devapi_port *
devapi_port::get_port(uint32_t port_num)
{
    if (port_db_.find(port_num) == port_db_.end()) {
        NIC_LOG_WARN("No port with port_num: {}",
                     port_num);
        return NULL;
    }

    return port_db_[port_num];
}

devapi_port *
devapi_port::find_or_create(uint32_t port_num)
{
    devapi_port *port = NULL;

    port = devapi_port::get_port(port_num);
    if (!port) {
        NIC_LOG_DEBUG("Failed to find devapi_port for port: {}. Creating ...",
                      port_num);
        port = devapi_port::factory(port_num);
        if (!port) {
            NIC_LOG_ERR("Failed to create devapi_port for: {}",
                        port_num);
            goto end;
        }
    }
end:
    return port;
}

sdk_ret_t
devapi_port::port_get_(PortGetResponse *rsp)
{
    sdk_ret_t ret = SDK_RET_OK;
    grpc::Status status;
    PortGetRequestMsg  req_msg;
    PortGetResponseMsg rsp_msg;
    PortGetResponse port_resp;

    auto req = req_msg.add_request();
    req->mutable_key_or_handle()->set_port_id(port_num_);
    VERIFY_HAL();
    status = hal->port_get(req_msg, rsp_msg);
    if (!status.ok()) {
        NIC_LOG_ERR("Failed to get port {}. err: {}, err_msg: {}",
                    port_num_,
                    status.error_code(),
                    status.error_message());
        ret = SDK_RET_ERR;
        goto end;
    }

    port_resp = rsp_msg.response(0);
    if (port_resp.api_status() != types::API_STATUS_OK) {
        NIC_LOG_ERR("Failed: API status {} port {}", port_resp.api_status(), port_num_);
        ret = SDK_RET_ERR;
        goto end;
    }
    rsp->CopyFrom(port_resp);

end:
    return ret;
}

sdk_ret_t
devapi_port::port_hal_get_config(port_config_t *cfg)
{
    sdk_ret_t       ret = SDK_RET_OK;
    PortGetResponse rsp;

    ret = port_get_(&rsp);
    if (ret != SDK_RET_OK) {
        goto end;
    }

    cfg->state = rsp.spec().admin_state();
    cfg->speed = port_speed_enum_to_mbps(rsp.spec().port_speed());
    NIC_FUNC_DEBUG("{}: port_speed_enum {} port_speed {}", port_num_,
                   rsp.spec().port_speed(), cfg->speed);
    cfg->mtu = rsp.spec().mtu();
    cfg->an_enable = rsp.spec().auto_neg_enable();
    cfg->fec_type = rsp.spec().fec_type();
    cfg->pause_type = rsp.spec().pause();
    if (rsp.spec().tx_pause_enable())
        cfg->pause_type |= PORT_CFG_PAUSE_F_TX;
    if (rsp.spec().rx_pause_enable())
        cfg->pause_type |= PORT_CFG_PAUSE_F_RX;
    cfg->loopback_mode = rsp.spec().loopback_mode();

    NIC_FUNC_DEBUG("{}: state {} speed {} mtu {} "
                   "an_enable {} fec_type {} pause_type {} loopback_mode {}",
                   port_num_, cfg->state, cfg->speed, cfg->mtu,
                   cfg->an_enable, cfg->fec_type, cfg->pause_type,
                   cfg->loopback_mode);

end:
    return ret;
}

sdk_ret_t
devapi_port::port_hal_get_status(port_status_t *st)
{
    sdk_ret_t       ret = SDK_RET_OK;
    PortGetResponse rsp;

    ret = port_get_(&rsp);
    if (ret != SDK_RET_OK) {
        goto end;
    }
    st->id = rsp.status().key_or_handle().port_id();
    st->status = rsp.status().link_status().oper_state();
    st->speed = port_speed_enum_to_mbps(
                    rsp.status().link_status().port_speed());
    NIC_FUNC_DEBUG("{}: port_speed_enum {} port_speed {}", port_num_,
                   rsp.status().link_status().port_speed(), st->speed);
    st->xcvr.state = rsp.status().xcvr_status().state();
    st->xcvr.phy = rsp.status().xcvr_status().cable_type();
    st->xcvr.pid = rsp.status().xcvr_status().pid();
    memcpy(st->xcvr.sprom,
           rsp.status().xcvr_status().xcvr_sprom().c_str(),
           MIN(rsp.status().xcvr_status().xcvr_sprom().size(),
               sizeof (st->xcvr.sprom)));

    NIC_FUNC_DEBUG("{}: id {} status {} speed {} "
                   " xcvr.state {} xcvr.phy {} xcvr.pid {}",
                   port_num_, st->id, st->status, st->speed,
                   st->xcvr.state, st->xcvr.phy, st->xcvr.pid);

end:
    return ret;
}

sdk_ret_t
devapi_port::port_hal_update_config(port_config_t *cfg)
{
    sdk_ret_t       ret = SDK_RET_OK;
    PortSpec        *req;
    PortRequestMsg  req_msg;
    PortResponseMsg rsp_msg;
    grpc::Status    status;
    PortGetResponse rsp;
    PortResponse    port_rsp;
    uint32_t num_lanes;
    port::PortSpeed port_speed;

    ret = port_get_(&rsp);
    if (ret != SDK_RET_OK) {
        goto end;
    }

    // TODO: Introduce port profiles in device.conf and 
    //       remvoe this micro-seg check.
    //       In GS case we dont want to propogate the IF down on host
    //       to physical port as we can lose venice connectivity.
    //       So in this scenario we should not propogate the IF down.
    if (hal->get_micro_seg_en()) {
        NIC_FUNC_DEBUG("Not allowed in classic nic mode ... ignoring!");
        return (SDK_RET_OK);
    }

    req = req_msg.add_request();
    req->CopyFrom(rsp.spec());
    req->set_admin_state((port::PortAdminState)cfg->state);
    ret = port_speed_mbps_to_enum(cfg->speed, &port_speed);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    req->set_port_speed(port_speed);
    req->set_mtu(cfg->mtu);
    req->set_auto_neg_enable(cfg->an_enable);
    req->set_fec_type((port::PortFecType)cfg->fec_type);
    req->set_pause((port::PortPauseType)(cfg->pause_type & PORT_CFG_PAUSE_TYPE_MASK));
    req->set_tx_pause_enable(cfg->pause_type & PORT_CFG_PAUSE_F_TX);
    req->set_rx_pause_enable(cfg->pause_type & PORT_CFG_PAUSE_F_RX);
    req->set_loopback_mode((port::PortLoopBackMode)cfg->loopback_mode);
    req->set_mac_stats_reset(cfg->reset_mac_stats);

    // TODO
    // For NRZ serdes, num_lanes can be assumed based on speed.
    // However, for PAM4, num_lanes needs to be decided based on how the
    // link must be brought up - with single lane or multiple lanes.
    // set the number of lanes based on speed if AN is disabled
    if (cfg->an_enable == 0) {
        ret = sdk::lib::port_num_lanes_speed(cfg->speed, &num_lanes);
        if (ret != SDK_RET_OK) {
            return ret;
        }
        req->set_num_lanes(num_lanes);
    }
    VERIFY_HAL();
    status = hal->port_update(req_msg, rsp_msg);
    if (status.ok()) {
        port_rsp = rsp_msg.response(0);
        if (port_rsp.api_status() == types::API_STATUS_OK) {
            NIC_LOG_DEBUG("port {} update succeded ", port_num_);
        } else {
            NIC_LOG_ERR("Failed to update port: {} err: {}", port_num_,
                        port_rsp.api_status());
        }
    } else {
        NIC_LOG_ERR("Failed to update port: {} err: {}, err_msg: {}",
                    port_num_,
                    status.error_code(),
                    status.error_message());
    }

    NIC_FUNC_DEBUG("{}: state {} speed {} mtu {} "
                   "an_enable {} fec_type {} pause_type {} loopback_mode {}",
                   port_num_, cfg->state, cfg->speed, cfg->mtu,
                   cfg->an_enable, cfg->fec_type, cfg->pause_type, cfg->loopback_mode);

end:
    return ret;
}

uint32_t
devapi_port::get_port_num(void)
{
    return port_num_;
}

uint32_t
devapi_port::port_speed_enum_to_mbps(port::PortSpeed speed_enum)
{
    uint32_t speed = 0;

    switch (speed_enum) {
    case port::PortSpeed::PORT_SPEED_NONE:
        speed = 0;
        break;
    case port::PortSpeed::PORT_SPEED_1G:
        speed = 1000;
        break;
    case port::PortSpeed::PORT_SPEED_10G:
        speed = 10000;
        break;
    case port::PortSpeed::PORT_SPEED_25G:
        speed = 25000;
        break;
    case port::PortSpeed::PORT_SPEED_40G:
        speed = 40000;
        break;
    case port::PortSpeed::PORT_SPEED_50G:
        speed = 50000;
        break;
    case port::PortSpeed::PORT_SPEED_100G:
        speed = 100000;
        break;
    default:
        NIC_FUNC_ERR("Invalid speed {}", speed);
        speed = 0;
    }

    return speed;
}

sdk_ret_t
devapi_port::port_speed_mbps_to_enum (uint32_t speed,
                                      port::PortSpeed *port_speed)
{
    switch (speed) {
    case 0:
        *port_speed = port::PortSpeed::PORT_SPEED_NONE;
        break;
    case 1000:
        *port_speed = port::PortSpeed::PORT_SPEED_1G;
        break;
    case 10000:
        *port_speed = port::PortSpeed::PORT_SPEED_10G;
        break;
    case 25000:
        *port_speed = port::PortSpeed::PORT_SPEED_25G;
        break;
    case 40000:
        *port_speed = port::PortSpeed::PORT_SPEED_40G;
        break;
    case 50000:
        *port_speed = port::PortSpeed::PORT_SPEED_50G;
        break;
    case 100000:
        *port_speed = port::PortSpeed::PORT_SPEED_100G;
        break;
    default:
        NIC_FUNC_ERR("Invalid port_speed {}", speed);
        *port_speed = port::PortSpeed::PORT_SPEED_NONE;
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

}    // namespace iris
