package upggosdk

import (
	"errors"

	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
)

//UpgState  of the state machine
type UpgState int

const (
	//COMPATCHECK state
	COMPATCHECK UpgState = 0
	//PRESWITCHROOT state
	PRESWITCHROOT UpgState = 1
	//POSTSWITCHROOT state
	POSTSWITCHROOT UpgState = 1
)

//UpgCtxGetPreUpgTableVersion is used to get the version of a component identified by meta
// in pre-upgrade image
func UpgCtxGetPreUpgTableVersion(ctx *UpgCtx, meta CompVersion) (string, error) {
	switch meta {
	case NICMGR:
		return ctx.PreUpgMeta.NicmgrVersion, nil
	case KERNEL:
		return ctx.PreUpgMeta.KernelVersion, nil
	case PCIE:
		return ctx.PreUpgMeta.PcieVersion, nil
	}
	return "", errors.New("Version not found")
}

//UpgCtxGetPostUpgTableVersion is used to get the version of a component identified by meta
// in post-upgrade image
func UpgCtxGetPostUpgTableVersion(ctx *UpgCtx, meta CompVersion) (string, error) {
	switch meta {
	case NICMGR:
		return ctx.PostUpgMeta.NicmgrVersion, nil
	case KERNEL:
		return ctx.PostUpgMeta.KernelVersion, nil
	case PCIE:
		return ctx.PostUpgMeta.PcieVersion, nil
	}
	return "", errors.New("Version not found")
}

//UpgCtxIsUpgTypeNonDisruptive used to know if it is non-disruptive upgrade
func UpgCtxIsUpgTypeNonDisruptive(ctx *UpgCtx) bool {
	return ctx.upgType == upgrade.UpgType_UpgTypeNonDisruptive
}

//UpgCtxIsUpgTypeDisruptive used to know if it is disruptive upgrade
func UpgCtxIsUpgTypeDisruptive(ctx *UpgCtx) bool {
	return ctx.upgType == upgrade.UpgType_UpgTypeDisruptive
}

//UpgCtxGetUpgState used to get previous upgrade state
func UpgCtxGetUpgState(ctx *UpgCtx) UpgState {
	switch ctx.prevExecState {
	case upgrade.UpgReqStateType_UpgStateCompatCheck:
		return COMPATCHECK
	case upgrade.UpgReqStateType_UpgStateLinkDown:
		return PRESWITCHROOT
	case upgrade.UpgReqStateType_UpgStateHostDown:
		return PRESWITCHROOT
	case upgrade.UpgReqStateType_UpgStatePostHostDown:
		return PRESWITCHROOT
	case upgrade.UpgReqStateType_UpgStateSaveState:
		return PRESWITCHROOT

	case upgrade.UpgReqStateType_UpgStatePostRestart:
		return POSTSWITCHROOT
	case upgrade.UpgReqStateType_UpgStateHostUp:
		return POSTSWITCHROOT
	case upgrade.UpgReqStateType_UpgStateLinkUp:
		return POSTSWITCHROOT
	case upgrade.UpgReqStateType_UpgStatePostLinkUp:
		return POSTSWITCHROOT
	}
	return COMPATCHECK
}
