package enterprise

import (
	"context"

	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	cloudCfg "github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/cloud"
	gsCfg "github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/gsCfg"
	vcenterCfg "github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/vcenterCfg"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
)

//CfgModel interface
type CfgModel interface {
	objClient.ObjClient

	//Initialize client
	InitClient(ctx context.Context, urls []string)

	PopulateConfig(params *base.ConfigParams) error
	IsConfigPushComplete() (bool, error)
	CleanupAllConfig() error
	PushConfig() error
	ObjClient() objClient.ObjClient
	//Get all generated objects which may be modified before push
	GetCfgObjects() base.CfgObjects
}

//CfgType type
type CfgType int

const (
	//GsCfgType for GS
	GsCfgType CfgType = iota
	//VcenterCfgType for Vcenter
	VcenterCfgType

	//CloudCfgType for cloud
	CloudCfgType
)

//NewCfgModel new config model
func NewCfgModel(cfgType CfgType) CfgModel {

	switch cfgType {
	case GsCfgType:
		return gsCfg.NewGsCfg()
	case VcenterCfgType:
		return vcenterCfg.NewVcenterCfg()
	case CloudCfgType:
		return cloudCfg.NewCloudCfg()
	}

	return nil
}
