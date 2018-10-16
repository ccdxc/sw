package cfg

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

// ConfigService implements config service API
type ConfigService struct {
	CfgState *iota.InitConfigMsg
}

// NewConfigServiceHandler returns an instance of config service
func NewConfigServiceHandler() *ConfigService {
	var cfgServer ConfigService
	return &cfgServer
}

// MakeCluster brings up venice cluster
func (c *ConfigService) MakeCluster(ctx context.Context, req *iota.MakeClusterMsg) (*iota.MakeClusterMsg, error) {
	log.Infof("CFG SVC | DEBUG | MakeCluster. Received Request Msg: %v", req)

	resp, err := common.HTTPPost(req.Endpoint, req.Config)
	log.Infof("CFG SVC | DEBUG | MakeCluster. Received REST Response Msg: %v", resp)

	if err != nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("server returned an error while making a cluster. Response: %v,  Err: %v", resp, err)
	}

	return req, nil
}

//InitCfgService initiates a config management service
func (c *ConfigService) InitCfgService(ctx context.Context, req *iota.InitConfigMsg) (*iota.InitConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | InitCfgService. Received Request Msg: %v", req)
	if len(req.Vlans) == 0 {
		log.Errorf("CFG SVC | InitCfgService call failed. | Missing allocated VLANs")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = "CFG SVC | InitCfgService call failed. | Missing allocated VLANs"
		return req, nil
	}
	if req.EntryPointType != iota.EntrypointType_VENICE_REST && req.EntryPointType != iota.EntrypointType_NAPLES_REST {
		log.Errorf("CFG SVC | InitCfgService call failed. | Bad entrypoint type. %v", req.EntryPointType)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | InitCfgService call failed. | Bad entrypoint type. %v", req.EntryPointType)
		return req, nil
	}
	c.CfgState = req
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// GenerateConfigs generates base configs
func (c *ConfigService) GenerateConfigs(ctx context.Context, req *iota.GenerateConfigMsg) (*iota.ConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | GenerateConfigs. Received Request Msg: %v", req)

	resp := &iota.ConfigMsg{}
	return resp, nil
}

// PushConfig pushes the config
func (c *ConfigService) PushConfig(ctx context.Context, req *iota.ConfigMsg) (*iota.ConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | PushConfig. Received Request Msg: %v", req)

	resp := &iota.ConfigMsg{}
	return resp, nil
}

// QueryConfig queries the configs
func (c *ConfigService) QueryConfig(ctx context.Context, req *iota.ConfigQueryMsg) (*iota.ConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | QueryConfig. Received Request Msg: %v", req)

	resp := &iota.ConfigMsg{}
	return resp, nil
}
