// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package base

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"os/exec"
	"strings"
	"time"

	"github.com/onsi/ginkgo"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	loginctx "github.com/pensando/sw/api/login/context"
	iota "github.com/pensando/sw/iota/protos/gogen"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	hostToolsDir       = "/pensando/iota"
	penctlPath         = "."
	penctlLinuxBinary  = "penctl.linux"
	penctlPkgName      = "bin/penctl/" + penctlLinuxBinary
	agentAuthTokenFile = "/tmp/auth_token"
)

// getVeniceHostNames returns a list of venice host names
func (sm *SysModel) getVeniceHostNames() []string {
	var hostNames []string

	// walk all venice nodes
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			hostNames = append(hostNames, node.NodeName)
		}
	}

	return hostNames
}

// getVeniceIPAddrs returns list of Venic enode IP addresses
func (sm *SysModel) getVeniceIPAddrs() []string {
	var veniceIPs []string

	// walk all venice nodes
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			sIP := sm.Tb.GetSecondaryIP(node.NodeName)
			if sIP != "" {
				veniceIPs = append(veniceIPs, sIP)
			} else {
				veniceIPs = append(veniceIPs, node.NodeMgmtIP)
			}
		}
	}

	return veniceIPs
}

/*
// CheckIotaClusterHealth checks iota cluster health
func .Tb *TestBed) CheckIotaClusterHealth() error {
	// check cluster health
	topoClient := iota.NewTopologyApiClient.Tb.iotaClient.Client)
.Tb.addNodeResp.ClusterDone = tb.makeClustrResp != nil
	healthResp, err := topoClient.CheckClusterHealth(context.Background(),.Tb.addNodeResp)
	if err != nil {
		log.Errorf("Failed to check health of the cluster. Err: %v", err)
		return fmt.Errorf("Cluster health check failed. %v", err)
	} else if healthResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to check health of the cluster {%+v}. Err: %v", healthResp.ApiResponse, err)
		return fmt.Errorf("Cluster health check failed %v", healthResp.ApiResponse.ApiStatus)
	}

	for _, h := range healthResp.Health {
		if h.HealthCode != iota.NodeHealth_HEALTH_OK {
			log.Errorf("Te.Tbed unhealthy. HealthCode: %v | Node: %v", h.HealthCode, h.NodeName)
			return fmt.Errorf("Cluster health check failed")
		}
	}

	log.Debugf("Got cluster health resp: %+v", healthResp)

	return nil
}

*/

// GetVeniceURL returns venice URL for the sysmodel
func (sm *SysModel) GetVeniceURL() []string {
	var veniceURL []string

	if sm.Tb.IsMockMode() {
		return []string{common.MockVeniceURL}
	}

	// walk all venice nodes
	for _, node := range sm.VeniceNodeMap {
		veniceURL = append(veniceURL, fmt.Sprintf("%s:%s", node.GetTestNode().NodeMgmtIP, globals.APIGwRESTPort))
	}

	return veniceURL
}

// VeniceLoggedInCtx returns loggedin context for venice taking a context
func (sm *SysModel) VeniceLoggedInCtx(ctx context.Context) (context.Context, error) {
	var err error

	for _, url := range sm.GetVeniceURL() {
		if sm.AuthToken == "" {
			_, err = sm.veniceNodeLoggedInCtxWithURL(url)
		}
		if err == nil {
			break
		}
	}
	if err != nil {
		return nil, err
	}
	return loginctx.NewContextWithAuthzHeader(ctx, sm.AuthToken), nil

}

// VeniceNodeLoggedInCtx logs in to a specified node and returns loggedin context
func (sm *SysModel) veniceNodeLoggedInCtxWithURL(nodeURL string) (context.Context, error) {
	// local user credentials

	userCred := auth.PasswordCredential{
		Username: "admin",
		Password: constants.UserPassword,
		Tenant:   "default",
	}

	// overwrite user-id/password in mock mode to match venice integ user cred
	if sm.Tb.IsMockMode() {
		userCred.Username = "test"
		userCred.Password = constants.UserPassword
	}

	// try to login
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), nodeURL, &userCred)
	if err != nil {
		log.Errorf("Error logging into Venice URL %v. Err: %v", nodeURL, err)
		return nil, err
	}
	authToken, ok := loginctx.AuthzHeaderFromContext(ctx)
	if ok {
		sm.AuthToken = authToken
	} else {
		return nil, fmt.Errorf("auth token not available in logged-in context")
	}

	return ctx, nil
}

// GetAuthorizationHeader gets and returns the authorization header from login context
func (sm *SysModel) GetAuthorizationHeader() (string, error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return "", err
	}

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return "", fmt.Errorf("failed to get authorization header from context")
	}

	return authzHeader, nil
}

// VeniceRestClient returns the REST client for venice
func (sm *SysModel) VeniceRestClient() ([]apiclient.Services, error) {
	var restcls []apiclient.Services
	for _, url := range sm.GetVeniceURL() {
		// connect to Venice
		restcl, err := apiclient.NewRestAPIClient(url)
		if err != nil {
			log.Errorf("Error connecting to Venice %v. Err: %v", url, err)
			return nil, err
		}

		restcls = append(restcls, restcl)
	}

	return restcls, nil
}

// VeniceNodeRestClient returns the REST client for venice node
func (sm *SysModel) VeniceNodeRestClient(nodeURL string) (apiclient.Services, error) {
	// connect to Venice
	restcl, err := apiclient.NewRestAPIClient(nodeURL)
	if err != nil {
		log.Errorf("Error connecting to Venice %v. Err: %v", nodeURL, err)
		return nil, err
	}
	return restcl, nil
}

const maxVeniceUpWait = 300

