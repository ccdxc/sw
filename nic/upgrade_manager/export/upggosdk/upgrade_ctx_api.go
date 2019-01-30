package upggosdk

import (
	"errors"

	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
)

//UpgCtxGetPreUpgTableVersion is used to get the version of a component identified by meta
// in pre-upgrade image
func UpgCtxGetPreUpgTableVersion(ctx *UpgCtx, meta CompVersion) (string, error) {
	switch meta {
	case NICMGR:
		return ctx.PreUpgMeta.NicmgrVersion, nil
	case KERNEL:
		// TODO
		return ctx.PreUpgMeta.NicmgrVersion, nil
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
		// TODO
		return ctx.PostUpgMeta.NicmgrVersion, nil
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
