// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package upg

import (
	"sync"

	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/venice/utils/log"

	clientAPI "github.com/pensando/sw/nic/delphi/gosdk/client_api"
	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk"
)

const upgAgentIntegration = false

// NaplesUpgClient provides the interface to upgrade manager i.e implements UpgMgrRolloutAPI() interface
type NaplesUpgClient struct {
	sync.Mutex

	nmd       state.NmdRolloutAPI // NMD server instance
	delphiSDK clientAPI.Client    // handle to delphi
	upgsdk    upggosdk.UpgSdk     // handle to upgrade manager interaction
}

// NewNaplesUpgradeClient returns a new mock upgrade agent
func NewNaplesUpgradeClient(delphiSDK clientAPI.Client) (*NaplesUpgClient, error) {
	nua := NaplesUpgClient{
		delphiSDK: delphiSDK,
	}

	var err error
	if delphiSDK != nil {
		nua.upgsdk, err = upggosdk.NewUpgSdk("nmd upgrade client", nua.delphiSDK, upggosdk.AgentRole, &nua, &nua)
		if err != nil {
			log.Errorf("Error %s creating upgrade sdk client", err)
			return nil, err
		}
	} else {
		log.Infof("Bypassing Delphi in upgrade Client")
	}

	return &nua, nil
}

// RegisterNMD for further use
func (u *NaplesUpgClient) RegisterNMD(nmd state.NmdRolloutAPI) error {
	u.nmd = nmd
	return nil
}

// StartUpgOnNextHostReboot using upgrade manager
func (u *NaplesUpgClient) StartUpgOnNextHostReboot(firmwarePkgName string) error {
	if u.upgsdk != nil {
		return u.upgsdk.StartNonDisruptiveUpgrade(firmwarePkgName)
	}
	go u.UpgSuccessful()
	return nil
}

// StartDisruptiveUpgrade using upgrade manager
func (u *NaplesUpgClient) StartDisruptiveUpgrade(firmwarePkgName string) error {
	if u.upgsdk != nil {
		return u.upgsdk.StartDisruptiveUpgrade(firmwarePkgName)
	}

	go u.UpgSuccessful()
	return nil
}

// StartPreCheckDisruptive using upgrade manager
func (u *NaplesUpgClient) StartPreCheckDisruptive(version string) error {
	if u.upgsdk != nil {
		return u.upgsdk.CanPerformDisruptiveUpgrade("naples_fw.tar")
	}
	go u.UpgPossible(nil)
	return nil
}

// StartPreCheckForUpgOnNextHostReboot currently just downloads the image and returns success
func (u *NaplesUpgClient) StartPreCheckForUpgOnNextHostReboot(version string) error {
	// TODO call the download image API and return success when that succeeds
	go u.UpgPossible(nil)
	return nil
}

// === AppHandlers ===

// CompatCheckHandler handles compat checks and replies to upgrade manager
func (u *NaplesUpgClient) CompatCheckHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePreUpgState called")
	return hdlrResp
}

// PostRestartHandler is called after Binaries Restarted
func (u *NaplesUpgClient) PostRestartHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePostRestart called")
	return hdlrResp
}

// ProcessQuiesceHandler to Quiesce traffic - Mainly for data traffic..
func (u *NaplesUpgClient) ProcessQuiesceHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateProcessesQuiesced called")
	return hdlrResp
}

// LinkDownHandler - when link goes down
func (u *NaplesUpgClient) LinkDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateLinkDown called")
	return hdlrResp
}

// LinkUpHandler is called when ...
func (u *NaplesUpgClient) LinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateLinkUp called")
	return hdlrResp
}

// DataplaneDowntimePhase1Handler is applicable mainly for datapath component
func (u *NaplesUpgClient) DataplaneDowntimePhase1Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase1Start called")
	return hdlrResp
}

// DataplaneDowntimePhase2Handler is applicable mainly for datapath component
func (u *NaplesUpgClient) DataplaneDowntimePhase2Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase2Start called")
	return hdlrResp
}

// DataplaneDowntimePhase3Handler is applicable mainly for datapath component
func (u *NaplesUpgClient) DataplaneDowntimePhase3Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase3Start called")
	return hdlrResp
}

