// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NICMGR_DELPHI_CLIENT_HPP__
#define __NICMGR_DELPHI_CLIENT_HPP__

#include <memory>
#include <string>
#include <iostream>
#include <grpc++/grpc++.h>

#include "gen/proto/nicmgr/nicmgr.delphi.hpp"
#include "gen/proto/nicmgr/accel_metrics.delphi.hpp"
#include "gen/proto/port.delphi.hpp"
#include "gen/proto/hal.delphi.hpp"
#include "gen/proto/device.delphi.hpp"

#include "sysmgr.hpp"
#include "upgrade.hpp"

namespace dobj = delphi::objects;

namespace nicmgr {

using nicmgr::nicmgr_upg_hndlr;
using nicmgr::sysmgr_client;
using grpc::Status;
using delphi::error;
using dobj::PortStatusPtr;
using dobj::HalStatusPtr;
using dobj::SystemSpecPtr;
using dobj::NcsiVlanFilterPtr;
using dobj::NcsiMacFilterPtr;
using dobj::NcsiBcastFilterPtr;
using dobj::NcsiChanRxPtr;
using dobj::NcsiChanTxPtr;
using dobj::NcsiMcastFilterPtr;
using google::protobuf::Message;

class NicMgrService : public delphi::Service,
                      public enable_shared_from_this<NicMgrService> {
private:
     std::shared_ptr<nicmgr::sysmgr_client>    sysmgr_;
    UpgSdkPtr                                  upgsdk_;
    delphi::SdkPtr                             sdk_;

public:
    delphi::SdkPtr sdk(void) const { return sdk_; }
    UpgSdkPtr upgsdk(void) const { return upgsdk_; }
    NicMgrService(delphi::SdkPtr sk);
    void OnMountComplete(void);
    virtual std::string Name();
};

// port_status_handler is the reactor for the Port object
class port_status_handler : public dobj::PortStatusReactor {
public:
    port_status_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    // OnPortCreate gets called when PortStatus object is created
    virtual error OnPortStatusCreate(PortStatusPtr port);

    // OnPortUpdate gets called when PortStatus object is updated
    virtual error OnPortStatusUpdate(PortStatusPtr port);

    // OnPortDelete gets called when PortStatus object is deleted
    virtual error OnPortStatusDelete(PortStatusPtr port);

    // update_port_status updates port status in delphi
    error update_port_status(PortStatusPtr port);
private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<port_status_handler> port_status_handler_ptr_t;

// init_port_status_handler creates a port reactor
Status init_port_status_handler(delphi::SdkPtr sdk);


// System Spec Watcher
class system_spec_handler : public dobj::SystemSpecReactor {
public:
    system_spec_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    virtual error OnSystemSpecCreate(SystemSpecPtr obj);
    virtual error OnSystemSpecUpdate(SystemSpecPtr obj);
    virtual error OnSystemSpecDelete(SystemSpecPtr obj);

    error update_system_spec(SystemSpecPtr obj);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<system_spec_handler> system_spec_handler_ptr_t;
Status init_system_spec_handler(delphi::SdkPtr sdk);


class hal_status_handler : public dobj::HalStatusReactor {
public:
    hal_status_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
        status = false;
    }
    virtual error OnHalStatusCreate(HalStatusPtr status);
    virtual error OnHalStatusUpdate(HalStatusPtr status);
    error update_hal_status(HalStatusPtr status);
private:
    delphi::SdkPtr    sdk_;
    bool status;
};
typedef std::shared_ptr<hal_status_handler> hal_status_handler_ptr_t;
// init_hal_status_handler creates a reactor for HAL status
Status init_hal_status_handler(delphi::SdkPtr sdk);

//------------------------------------------------------------------------------
// Vlan filter
//------------------------------------------------------------------------------
class ncsi_vlan_filter_handler : public dobj::NcsiVlanFilterReactor {
public:
    ncsi_vlan_filter_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    virtual error OnNcsiVlanFilterCreate(NcsiVlanFilterPtr ncsiVlanFilter);
    virtual error OnNcsiVlanFilterDelete(NcsiVlanFilterPtr ncsiVlanFilter);
    error update_vlan_filter(NcsiVlanFilterPtr ncsiVlanFilter, bool create);
private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<ncsi_vlan_filter_handler> ncsi_vlan_filter_handler_ptr_t;
Status init_ncsi_vlan_filter_handler(delphi::SdkPtr sdk);

//------------------------------------------------------------------------------
// MAC filter 
//------------------------------------------------------------------------------
class ncsi_mac_filter_handler : public dobj::NcsiMacFilterReactor {
public:
    ncsi_mac_filter_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }

