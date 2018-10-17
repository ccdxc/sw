package cfg

import (
	"context"
	"encoding/json"
	"fmt"
	"time"

	"github.com/pensando/sw/api/generated/workload"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

// ConfigService implements config service API
type ConfigService struct {
	CfgState   *iota.InitConfigMsg
	AuthToken  string
	NaplesUUID []string
	Workloads  []*workload.Workload
}

// NewConfigServiceHandler returns an instance of config service
func NewConfigServiceHandler() *ConfigService {
	var cfgServer ConfigService
	return &cfgServer
}

// MakeCluster brings up venice cluster
func (c *ConfigService) MakeCluster(ctx context.Context, req *iota.MakeClusterMsg) (*iota.MakeClusterMsg, error) {
	log.Infof("CFG SVC | DEBUG | MakeCluster. Received Request Msg: %v", req)
	var clusterObj cluster.Cluster
	var response string

	err := json.Unmarshal([]byte(req.Config), &clusterObj)
	if err != nil {
		log.Errorf("CFG SVC | DEBUG | MakeCluster. Could not unmarshal %v into a cluster object. Err: %v", req.Config, err)
	}

	log.Infof("CFG SVC | DEBUG | MakeCluster. Received cluster obj, %v", clusterObj)

	_, err = common.HTTPPost(req.Endpoint, "", &clusterObj, response)
	log.Infof("CFG SVC | DEBUG | MakeCluster. Received REST Response Msg: %v", response)

	if err != nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("server returned an error while making a cluster. Response: %v,  Err: %v", response, err)
		return req, nil
	}

	//err = common.WaitForSvcUp(req.HealthEndpoint, common.MakeClusterTimeout, http.StatusUnauthorized)
	//if err != nil {
	//	req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	//	req.ApiResponse.ErrorMsg = fmt.Sprintf("Cluster failed to come up.  Err: %v", err)
	//	return req, nil
	//}

	// TODO remove this hack once venice health APIs are available
	time.Sleep(time.Minute * 2)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
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

	if len(req.Endpoints) == 0 {
		log.Errorf("CFG SVC | InitCfgService call failed. | Missing REST Endpoints type. %v", req.EntryPointType)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | InitCfgService call failed. | Missing REST Endpoints type. %v", req.EntryPointType)
		return req, nil
	}
	c.CfgState = req
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// GenerateConfigs generates base configs
func (c *ConfigService) GenerateConfigs(ctx context.Context, req *iota.GenerateConfigMsg) (*iota.ConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | GenerateConfigs. Received Request Msg: %v", req)

	c.NaplesUUID = req.NaplesUuids

	genConfigs, err := c.generateConfigs()
	if err != nil {
		log.Errorf("CFG SVC | DEBUG | Generating Configs failed. Err: %v.", err)
		cfgMsg := iota.ConfigMsg{
			ApiResponse: &iota.IotaAPIResponse{
				ApiStatus: iota.APIResponseType_API_SERVER_ERROR,
				ErrorMsg:  fmt.Sprintf("genearting configs failed. Err: %v", err),
			},
		}
		return &cfgMsg, nil
	}

	cfgMsg := iota.ConfigMsg{
		Configs:   genConfigs,
		AuthToken: c.AuthToken,
		ApiResponse: &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_STATUS_OK,
		},
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return &cfgMsg, nil
}

// ConfigureAuth configures auth and returns a super admin JWT Token
func (c *ConfigService) ConfigureAuth(ctx context.Context, req *iota.AuthMsg) (*iota.AuthMsg, error) {
	// TODO fix this for Agent configs
	var response string
	veniceAPIGw := c.CfgState.Endpoints[0]
	//tenantURL := fmt.Sprintf("%s/configs/cluster/v1/tenants", veniceAPIGw)
	authPolicyURL := fmt.Sprintf("%s/configs/auth/v1/authn-policy", veniceAPIGw)
	userURL := fmt.Sprintf("%s/configs/auth/v1/tenant/default/users", veniceAPIGw)
	roleBindingURL := fmt.Sprintf("%s/configs/auth/v1/tenant/default/role-bindings", veniceAPIGw)
	loginURL := fmt.Sprintf("%s/v1/login/", veniceAPIGw)

	log.Infof("CFG SVC | DEBUG | ConfigureAuth. Received Request Msg: %v", req)

	//tenant := cluster.Tenant{
	//	TypeMeta: api.TypeMeta{
	//		Kind:       "Tenant",
	//		APIVersion: "v1",
	//	},
	//	ObjectMeta: api.ObjectMeta{
	//		Name:   "default",
	//		Tenant: "default",
	//	},
	//	Spec: cluster.TenantSpec{
	//	//
	//	},
	//}
	//
	//fmt.Println("BALERION: ", tenantURL)
	//
	//err := common.HTTPPost(tenantURL, &tenant, response)
	//log.Infof("CFG SVC | INFO | Creating Tenant | Received Response Msg: %v", response)
	//if err != nil {
	//	log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to create a tenant. Received Response Msg: %v. Err: %v", response, err)
	//}

	authPolicy := auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{
			Kind:       "AuthenticationPolicy",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Local: &auth.Local{
					Enabled: true,
				},
				AuthenticatorOrder: []string{"LOCAL"},
			},
		},
	}

	_, err := common.HTTPPost(authPolicyURL, "", &authPolicy, response)
	log.Infof("CFG SVC | INFO | Creating Authentication Policy | Received Response Msg: %v", response)
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to create authentication policy. Received Response Msg: %v. Err: %v", response, err)
	}

	adminUser := auth.User{
		TypeMeta: api.TypeMeta{
			Kind: "User",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "admin",
			Tenant: "default",
		},
		Spec: auth.UserSpec{
			Fullname: "Admin",
			Email:    "dev@pensando.io",
			Password: "password",
			Type:     "LOCAL",
		},
	}

	_, err = common.HTTPPost(userURL, "", &adminUser, response)
	log.Infof("CFG SVC | INFO | Creating Admin User | Received Response Msg: %v", response)
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to create admin user policy. Received Response Msg: %v. Err: %v", response, err)
	}

	adminRole := auth.RoleBinding{
		TypeMeta: api.TypeMeta{
			Kind: "RoleBinding",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "AdminRoleBinding",
			Tenant: "default",
		},
		Spec: auth.RoleBindingSpec{
			Users: []string{"admin"},
			Role:  "AdminRole",
		},
	}

	_, err = common.HTTPPost(roleBindingURL, "", &adminRole, response)
	log.Infof("CFG SVC | INFO | Assigning Admin role | Received Response Msg: %v", response)
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to assign admin role. Received Response Msg: %v. Err: %v", response, err)
	}

	login := auth.PasswordCredential{
		Username: "admin",
		Password: "password",
		Tenant:   "default",
	}

	cookies, err := common.HTTPPost(loginURL, "", &login, response)
	log.Infof("CFG SVC | INFO | Logging in as admin | Received Response Msg: %v", response)
	log.Infof("CFG SVC | INFO | Logging in as admin | Received Cookies. %v", cookies)

	c.AuthToken = cookies[0].Value
	req.AuthToken = c.AuthToken
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to login as admin user. Received Response Msg: %v. Err: %v", response, err)
	}
	return req, nil
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

