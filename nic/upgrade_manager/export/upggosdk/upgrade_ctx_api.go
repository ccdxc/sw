package upggosdk

import (
	"errors"

	"github.com/pensando/sw/nic/upgrade_manager/export/upggosdk/nic/upgrade_manager/upgrade"
)

func upgCtxGetTableVersion(name string, table map[string]TableMeta) (int, error) {
	if name == "" {
		return 0, errors.New("Table name is not set")
	}
	if value, ok := table[name]; ok {
		return value.Version, nil
	}
	return 0, errors.New("Table not found")
}

//UpgCtxGetPreUpgTableVersion is used to get the version of a table identified by name
// in pre-upgrade image
func UpgCtxGetPreUpgTableVersion(ctx *UpgCtx, name string) (int, error) {
	return upgCtxGetTableVersion(name, ctx.PreUpgTables)
}

//UpgCtxGetPostUpgTableVersion is used to get the version of a table identified by name
// in post-upgrade image
func UpgCtxGetPostUpgTableVersion(ctx *UpgCtx, name string) (int, error) {
	return upgCtxGetTableVersion(name, ctx.PostUpgTables)
}

func upgCtxGetComponentVersion(name string, comp map[string]ComponentMeta) (int, error) {
	if name == "" {
		return 0, errors.New("Component name is not set")
	}
	if value, ok := comp[name]; ok {
		return value.Version, nil
	}
	return 0, errors.New("Component not found")
}

//UpgCtxGetPreUpgComponentVersion is used to get the version of a component identified by name
// in pre-upgrade image
func UpgCtxGetPreUpgComponentVersion(ctx *UpgCtx, name string) (int, error) {
	return upgCtxGetComponentVersion(name, ctx.PreUpgComps)
}

//UpgCtxGetPostUpgComponentVersion is used to get the version of a component identified by name
// in post-upgrade image
func UpgCtxGetPostUpgComponentVersion(ctx *UpgCtx, name string) (int, error) {
	return upgCtxGetComponentVersion(name, ctx.PostUpgComps)
}

//UpgCtxIsUpgTypeNonDisruptive used to know if it is non-disruptive upgrade
func UpgCtxIsUpgTypeNonDisruptive(ctx *UpgCtx) bool {
	return ctx.upgType == upgrade.UpgType_UpgTypeNonDisruptive
}

//UpgCtxIsUpgTypeDisruptive used to know if it is disruptive upgrade
func UpgCtxIsUpgTypeDisruptive(ctx *UpgCtx) bool {
	return ctx.upgType == upgrade.UpgType_UpgTypeDisruptive
}