// WaitForVeniceClusterUp wait for venice cluster to come up
func (sm *SysModel) WaitForVeniceClusterUp(ctx context.Context) error {
	// wait for cluster to come up
	for i := 0; i < maxVeniceUpWait; i++ {
		restcls, err := sm.VeniceRestClient()
		if err == nil {
			ctx2, err := sm.VeniceLoggedInCtx(ctx)
			if err == nil {
				for _, restcl := range restcls {
					_, err = restcl.ClusterV1().Cluster().Get(ctx2, &api.ObjectMeta{Name: "iota-cluster"})
					if err == nil {
						return nil
					}
				}
			}
		}

		time.Sleep(time.Second)
		if e := ctx.Err(); e != nil {
			return e
		}
	}

	// if we reached here, it means we werent able to connect to Venice API GW
	return fmt.Errorf("Failed to connect to Venice")
}

// InitVeniceConfig initializes base configuration for venice
func (sm *SysModel) InitVeniceConfig(ctx context.Context) error {
	// base configs
	/*cfgMsg := &iota.InitConfigMsg{
		ApiResponse:    &iota.IotaAPIResponse{},
		EntryPointType: iota.EntrypointType_VENICE_REST,
		Endpoints:      sm.GetVeniceURL(),
		Vlans:          sm.Tb.allocatedVlans,
	}

	// Push base configs
	cfgClient := iota.NewConfigMgmtApiClient(sm.Tb.iotaClient.Client)
	cfgInitResp, err := cfgClient.InitCfgService(ctx, cfgMsg)
	if err != nil || cfgInitResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Config service Init failed. API Status: %v , Err: %v", cfgInitResp.ApiResponse.ApiStatus, err)
		return fmt.Errorf("Config service init failed")
	}
	log.Debugf("Got init config Resp: %+v", cfgInitResp)*/

	log.Infof("Setting up Auth on Venice cluster...")

	var err error
	for i := 0; i < maxVeniceUpWait; i++ {
		err = sm.SetupAuth("admin", constants.UserPassword)
		if err == nil {
			break
		}
		time.Sleep(time.Second)
		if ctx.Err() != nil {
			return ctx.Err()
		}
	}
	if err != nil {
		log.Errorf("Setting up Auth failed. Err: %v", err)
		return err
	}

	log.Infof("Auth setup complete...")

	// wait for venice cluster to come up
	return sm.WaitForVeniceClusterUp(ctx)
}

// SetupLicenses setsup licesses
func (sm *SysModel) setupLicenses(licenses []string) error {
	// set bootstrap flag

	log.Infof("Setting up licenses %v", len(licenses))
	if len(licenses) == 0 {
		return nil
	}

	apicl, err := apiclient.NewRestAPIClient(sm.GetVeniceURL()[0])
	if err != nil {
		return fmt.Errorf("cannot create rest client, err: %v", err)
	}

	features := []cluster.Feature{}
	for _, license := range licenses {
		log.Infof("Adding license %v", license)
		features = append(features, cluster.Feature{FeatureKey: license})
	}

	_, err = testutils.CreateLicense(apicl, features)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("SetAuthBootstrapFlag failed with err: %v", err))
		}
	}

	return nil
}

// SetupAuth bootstraps default tenant, authentication policy, local user and super admin role
func (sm *SysModel) SetupAuth(userID, password string) error {
	// no need to setup auth in mock mode
	if sm.Tb.IsMockMode() {
		return nil
	}

	apicl, err := apiclient.NewRestAPIClient(sm.GetVeniceURL()[0])
	if err != nil {
		return fmt.Errorf("cannot create rest client, err: %v", err)
	}

	// create tenant. default roles (admin role) are created automatically when a tenant is created
	_, err = testutils.CreateTenant(apicl, globals.DefaultTenant)
	if err != nil {
		// 412 is returned when tenant and default roles already exist. 401 when auth is already bootstrapped. we are ok with that
		// already exists
		if !strings.HasPrefix(err.Error(), "Status:(412)") && !strings.HasPrefix(err.Error(), "Status:(401)") &&
			!strings.HasPrefix(err.Error(), "already exists") {
			return fmt.Errorf("CreateTenant failed with err: %v", err)
		}
	}

	// create authentication policy with local auth enabled
	_, err = testutils.CreateAuthenticationPolicyWithOrder(apicl, &auth.Local{}, nil, nil, []string{auth.Authenticators_LOCAL.String()}, testutils.ExpiryDuration)
	if err != nil {
		// 409 is returned when authpolicy already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") &&
			!strings.HasPrefix(err.Error(), "already exists") {
			return fmt.Errorf("CreateAuthenticationPolicy failed with err: %v", err)
		}
	}

	// create user is only allowed after auth policy is created and local auth is enabled
	_, err = testutils.CreateTestUser(context.TODO(), apicl, userID, password, globals.DefaultTenant)
	if err != nil {
		// 409 is returned when user already exists. 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(409)") && !strings.HasPrefix(err.Error(), "Status:(401)") &&
			!strings.HasPrefix(err.Error(), "already exists") {
			return fmt.Errorf("CreateTestUser failed with err: %v", err)
		}
	}

	// update admin role binding
	_, err = testutils.UpdateRoleBinding(context.TODO(), apicl, globals.AdminRoleBinding, globals.DefaultTenant, globals.AdminRole, []string{userID}, nil)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			return fmt.Errorf("UpdateRoleBinding failed with err: %v", err)
		}
	}

	//Setup any lines
	err = sm.setupLicenses(sm.Licenses)
	if err != nil {
		return err
	}

	// set bootstrap flag
	_, err = testutils.SetAuthBootstrapFlag(apicl)
	if err != nil {
		// 401 when auth is already bootstrapped. we are ok with that
		if !strings.HasPrefix(err.Error(), "Status:(401)") {
			ginkgo.Fail(fmt.Sprintf("SetAuthBootstrapFlag failed with err: %v", err))
		}
	}

	return nil
}

// GetVeniceNode gets venice node state from venice cluster
func (sm *SysModel) GetVeniceNode(name string) (n *cluster.Node, err error) {
	ctx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return nil, err
	}
	restcls, err := sm.VeniceRestClient()
	if err != nil {
		return nil, err
	}

	meta := api.ObjectMeta{
		Tenant:    "default",
		Namespace: "default",
		Name:      name,
	}

	for _, restcl := range restcls {
		n, err = restcl.ClusterV1().Node().Get(ctx, &meta)
		if err == nil {
			break
		}
	}

	return n, err
}