// DataplaneDowntimePhase4Handler is applicable mainly for datapath component
func (u *NaplesUpgClient) DataplaneDowntimePhase4Handler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase4Start called")
	return hdlrResp
}

// SuccessHandler is called after upgrade is declared success
func (u *NaplesUpgClient) SuccessHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgSuccess called")
	return
}

// FailedHandler is called after upgrade is declared failure
func (u *NaplesUpgClient) FailedHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgFailed called")
	return
}

// AbortHandler is called when upgrade is aborted
func (u *NaplesUpgClient) AbortHandler(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgAborted called")
	return
}

// HostUpHandler is called when ...
func (u *NaplesUpgClient) HostUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HostUp called")
	return hdlrResp
}

// HostDownHandler is called when ...
func (u *NaplesUpgClient) HostDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HostDown called")
	return hdlrResp
}

// PostHostDownHandler is called when ...
func (u *NaplesUpgClient) PostHostDownHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("PostHostDown called")
	return hdlrResp
}

// SaveStateHandler is called when ...
func (u *NaplesUpgClient) SaveStateHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("SaveState called")
	return hdlrResp
}

// PostLinkUpHandler is called when ...
func (u *NaplesUpgClient) PostLinkUpHandler(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("PostLinkUp called")
	return hdlrResp
}

// === Agent handler ===

// UpgSuccessful from upgrade manager
func (u *NaplesUpgClient) UpgSuccessful() {
	u.nmd.UpgSuccessful()
}

// UpgAborted from upgrade manager
func (u *NaplesUpgClient) UpgAborted(errStrList *[]string) {
	u.nmd.UpgAborted(errStrList)
}

// UpgFailed from upgrade manager
func (u *NaplesUpgClient) UpgFailed(errStrList *[]string) {
	u.nmd.UpgFailed(errStrList)
}

// UpgPossible is response for prechecks
func (u *NaplesUpgClient) UpgPossible(ctx *upggosdk.UpgCtx) {
	u.nmd.UpgPossible()
}

// UpgNotPossible is response for prechecks
func (u *NaplesUpgClient) UpgNotPossible(ctx *upggosdk.UpgCtx, errStrList *[]string) {
	u.nmd.UpgNotPossible(errStrList)
}

//CompatCheckCompletionHandler is called after Compat check is completed
func (u *NaplesUpgClient) CompatCheckCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCompatCheckCompletionHandlergot called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//ProcessQuiesceCompletionHandler is called after Quiesceing
func (u *NaplesUpgClient) ProcessQuiesceCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateProcessQuiesceCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//LinkDownCompletionHandler is called after linkdown
func (u *NaplesUpgClient) LinkDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateLinkDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//LinkUpCompletionHandler is called after linkdown
func (u *NaplesUpgClient) LinkUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateLinkUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//PostRestartCompletionHandler is called after Binaries Restarted
func (u *NaplesUpgClient) PostRestartCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePostRestartCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//DataplaneDowntimePhase1CompletionHandler is called after phase1 completed
func (u *NaplesUpgClient) DataplaneDowntimePhase1CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase1CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//DataplaneDowntimePhase2CompletionHandler is called after phase2 completed
func (u *NaplesUpgClient) DataplaneDowntimePhase2CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase2CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//DataplaneDowntimePhase3CompletionHandler is called after phase3 completed
func (u *NaplesUpgClient) DataplaneDowntimePhase3CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase3CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//DataplaneDowntimePhase4CompletionHandler is called after phase4 completed
func (u *NaplesUpgClient) DataplaneDowntimePhase4CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase4CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//AbortCompletionHandler is called upgrade is aborted
func (u *NaplesUpgClient) AbortCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateAbortCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//HostDownCompletionHandler is called when HostDown is completed
func (u *NaplesUpgClient) HostDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("HostDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//HostUpCompletionHandler is called when HostUp is completed
func (u *NaplesUpgClient) HostUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("HostUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//PostHostDownCompletionHandler is called when PostHostDown is completed
func (u *NaplesUpgClient) PostHostDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("PostHostDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//PostLinkUpCompletionHandler is called when PostLinkUp is completed
func (u *NaplesUpgClient) PostLinkUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("PostLinkUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

//SaveStateCompletionHandler is called when IsSystemReady is completed
func (u *NaplesUpgClient) SaveStateCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("SaveStateCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}
