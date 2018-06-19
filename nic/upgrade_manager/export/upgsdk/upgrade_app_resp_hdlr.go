package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

func canInvokeHandler(reqType upgrade.UpgReqStateType) bool {
	return true
}

func createUpgAppResp(sdkClient gosdk.Client, name string) {
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp == nil {
		upgAppResp = upgrade.NewUpgAppResp(sdkClient)
		if upgAppResp == nil {
			log.Infof("application unable to create response object")
		}
		upgAppResp.SetKey(name)
	}
}

func upgAppRespValToStr(respStateType upgrade.UpgRespStateType) string {
	if respStateType%2 == 0 {
		return stateMachine[respStateType/2].upgAppRespValToStrPass
	}
	return stateMachine[respStateType/2].upgAppRespValToStrFail
}

func updateUpgAppResp(respStateType upgrade.UpgRespStateType, appHdlrResp *HdlrResp, name string, sdkClient gosdk.Client) {
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp == nil {
		log.Infof("UpgAppRespHdlr::UpdateUpgAppResp returning error for %s", name)
		return
	}
	if upgAppRespValToStr(respStateType) != "" {
		log.Infof("%s", upgAppRespValToStr(respStateType))
	}
	upgAppResp.SetUpgAppRespVal(respStateType)
	return
}

func deleteUpgAppResp(name string, sdkClient gosdk.Client) {
	log.Infof("deleteUpgAppResp called")
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp != nil {
		upgAppResp.Delete()
	}
}

func getUpgAppRespNextPass(reqType upgrade.UpgReqStateType) upgrade.UpgRespStateType {
	return stateMachine[reqType].statePassResp
}

func getUpgAppRespNextFail(reqType upgrade.UpgReqStateType) upgrade.UpgRespStateType {
	return stateMachine[reqType].stateFailResp
}

func getUpgAppRespNext(reqStateType upgrade.UpgReqStateType, isReqSuccess bool) upgrade.UpgRespStateType {
	if isReqSuccess {
		return getUpgAppRespNextPass(reqStateType)
	}
	return getUpgAppRespNextFail(reqStateType)
}