// ListEvents makes a http request to the APIGw with the given list watch options and returns the response
func (sm *SysModel) ListEvents(listWatchOptions *api.ListWatchOptions) (evtsapi.EventList, error) {
	resp := evtsapi.EventList{}
	authzHdr, err := sm.GetAuthorizationHeader()
	if err != nil {
		return resp, err
	}
	httpClient := netutils.NewHTTPClient()
	httpClient.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	httpClient.SetHeader("Authorization", authzHdr)
	httpClient.DisableKeepAlives()
	defer httpClient.CloseIdleConnections()

	URL := fmt.Sprintf("https://%s/events/v1/events", sm.GetVeniceURL()[0])
	_, err = httpClient.Req("GET", URL, *listWatchOptions, &resp)
	return resp, err
}

// MakeVeniceCluster inits the venice cluster
func (sm *SysModel) MakeVeniceCluster(ctx context.Context) error {
	// get CMD URL URL
	var veniceCmdURL []string
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			veniceCmdURL = append(veniceCmdURL, fmt.Sprintf("%s:9001", node.NodeMgmtIP))
		}
	}

	// cluster message to init cluster
	clusterCfg := cluster.Cluster{
		TypeMeta: api.TypeMeta{Kind: "Cluster"},
		ObjectMeta: api.ObjectMeta{
			Name: "iota-cluster",
		},
		Spec: cluster.ClusterSpec{
			AutoAdmitDSCs: true,
			QuorumNodes:   sm.getVeniceIPAddrs(),
		},
	}

	// make cluster message to be sent to API server
	clusterStr, _ := json.Marshal(clusterCfg)
	makeCluster := iota.MakeClusterMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Endpoint:    veniceCmdURL[0] + "/api/v1/cluster",
		Config:      string(clusterStr),
	}

	log.Infof("Making Venice cluster..")

	// ask iota server to make cluster
	log.Debugf("Making cluster with params: %+v", makeCluster)
	cfgClient := iota.NewConfigMgmtApiClient(sm.Tb.Client().Client)
	resp, err := cfgClient.MakeCluster(ctx, &makeCluster)
	if err != nil {
		log.Errorf("Error initing venice cluster. Err: %v", err)
		return err
	}
	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Error making venice cluster: ApiResp: %+v. Err %v", resp.ApiResponse, err)
		return fmt.Errorf("Error making venice cluster")
	}
	//sm.Tb.makeClustrResp = resp

	// done
	return nil
}