// Generate Venice Configs. It includes 2 L2 Segments and 4 Endpoints per node. TODO Parameterize this
func (c *ConfigService) generateConfigs() ([]*iota.ConfigObject, error) {
	var iotaCfgObjects []*iota.ConfigObject
	var workloads []*workload.Workload
	var macAddresses []string
	vlan1 := c.CfgState.Vlans[0]
	vlan2 := c.CfgState.Vlans[1]

	totalWorkloads := common.WorkloadsPerNode * len(c.NaplesUUID)
	macAddresses, err := common.GenMACAddresses(totalWorkloads)
	if err != nil {
		return nil, err
	}

	for _, u := range c.NaplesUUID {
		uSegVlanIdx := uint32(100)
		for i := 0; i < common.WorkloadsPerNode; i++ {
			var mac string
			var vlan uint32
			mac, macAddresses = macAddresses[0], macAddresses[1:]
			var name string
			if i%2 == 0 {
				vlan = vlan1
			} else {
				vlan = vlan2
			}
			name = fmt.Sprintf("wrkld_%s_%d_%d", u, vlan, uSegVlanIdx)
			w := workload.Workload{
				TypeMeta: api.TypeMeta{Kind: "Workload"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "default",
					Name:   name,
				},
				Spec: workload.WorkloadSpec{
					HostName: u,
					Interfaces: map[string]workload.WorkloadIntfSpec{
						mac: {
							MicroSegVlan: uSegVlanIdx,
							ExternalVlan: vlan,
						},
					},
				},
			}
			workloads = append(workloads, &w)
		}
	}

	c.Workloads = workloads

	b, _ := json.MarshalIndent(c.Workloads, "", "   ")
	log.Infof("CFG SVC | Gen Workloads: %v", string(b))

	for _, w := range workloads {
		b, _ := json.Marshal(w)
		cfg := iota.ConfigObject{
			Method: iota.CfgMethodType_CFG_METHOD_CREATE,
			Config: string(b),
		}
		iotaCfgObjects = append(iotaCfgObjects, &cfg)

	}
	return iotaCfgObjects, nil
}
