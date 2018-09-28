package server

import (
	"context"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

type CfgServer struct{}

func NewConfigService() *CfgServer {
	var cfgServer CfgServer
	return &cfgServer
}

//InitCfgService initiates a config management service
func (c CfgServer) InitCfgService(ctx context.Context, req *iota.InitConfigMsg) (resp *iota.InitConfigMsg, err error) {

	return
}


// GenerateConfigs generates base configs
func (c *CfgServer) GenerateConfigs(ctx context.Context, req *iota.GenerateConfigMsg) (resp *iota.ConfigMsg, err error) {
	return
}

// PushConfig pushes the config
func (c *CfgServer) PushConfig(ctx context.Context, req *iota.ConfigMsg) (resp *iota.ConfigMsg, err error) {
	return
}

// QueryConfig queries the configs
func (c *CfgServer) QueryConfig(ctx context.Context, req *iota.ConfigQueryMsg) (resp *iota.ConfigMsg, err error) {
	return
}
