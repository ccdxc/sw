package cloud

import (
	"context"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/cfgen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
)

//GsCfg encapsulate all Gs configuration objects
type CloudCfg struct {
	*objClient.Client
	//naples map[string]*Naples // Naples instances
	Dscs []*cluster.DistributedServiceCard

	Cfg *cfgen.Cfgen
}

func NewCloudCfg() *CloudCfg {
	return &CloudCfg{}
}

func (cl *CloudCfg) ObjClient() objClient.ObjClient {
	return cl.Client
}

//InitClient init client
func (cl *CloudCfg) InitClient(ctx context.Context, urls []string) {
	cl.Client = objClient.NewClient(ctx, urls).(*objClient.Client)
}

//PopulateConfig populate configuration
func (cl *CloudCfg) PopulateConfig(params *base.ConfigParams) error {

	cl.Cfg = cfgen.DefaultCfgenParams

	//Reset config stats so that we can start fresh

	return nil
}

//IsConfigPushComplete checks whether config push is complete.
func (cl *CloudCfg) IsConfigPushComplete() (bool, error) {

	return false, nil
}

//CleanupAllConfig clean up all config
func (cl *CloudCfg) CleanupAllConfig() error {

	return nil
}

//PushConfig populate configuration
func (cl *CloudCfg) PushConfig() error {
	return nil
}