//RestoreVeniceDefaults restore some defaults for cluster
func (sm *SysModel) RestoreVeniceDefaults(nodes []*testbed.TestNode) error {

	// walk all venice nodes
	trig := sm.Tb.NewTrigger()

	naplesInbandIPs := []string{}
	for _, node := range sm.Tb.Nodes {
		if testbed.IsNaplesHW(node.Personality) {
			ips := sm.Tb.GetInbandIPs(node.NodeName)
			naplesInbandIPs = append(naplesInbandIPs, ips...)
			for index := range node.NaplesConfigs.Configs {
				loopackIP := sm.Tb.GetLoopBackIP(node.NodeName, index+1)
				naplesInbandIPs = append(naplesInbandIPs, loopackIP)
			}
		}
	}
	//Clean up old routes
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			entity := node.NodeName + "_venice"
			for _, ip := range naplesInbandIPs {
				//for now using eth1 has default route
				trig.AddCommand(fmt.Sprintf("ip route delete %s", ip), entity, node.NodeName)
				trig.AddCommand(fmt.Sprintf("iptables -F"), entity, node.NodeName)
			}
		}
	}
	trig.Run()

	trig = sm.Tb.NewTrigger()
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			entity := node.NodeName + "_venice"
			for _, ip := range naplesInbandIPs {
				//for now using eth1 has default route
				if sm.Tb.Params.Network.InbandDefaultRoute != "" {
					trig.AddCommand(fmt.Sprintf("ip route add %s/32 via %s dev eth1", ip, sm.Tb.Params.Network.InbandDefaultRoute), entity, node.NodeName)
				}
			}
		}
	}
	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to setup venice node. Err: %v", err)
		return fmt.Errorf("Error triggering commands on venice nodes: %v", err)
	}

	for _, cmdResp := range triggerResp {
		// 'echo' command sometimes has exit code 1. ignore it
		if cmdResp.ExitCode != 0 && !strings.HasPrefix(cmdResp.Command, "echo") {
			return fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

//RestoreClusterDefaults restore some defaults for cluster
func (sm *SysModel) RestoreClusterDefaults(nodes []*testbed.TestNode) error {

	log.Infof("Restoring cluster defaults")

	err := sm.readNodeUUIDs(sm.Tb.Nodes)
	if err != nil {
		return err
	}

	return sm.RestoreVeniceDefaults(nodes)
}

// SetupVeniceNodes sets up some test tools on venice nodes
func (sm *SysModel) SetupVeniceNodes() error {

	log.Infof("Setting up venice nodes..")

	// walk all venice nodes
	trig := sm.Tb.NewTrigger()

	naplesInbandIPs := []string{}
	for _, node := range sm.Tb.Nodes {
		if testbed.IsNaplesHW(node.Personality) {
			ips := sm.Tb.GetInbandIPs(node.NodeName)
			naplesInbandIPs = append(naplesInbandIPs, ips...)
			for index := range node.NaplesConfigs.Configs {
				loopackIP := sm.Tb.GetLoopBackIP(node.NodeName, index+1)
				naplesInbandIPs = append(naplesInbandIPs, loopackIP)
			}
		}
	}
	//Clean up old routes
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			entity := node.NodeName + "_venice"
			for _, ip := range naplesInbandIPs {
				//for now using eth1 has default route
				trig.AddCommand(fmt.Sprintf("ip route  delete %s", ip), entity, node.NodeName)
			}
		}
	}
	trig.Run()

	trig = sm.Tb.NewTrigger()
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			entity := node.NodeName + "_venice"
			trig.AddCommand(fmt.Sprintf("mkdir -p /pensando/iota/k8s/"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("sudo cp -r /var/lib/pensando/pki/kubernetes/apiserver-client /pensando/iota/k8s/"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("sudo chmod -R 777 /pensando/iota/k8s/"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("mkdir -p /pensando/iota/bin; docker run -v /pensando/iota/bin:/import registry.test.pensando.io:5000/pens-debug:v0.1"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf(`echo '/pensando/iota/bin/kubectl config set-cluster e2e --server=https://%s:6443 --certificate-authority=/pensando/iota/k8s/apiserver-client/ca-bundle.pem;
				/pensando/iota/bin/kubectl config set-context e2e --cluster=e2e --user=admin;
				/pensando/iota/bin/kubectl config use-context e2e;
				/pensando/iota/bin/kubectl config set-credentials admin --client-certificate=/pensando/iota/k8s/apiserver-client/cert.pem --client-key=/pensando/iota/k8s/apiserver-client/key.pem;
				' > /pensando/iota/setup_kubectl.sh
				`, node.NodeMgmtIP), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("chmod +x /pensando/iota/setup_kubectl.sh"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("/pensando/iota/setup_kubectl.sh"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf(`echo 'docker run --rm --name kibana --network host \
				-v /var/lib/pensando/pki/shared/elastic-client-auth:/usr/share/kibana/config/auth \
				-e ELASTICSEARCH_URL=https://%s:9200 \
				-e ELASTICSEARCH_SSL_CERTIFICATEAUTHORITIES="config/auth/ca-bundle.pem" \
				-e ELASTICSEARCH_SSL_CERTIFICATE="config/auth/cert.pem" \
				-e ELASTICSEARCH_SSL_KEY="config/auth/key.pem" \
				-e xpack.security.enabled=false \
				-e xpack.logstash.enabled=false \
				-e xpack.graph.enable=false \
				-e xpack.watcher.enabled=false \
				-e xpack.ml.enabled=false \
				-e xpack.monitoring.enabled=false \
				-d docker.elastic.co/kibana/kibana:6.3.0
				' > /pensando/iota/start_kibana.sh
				`, node.NodeName), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("chmod +x /pensando/iota/start_kibana.sh"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("rm /etc/localtime"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("ln -s /usr/share/zoneinfo/US/Pacific /etc/localtime"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("docker run -d --name=grafana --net=host -e \"GF_SECURITY_ADMIN_PASSWORD=password\" registry.test.pensando.io:5000/pensando/grafana:0.1"), entity, node.NodeName)

			for _, ip := range naplesInbandIPs {
				if sm.Tb.Params.Network.InbandDefaultRoute != "" {
					//for now using eth1 has default route
					trig.AddCommand(fmt.Sprintf("ip route add %s/32 via %s dev eth1", ip, sm.Tb.Params.Network.InbandDefaultRoute), entity, node.NodeName)
				}
			}
		}
	}

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to setup venice node. Err: %v", err)
		return fmt.Errorf("Error triggering commands on venice nodes: %v", err)
	}

	for _, cmdResp := range triggerResp {
		// 'echo' command sometimes has exit code 1. ignore it
		if cmdResp.ExitCode != 0 && !strings.HasPrefix(cmdResp.Command, "echo") {
			return fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

//CheckCitadelServiceStatus check citadel status
func (sm *SysModel) CheckCitadelServiceStatus() error {

	// walk all venice nodes
	trig := sm.Tb.NewTrigger()
	for _, node := range sm.VeniceNodeMap {
		entity := node.Name() + "_venice"
		trig.AddCommand(fmt.Sprintf(`curl  http://localhost:7086/query --data-urlencode "db=default" --data-urlencode "q=SELECT * FROM Node" `),
			entity, node.Name())
	}

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run citadel status status. Err: %v", err)
		return err
	}

	for _, cmdResp := range triggerResp {
		if cmdResp.ExitCode != 0 {
			return fmt.Errorf("Venice trigger for citadel check failed %v failed. code %v, Out: %v, StdErr: %v",
				cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// CheckVeniceServiceStatus checks if all services are running on venice nodes
func (sm *SysModel) CheckVeniceServiceStatus(leaderNode string) (string, error) {
	ret := ""
	trig := sm.Tb.NewTrigger()
	for _, node := range sm.VeniceNodeMap {
		entity := node.Name() + "_venice"
		trig.AddCommand(fmt.Sprintf("docker ps -q -f Name=pen-cmd"), entity, node.Name())
		trig.AddCommand(fmt.Sprintf("docker ps -q -f Name=pen-etcd"), entity, node.Name())
	}

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to check cmd/etcd service status. Err: %v", err)
		return ret, err
	}

	for _, cmdResp := range triggerResp {
		if cmdResp.ExitCode != 0 {
			return ret, fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
		if cmdResp.Stdout == "" {
			return ret, fmt.Errorf("Venice required service not running: %v", cmdResp.Command)
		}
	}

	// check all pods on leader node
	for _, node := range sm.VeniceNodeMap {
		if node.IP() == leaderNode {
			trig = sm.Tb.NewTrigger()
			entity := node.Name() + "_venice"
			trig.AddCommand(fmt.Sprintf("/pensando/iota/bin/kubectl  get pods -owide --server=https://%s:6443  --no-headers", leaderNode), entity, node.Name())

			// trigger commands
			triggerResp, err = trig.Run()
			if err != nil {
				log.Errorf("Failed to get k8s service status Err: %v", err)
				return ret, err
			}

			for _, cmdResp := range triggerResp {
				if cmdResp.ExitCode != 0 {
					return ret, fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
				}
				if cmdResp.Stdout == "" {
					return ret, fmt.Errorf("Could not get any information from k8s: %v", cmdResp.Command)
				}
				log.Debugf("Got kubectl resp\n%v", cmdResp.Stdout)
				ret = cmdResp.Stdout
				out := strings.Split(cmdResp.Stdout, "\n")
			checkLoop:
				for _, line := range out {
					if line != "" && !strings.Contains(line, "Running") {
						for _, downNode := range sm.VeniceNodesMapDisconnected {
							if strings.Contains(line, downNode.IP()) {
								//Ignore the nodes which are disconnected.
								break checkLoop
							}
						}
						fmt.Printf("Some kuberneted services were not running: %v", cmdResp.Stdout)
						return ret, fmt.Errorf("Some pods not running: %v", line)
					}
				}
			}

			cNodes := []string{}
			for _, vn := range sm.VeniceNodeMap {
				cNodes = append(cNodes, fmt.Sprintf("%q", vn.IP()))
			}

			hostSelector := "select([.status.hostIP]|inside([" + strings.Join(cNodes, ",") + "]))"
			trig = sm.Tb.NewTrigger()

			trig.AddCommand(`/pensando/iota/bin/kubectl get pods -a --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | `+hostSelector+
				`| select(.status.phase != "Running" or ([ .status.conditions[] | select(.type == "Ready" and .status == "False") ] | length ) == 1 ) | .metadata.namespace + "/" + .metadata.name' `,
				entity, node.Name())

			// trigger commands
			triggerResp, err = trig.Run()
			if err != nil {
				log.Errorf("Failed to get k8s service status Err: %v", err)
				return ret, err
			}

			for _, cmdResp := range triggerResp {
				if cmdResp.ExitCode != 0 {
					return ret, fmt.Errorf("Venice trigger %v failed. code %v, Out: %v, StdErr: %v", cmdResp.Command, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
				}
				if cmdResp.Stdout != "" {
					fmt.Printf("Some pods not ready: %v, ignoring....", cmdResp.Stdout)
				}
			}
		}

	}
	return ret, nil
}

// CheckNaplesHealth checks if naples is healthy
func (sm *SysModel) CheckNaplesHealth(node *objects.Naples) error {
	nodeIP := node.IP()

	// get naples status from NMD
	// Note: struct redefined here to avoid dependency on NMD packages
	var naplesStatus struct {
		api.TypeMeta   `protobuf:"bytes,1,opt,name=T,embedded=T" json:",inline"`
		api.ObjectMeta `protobuf:"bytes,2,opt,name=O,embedded=O" json:"meta,omitempty"`
		Spec           struct {
			PrimaryMAC  string   `protobuf:"bytes,1,opt,name=PrimaryMAC,proto3" json:"primary-mac,omitempty"`
			Hostname    string   `protobuf:"bytes,2,opt,name=Hostname,proto3" json:"hostname,omitempty"`
			Mode        string   `protobuf:"bytes,4,opt,name=Mode,proto3" json:"mode"`
			NetworkMode string   `protobuf:"bytes,5,opt,name=NetworkMode,proto3" json:"network-mode"`
			MgmtVlan    uint32   `protobuf:"varint,6,opt,name=MgmtVlan,proto3" json:"vlan,omitempty"`
			Controllers []string `protobuf:"bytes,7,rep,name=Controllers" json:"controllers,omitempty"`
			Profile     string   `protobuf:"varint,8,opt,name=Profile,proto3,enum=nmd.NaplesSpec_FeatureProfile" json:"feature-profile,omitempty"`
		}
		Status struct {
			Phase           string   `protobuf:"varint,1,opt,name=Phase,proto3,enum=cluster.SmartNICStatus_Phase" json:"phase,omitempty"`
			Controllers     []string `protobuf:"bytes,3,rep,name=Controllers" json:"controllers,omitempty"`
			TransitionPhase string   `protobuf:"bytes,4,opt,name=TransitionPhase,proto3" json:"transition-phase,omitempty"`
			Mode            string   `protobuf:"bytes,5,opt,name=Mode,proto3" json:"mode"`
			NetworkMode     string   `protobuf:"bytes,6,opt,name=NetworkMode,proto3" json:"network-mode"`
		}
	}

	// NAPLES is supposed to be part of a Cluster, so we need auth token to talk to Agent
	veniceCtx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		nerr := fmt.Errorf("Could not get Venice logged in context: %v", err)
		log.Errorf("%v", nerr)
		return nerr
	}
	ctx, cancel := context.WithTimeout(veniceCtx, 5*time.Second)
	defer cancel()
	agentClient, err := utils.GetNodeAuthTokenHTTPClient(ctx, sm.GetVeniceURL()[0], []string{"*"})
	if err != nil {
		nerr := fmt.Errorf("Could not get naples authenticated client from Venice: %v", err)
		log.Errorf("%v", nerr)
		return nerr
	}
	status, err := agentClient.Req("GET", "https://"+nodeIP+":8888/api/v1/naples/", nil, &naplesStatus)
	if err != nil || status != http.StatusOK {
		nerr := fmt.Errorf("Could not get naples status from NMD. Status: %v, err: %v", status, err)
		log.Errorf("%v", nerr)
		return nerr
	}
	//mode := "OOB"
	//for _, naples := range sm.NaplesNodes {
	//	if naples.Nodeuuid == node.Nodeuuid && naples.GetTestNode().InstanceParams().Resource.InbandMgmt {
	//		mode = "INBAND"
	//	}
	//}
	// check naples status
	if naplesStatus.Spec.Mode != "NETWORK" {
		nerr := fmt.Errorf("Invalid NMD mode configuration: %+v", naplesStatus.Spec)
		log.Errorf("%v", nerr)
		return nerr
	}
	/*if testbed.IsNaplesHW(node.Personality()) {
		if !strings.Contains(naplesStatus.Status.TransitionPhase, "REGISTRATION_DONE") {
			nerr := fmt.Errorf("Invalid NMD phase: %v", naplesStatus.Status.TransitionPhase)
			log.Errorf("%v", nerr)
			return nerr
		}
	} else {
		if !strings.Contains(naplesStatus.Status.TransitionPhase, "REGISTRATION_DONE") && !strings.Contains(naplesStatus.Status.TransitionPhase, "REBOOT_PENDING") {
			nerr := fmt.Errorf("Invalid NMD phase: %v", naplesStatus.Status.TransitionPhase)
			log.Errorf("%v", nerr)
			return nerr
		}
	}*/

	// get naples info from Netagent
	// Note: struct redefined here to avoid dependency on netagent package
	var naplesInfo struct {
		DSCMode             string   `json:"dsc-mode,omitempty"`
		DSCName             string   `json:"dsc-name,omitempty"`
		MgmtIP              string   `json:"mgmt-ip,omitempty"`
		Controllers         []string `json:"controllers,omitempty"`
		IsConnectedToVenice bool     `json:"is-connected-to-venice"`
	}

	status, err = agentClient.Req("GET", "https://"+nodeIP+":8888/api/mode/", nil, &naplesInfo)
	if err != nil || status != http.StatusOK {
		nerr := fmt.Errorf("Error checking netagent health. Status: %v, err: %v", status, err)
		log.Errorf("%v", nerr)
		return nerr
	}

	// Use type safe strings here. TODO
	if !strings.Contains(strings.ToLower(naplesInfo.DSCMode), "network") {
		nerr := fmt.Errorf("Naples/Netagent is in incorrect mode: %s", naplesInfo.DSCMode)
		log.Errorf("%v", nerr)
		return nerr
	} else if !naplesInfo.IsConnectedToVenice {
		nerr := fmt.Errorf("Netagent is not connected to Venice")
		log.Errorf("%v", nerr)
		return nerr
	}

	return nil
}

/*
 * Create system config file to eanble cosole with out triggering
 * authentivcation.
 */
const NaplesConfigSpecLocal = "/tmp/system-config.json"

type ConsoleMode struct {
	Console string `json:"console"`
}

func CreateConfigConsoleNoAuth() {
	var ConfigSpec = []byte(`
		 {"console":"enable"}`)

	consolemode := ConsoleMode{}
	json.Unmarshal(ConfigSpec, &consolemode)

	ConfigSpecJson, _ := json.Marshal(consolemode)
	ioutil.WriteFile(NaplesConfigSpecLocal, ConfigSpecJson, 0644)
}

//SetupPenctl setting up penctl
func (sm *SysModel) SetupPenctl(nodes []*testbed.TestNode) error {

	log.Infof("Setting up penctl")

	// set date, untar penctl and trigger mode switch
	trig := sm.Tb.NewTrigger()
	for _, node := range nodes {
		if testbed.IsNaplesHW(node.Personality) {
			err := sm.Tb.CopyToHost(node.NodeName, []string{penctlPkgName}, "")
			if err != nil {
				return fmt.Errorf("Error copying penctl package to host. Err: %v", err)
			}
			// untar the package
			//cmd := fmt.Sprintf("tar -xvf %s", filepath.Base(penctlPkgName))
			//trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
		}
	}

	resp, err := trig.Run()
	if err != nil {
		return fmt.Errorf("Error untaring penctl package. Err: %v", err)
	}
	log.Debugf("Got trigger resp: %+v", resp)

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Changing naples mode failed. %+v", cmdResp)
			return fmt.Errorf("Changing naples mode failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	return nil

}

//DoModeSwitchOfNaples do mode switch of naples
func (sm *SysModel) DoModeSwitchOfNaples(nodes []*testbed.TestNode, noReboot bool) error {

	if os.Getenv("REBOOT_ONLY") != "" {
		log.Infof("Skipping naples setup as it is just reboot")
		return nil
	}

	if err := sm.SetupPenctl(nodes); err != nil {
		log.Infof("Setting up of penctl failed")
		return err
	}
	log.Infof("Setting up Naples in network managed mode")

	// set date, untar penctl and trigger mode switch
	trig := sm.Tb.NewTrigger()
	for _, node := range nodes {
		if testbed.IsNaplesHW(node.Personality) {
			modeNW := "oob"
			if node.InstanceParams().Resource.InbandMgmt {
				modeNW = "inband"
			}
			for _, naplesConfig := range node.NaplesConfigs.Configs {

				veniceIPs := strings.Join(naplesConfig.VeniceIps, ",")

				// clean up roots of trust, if any
				trig.AddCommand(fmt.Sprintf("rm -rf %s", globals.NaplesTrustRootsFile), naplesConfig.Name, node.NodeName)

				// disable watchdog for naples
				trig.AddCommand(fmt.Sprintf("touch /data/no_watchdog"), naplesConfig.Name, node.NodeName)

				// Set up config file to enable console unconditionally (i.e.
				// with out triggering authentication).
				CreateConfigConsoleNoAuth()
				err := sm.Tb.CopyToNaples(node.NodeName, []string{NaplesConfigSpecLocal}, globals.NaplesConfig)
				if err != nil {
					return fmt.Errorf("Error copying config spec file to Naples. Err: %v", err)
				}

				// trigger mode switch
				for _, naples := range node.NaplesConfigs.Configs {
					penctlNaplesURL := "http://" + naples.NaplesIpAddress
					cmd := fmt.Sprintf("NAPLES_URL=%s %s/entities/%s_host/%s/%s update naples --managed-by network --management-network %s --controllers %s --id %s --primary-mac %s",
						penctlNaplesURL, hostToolsDir, node.NodeName, penctlPath, penctlLinuxBinary, modeNW, veniceIPs, naplesConfig.Name, naplesConfig.NodeUuid)
					trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
				}
			}
		} else if node.Personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM {
			// trigger mode switch on Naples sim
			for _, naplesConfig := range node.NaplesConfigs.Configs {
				veniceIPs := strings.Join(naplesConfig.VeniceIps, ",")
				cmd := fmt.Sprintf("LD_LIBRARY_PATH=/naples/nic/lib64 /naples/nic/bin/penctl update naples --managed-by network --management-network oob --controllers %s --mgmt-ip %s/16  --primary-mac %s --id %s --localhost", veniceIPs, naplesConfig.ControlIp, naplesConfig.NodeUuid, naplesConfig.Name)
				trig.AddCommand(cmd, naplesConfig.Name, node.NodeName)
			}
		}
	}
	resp, err := trig.Run()
	if err != nil {
		return fmt.Errorf("Error untaring penctl package. Err: %v", err)
	}
	log.Debugf("Got trigger resp: %+v", resp)

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Changing naples mode failed. %+v", cmdResp)
			return fmt.Errorf("Changing naples mode failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	if noReboot {
		return nil
	}
	//No longeer reload  of naples is needed : Auto Discovery / DSCProfile change should help
	// Retaining this, in case , we have to revert
	/*var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}
	for _, node := range nodes {
		if testbed.IsNaplesHW(node.Personality) {
			nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: node.NodeName})
			hostNames += node.NodeName + ", "

		}
	}
	log.Info("Skipping Naples Reloads")

	//reloadMsg := &iota.ReloadMsg{
	//	NodeMsg: nodeMsg,
	//}
	// Trigger App
	//topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	//reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	//if err != nil {
	//	return fmt.Errorf("Failed to reload Naples %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	//} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
	//	return fmt.Errorf("Failed to reload Naples %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	//}
	}*/

	return nil
}

func (sm *SysModel) readNodeUUIDs(nodes []*testbed.TestNode) error {

	naplesHwUUIDFile := "/tmp/fru.json"

	readUUID := func(nodeName, name string) (uuid string, err error) {

		trig := sm.Tb.NewTrigger()

		cmd := fmt.Sprintf("cat " + naplesHwUUIDFile)
		trig.AddCommand(cmd, name, nodeName)

		resp, err := trig.Run()
		if err != nil {
			return "", fmt.Errorf("Error reading fru.json. Err: %v", err)
		}

		if resp[0].ExitCode != 0 {
			return "", fmt.Errorf("Error reading fru.json. Err: %v", resp[0].Stderr)
		}

		var deviceJSON map[string]interface{}
		if err := json.Unmarshal([]byte(resp[0].Stdout), &deviceJSON); err != nil {
			return "", fmt.Errorf("Error reading %s file", naplesHwUUIDFile)
		}

		if val, ok := deviceJSON["mac-address"]; ok {
			return val.(string), nil
		}

		return "", fmt.Errorf("Mac address not present in %s file", naplesHwUUIDFile)

	}

	//Read NODE UUID as ssh is now up and we can do it now.
	for _, node := range nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
			for _, naples := range node.NaplesConfigs.Configs {
				uuid, err := readUUID(node.NodeName, naples.Name)
				if err != nil {
					return err
				}
				naples.NodeUuid = uuid
			}
		}
	}

	return nil
}

func (sm *SysModel) enableSSH(nodes []*testbed.TestNode, token string) error {
	//After reloading make sure we setup the host
	trig := sm.Tb.NewTrigger()
	for _, node := range nodes {
		if testbed.IsNaplesHW(node.Personality) {
			cmd := fmt.Sprintf("echo \"%s\" > %s", token, agentAuthTokenFile)
			trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
			for _, naples := range node.NaplesConfigs.Configs {
				penctlNaplesURL := "http://" + naples.NaplesIpAddress
				cmd = fmt.Sprintf("NAPLES_URL=%s %s/entities/%s_host/%s/%s  -a %s update ssh-pub-key -f ~/.ssh/id_rsa.pub",
					penctlNaplesURL, hostToolsDir, node.NodeName, penctlPath, penctlLinuxBinary, agentAuthTokenFile)
				trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
				//enable sshd
				cmd = fmt.Sprintf("NAPLES_URL=%s %s/entities/%s_host/%s/%s  -a %s system enable-sshd",
					penctlNaplesURL, hostToolsDir, node.NodeName, penctlPath, penctlLinuxBinary, agentAuthTokenFile)
				trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
			}
		}
	}

	resp, err := trig.Run()
	if err != nil {
		return fmt.Errorf("Error update public key on naples. Err: %v", err)
	}

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Changing naples mode failed. %+v", cmdResp)
			return fmt.Errorf("Changing naples mode failed. exit code %v, Out: %v, StdErr: %v",
				cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	return nil

}

func (sm *SysModel) SetUpNaplesPostCluster(nodes []*testbed.TestNode) error {

	// get token ao authenticate to agent
	veniceCtx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		nerr := fmt.Errorf("Could not get Venice logged in context: %v", err)
		log.Errorf("%v", nerr)
		return nerr
	}

	ctx, cancel := context.WithTimeout(veniceCtx, 180*time.Second)
	defer cancel()
	var token string
	for i := 0; true; i++ {

		token, err = utils.GetNodeAuthToken(ctx, sm.GetVeniceURL()[0], []string{"*"})
		if err == nil {
			break
		}
		if i == 6 {
			nerr := fmt.Errorf("Could not get naples authentication token from Venice: %v", err)
			log.Errorf("%v", nerr)
			return nerr
		}
	}

	//Naples may take time to join in auto discovery
	for i := 0; true; i++ {
		err = sm.enableSSH(nodes, token)
		if err == nil {
			break
		}
		if i == 6 {
			nerr := fmt.Errorf("Could not enable ssh on naples: %v", err)
			log.Errorf("%v", nerr)
			return nerr
		}
		time.Sleep(3 * time.Second)
	}

	if !sm.NoSetupDataPathAfterSwitch {
		trig := sm.Tb.NewTrigger()
		//Make sure we can run command on naples
		for _, node := range nodes {
			if testbed.IsNaplesHW(node.Personality) {
				for _, naples := range node.NaplesConfigs.Configs {
					trig.AddCommand(fmt.Sprintf("date"), naples.Name, node.NodeName)
					//DSCProfile is pushed now. Disable penctl
					//trig.AddCommand(fmt.Sprintf("/nic/bin/halctl debug system --fwd ms --pol enf"), naples.Name, node.NodeName)
				}
			}
		}

		resp, err := trig.Run()
		if err != nil {
			return fmt.Errorf("Error update public key on naples. Err: %v", err)
		}

		// check the response
		//for _, cmdResp := range resp {
		//	if cmdResp.ExitCode != 0 {
		//		log.Errorf("Running commad on naples failed after mode switch. %+v", cmdResp)
		//		return fmt.Errorf("Changing naples mode failed. exit code %v, Out: %v, StdErr: %v",
		//			cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		//	}
		//}
	}

	return nil
}

func (sm *SysModel) CollectLogs() error {

	// create logs directory if it doesnt exists
	cmdStr := fmt.Sprintf("mkdir -p %s/src/github.com/pensando/sw/iota/logs", os.Getenv("GOPATH"))
	cmd := exec.Command("bash", "-c", cmdStr)
	out, err := cmd.CombinedOutput()
	if err != nil {
		log.Errorf("creating log directory failed with: %s\n", err)
	}

	if sm.Tb.IsMockMode() {
		// create a tar.gz from all log files
		cmdStr := fmt.Sprintf("pushd %s/src/github.com/pensando/sw/iota/logs && tar cvzf venice-iota.tgz ../*.log && popd", os.Getenv("GOPATH"))
		cmd = exec.Command("bash", "-c", cmdStr)
		out, err = cmd.CombinedOutput()
		if err != nil {
			fmt.Printf("tar command out:\n%s\n", string(out))
			log.Errorf("Collecting server log files failed with: %s.\n", err)
		} else {
			log.Infof("created %s/src/github.com/pensando/sw/iota/logs/venice-iota.tgz", os.Getenv("GOPATH"))
		}

		return nil
	}

	// walk all venice nodes
	trig := sm.Tb.NewTrigger()
	for _, node := range sm.Tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			entity := node.NodeName + "_venice"
			trig.AddCommand(fmt.Sprintf("mkdir -p /pensando/iota/entities/%s", entity), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("journalctl -a > /var/log/pensando/iotajournalctl"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("uptime > /var/log/pensando/uptime"), entity, node.NodeName)
			trig.AddCommand(fmt.Sprintf("tar -cvf  /pensando/iota/entities/%s/%s.tar /var/log/pensando/* /var/log/dmesg* /etc/pensando/ /var/lib/pensando/pki/ /var/lib/pensando/events/", entity, entity), entity, node.NodeName)
		}
	}

	// trigger commands
	_, err = trig.Run()
	if err != nil {
		log.Errorf("Failed to setup venice node. Err: %v", err)
		return fmt.Errorf("Error triggering commands on venice nodes: %v", err)
	}

	for _, node := range sm.Tb.Nodes {
		switch node.Personality {
		case iota.PersonalityType_PERSONALITY_VENICE:
			sm.Tb.CopyFromVenice(node.NodeName, []string{fmt.Sprintf("%s_venice.tar", node.NodeName)}, "logs")
		}
	}

	// get token ao authenticate to agent
	veniceCtx, err := sm.VeniceLoggedInCtx(context.Background())
	// get token ao authenticate to agent
	trig = sm.Tb.NewTrigger()
	if err == nil {
		ctx, cancel := context.WithTimeout(veniceCtx, 5*time.Second)
		defer cancel()
		token, err := utils.GetNodeAuthToken(ctx, sm.GetVeniceURL()[0], []string{"*"})
		if err == nil {
			// collect tech-support on
			for _, node := range sm.Tb.Nodes {
				if testbed.IsNaplesHW(node.Personality) {
					cmd := fmt.Sprintf("echo \"%s\" > %s", token, agentAuthTokenFile)
					trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
					for _, naples := range node.NaplesConfigs.Configs {
						penctlNaplesURL := "http://" + naples.NaplesIpAddress
						cmd = fmt.Sprintf("NAPLES_URL=%s %s/entities/%s_host/%s/%s system tech-support -a %s -b %s-tech-support", penctlNaplesURL, hostToolsDir, node.NodeName, penctlPath, penctlLinuxBinary, agentAuthTokenFile, node.NodeName)
						trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
					}
				}
			}
			resp, err := trig.Run()
			if err != nil {
				log.Errorf("Error collecting logs. Err: %v", err)
			}
			// check the response
			for _, cmdResp := range resp {
				if cmdResp.ExitCode != 0 {
					log.Errorf("collecting logs failed. %+v", cmdResp)
				}
			}
			for _, node := range sm.Tb.Nodes {
				if testbed.IsNaplesHW(node.Personality) {
					sm.Tb.CopyFromHost(node.NodeName, []string{fmt.Sprintf("%s-tech-support.tar.gz", node.NodeName)}, "logs")
				}
			}
		} else {
			nerr := fmt.Errorf("Could not get naples authentication token from Venice: %v", err)
			log.Errorf("%v", nerr)
		}
	} else {
		nerr := fmt.Errorf("Could not get Venice logged in context: %v", err)
		log.Errorf("%v", nerr)
	}

	// create a tar.gz from all log files
	cmdStr = fmt.Sprintf("pushd %s/src/github.com/pensando/sw/iota/logs && tar cvzf venice-iota.tgz *.tar* ../*.log && popd", os.Getenv("GOPATH"))
	cmd = exec.Command("bash", "-c", cmdStr)
	out, err = cmd.CombinedOutput()
	if err != nil {
		fmt.Printf("tar command out:\n%s\n", string(out))
		log.Errorf("Collecting log files failed with: %s. trying to collect server logs\n", err)
		cmdStr = fmt.Sprintf("pushd %s/src/github.com/pensando/sw/iota/logs && tar cvzf venice-iota.tgz ../*.log && popd", os.Getenv("GOPATH"))
		cmd = exec.Command("bash", "-c", cmdStr)
		out, err = cmd.CombinedOutput()
		if err != nil {
			fmt.Printf("tar command out:\n%s\n", string(out))
			log.Errorf("Collecting server log files failed with: %s.\n", err)
		}
	}

	log.Infof("created %s/src/github.com/pensando/sw/iota/logs/venice-iota.tgz", os.Getenv("GOPATH"))
	return nil
}
