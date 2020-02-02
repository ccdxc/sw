package factory

import (
	cfgModel "github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/enterprise"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/vcenter"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
)

// NewDefaultSysModel creates a sysmodel for a testbed
func NewDefaultSysModel(tb *testbed.TestBed, cfgType cfgModel.CfgType) (*enterprise.SysModel, error) {

	sm := &enterprise.SysModel{Type: common.DefaultModel}

	if err := sm.Init(tb, cfgType); err != nil {
		return nil, err
	}

	return sm, nil
}

// NewVcenterSysModel creates a sysmodel for a testbed
func NewVcenterSysModel(tb *testbed.TestBed, cfgType cfgModel.CfgType) (*vcenter.VcenterSysModel, error) {

	vsm := &vcenter.VcenterSysModel{SysModel: enterprise.SysModel{Type: common.VcenterModel}}

	if err := vsm.SysModel.Init(tb, cfgType); err != nil {
		return nil, err
	}

	return vsm, nil
}
