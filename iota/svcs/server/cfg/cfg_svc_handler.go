package cfg

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

// max number of times to retry venice cluster init
const maxClusterInitRetry = 60

// ConfigService implements config service API
type ConfigService struct {
	CfgState    *iota.InitConfigMsg
	AuthToken   string
	NaplesHosts []*iota.NaplesHost
	Workloads   []*workload.Workload
	Hosts       []*cluster.Host
	SGPolicies  []*security.NetworkSecurityPolicy
	Apps        []*security.App
}

// NewConfigServiceHandler returns an instance of config service
func NewConfigServiceHandler() *ConfigService {
	var cfgServer ConfigService
	return &cfgServer
}

// MakeCluster brings up venice cluster
func (c *ConfigService) MakeCluster(ctx context.Context, req *iota.MakeClusterMsg) (*iota.MakeClusterMsg, error) {
	log.Infof("CFG SVC | DEBUG | MakeCluster. Received Request Msg: %v", req)
	defer log.Infof("CFG SVC | DEBUG | MakeCluster Returned: %v", req)
	var clusterObj cluster.Cluster
	var response string

	err := json.Unmarshal([]byte(req.Config), &clusterObj)
	if err != nil {
		log.Errorf("CFG SVC | DEBUG | MakeCluster. Could not unmarshal %v into a cluster object. Err: %v", req.Config, err)
	}

	log.Infof("CFG SVC | DEBUG | MakeCluster. Received cluster obj, %v", clusterObj)

	// retry cluster init few times
	for i := 0; i < maxClusterInitRetry; i++ {
		_, response, err = common.HTTPPost(req.Endpoint, "", &clusterObj)
		log.Infof("CFG SVC | DEBUG | MakeCluster. Received REST Response Msg: %v, err: %v", response, err)
		if err == nil {
			break
		}

		// sleep for a second and retry again
		time.Sleep(time.Second)
	}
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
	defer log.Infof("CFG SVC | DEBUG | InitCfgService Returned: %v", req)

	if len(req.Vlans) == 0 {
		//log.Errorf("CFG SVC | InitCfgService call failed. | Missing allocated VLANs")
		//req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		//req.ApiResponse.ErrorMsg = "CFG SVC | InitCfgService call failed. | Missing allocated VLANs"
		//return req, nil
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
	defer log.Infof("CFG SVC | DEBUG | GenerateConfigs Returned: %v", req)

	c.NaplesHosts = req.Hosts

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
	log.Infof("CFG SVC | DEBUG | ConfigAuth. Received Request Msg: %v", req)
	defer log.Infof("CFG SVC | DEBUG | ConfigAuth Returned: %v", req)
	veniceAPIGw := c.CfgState.Endpoints[0]
	tenantURL := fmt.Sprintf("%s/configs/cluster/v1/tenants", veniceAPIGw)
	authPolicyURL := fmt.Sprintf("%s/configs/auth/v1/authn-policy", veniceAPIGw)
	userURL := fmt.Sprintf("%s/configs/auth/v1/tenant/default/users", veniceAPIGw)
	roleBindingURL := fmt.Sprintf("%s/configs/auth/v1/tenant/default/role-bindings/AdminRoleBinding", veniceAPIGw)
	loginURL := fmt.Sprintf("%s/v1/login", veniceAPIGw)

	log.Infof("CFG SVC | DEBUG | ConfigureAuth. Received Request Msg: %v", req)
	tenant := cluster.Tenant{
		TypeMeta: api.TypeMeta{
			Kind: "Tenant",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "default",
		},
		Spec: cluster.TenantSpec{},
	}

	_, response, err := common.HTTPSPost(tenantURL, c.AuthToken, &tenant)
	log.Infof("CFG SVC | INFO | Creating Tenant | Received Response Msg: %v", response)
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to create a tenant. Received Response Msg: %v. Err: %v", response, err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | ERROR | ConfigureAuth call failed to create a tenant. Received Response Msg: %v. Err: %v", response, err)
		return req, nil
	}

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
				Local:              &auth.Local{},
				AuthenticatorOrder: []string{"LOCAL"},
			},
		},
	}

	_, response, err = common.HTTPSPost(authPolicyURL, "", &authPolicy)
	log.Infof("CFG SVC | INFO | Creating Authentication Policy | Received Response Msg: %v", response)
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to create authentication policy. Received Response Msg: %v. Err: %v", response, err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | ERROR | ConfigureAuth call failed to create authentication policy. Received Response Msg: %v. Err: %v", response, err)
		return req, nil
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
			Password: common.UserPassword,
			Type:     "Local",
		},
	}

	_, response, err = common.HTTPSPost(userURL, "", &adminUser)
	log.Infof("CFG SVC | INFO | Creating Admin User | Received Response Msg: %v", response)
	if err != nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | ERROR | ConfigureAuth call failed to create admin user policy. Received Response Msg: %v. Err: %v", response, err)
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to create admin user policy. Received Response Msg: %v. Err: %v", response, err)
		return req, nil
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

	response, err = common.HTTPSPut(roleBindingURL, "", &adminRole)
	log.Infof("CFG SVC | INFO | Assigning Admin role | Received Response Msg: %v", response)
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to assign admin role. Received Response Msg: %v. Err: %v", response, err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | ERROR | ConfigureAuth call failed to assign admin role. Received Response Msg: %v. Err: %v", response, err)
		return req, nil
	}

	login := auth.PasswordCredential{
		Username: "admin",
		Password: common.UserPassword,
		Tenant:   "default",
	}

	cookies, response, err := common.HTTPSPost(loginURL, "", &login)
	log.Infof("CFG SVC | INFO | Logging in as admin | Received Response Msg: %v", response)
	log.Infof("CFG SVC | INFO | Logging in as admin | Received Cookies. %v", cookies)

	c.AuthToken = cookies[0].Value
	req.AuthToken = c.AuthToken
	if err != nil {
		log.Errorf("CFG SVC | ERROR | ConfigureAuth call failed to login as admin user. Received Response Msg: %v. Err: %v", response, err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("CFG SVC | ERROR | ConfigureAuth call failed to login as admin user. Received Response Msg: %v. Err: %v", response, err)
		return req, nil
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	return req, nil
}

// PushConfig pushes the config. Todo implement custom unmarshallers on a per object basis
func (c *ConfigService) PushConfig(ctx context.Context, req *iota.ConfigMsg) (*iota.ConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | PushConfig. Received Request Msg: %v", req)
	workloadURL := fmt.Sprintf("%s/configs/workload/v1/workloads", c.CfgState.Endpoints[0])
	hostURL := fmt.Sprintf("%s/configs/cluster/v1/hosts", c.CfgState.Endpoints[0])
	sgPolicyURL := fmt.Sprintf("%s/configs/security/v1/networksecuritypolicies", c.CfgState.Endpoints[0])
	appURL := fmt.Sprintf("%s/configs/security/v1/apps", c.CfgState.Endpoints[0])
	for _, cfg := range req.Configs {
		var object interface{}
		jsonBytes := []byte(cfg.Config)
		json.Unmarshal(jsonBytes, &object)

		m := object.(map[string]interface{})
		switch strings.ToLower(m["kind"].(string)) {
		case "host":
			var hostObj cluster.Host
			err := json.Unmarshal(jsonBytes, &hostObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | PushConfig. Could not unmarshal %v into a host object. Err: %v", cfg.Config, err)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
				return req, nil
			}

			// ToDO Add Error checking and returns back. This is a hack to avoid 409s due to CMD auto creating host objects.
			_, response, err := common.HTTPSPost(hostURL, c.AuthToken, hostObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | Failed to create host object. %v. URL: %v. Response: %v. Err: %v", hostObj, hostURL, response, err)
				log.Errorf("CFG SVC | DEBUG | Using Auth Token: %v", c.AuthToken)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to create host object. %v. URL: %v. Response: %v. Err: %v", hostObj, hostURL, response, err)
				//return req, nil
			}
		case "workload":
			var workloadObj workload.Workload
			err := json.Unmarshal(jsonBytes, &workloadObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | PushConfig. Could not unmarshal %v into a workload object. Err: %v", cfg.Config, err)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
				return req, nil
			}

			_, response, err := common.HTTPSPost(workloadURL, c.AuthToken, workloadObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | Failed to create host object. %v. URL: %v. Response: %v. Err: %v", workloadObj, hostURL, response, err)
				log.Errorf("CFG SVC | DEBUG | Using Auth Token: %v", c.AuthToken)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to create workload object. %v. URL: %v. Response: %v. Err: %v", workloadObj, hostURL, response, err)
				return req, nil
			}
		case "sgpolicy":
			var sgPolicyObj security.NetworkSecurityPolicy
			err := json.Unmarshal(jsonBytes, &sgPolicyObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | PushConfig. Could not unmarshal %v into a workload object. Err: %v", cfg.Config, err)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
				return req, nil
			}

			_, response, err := common.HTTPSPost(sgPolicyURL, c.AuthToken, sgPolicyObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | Failed to create sg policy object. %v. URL: %v. Response: %v. Err: %v", sgPolicyObj, hostURL, response, err)
				log.Errorf("CFG SVC | DEBUG | Using Auth Token: %v", c.AuthToken)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to create sg policy object. %v. URL: %v. Response: %v. Err: %v", sgPolicyObj, hostURL, response, err)
				return req, nil
			}
		case "app":
			var appObj security.App
			err := json.Unmarshal(jsonBytes, &appObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | PushConfig. Could not unmarshal %v into a workload object. Err: %v", cfg.Config, err)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
				return req, nil
			}

			_, response, err := common.HTTPSPost(appURL, c.AuthToken, appObj)
			if err != nil {
				log.Errorf("CFG SVC | DEBUG | Failed to create app object. %v. URL: %v. Response: %v. Err: %v", appObj, hostURL, response, err)
				log.Errorf("CFG SVC | DEBUG | Using Auth Token: %v", c.AuthToken)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to create app object. %v. URL: %v. Response: %v. Err: %v", appObj, hostURL, response, err)
				return req, nil
			}
		default:
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Invalid Object Type: %v", m["kind"])
			return req, nil
		}
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// QueryConfig queries the configs
func (c *ConfigService) QueryConfig(ctx context.Context, req *iota.ConfigQueryMsg) (*iota.ConfigMsg, error) {
	log.Infof("CFG SVC | DEBUG | QueryConfig. Received Request Msg: %v", req)
	defer log.Infof("CFG SVC | DEBUG | QueryConfig Returned: %v", req)

	resp := &iota.ConfigMsg{
		ApiResponse: &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_STATUS_OK,
		},
	}
	return resp, nil
}

// Generate Venice Configs. It includes 2 L2 Segments and 4 Endpoints per node. TODO Parameterize this
func (c *ConfigService) generateConfigs() ([]*iota.ConfigObject, error) {
	var iotaCfgObjects []*iota.ConfigObject
	var workloads []*workload.Workload
	var sgPolicies []*security.NetworkSecurityPolicy
	var apps []*security.App
	var hosts []*cluster.Host
	var macAddresses []string
	vlan1 := c.CfgState.Vlans[0]
	vlan2 := c.CfgState.Vlans[1]

	totalWorkloads := common.WorkloadsPerNode * len(c.NaplesHosts)
	macAddresses, err := common.GenMACAddresses(totalWorkloads)
	if err != nil {
		return nil, err
	}

	// Create Host Objects
	for _, n := range c.NaplesHosts {
		host := cluster.Host{
			TypeMeta: api.TypeMeta{Kind: "Host"},
			ObjectMeta: api.ObjectMeta{
				Name: n.Name,
			},
			Spec: cluster.HostSpec{
				DSCs: []cluster.DistributedServiceCardID{{MACAddress: n.Uuid}},
			},
		}
		hosts = append(hosts, &host)
	}

	for _, n := range c.NaplesHosts {
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
			name = fmt.Sprintf("%s_wrkld_%d", n.Name, i)
			w := workload.Workload{
				TypeMeta: api.TypeMeta{Kind: "Workload"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "default",
					Name:   name,
				},
				Spec: workload.WorkloadSpec{
					HostName: n.Name,
					Interfaces: []workload.WorkloadIntfSpec{
						{
							ExternalVlan: vlan,
							MicroSegVlan: uSegVlanIdx,
							MACAddress:   mac,
						},
					},
				},
			}
			workloads = append(workloads, &w)
			uSegVlanIdx++
		}
	}
	// ToDO add more SG Policies here.
	sgp := security.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "all-allow-policy",
		},
		Spec: security.NetworkSecurityPolicySpec{
			AttachTenant: true,
			Rules: []security.SGRule{
				{
					ProtoPorts: []security.ProtoPort{
						{
							Protocol: "any",
							Ports:    "0-65535",
						},
					},
					Action:          "PERMIT",
					FromIPAddresses: []string{"any"},
					ToIPAddresses:   []string{"any"},
				},
			},
		},
	}
	k, _ := json.Marshal(&sgp)
	fmt.Println("RHAEGON: ", string(k))
	sgPolicies = append(sgPolicies, &sgp)

	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default-app",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{},
			Timeout:    "5m",
			ALG: &security.ALG{
				Type: "ICMP",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "2",
				},
			},
		},
	}
	apps = append(apps, &app)

	c.Workloads = workloads
	c.Hosts = hosts
	c.SGPolicies = sgPolicies
	c.Apps = apps

	b, _ := json.MarshalIndent(c.Workloads, "", "   ")
	log.Infof("CFG SVC | Gen Workloads: %v", string(b))

	// Append Hosts
	for _, h := range hosts {
		b, _ := json.Marshal(h)
		cfg := iota.ConfigObject{
			Method: iota.CfgMethodType_CFG_METHOD_CREATE,
			Config: string(b),
		}
		iotaCfgObjects = append(iotaCfgObjects, &cfg)

	}

	// Append Workloads
	for _, w := range workloads {
		b, _ := json.Marshal(w)
		cfg := iota.ConfigObject{
			Method: iota.CfgMethodType_CFG_METHOD_CREATE,
			Config: string(b),
		}
		iotaCfgObjects = append(iotaCfgObjects, &cfg)

	}

	// Append Apps
	for _, s := range apps {
		b, _ := json.Marshal(s)
		cfg := iota.ConfigObject{
			Method: iota.CfgMethodType_CFG_METHOD_CREATE,
			Config: string(b),
		}
		iotaCfgObjects = append(iotaCfgObjects, &cfg)
	}

	// Append SGPolicies
	for _, s := range sgPolicies {
		b, _ := json.Marshal(s)
		cfg := iota.ConfigObject{
			Method: iota.CfgMethodType_CFG_METHOD_CREATE,
			Config: string(b),
		}
		iotaCfgObjects = append(iotaCfgObjects, &cfg)
	}

	return iotaCfgObjects, nil
}
