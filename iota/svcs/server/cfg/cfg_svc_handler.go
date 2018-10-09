package cfg

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
)

// ConfigService implements config service API
type ConfigService struct{}

// NewConfigServiceHandler returns an instance of config service
func NewConfigServiceHandler() *ConfigService {
	var cfgServer ConfigService
	return &cfgServer
}

// MakeCluster brings up venice cluster
func (c *ConfigService) MakeCluster(ctx context.Context, req *iota.MakeClusterMsg) (*iota.MakeClusterMsg, error) {
	resp, err := common.HTTPPost(req.Endpoint, req.Config)
	fmt.Println("BALERION: ", resp)

	if err != nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("server returned an error while making a cluster. Response: %v,  Err: %v", resp, err)
	}

	return req, nil
}

//InitCfgService initiates a config management service
func (c *ConfigService) InitCfgService(ctx context.Context, req *iota.InitConfigMsg) (*iota.InitConfigMsg, error) {
	resp := &iota.InitConfigMsg{}
	return resp, nil
}

// GenerateConfigs generates base configs
func (c *ConfigService) GenerateConfigs(ctx context.Context, req *iota.GenerateConfigMsg) (*iota.ConfigMsg, error) {
	resp := &iota.ConfigMsg{}
	return resp, nil
}

// PushConfig pushes the config
func (c *ConfigService) PushConfig(ctx context.Context, req *iota.ConfigMsg) (*iota.ConfigMsg, error) {
	resp := &iota.ConfigMsg{}
	return resp, nil
}

// QueryConfig queries the configs
func (c *ConfigService) QueryConfig(ctx context.Context, req *iota.ConfigQueryMsg) (*iota.ConfigMsg, error) {
	resp := &iota.ConfigMsg{}
	return resp, nil
}
