package enterprise

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	"github.com/olekukonko/tablewriter"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	modelUtils "github.com/pensando/sw/iota/test/venice/iotakit/model/utils"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	agentAuthTokenFile = "/tmp/auth_token"
)

//ConfigPushStatsFile
const ConfigPushStatsFile = "/tmp/scale-cfg-push-stats.json"
const ConfigStatsFile = "/tmp/scale-cfg-stats.json"

// default number of.Workloads per host
const defaultWorkloadPerHost = 4

// default number of networks in the model
const defaultNumNetworks = 2

//Orchestrator Return orchestrator

// SysModel represents a objects.of the system under test
type SysModel struct {
	Type common.ModelType
	enterprise.CfgModel
	NaplesHosts             map[string]*objects.Host                   // naples tp hosts map
	ThirdPartyHosts         map[string]*objects.Host                   // naples tp hosts map
	switches                map[string]*objects.Switch                 // switches in test
	switchPorts             []*objects.SwitchPort                      // switches in test
	NaplesNodes             map[string]*objects.Naples                 // Naples instances
	ThirdPartyNodes         map[string]*objects.ThirdPartyNode         // Naples instances
	WorkloadsObjs           map[string]*objects.Workload               // workloads
	sgpolicies              map[string]*objects.NetworkSecurityPolicy  // security policies
	msessions               map[string]*objects.MirrorSession          // mirror sessions
	veniceNodes             map[string]*objects.VeniceNode             // Venice nodes
	veniceNodesDisconnected map[string]*objects.VeniceNode             // Venice which are not part of cluster
	fakeHosts               map[string]*objects.Host                   // simulated hosts
	fakeNaples              map[string]*objects.Naples                 // simulated Naples instances
	fakeWorkloads           map[string]*objects.Workload               // simulated.Workloads
	fakeSubnets             map[string]*objects.Network                // simulated subnets
	fakeApps                map[string]*objects.App                    // simulated apps
	fakeSGPolicies          map[string]*objects.NetworkSecurityPolicy  // simulated security policies
	defaultSgPolicies       []*objects.NetworkSecurityPolicyCollection //default sg policy pushed
	authToken               string                                     // authToken obtained after logging in

	Tb *testbed.TestBed // testbed

	allocatedMac map[string]bool // allocated mac addresses
}

//ConfigPushTime keep track of config push time
type ConfigPushTime struct {
	Config string `json:"Config"`
	Object struct {
		SgPolicy string `json:"SgPolicy"`
	} `json:"Object"`
}

//ConfigPushStats keep track of config push stats
type ConfigPushStats struct {
	Stats []ConfigPushTime `json:"Stats"`
}

type veniceRawData struct {
	Diagnostics struct {
		String string `json:"string"`
	} `json:"diagnostics"`
}

