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
func (u *NaplesUpgClient) StartUpgOnNextHostReboot() error {
	if u.upgsdk != nil {
		return u.upgsdk.StartNonDisruptiveUpgrade()
	}
	go u.UpgSuccessful()
	return nil
}

// StartDisruptiveUpgrade using upgrade manager
func (u *NaplesUpgClient) StartDisruptiveUpgrade() error {
	if u.upgsdk != nil {
		return u.upgsdk.StartDisruptiveUpgrade()
	}

	go u.UpgSuccessful()
	return nil
}

// StartPreCheckDisruptive using upgrade manager
func (u *NaplesUpgClient) StartPreCheckDisruptive(version string) error {
	if u.upgsdk != nil {
		return u.upgsdk.CanPerformDisruptiveUpgrade()
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

// HandleUpgStateCompatCheck handles compat checks and replies to upgrade manager
func (u *NaplesUpgClient) HandleUpgStateCompatCheck(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePreUpgState called")
	return hdlrResp
}

// HandleUpgStatePostBinRestart is called after Binaries Restarted
func (u *NaplesUpgClient) HandleUpgStatePostBinRestart(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStatePostBinRestart called")
	return hdlrResp
}

// HandleUpgStateProcessQuiesce to Quiesce traffic - Mainly for data traffic..
func (u *NaplesUpgClient) HandleUpgStateProcessQuiesce(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateProcessesQuiesced called")
	return hdlrResp
}

// HandleUpgStateLinkDown - when link goes down
func (u *NaplesUpgClient) HandleUpgStateLinkDown(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateLinkDown called")
	return hdlrResp
}

// HandleUpgStateDataplaneDowntimeStart is applicable mainly for datapath component
func (u *NaplesUpgClient) HandleUpgStateDataplaneDowntimeStart(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateDataplaneDowntimeStart called")
	return hdlrResp
}

// HandleUpgStateDataplaneDowntimePhase1 is applicable mainly for datapath component
func (u *NaplesUpgClient) HandleUpgStateDataplaneDowntimePhase1(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase1Start called")
	return hdlrResp
}

// HandleUpgStateDataplaneDowntimePhase2 is applicable mainly for datapath component
func (u *NaplesUpgClient) HandleUpgStateDataplaneDowntimePhase2(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase2Start called")
	return hdlrResp
}

// HandleUpgStateDataplaneDowntimePhase3 is applicable mainly for datapath component
func (u *NaplesUpgClient) HandleUpgStateDataplaneDowntimePhase3(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase3Start called")
	return hdlrResp
}

// HandleUpgStateDataplaneDowntimePhase4 is applicable mainly for datapath component
func (u *NaplesUpgClient) HandleUpgStateDataplaneDowntimePhase4(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateDataplaneDowntimePhase4Start called")
	return hdlrResp
}

// HandleUpgStateCleanup cleans up any upgrade state
func (u *NaplesUpgClient) HandleUpgStateCleanup(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleStateCleanup called")
	return hdlrResp
}

// HandleUpgStateSuccess is called after upgrade is declared success
func (u *NaplesUpgClient) HandleUpgStateSuccess(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgSuccess called")
	return
}

// HandleUpgStateFailed is called after upgrade is declared failure
func (u *NaplesUpgClient) HandleUpgStateFailed(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgFailed called")
	return
}

// HandleUpgStateAbort is called when upgrade is aborted
func (u *NaplesUpgClient) HandleUpgStateAbort(upgCtx *upggosdk.UpgCtx) {
	log.Infof("HandleStateUpgAborted called")
	return
}

// HandleUpgStateIsSystemReady is called when ...
func (u *NaplesUpgClient) HandleUpgStateIsSystemReady(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateIsSystemReady called")
	return hdlrResp
}

// HandleUpgStateLinkUp is called when ...
func (u *NaplesUpgClient) HandleUpgStateLinkUp(upgCtx *upggosdk.UpgCtx) upggosdk.HdlrResp {
	var hdlrResp upggosdk.HdlrResp
	hdlrResp.Resp = upggosdk.Success
	hdlrResp.ErrStr = ""
	log.Infof("HandleUpgStateLinkUp called")
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

// UpgStateCompatCheckCompletionHandler is called after Compat check is completed
func (u *NaplesUpgClient) UpgStateCompatCheckCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCompatCheckCompletionHandlergot called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateProcessQuiesceCompletionHandler is called after Quiesceing
func (u *NaplesUpgClient) UpgStateProcessQuiesceCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateProcessQuiesceCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateLinkDownCompletionHandler is called after linkdown
func (u *NaplesUpgClient) UpgStateLinkDownCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateLinkDownCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStatePostBinRestartCompletionHandler is called after Binaries Restarted
func (u *NaplesUpgClient) UpgStatePostBinRestartCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStatePostBinRestartCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateDataplaneDowntimePhase1CompletionHandler is called after phase1 completed
func (u *NaplesUpgClient) UpgStateDataplaneDowntimePhase1CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase1CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateDataplaneDowntimePhase2CompletionHandler is called after phase2 completed
func (u *NaplesUpgClient) UpgStateDataplaneDowntimePhase2CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase2CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateDataplaneDowntimePhase3CompletionHandler is called after phase3 completed
func (u *NaplesUpgClient) UpgStateDataplaneDowntimePhase3CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase3CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateDataplaneDowntimePhase4CompletionHandler is called after phase4 completed
func (u *NaplesUpgClient) UpgStateDataplaneDowntimePhase4CompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimePhase4CompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateCleanupCompletionHandler is called after cleanup is completed
func (u *NaplesUpgClient) UpgStateCleanupCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateCleanupCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateAbortCompletionHandler is called upgrade is aborted
func (u *NaplesUpgClient) UpgStateAbortCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateAbortCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateDataplaneDowntimeStartCompletionHandler is called when DataplaneDowntimeStart is completed
func (u *NaplesUpgClient) UpgStateDataplaneDowntimeStartCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateDataplaneDowntimeStartCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateIsSystemReadyCompletionHandler is called when IsSystemReady is completed
func (u *NaplesUpgClient) UpgStateIsSystemReadyCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateIsSystemReadyCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}

// UpgStateLinkUpCompletionHandler is called when IsSystemReady is completed
func (u *NaplesUpgClient) UpgStateLinkUpCompletionHandler(resp *upggosdk.HdlrResp, svcName string) {
	log.Infof("UpgStateLinkUpCompletionHandler got called with status %d error %s for service %s", resp.Resp, resp.ErrStr, svcName)
}
