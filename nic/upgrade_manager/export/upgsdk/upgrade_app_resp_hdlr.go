package upgsdk

import (
	"github.com/pensando/sw/nic/delphi/gosdk"
	"github.com/pensando/sw/nic/upgrade_manager/export/upgsdk/nic/upgrade_manager/upgrade"
	"github.com/pensando/sw/venice/utils/log"
)

func canInvokeHandler(sdkClient gosdk.Client, name string, reqType upgrade.UpgReqStateType) bool {
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp == nil {
		log.Infof("UpgAppResp not found for %s", name)
		return true
	}
	if getUpgAppRespNextPass(reqType) == upgAppResp.GetUpgAppRespVal() ||
		getUpgAppRespNextFail(reqType) == upgAppResp.GetUpgAppRespVal() {
		log.Infof("Application %s already responded with %s", name, upgAppRespValToStr(upgAppResp.GetUpgAppRespVal()))
		return false
	}
	log.Infof("Can invoke appliation handler")
	return true
}

func createUpgAppResp(sdkClient gosdk.Client, name string) {
	log.Infof("Creating UpgAppResp for %s", name)
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp == nil {
		upgAppResp = upgrade.NewUpgAppRespWithKey(sdkClient, name)
		if upgAppResp == nil {
			log.Infof("application unable to create response object")
		}
	}
}

func upgAppRespValToStr(respStateType upgrade.UpgStateRespType) string {
	return getUpgAppRespValToStr(respStateType)
}

func updateUpgAppResp(respStateType upgrade.UpgStateRespType, appHdlrResp *HdlrResp, name string, sdkClient gosdk.Client) {
	log.Infof("Updating UpgAppResp for %s", name)
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp == nil {
		log.Infof("UpgAppRespHdlr::UpdateUpgAppResp returning error for %s", name)
		return
	}
	if upgAppRespValToStr(respStateType) != "" {
		log.Infof("%s", upgAppRespValToStr(respStateType))
	}
	upgAppResp.SetUpgAppRespVal(respStateType)
	if appHdlrResp.Resp == Fail {
		upgAppResp.SetUpgAppRespStr(appHdlrResp.ErrStr)
	}
	return
}

func deleteUpgAppResp(name string, sdkClient gosdk.Client) {
	log.Infof("deleteUpgAppResp called")
	upgAppResp := upgrade.GetUpgAppResp(sdkClient, name)
	if upgAppResp != nil {
		upgAppResp.Delete()
	}
}

func getUpgAppRespNextPass(reqType upgrade.UpgReqStateType) upgrade.UpgStateRespType {
	return stateMachine[reqType].statePassResp
}

func getUpgAppRespNextFail(reqType upgrade.UpgReqStateType) upgrade.UpgStateRespType {
	return stateMachine[reqType].stateFailResp
}

func getUpgAppRespNext(reqStateType upgrade.UpgReqStateType, isReqSuccess bool) upgrade.UpgStateRespType {
	if isReqSuccess {
		return getUpgAppRespNextPass(reqStateType)
	}
	return getUpgAppRespNextFail(reqStateType)
}