type veniceConfigStatus struct {
	KindObjects struct {
		App                   int `json:"App"`
		Endpoint              int `json:"Endpoint"`
		NetworkSecurityPolicy int `json:"NetworkSecurityPolicy"`
	} `json:"KindObjects"`
	NodesStatus []struct {
		NodeID     string `json:"NodeID"`
		KindStatus struct {
			App struct {
				Status struct {
					Create bool `json:"create-event"`
					Update bool `json:"update-event"`
					Delete bool `json:"delete-event"`
				} `json:"Status"`
			} `json:"App"`
			Endpoint struct {
				Status struct {
					Create bool `json:"create-event"`
					Update bool `json:"update-event"`
					Delete bool `json:"delete-event"`
				} `json:"Status"`
			} `json:"Endpoint"`
			SgPolicy struct {
				Status struct {
					Create bool `json:"create-event"`
					Update bool `json:"update-event"`
					Delete bool `json:"delete-event"`
				} `json:"Status"`
			} `json:"NetworkSecurityPolicy"`
		} `json:"KindStatus"`
	} `json:"NodesStatus"`
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

func (sm *SysModel) ConfigClient() objClient.ObjClient {
	return sm.CfgModel.ObjClient()
}

func (sm *SysModel) doModeSwitchOfNaples(nodes []*testbed.TestNode) error {

	if os.Getenv("REBOOT_ONLY") != "" {
		log.Infof("Skipping naples setup as it is just reboot")
		return nil
	}

	log.Infof("Setting up Naples in network managed mode")

	// set date, untar penctl and trigger mode switch
	trig := sm.Tb.NewTrigger()
	for _, node := range nodes {
		if testbed.IsNaplesHW(node.Personality) {
			for _, naplesConfig := range node.NaplesConfigs.Configs {

				veniceIPs := strings.Join(naplesConfig.VeniceIps, ",")
				err := sm.Tb.CopyToHost(node.NodeName, []string{penctlPkgName}, "")
				if err != nil {
					return fmt.Errorf("Error copying penctl package to host. Err: %v", err)
				}
				// untar the package
				cmd := fmt.Sprintf("tar -xvf %s", filepath.Base(penctlPkgName))
				trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)

				// clean up roots of trust, if any
				trig.AddCommand(fmt.Sprintf("rm -rf %s", globals.NaplesTrustRootsFile), naplesConfig.Name, node.NodeName)

				// disable watchdog for naples
				trig.AddCommand(fmt.Sprintf("touch /data/no_watchdog"), naplesConfig.Name, node.NodeName)

				// Set up config file to enable console unconditionally (i.e.
				// with out triggering authentication).
				CreateConfigConsoleNoAuth()
				err = sm.Tb.CopyToNaples(node.NodeName, []string{NaplesConfigSpecLocal}, globals.NaplesConfig)
				if err != nil {
					return fmt.Errorf("Error copying config spec file to Naples. Err: %v", err)
				}

				// trigger mode switch
				for _, naples := range node.NaplesConfigs.Configs {
					penctlNaplesURL := "http://" + naples.NaplesIpAddress
					cmd = fmt.Sprintf("NAPLES_URL=%s %s/entities/%s_host/%s/%s update naples --managed-by network --management-network oob --controllers %s --id %s --primary-mac %s",
						penctlNaplesURL, hostToolsDir, node.NodeName, penctlPath, penctlLinuxBinary, veniceIPs, naplesConfig.Name, naplesConfig.NodeUuid)
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

	// reload naples
	var hostNames string
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
	log.Infof("Reloading Naples: %v", hostNames)

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload Naples %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload Naples %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	return nil
}

func (sm *SysModel) joinNaplesToVenice(nodes []*testbed.TestNode) error {

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

	trig = sm.Tb.NewTrigger()
	//Make sure we can run command on naples
	for _, node := range nodes {
		if testbed.IsNaplesHW(node.Personality) {
			for _, naples := range node.NaplesConfigs.Configs {
				trig.AddCommand(fmt.Sprintf("date"), naples.Name, node.NodeName)
			}
		}
	}

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Running commad on naples failed after mode switch. %+v", cmdResp)
			return fmt.Errorf("Changing naples mode failed. exit code %v, Out: %v, StdErr: %v",
				cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	return nil
}

// SetupConfig sets up the venice cluster and basic config (like auth etc)
func (sm *SysModel) SetupConfig(ctx context.Context) error {

	skipSetup := os.Getenv("SKIP_SETUP")
	if skipSetup != "" {
		return nil
	}

	// make venice cluster
	setupVenice := func(done chan error) {
		err := sm.MakeVeniceCluster(ctx)
		if err != nil {
			log.Errorf("Error creating venice cluster. Err: %v", err)
			done <- err
			return
		}

		// setup auth and wait for venice cluster to come up
		err = sm.InitVeniceConfig(ctx)
		if err != nil {
			log.Errorf("Error configuring cluster. Err: %v", err)
			done <- err
			return
		}

		// setup some tooling on venice nodes
		err = sm.SetupVeniceNodes()
		if err != nil {
			log.Errorf("Error setting up venice nodes. Err: %v", err)
			done <- err
			return
		}
		done <- nil
	}

	doModeSwitch := func(done chan error) {
		// move naples to managed mode
		err := sm.doModeSwitchOfNaples(sm.Tb.Nodes)
		if err != nil {
			log.Errorf("Setting up naples failed. Err: %v", err)
			done <- err
			return
		}

		done <- nil
	}
	doneChan := make(chan error, 2)
	//Paralleize venice and naples setup.
	go setupVenice(doneChan)
	go doModeSwitch(doneChan)
	for i := 0; i < 2; i++ {
		err := <-doneChan
		if err != nil {
			return err
		}
	}

	// connect naples nodes to venice
	return sm.joinNaplesToVenice(sm.Tb.Nodes)

}

// createNaples creates a naples instance
func (sm *SysModel) createNaples(node *testbed.TestNode) error {

	snicInRange := func(macAddr string) (sn *cluster.DistributedServiceCard, err error) {

		const maxMacDiff = 24
		snicList, err := sm.ListSmartNIC()
		if err != nil {
			return nil, err
		}

		// walk all smartnics and see if the mac addr range matches
		for _, snic := range snicList {
			snicMacNum := modelUtils.MacAddrToUint64(snic.Status.PrimaryMAC)
			reqMacNum := modelUtils.MacAddrToUint64(macAddr)
			if (snicMacNum == reqMacNum) || ((reqMacNum - snicMacNum) < maxMacDiff) {
				return snic, nil
			}
		}

		return nil, fmt.Errorf("Could not find smartnic with mac addr %s", macAddr)
	}
	for _, naplesConfig := range node.NaplesConfigs.Configs {
		snic, err := sm.GetSmartNICByName(naplesConfig.Name)
		if sm.Tb.IsMockMode() {
			snic, err = snicInRange(naplesConfig.NodeUuid)
		}
		if err != nil {
			err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", node.NodeName, err)
			log.Errorf("%v", err)
			snic = &cluster.DistributedServiceCard{
				TypeMeta: api.TypeMeta{
					Kind: "DistributedServiceCard",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "dsc-1",
				},
				Spec: cluster.DistributedServiceCardSpec{
					ID: "host-1",
					IPConfig: &cluster.IPConfig{
						IPAddress: "1.2.3.4/32",
					},
					MgmtMode:    "NETWORK",
					NetworkMode: "OOB",
				},
				Status: cluster.DistributedServiceCardStatus{
					AdmissionPhase: "ADMITTED",
					PrimaryMAC:     "502f.9ac7.c246",
					IPConfig: &cluster.IPConfig{
						IPAddress: "1.2.3.4",
					},
				},
			}
			return err
		}

		sm.NaplesNodes[naplesConfig.Name] = objects.NewNaplesNode(naplesConfig.Name, node, snic)
	}

	return nil
}

// createThirdParty creates a naples instance
func (sm *SysModel) createThirdParty(node *testbed.TestNode) error {

	sm.ThirdPartyNodes[node.NodeName] = objects.NewThirdPartyNode(node.NodeName, node)
	return nil
}

// createNaples creates a naples instance
func (sm *SysModel) createMultiSimNaples(node *testbed.TestNode) error {

	numInstances := node.NaplesMultSimConfig.GetNumInstances()
	if len(node.GetIotaNode().GetNaplesMultiSimConfig().GetSimsInfo()) != int(numInstances) {
		err := fmt.Errorf("Number of instances mismatch in iota node and config expected (%v), actual (%v)",
			numInstances, len(node.GetIotaNode().GetNaplesMultiSimConfig().GetSimsInfo()))
		log.Errorf("%v", err)
		return err

	}
	log.Infof("Adding fake naples : %v", (node.NaplesMultSimConfig.GetNumInstances()))

	success := false
	var err error
	for i := 0; i < 3; i++ {
		var snicList []*cluster.DistributedServiceCard
		snicList, err = sm.ListSmartNIC()
		if err != nil {
			continue
		}
		for _, simInfo := range node.GetIotaNode().GetNaplesMultiSimConfig().GetSimsInfo() {
			//TODO: (iota agent is also following the same format.)
			simName := simInfo.GetName()
			success = false
			for _, snic := range snicList {
				if snic.Spec.ID == simName {
					sm.fakeNaples[simName] = objects.NewNaplesNode(simName, node, snic)
					success = true
				}
			}

			if !success {
				err = fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", node.NodeName, err)
				log.Errorf("%v", err)
				break
			}
		}
		//All got added, success!
		if success {
			break
		}
	}

	if !success {
		return fmt.Errorf("Errorr adding fake naples  %v", err.Error())
	}

	return nil
}

func (sm *SysModel) Init(tb *testbed.TestBed, cfgType enterprise.CfgType) error {
	sm.Tb = tb
	sm.NaplesHosts = make(map[string]*objects.Host)
	sm.ThirdPartyHosts = make(map[string]*objects.Host)
	sm.switches = make(map[string]*objects.Switch)
	sm.NaplesNodes = make(map[string]*objects.Naples)
	sm.ThirdPartyNodes = make(map[string]*objects.ThirdPartyNode)
	sm.veniceNodes = make(map[string]*objects.VeniceNode)
	sm.veniceNodesDisconnected = make(map[string]*objects.VeniceNode)
	sm.WorkloadsObjs = make(map[string]*objects.Workload)
	sm.sgpolicies = make(map[string]*objects.NetworkSecurityPolicy)
	sm.msessions = make(map[string]*objects.MirrorSession)
	sm.fakeHosts = make(map[string]*objects.Host)
	sm.fakeNaples = make(map[string]*objects.Naples)
	sm.fakeWorkloads = make(map[string]*objects.Workload)
	sm.fakeSubnets = make(map[string]*objects.Network)
	sm.fakeApps = make(map[string]*objects.App)
	sm.fakeSGPolicies = make(map[string]*objects.NetworkSecurityPolicy)
	sm.allocatedMac = make(map[string]bool)

	sm.CfgModel = enterprise.NewCfgModel(cfgType)
	if sm.CfgModel == nil {
		return errors.New("could not initialize config objects")
	}

	ctx, cancel := context.WithTimeout(context.TODO(), 30*time.Minute)
	defer cancel()

	// build venice nodes
	for _, nr := range sm.Tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			// create
			sm.veniceNodes[nr.NodeName] = objects.NewVeniceNode(nr)
		}
	}

	// make cluster & setup auth
	err := sm.SetupConfig(ctx)
	if err != nil {
		sm.Tb.CollectLogs()
		return err
	}

	//Venice is up so init config model
	sm.initCfgModel()

	clusterNodes, err := sm.ListClusterNodes()
	if err != nil {
		return err
	}

	// build naples nodes
	for _, nr := range sm.Tb.Nodes {
		if testbed.IsNaples(nr.Personality) {
			err := sm.createNaples(nr)
			if err != nil {
				return err
			}
		} else if nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM {
			err := sm.createMultiSimNaples(nr)
			if err != nil {
				return err
			}
		} else if nr.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			for _, cnode := range clusterNodes {
				if cnode.Name == nr.NodeName {
					vnode := sm.veniceNodes[nr.NodeName]
					vnode.ClusterNode = cnode
				}
			}
		} else if testbed.IsThirdParty(nr.Personality) {
			err := sm.createThirdParty(nr)
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// GetVeniceURL returns venice URL for the sysmodel
func (sm *SysModel) GetVeniceURL() []string {
	var veniceURL []string

	if sm.Tb.IsMockMode() {
		return []string{common.MockVeniceURL}
	}

	// walk all venice nodes
	for _, node := range sm.veniceNodes {
		veniceURL = append(veniceURL, fmt.Sprintf("%s:%s", node.IP(), globals.APIGwRESTPort))
	}

	return veniceURL
}

//GetOrchestrator Default objects.has no orchestrator
func (sm *SysModel) GetOrchestrator() (*objects.Orchestrator, error) {
	return nil, nil
}

//GetWorkloadsForScale get all.Workloads with allowa
func (sm *SysModel) GetWorkloadsForScale(hosts []*objects.Host, policyCollection *objects.NetworkSecurityPolicyCollection, proto string) (*objects.WorkloadCollection, error) {
	newCollection := objects.NewWorkloadCollection(sm.ObjClient(), sm.Tb)

	type ipPair struct {
		sip   string
		dip   string
		proto string
		ports string
	}
	actionCache := make(map[string][]ipPair)

	ipPairPresent := func(pair ipPair) bool {
		for _, pairs := range actionCache {
			for _, ippair := range pairs {
				if ippair.dip == pair.dip && ippair.sip == pair.sip && ippair.proto == pair.proto {
					return true
				}
			}
		}
		return false
	}

	for _, pol := range policyCollection.Policies {
		for _, rule := range pol.VenicePolicy.Spec.Rules {
			for _, sip := range rule.FromIPAddresses {
				for _, dip := range rule.ToIPAddresses {
					for _, proto := range rule.ProtoPorts {
						pair := ipPair{sip: sip, dip: dip, proto: proto.Protocol, ports: proto.Ports}
						if _, ok := actionCache[rule.Action]; !ok {
							actionCache[rule.Action] = []ipPair{}
						}
						//if this IP pair was already added, then don't pick it as precedence is based on order
						if !ipPairPresent(pair) {
							//Copy ports
							actionCache[rule.Action] = append(actionCache[rule.Action], pair)
						}
					}
				}
			}
		}
	}
	workloadHostMap := make(map[string][]*workload.Workload)
	hostMap := make(map[string]*objects.Host)
	workloadIPMap := make(map[string]*workload.Workload)

	for _, host := range hosts {
		wloads, err := sm.ListWorkloadsOnHost(host.VeniceHost)
		if err != nil {
			log.Error("Error finding real.Workloads on hosts.")
			return nil, err
		}
		for _, w := range wloads {
			workloadIPMap[w.Spec.Interfaces[0].GetIpAddresses()[0]] = w
		}
		hostMap[host.VeniceHost.Name] = host
	}

	addWorkload := func(newW *workload.Workload) {
		hWloads, ok := workloadHostMap[newW.Spec.HostName]
		if !ok {
			workloadHostMap[newW.Spec.HostName] = []*workload.Workload{newW}
		} else {
			if len(hWloads) >= defaultWorkloadPerHost {
				//Not adding as number of.Workloads per host execeeds
				return
			}
			added := false
			for _, w := range hWloads {
				if w.Name == newW.Name {
					added = true
					break
				}
			}
			if !added {
				workloadHostMap[newW.Spec.HostName] = append(workloadHostMap[newW.Spec.HostName], newW)
			}
		}
	}
	addActionWorkloads := func(action string) {
		cache, ok := actionCache[action]
		if ok {
			for _, ippair := range cache {
				w1, ok1 := workloadIPMap[ippair.sip]
				w2, ok2 := workloadIPMap[ippair.dip]
				if ok1 && ok2 && w1.Spec.Interfaces[0].ExternalVlan == w2.Spec.Interfaces[0].ExternalVlan && ippair.proto == proto {
					//Try to add.Workloads
					addWorkload(w1)
					addWorkload(w2)
				}
			}
		}
	}
	addActionWorkloads("PERMIT")
	addActionWorkloads("DENY")

	for hostName, wloads := range workloadHostMap {
		host, _ := hostMap[hostName]
		for _, w := range wloads {
			sm.WorkloadsObjs[w.Name] = objects.NewWorkload(host, w, sm.Tb.Topo.WorkloadType, sm.Tb.Topo.WorkloadImage)
			newCollection.Workloads = append(newCollection.Workloads, sm.WorkloadsObjs[w.Name])
		}
	}
	return newCollection, nil
}

//SetupWorkloadsOnHost sets up workload on host
func (sm *SysModel) SetupWorkloadsOnHost(h *objects.Host) (*objects.WorkloadCollection, error) {

	wc := objects.NewWorkloadCollection(sm.ObjClient(), sm.Tb)
	nwMap := make(map[uint32]uint32)

	allocatedVlans := sm.Tb.AllocatedVlans()
	for i := 0; i < defaultNumNetworks; i++ {
		log.Infof("Allocated vlan %v\n", allocatedVlans)
		nwMap[allocatedVlans[i]] = 0
	}

	wloadsPerNetwork := defaultWorkloadPerHost / defaultNumNetworks

	//Keep track of number of.Workloads per network
	wloadsToCreate := []*workload.Workload{}

	wloads, err := sm.ListWorkloadsOnHost(h.VeniceHost)
	if err != nil {
		log.Error("Error finding real.Workloads on hosts.")
		return nil, err
	}

	if len(wloads) == 0 {
		log.Error("No.Workloads created on real hosts.")
		return nil, err
	}

	for _, wload := range wloads {
		nw := wload.GetSpec().Interfaces[0].GetExternalVlan()
		if _, ok := nwMap[nw]; ok {
			if nwMap[nw] > ((uint32)(wloadsPerNetwork)) {
				//This network is done.
				continue
			}
			nwMap[nw]++
			wloadsToCreate = append(wloadsToCreate, wload)
			log.Infof("Adding workload %v (host:%v iotaNode:%v nw:%v) to create list", wload.GetName(), h.VeniceHost.GetName(), h.Name(), nw)

			if len(wloadsToCreate) == defaultWorkloadPerHost {
				// We have enough.Workloads already for this host.
				break
			}
		}
	}

	for _, wload := range wloadsToCreate {
		sm.WorkloadsObjs[wload.Name] = objects.NewWorkload(h, wload, sm.Tb.Topo.WorkloadType, sm.Tb.Topo.WorkloadImage)
		wc.Workloads = append(wc.Workloads, sm.WorkloadsObjs[wload.Name])
	}

	return wc, nil

}

//BringupWorkloads bring up.Workloads on host
func (sm *SysModel) BringupWorkloads() error {

	wc := objects.NewWorkloadCollection(sm.ObjClient(), sm.Tb)

	for _, wload := range sm.WorkloadsObjs {
		wc.Workloads = append(wc.Workloads, wload)
	}

	skipSetup := os.Getenv("SKIP_SETUP")
	// if we are skipping setup we dont need to bringup the workload
	if skipSetup != "" {
		// first get a list of all existing.Workloads from iota
		gwlm := &iota.WorkloadMsg{
			ApiResponse: &iota.IotaAPIResponse{},
			WorkloadOp:  iota.Op_GET,
		}
		topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
		getResp, err := topoClient.GetWorkloads(context.Background(), gwlm)
		log.Debugf("Got get workload resp: %+v, err: %v", getResp, err)
		if err != nil {
			log.Errorf("Failed to instantiate Apps. Err: %v", err)
			return fmt.Errorf("Error creating IOTA workload. err: %v", err)
		} else if getResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Failed to instantiate Apps. resp: %+v.", getResp.ApiResponse)
			return fmt.Errorf("Error creating IOTA workload. Resp: %+v", getResp.ApiResponse)
		}

		// check if all the.Workloads are already running
		allFound := true
		for _, wrk := range wc.Workloads {
			found := false
			for _, gwrk := range getResp.Workloads {
				if gwrk.WorkloadName == wrk.Name() {
					wrk.SetMgmtIP(gwrk.MgmtIp)
					wrk.SetInterface(gwrk.GetInterface())
					found = true
				}
			}
			if !found {
				allFound = false
			}
		}

		if !allFound {
			log.Infof("not all.Workloads found")
			getResp.WorkloadOp = iota.Op_DELETE
			delResp, err := topoClient.DeleteWorkloads(context.Background(), getResp)
			log.Debugf("Got get workload resp: %+v, err: %v", delResp, err)
			if err != nil {
				log.Errorf("Failed to delete old Apps. Err: %v", err)
				return fmt.Errorf("Error deleting IOTA workload. err: %v", err)
			}

			// bringup the.Workloads
			err = wc.Bringup(sm.Tb)
			if err != nil {
				return err
			}
		} else {
			//Every workload is found, just send arping so that they can get discovered again by datapath
			sm.WorkloadsSayHelloToDataPath()
		}
	} else {
		// bringup the.Workloads
		err := wc.Bringup(sm.Tb)
		if err != nil {
			return err
		}
	}

	// update workload with management ip in the meta
	workloads := []*workload.Workload{}
	for _, wr := range wc.Workloads {
		wr.VeniceWorkload.ObjectMeta.Labels = map[string]string{"MgmtIp": wr.GetMgmtIP()}
		workloads = append(workloads, wr.VeniceWorkload)
	}

	if err := sm.CreateWorkloads(workloads); err != nil {
		log.Errorf("unable to update the.Workloads label")
	}

	return nil
}

//SetupWorkloads bring up.Workloads on host
func (sm *SysModel) SetupWorkloads(scale bool) error {

	hosts, err := sm.ListRealHosts()
	if err != nil {
		log.Error("Error finding real hosts to run traffic tests")
		return err
	}

	if scale {
		_, err := sm.GetWorkloadsForScale(hosts, sm.DefaultNetworkSecurityPolicy(), "tcp")
		if err != nil {
			log.Errorf("Error finding scale.Workloads Err: %v", err)
			return err
		}
	} else {
		for _, h := range hosts {
			_, err := sm.SetupWorkloadsOnHost(h)
			if err != nil {
				return err
			}
		}
	}

	return sm.BringupWorkloads()
}

func (sm *SysModel) Cleanup() error {
	// collect all log files
	sm.CollectLogs()
	return nil
}

func getThirdPartyNic(name, mac string) *cluster.DistributedServiceCard {

	return &cluster.DistributedServiceCard{
		TypeMeta: api.TypeMeta{
			Kind: "DistributedServiceCard",
		},
		ObjectMeta: api.ObjectMeta{
			Name: name,
		},
		Spec: cluster.DistributedServiceCardSpec{
			ID: "host-1",
			IPConfig: &cluster.IPConfig{
				IPAddress: "1.2.3.4/32",
			},
			MgmtMode:    "NETWORK",
			NetworkMode: "OOB",
		},
		Status: cluster.DistributedServiceCardStatus{
			AdmissionPhase: "ADMITTED",
			PrimaryMAC:     mac,
			IPConfig: &cluster.IPConfig{
				IPAddress: "1.2.3.4",
			},
		},
	}
}

// InitConfig sets up a default config for the system
func (sm *SysModel) InitConfig(scale, scaleData bool) error {
	skipSetup := os.Getenv("SKIP_SETUP")
	skipConfig := os.Getenv("SKIP_CONFIG")
	cfgParams := &base.ConfigParams{
		Scale:      scale,
		Regenerate: skipSetup == "",
		Vlans:      sm.Tb.AllocatedVlans(),
	}
	for _, naples := range sm.NaplesNodes {
		cfgParams.Dscs = append(cfgParams.Dscs, naples.SmartNic)
	}

	index := 0
	for name, node := range sm.ThirdPartyNodes {
		node.Node.Nodeuuid = "50df.9ac7.c24" + fmt.Sprintf("%v", index)
		n := getThirdPartyNic(name, node.Node.Nodeuuid)
		cfgParams.Dscs = append(cfgParams.Dscs, n)
		index++
	}

	for _, naples := range sm.fakeNaples {
		cfgParams.Dscs = append(cfgParams.Dscs, naples.SmartNic)
	}

	err := sm.PopulateConfig(cfgParams)
	if err != nil {
		return err
	}

	if skipConfig == "" {
		err = sm.CleanupAllConfig()
		if err != nil {
			return err
		}

		err = sm.PushConfig()
		if err != nil {
			return err
		}

		ok, err := sm.IsConfigPushComplete()
		if !ok || err != nil {
			return err
		}

	}

	//TODO, we have to move this out of sysmodel
	defaultSgPolicies, err := sm.ListNetworkSecurityPolicy()
	if err != nil {
		return fmt.Errorf("Error in listing policies %v", err)
	}

	for _, pol := range defaultSgPolicies {
		nPolicy := &objects.NetworkSecurityPolicy{VenicePolicy: pol}
		sm.defaultSgPolicies = append(sm.defaultSgPolicies, objects.NewNetworkSecurityPolicyCollection(nPolicy, sm.ObjClient(), sm.Tb))
	}

	return nil
}

// InitConfig sets up a default config for the system
func (sm *SysModel) AssociateConfigs(scale, scaleData bool) error {
	return nil
}

// NetworkSecurityPolicy finds an SG policy by name
func (sm *SysModel) NetworkSecurityPolicy(name string) *objects.NetworkSecurityPolicyCollection {
	pol, ok := sm.sgpolicies[name]
	if !ok {
		pol := objects.NewNetworkSecurityPolicyCollection(nil, sm.ObjClient(), sm.Tb)
		pol.SetError(fmt.Errorf("Policy %v not found", name))
		log.Infof("Error %v", pol.Error())
		return pol
	}

	policyCollection := objects.NewNetworkSecurityPolicyCollection(pol, sm.ObjClient(), sm.Tb)
	policyCollection.Policies = []*objects.NetworkSecurityPolicy{pol}
	return policyCollection
}

// DefaultNetworkSecurityPolicy resturns default-policy that prevails across tests cases in the system
func (sm *SysModel) DefaultNetworkSecurityPolicy() *objects.NetworkSecurityPolicyCollection {
	return sm.defaultSgPolicies[0]
}

// SetupDefaultConfig sets up a default config for the system
func (sm *SysModel) SetupDefaultCommon(ctx context.Context, scale, scaleData bool) error {
	log.Infof("Setting up default config...")

	err := sm.InitConfig(scale, scaleData)
	if err != nil {
		return err
	}

	err = sm.AssociateHosts()
	if err != nil {
		return fmt.Errorf("Error associating hosts: %s", err)
	}

	for _, sw := range sm.Tb.DataSwitches {
		_, err := sm.createSwitch(sw)
		if err != nil {
			log.Errorf("Error creating switch: %#v. Err: %v", sw, err)
			return err
		}

	}

	// create a default firewall profile
	err = sm.updateDefaultFwprofile()
	if err != nil {
		log.Errorf("Error creating firewall profile: %v", err)
		return err
	}

	// create default ALGs
	err = sm.createDefaultAlgs()
	if err != nil {
		log.Errorf("Error creating ALGs: %v", err)
		return err
	}

	// start with default allow policy
	err = sm.DefaultNetworkSecurityPolicy().Restore()
	if err != nil {
		log.Errorf("Error creating default policy. Err: %v", err)
		return err
	}
	return nil
}

// SetupDefaultConfig sets up a default config for the system
func (sm *SysModel) SetupDefaultConfig(ctx context.Context, scale, scaleData bool) error {

	if err := sm.SetupDefaultCommon(ctx, scale, scaleData); err != nil {
		return err
	}
	return sm.SetupWorkloads(scale)
}

//PrintConfigPushStat Print Push Stats
func (sm *SysModel) PrintConfigPushStat() {

	cfgPushStats := ConfigPushStats{}
	readStatConfig := func() {
		jsonFile, err := os.OpenFile(ConfigPushStatsFile, os.O_RDONLY, 0755)
		if err != nil {
			panic(err)
		}
		byteValue, _ := ioutil.ReadAll(jsonFile)

		err = json.Unmarshal(byteValue, &cfgPushStats)
		if err != nil {
			panic(err)
		}
		jsonFile.Close()
	}

	readStatConfig()

	var totalCfgDuration time.Duration
	maxDuration := time.Duration(0)
	minDuration := time.Duration(math.MaxInt64)
	for _, stat := range cfgPushStats.Stats {
		cfgDuration, err := time.ParseDuration(stat.Config)
		if err != nil {
			fmt.Printf("Invalid duration value %v", stat.Config)
			return
		}
		if maxDuration < cfgDuration {
			maxDuration = cfgDuration
		}
		if minDuration > cfgDuration {
			minDuration = cfgDuration
		}
		totalCfgDuration += cfgDuration
	}

	averageDuration := time.Duration(int64(totalCfgDuration) / int64(len(cfgPushStats.Stats)))

	table := tablewriter.NewWriter(os.Stdout)
	table.SetHeader([]string{"MinDuration", "MaxDuration", "Average"})
	table.SetAutoMergeCells(true)
	table.SetBorder(false) // Set Border to false
	table.SetRowLine(true) // Enable row line
	// Change table lines
	table.SetCenterSeparator("*")
	table.SetColumnSeparator("╪")
	table.SetRowSeparator("-")
	table.Append([]string{minDuration.String(),
		maxDuration.String(), averageDuration.String()})
	table.Render()
}

// update the default firewall profile for the tests
func (sm *SysModel) updateDefaultFwprofile() error {
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default",
			Namespace: "default",
			Tenant:    "default",
		},
		// FIXME: set short timeout value till HAL fixes the drop->allow transition issue
		Spec: security.FirewallProfileSpec{
			SessionIdleTimeout:        "10s",
			TCPConnectionSetupTimeout: "10s",
			TCPCloseTimeout:           "10s",
			TCPHalfClosedTimeout:      "10s",
			TCPDropTimeout:            "10s",
			UDPDropTimeout:            "10s",
			DropTimeout:               "10s",
			TcpTimeout:                "10s",
			UdpTimeout:                "10s",
			ICMPDropTimeout:           "10s",
			IcmpTimeout:               "10s",
		},
	}

	return sm.UpdateFirewallProfile(&fwp)
}

// createDefaultAlgs creates some algs
func (sm *SysModel) createDefaultAlgs() error {
	ftpApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "ftp-alg",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "FTP",
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	// create FTP app
	err := sm.CreateApp(&ftpApp)
	if err != nil {
		return fmt.Errorf("Error creating FTP app. Err: %v", err)
	}

	// ICMP echo req
	icmpApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmp-echo-req",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "ICMP",
				Icmp: &security.Icmp{
					Type: "8",
				},
			},
		},
	}
	// create ICMP app
	err = sm.CreateApp(&icmpApp)
	if err != nil {
		return fmt.Errorf("Error creating ICMP req app. Err: %v", err)
	}
	// ICMP echo resp
	icmpApp = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "icmp-echo-resp",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "ICMP",
				Icmp: &security.Icmp{
					Type: "0",
				},
			},
		},
	}
	// create ICMP app
	err = sm.CreateApp(&icmpApp)
	if err != nil {
		return fmt.Errorf("Error creating ICMP resp app. Err: %v", err)
	}

	// DNS app
	dnsApp := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Name:      "dns-app",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "udp",
					Ports:    "68",
				},
			},
			Timeout: "5m",
			ALG: &security.ALG{
				Type: "DNS",
				Dns: &security.Dns{
					DropMultiQuestionPackets:   true,
					DropLargeDomainNamePackets: true,
					DropLongLabelPackets:       true,
					MaxMessageLength:           100,
					QueryResponseTimeout:       "60s",
				},
			},
		},
	}

	// create DNS app
	err = sm.CreateApp(&dnsApp)
	if err != nil {
		return fmt.Errorf("Error creating DNS app. Err: %v", err)
	}

	return nil
}

