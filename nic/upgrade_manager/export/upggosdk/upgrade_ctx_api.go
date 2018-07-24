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

//UpgCtxGetUpgType used to know if it is disruptive or non-disruptive upgrade
func UpgCtxGetUpgType(ctx *UpgCtx) upgrade.UpgType {
	return ctx.upgType
}
