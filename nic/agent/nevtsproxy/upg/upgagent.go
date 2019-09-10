// {C} Copyright 2017-2019 Pensando Systems Inc. All rights reserved.

package upg

import (
	"os"
	"sync"

	"github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/utils/log"
)

// NaplesUpgClient provides the interface to upgrade manager client with callbacks
type NaplesUpgClient struct {
	sync.Mutex
	evtsProxy *evtsproxy.EventsProxy // evtsproxy for updating the maintenance mode
	delphiSDK clientApi.Client       // handle to delphi
	upgsdk    upggosdk.UpgSdk        // handle to upgrade manager interaction
	logger    log.Logger             // logger
}

// NewNaplesUpgClient returns a new upgrade client
func NewNaplesUpgClient(delphiSDK clientApi.Client, logger log.Logger) (*NaplesUpgClient, error) {
	nua := NaplesUpgClient{
		delphiSDK: delphiSDK,
		logger:    logger,
	}

	var err error
	if delphiSDK != nil {
		nua.upgsdk, err = upggosdk.NewUpgSdk("nevtsproxy upgrade client", nua.delphiSDK, upggosdk.NonAgentRole, nil, &nua)
		if err != nil {
			logger.Errorf("error creating upgrade sdk client, err: %v", err)
			return nil, err
		}
	} else {
		logger.Info("bypassing delphi in upgrade Client")
	}

	return &nua, nil
}

// IsUpgradeInProcess returns true if the upgrade is in process, false otherwise
func (n *NaplesUpgClient) IsUpgradeInProcess() bool {
	_, err := os.Stat("/update/upgrade_halt_state_machine")
	if os.IsNotExist(err) {
		n.logger.Info("no upgrade in process, starting fresh")
		return false
	}

	n.logger.Info("upgrade in process, entering maintenance mode")
	return true
}

// RegisterEvtsProxy registers evtsproxy that will be used later
func (n *NaplesUpgClient) RegisterEvtsProxy(evtsProxy *evtsproxy.EventsProxy) {
	n.evtsProxy = evtsProxy
}

/* app handlers */

// CompatCheckHandler ...
func (n *NaplesUpgClient) CompatCheckHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// PostRestartHandler ...
func (n *NaplesUpgClient) PostRestartHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// ProcessQuiesceHandler ...
func (n *NaplesUpgClient) ProcessQuiesceHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// LinkDownHandler is called when the link goes down
func (n *NaplesUpgClient) LinkDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp

	if n.evtsProxy != nil {
		n.logger.Info("upgrade in process, entering maintenance mode")
		n.evtsProxy.SetMaintenanceMode(true)
	}

	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// LinkUpHandler ...
func (n *NaplesUpgClient) LinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// HostUpHandler ...
func (n *NaplesUpgClient) HostUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// HostDownHandler ...
func (n *NaplesUpgClient) HostDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// PostHostDownHandler ...
func (n *NaplesUpgClient) PostHostDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// SaveStateHandler ...
func (n *NaplesUpgClient) SaveStateHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// PostLinkUpHandler ...
func (n *NaplesUpgClient) PostLinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// DataplaneDowntimePhase1Handler ...
func (n *NaplesUpgClient) DataplaneDowntimePhase1Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// DataplaneDowntimePhase2Handler ...
func (n *NaplesUpgClient) DataplaneDowntimePhase2Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// DataplaneDowntimePhase3Handler ...
func (n *NaplesUpgClient) DataplaneDowntimePhase3Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// DataplaneDowntimePhase4Handler ...
func (n *NaplesUpgClient) DataplaneDowntimePhase4Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// SuccessHandler ...
func (n *NaplesUpgClient) SuccessHandler(upgCtx *upggosdk.UpgCtx) {
	if n.evtsProxy != nil {
		n.logger.Info("upgrade finished, leaving maintenance mode")
		n.evtsProxy.SetMaintenanceMode(false)
		return
	}
}

// FailedHandler ...
func (n *NaplesUpgClient) FailedHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp

	if n.evtsProxy != nil {
		n.logger.Info("upgrade finished, leaving maintenance mode")
		n.evtsProxy.SetMaintenanceMode(false)
	}

	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	return hdlrResp
}

// AbortHandler ...
func (n *NaplesUpgClient) AbortHandler(upgCtx *upggosdk.UpgCtx) {}