// AddNaplesNodes node on the fly
func (sm *SysModel) AddNaplesNodes(names []string) error {
	//First add to testbed.
	log.Infof("Adding naples nodes : %v", names)
	nodes, err := sm.Tb.AddNodes(iota.PersonalityType_PERSONALITY_NAPLES, names)
	if err != nil {
		return err
	}

	// move naples to managed mode
	err = sm.doModeSwitchOfNaples(nodes)
	if err != nil {
		log.Errorf("Setting up naples failed. Err: %v", err)
		return err
	}

	// add venice node to naples
	err = sm.joinNaplesToVenice(nodes)
	if err != nil {
		log.Errorf("Setting up naples failed. Err: %v", err)
		return err
	}

	for _, node := range nodes {
		if err := sm.createNaples(node); err != nil {
			return err
		}

	}

	//Reassociate hosts as new naples is added now.
	if err := sm.AssociateHosts(); err != nil {
		log.Infof("Error in host association %v", err.Error())
		return err
	}

	log.Infof("Bringing up.Workloads naples nodes : %v", names)
	wc := objects.NewWorkloadCollection(sm.ObjClient(), sm.Tb)
	for _, h := range sm.NaplesHosts {
		for _, node := range nodes {
			if node.GetIotaNode() == h.GetIotaNode() {
				hwc, err := sm.SetupWorkloadsOnHost(h)
				if err != nil {
					return err
				}
				wc.Workloads = append(wc.Workloads, hwc.Workloads...)
			}
		}
	}

	return wc.Bringup(sm.Tb)
}

