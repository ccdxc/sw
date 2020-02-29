package enterprise

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math"
	"os"
	"time"

	"github.com/olekukonko/tablewriter"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise"
	baseModel "github.com/pensando/sw/iota/test/venice/iotakit/model/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
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
	baseModel.SysModel
	sgpolicies        map[string]*objects.NetworkSecurityPolicy  // security policies
	msessions         map[string]*objects.MirrorSession          // mirror sessions
	fakeWorkloads     map[string]*objects.Workload               // simulated.Workloads
	fakeSubnets       map[string]*objects.Network                // simulated subnets
	fakeApps          map[string]*objects.App                    // simulated apps
	fakeSGPolicies    map[string]*objects.NetworkSecurityPolicy  // simulated security policies
	defaultSgPolicies []*objects.NetworkSecurityPolicyCollection //default sg policy pushed

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

func (sm *SysModel) Init(tb *testbed.TestBed, cfgType enterprise.CfgType) error {
	err := sm.SysModel.Init(tb, cfgType)
	if err != nil {
		return err
	}
	sm.sgpolicies = make(map[string]*objects.NetworkSecurityPolicy)
	sm.msessions = make(map[string]*objects.MirrorSession)
	sm.FakeNaples = make(map[string]*objects.Naples)
	sm.fakeWorkloads = make(map[string]*objects.Workload)
	sm.fakeSubnets = make(map[string]*objects.Network)
	sm.fakeApps = make(map[string]*objects.App)
	sm.fakeSGPolicies = make(map[string]*objects.NetworkSecurityPolicy)
	sm.allocatedMac = make(map[string]bool)

	sm.CfgModel = enterprise.NewCfgModel(cfgType)
	if sm.CfgModel == nil {
		return errors.New("could not initialize config objects")
	}

	err = sm.SetupVeniceNaples()
	if err != nil {
		return err
	}

	//Venice is up so init config model
	err = sm.InitCfgModel()
	if err != nil {
		return err
	}

	return sm.SetupNodes()

}

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
			sm.WorkloadsObjs[w.Name] = objects.NewWorkload(host, w, sm.Tb.Topo.WorkloadType, sm.Tb.Topo.WorkloadImage, "", "")
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
		sm.WorkloadsObjs[wload.Name] = objects.NewWorkload(h, wload, sm.Tb.Topo.WorkloadType, sm.Tb.Topo.WorkloadImage, "", "")
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

// InitConfig sets up a default config for the system
func (sm *SysModel) InitConfig(scale, scaleData bool) error {

	err := sm.SysModel.InitConfig(scale, scaleData)
	if err != nil {
		return fmt.Errorf("Error initing config %v", err)
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

	//TODO, we have to move this out of sysmodel
	defaultSgPolicies, err := sm.ListNetworkSecurityPolicy()
	if err != nil {
		return fmt.Errorf("Error in listing policies %v", err)
	}

	for _, pol := range defaultSgPolicies {
		nPolicy := &objects.NetworkSecurityPolicy{VenicePolicy: pol}
		sm.defaultSgPolicies = append(sm.defaultSgPolicies, objects.NewNetworkSecurityPolicyCollection(nPolicy, sm.ObjClient(), sm.Tb))
	}

	err = sm.AssociateHosts()
	if err != nil {
		return fmt.Errorf("Error associating hosts: %s", err)
	}

	for _, sw := range sm.Tb.DataSwitches {
		_, err := sm.CreateSwitch(sw)
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

	err := sm.InitConfig(scale, scaleData)
	if err != nil {
		return err
	}

	if err = sm.SetupDefaultCommon(ctx, scale, scaleData); err != nil {
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
	if err := sm.SysModel.AddNaplesNodes(names); err != nil {
		return err
	}

	log.Infof("Bringing up.Workloads naples nodes : %v", names)
	wc := objects.NewWorkloadCollection(sm.ObjClient(), sm.Tb)
	for _, h := range sm.NaplesHosts {
		for _, node := range sm.NaplesNodes {
			for _, name := range names {
				if name == node.Name() && node.GetIotaNode() == h.GetIotaNode() {
					hwc, err := sm.SetupWorkloadsOnHost(h)
					if err != nil {
						return err
					}
					wc.Workloads = append(wc.Workloads, hwc.Workloads...)
				}
			}
		}
	}

	return wc.Bringup(sm.Tb)
}

// NewNetworkSecurityPolicy nodes on the fly
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