    virtual error OnNcsiMacFilterCreate(NcsiMacFilterPtr obj);
    virtual error OnNcsiMacFilterDelete(NcsiMacFilterPtr obj);
    error update_mac_filter(NcsiMacFilterPtr obj, bool create);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<ncsi_mac_filter_handler> ncsi_mac_filter_handler_ptr_t;
Status init_ncsi_mac_filter_handler(delphi::SdkPtr sdk);

//------------------------------------------------------------------------------
// RX Enable
//------------------------------------------------------------------------------
class ncsi_chan_rx_handler : public dobj::NcsiChanRxReactor {
public:
    ncsi_chan_rx_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }
    virtual error OnNcsiChanRxCreate(NcsiChanRxPtr obj);
    virtual error OnNcsiChanRxDelete(NcsiChanRxPtr obj);
    error update_rx(NcsiChanRxPtr obj, bool create);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<ncsi_chan_rx_handler> ncsi_chan_rx_handler_ptr_t;
Status init_ncsi_chan_rx_handler(delphi::SdkPtr sdk);


//------------------------------------------------------------------------------
// TX Enable
//------------------------------------------------------------------------------
class ncsi_chan_tx_handler : public dobj::NcsiChanTxReactor {
public:
    ncsi_chan_tx_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }
    virtual error OnNcsiChanTxCreate(NcsiChanTxPtr obj);
    virtual error OnNcsiChanTxDelete(NcsiChanTxPtr obj);
    error update_tx(NcsiChanTxPtr obj, bool create);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<ncsi_chan_tx_handler> ncsi_chan_tx_handler_ptr_t;
Status init_ncsi_chan_tx_handler(delphi::SdkPtr sdk);

//------------------------------------------------------------------------------
// Bcast
//------------------------------------------------------------------------------
class ncsi_bcast_filter_handler : public dobj::NcsiBcastFilterReactor {
public:
    ncsi_bcast_filter_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }
    virtual error OnNcsiBcastFilterCreate(NcsiBcastFilterPtr obj);
    virtual error OnNcsiBcastFilterDelete(NcsiBcastFilterPtr obj);
    virtual error OnNcsiBcastFilterUpdate(NcsiBcastFilterPtr obj);
    error update_bcast(NcsiBcastFilterPtr obj, bool create_update);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<ncsi_bcast_filter_handler> ncsi_bcast_filter_handler_ptr_t;
Status init_ncsi_bcast_filter_handler(delphi::SdkPtr sdk);

//------------------------------------------------------------------------------
// Mcast
//------------------------------------------------------------------------------
class ncsi_mcast_filter_handler : public dobj::NcsiMcastFilterReactor {
public:
    ncsi_mcast_filter_handler(delphi::SdkPtr sdk) {
        this->sdk_ = sdk;
    }
    virtual error OnNcsiMcastFilterCreate(NcsiMcastFilterPtr obj);
    virtual error OnNcsiMcastFilterDelete(NcsiMcastFilterPtr obj);
    virtual error OnNcsiMcastFilterUpdate(NcsiMcastFilterPtr obj);
    error update_mcast(NcsiMcastFilterPtr obj, bool create_update);

private:
    delphi::SdkPtr    sdk_;
};
typedef std::shared_ptr<ncsi_mcast_filter_handler> ncsi_mcast_filter_handler_ptr_t;
Status init_ncsi_mcast_filter_handler(delphi::SdkPtr sdk);

// init_accel_objects mounts accelerator objects
Status init_accel_objects(delphi::SdkPtr sdk);

extern shared_ptr<NicMgrService> g_nicmgr_svc;

void delphi_init(void);

void handle_hal_up (void);

}    // namespace nicmgr

#endif