// DeleteNaplesNodes nodes on the fly
func (sm *SysModel) DeleteNaplesNodes(names []string) error {
	//First add to testbed.

	nodes := []*testbed.TestNode{}
	naplesMap := make(map[string]bool)
	for _, name := range names {
		log.Infof("Deleting naples node : %v", name)
		naples, ok := sm.NaplesNodes[name]
		if !ok {
			return errors.New("naples not found to delete")
		}

		if _, ok := naplesMap[naples.GetIotaNode().Name]; ok {
			//Node already added
			continue
		}
		naplesMap[naples.GetIotaNode().Name] = true
		nodes = append(nodes, naples.GetTestNode())
	}

	err := sm.Tb.DeleteNodes(nodes)
	if err != nil {
		return err
	}

	for _, name := range names {

		if naples, ok := sm.NaplesNodes[name]; ok {
			delete(sm.NaplesNodes, name)
			delete(sm.NaplesHosts, naples.GetTestNode().NodeName)

		}
	}
	//Reassociate hosts as new naples is added now.
	return sm.AssociateHosts()
}

func (sm *SysModel) initCfgModel() error {

	veniceCtx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	veniceUrls := sm.GetVeniceURL()

	sm.InitClient(veniceCtx, veniceUrls)
	return nil

}

// DeleteVeniceNodes nodes on the fly
func (sm *SysModel) DeleteVeniceNodes(names []string) error {
	//First add to testbed.

	clusterNodes, err := sm.ListClusterNodes()
	if err != nil {
		return err
	}

	nodes := []*testbed.TestNode{}
	veniceMap := make(map[string]bool)
	for _, name := range names {
		log.Infof("Deleting venice node : %v", name)
		venice, ok := sm.veniceNodes[name]
		if !ok {
			return errors.New("venice not found to delete")
		}

		if _, ok := veniceMap[venice.Name()]; ok {
			//Node already added
			continue
		}
		veniceMap[venice.Name()] = true
		nodes = append(nodes, venice.GetTestNode())
	}

	clusterDelNodes := []*cluster.Node{}
	for _, node := range nodes {
		added := false
		for _, cnode := range clusterNodes {
			if cnode.ObjectMeta.Name == node.GetIotaNode().IpAddress {
				clusterDelNodes = append(clusterDelNodes, cnode)
				added = true
				break
			}
		}
		if !added {
			return fmt.Errorf("Node %v not found in the cluster", node.GetIotaNode().Name)
		}
	}

	//Remove from the cluster
	for _, node := range clusterDelNodes {

		//Remember the cluster node if we want to create again
		for name, vnode := range sm.veniceNodes {
			if vnode.IP() == node.Name {
				veniceNode, _ := sm.VeniceNodes().Select("name=" + vnode.IP())
				if err != nil {
					log.Errorf("Error finding venice node .%v", "name="+vnode.Name())
					return err
				}
				//Disconnect node before deleting
				err = sm.DisconnectVeniceNodesFromCluster(veniceNode, sm.Naples())
				if err != nil {
					log.Errorf("Error disonnecting venice node.")
					return err
				}
				vnode.ClusterNode = node
				sm.veniceNodesDisconnected[name] = vnode
				break
			}
		}

		//Sleep for 2 minutes to for cluster to be reformed.
		time.Sleep(120 * time.Second)
		log.Infof("Deleting venice node from cluster : %v", node.Name)
		err := sm.DeleteClusterNode(node)
		if err != nil {
			log.Errorf("Error deleting cluster venice node.%v", err)
			return err
		}

	}

	log.Infof("Deleting venice node from testbed : %v", nodes)
	err = sm.Tb.DeleteNodes(nodes)
	if err != nil {
		log.Errorf("Error cleaning up venice node.%v", err)
		return err
	}

	for _, name := range names {
		delete(sm.veniceNodes, name)
	}
	//Sleep for a while to for the cluster
	time.Sleep(120 * time.Second)
	log.Infof("Deleting venice complete")

	return sm.initCfgModel()
}

// AddVeniceNodes node on the fly
func (sm *SysModel) AddVeniceNodes(names []string) error {
	//First add to testbed.
	log.Infof("Adding venice nodes : %v", names)
	nodes, err := sm.Tb.AddNodes(iota.PersonalityType_PERSONALITY_VENICE, names)
	if err != nil {
		return err
	}

	//Add to cluster
	for _, node := range nodes {
		added := false
		for name, vnode := range sm.veniceNodesDisconnected {
			if vnode.IP() == node.GetIotaNode().IpAddress {
				err := sm.AddClusterNode(vnode.ClusterNode)
				if err != nil {
					return fmt.Errorf("Node add failed %v", err)
				}
			}
			added = true
			//Add to connected nodes
			sm.veniceNodes[name] = vnode
		}

		if !added {
			return fmt.Errorf("Node %v not added to cluster", node.GetIotaNode().Name)
		}

	}

	for _, name := range names {
		delete(sm.veniceNodesDisconnected, name)
	}

	//Sleep for a while to for the cluster
	time.Sleep(120 * time.Second)
	//Setup venice nodes again.
	sm.SetupVeniceNodes()

	return sm.initCfgModel()

}

// CollectLogs collects all logs files from the testbed
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

// SetUpNaplesAuthenticationOnHosts changes naples to managed mode
func (sm *SysModel) SetUpNaplesAuthenticationOnHosts(hc *objects.HostCollection) error {

	testNodes := []*testbed.TestNode{}

	for _, node := range hc.Hosts {
		for _, testNode := range sm.Tb.Nodes {
			if node.GetIotaNode().Name == testNode.GetIotaNode().Name {
				testNodes = append(testNodes, testNode)
			}
		}
	}

	return sm.joinNaplesToVenice(testNodes)
}

func (sm *SysModel) NewNetworkSecurityPolicy(name string) *objects.NetworkSecurityPolicyCollection {
	policy := &objects.NetworkSecurityPolicy{
		VenicePolicy: &security.NetworkSecurityPolicy{
			TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      name,
			},
			Spec: security.NetworkSecurityPolicySpec{
				AttachTenant: true,
			},
		},
	}
	sm.sgpolicies[name] = policy
	return objects.NewNetworkSecurityPolicyCollection(policy, sm.ObjClient(), sm.Tb)
}
