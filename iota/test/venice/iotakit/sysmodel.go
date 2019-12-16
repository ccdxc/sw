// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"math"
	"math/rand"
	"os"
	"os/exec"
	"time"

	"github.com/olekukonko/tablewriter"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfgen"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

//ConfigPushStatsFile
const ConfigPushStatsFile = "/tmp/scale-cfg-push-stats.json"
const ConfigStatsFile = "/tmp/scale-cfg-stats.json"

// default number of workloads per host
const defaultWorkloadPerHost = 4

// default number of networks in the model
const defaultNumNetworks = 2

// SysModel represents a model of the system under test
type SysModel struct {
	hosts                   map[string]*Host                   // hosts
	switches                map[string]*Switch                 // switches in test
	switchPorts             []*SwitchPort                      // switches in test
	naples                  map[string]*Naples                 // Naples instances
	workloads               map[string]*Workload               // workloads
	subnets                 []*Network                         // subnets
	sgpolicies              map[string]*NetworkSecurityPolicy  // security policies
	msessions               map[string]*MirrorSession          // mirror sessions
	veniceNodes             map[string]*VeniceNode             // Venice nodes
	veniceNodesDisconnected map[string]*VeniceNode             // Venice which are not part of cluster
	fakeHosts               map[string]*Host                   // simulated hosts
	fakeNaples              map[string]*Naples                 // simulated Naples instances
	fakeWorkloads           map[string]*Workload               // simulated workloads
	fakeSubnets             map[string]*Network                // simulated subnets
	fakeApps                map[string]*App                    // simulated apps
	fakeSGPolicies          map[string]*NetworkSecurityPolicy  // simulated security policies
	defaultSgPolicies       []*NetworkSecurityPolicyCollection //default sg policy pushed
	authToken               string                             // authToken obtained after logging in

	tb *TestBed // testbed

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

type veniceConfigStat struct {
	KindHistogram struct {
		App struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"App"`
		Endpoint struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"Endpoint"`
		NetworkSecurityPolicy struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"NetworkSecurityPolicy"`
	} `json:"KindHistogram"`
	NodeConfigHistogram []struct {
		NodeID    string `json:"NodeID"`
		KindStats struct {
			App struct {
				MinMs  float64 `json:"MinMs"`
				MaxMs  float64 `json:"MaxMs"`
				MeanMs float64 `json:"MeanMs"`
			} `json:"App"`
			Endpoint struct {
				MinMs  float64 `json:"MinMs"`
				MaxMs  float64 `json:"MaxMs"`
				MeanMs float64 `json:"MeanMs"`
			} `json:"Endpoint"`
			NetworkSecurityPolicy struct {
				MinMs  float64 `json:"MinMs"`
				MaxMs  float64 `json:"MaxMs"`
				MeanMs float64 `json:"MeanMs"`
			} `json:"NetworkSecurityPolicy"`
		} `json:"KindStats"`
		AggrStats struct {
			MinMs  float64 `json:"MinMs"`
			MaxMs  float64 `json:"MaxMs"`
			MeanMs float64 `json:"MeanMs"`
		} `json:"AggrStats"`
	} `json:"NodeConfigHistogram"`
}

// NewSysModel creates a sysmodel for a testbed
func NewSysModel(tb *TestBed) (*SysModel, error) {
	sm := SysModel{
		tb:                      tb,
		hosts:                   make(map[string]*Host),
		switches:                make(map[string]*Switch),
		naples:                  make(map[string]*Naples),
		veniceNodes:             make(map[string]*VeniceNode),
		veniceNodesDisconnected: make(map[string]*VeniceNode),
		workloads:               make(map[string]*Workload),
		sgpolicies:              make(map[string]*NetworkSecurityPolicy),
		msessions:               make(map[string]*MirrorSession),
		fakeHosts:               make(map[string]*Host),
		fakeNaples:              make(map[string]*Naples),
		fakeWorkloads:           make(map[string]*Workload),
		fakeSubnets:             make(map[string]*Network),
		fakeApps:                make(map[string]*App),
		fakeSGPolicies:          make(map[string]*NetworkSecurityPolicy),
		allocatedMac:            make(map[string]bool),
	}

	ctx, cancel := context.WithTimeout(context.TODO(), 30*time.Minute)
	defer cancel()

	// build venice nodes
	for _, nr := range sm.tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			// create
			err := sm.createVeniceNode(nr)
			if err != nil {
				return nil, err
			}
		}
	}

	// make cluster & setup auth
	err := sm.SetupConfig(ctx)
	if err != nil {
		sm.tb.CollectLogs()
		return nil, err
	}
	// build naples nodes
	for _, nr := range sm.tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM || nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
			err := sm.createNaples(nr)
			if err != nil {
				return nil, err
			}
		} else if nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM {
			err := sm.createMultiSimNaples(nr)
			if err != nil {
				return nil, err
			}
		}
	}

	return &sm, nil
}

// GetVeniceURL returns venice URL for the sysmodel
func (sm *SysModel) GetVeniceURL() []string {
	var veniceURL []string

	if sm.tb.mockMode {
		return []string{mockVeniceURL}
	}

	// walk all venice nodes
	for _, node := range sm.veniceNodes {
		veniceURL = append(veniceURL, fmt.Sprintf("%s:%s", node.iotaNode.IpAddress, globals.APIGwRESTPort))
	}

	return veniceURL
}

// CleanupAllConfig cleans up any configuration present in the system
// this function would query all Venice objects and delete them from Venice
// - it does not clean up any state left out in Naples or inernal components of Venice
func (sm *SysModel) CleanupAllConfig() error {
	var err error

	// get all venice configs
	veniceHosts, err := sm.ListHost()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceSGPolicies, err := sm.ListNetworkSecurityPolicy()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceNetworks, err := sm.ListNetwork()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceApps, err := sm.ListApp()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceWorkloads, err := sm.ListWorkload()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	log.Infof("Cleanup: hosts %d, sgpolicy %d workloads %d hosts %d networks %d",
		len(veniceHosts), len(veniceSGPolicies), len(veniceWorkloads), len(veniceHosts), len(veniceNetworks))

	// delete venice objects
	for _, obj := range veniceSGPolicies {
		if err := sm.DeleteNetworkSecurityPolicy(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	for _, obj := range veniceApps {
		if err := sm.DeleteApp(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	if err := sm.DeleteWorkloads(veniceWorkloads); err != nil {
		err = fmt.Errorf("Error deleting workloads Err: %v", err)
		log.Errorf("%s", err)
		return err
	}
	for _, obj := range veniceHosts {
		if err := sm.DeleteHost(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	//Sleep for a while to make sure to avoid stress on etcd for scale
	//if scale {
	//	time.Sleep(5 * time.Minute)
	//}
	return nil
}

//GetWorkloadsForScale get all workloads with allowa
func (sm *SysModel) GetWorkloadsForScale(hosts []*Host, policyCollection *NetworkSecurityPolicyCollection, proto string) (*WorkloadCollection, error) {
	newCollection := WorkloadCollection{}

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

	for _, pol := range policyCollection.policies {
		for _, rule := range pol.venicePolicy.Spec.Rules {
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
	hostMap := make(map[string]*Host)
	workloadIPMap := make(map[string]*workload.Workload)

	for _, host := range hosts {
		wloads, err := sm.ListWorkloadsOnHost(host.veniceHost)
		if err != nil {
			log.Error("Error finding real workloads on hosts.")
			return nil, err
		}
		for _, w := range wloads {
			workloadIPMap[w.Spec.Interfaces[0].GetIpAddresses()[0]] = w
		}
		hostMap[host.veniceHost.Name] = host
	}

	addWorkload := func(newW *workload.Workload) {
		hWloads, ok := workloadHostMap[newW.Spec.HostName]
		if !ok {
			workloadHostMap[newW.Spec.HostName] = []*workload.Workload{newW}
		} else {
			if len(hWloads) >= defaultWorkloadPerHost {
				//Not adding as number of workloads per host execeeds
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
					//Try to add workloads
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
			wrk, err := sm.createWorkload(w, sm.tb.Topo.WorkloadType, sm.tb.Topo.WorkloadImage, host)
			if err != nil {
				log.Errorf("Error creating workload %v. Err: %v", w.GetName(), err)
				return nil, err
			}
			newCollection.workloads = append(newCollection.workloads, wrk)
		}
	}
	return &newCollection, nil
}

//SetupWorkloadsOnHost sets up workload on host
func (sm *SysModel) SetupWorkloadsOnHost(h *Host) (*WorkloadCollection, error) {

	var wc WorkloadCollection
	nwMap := make(map[uint32]uint32)

	for i := 0; i < defaultNumNetworks; i++ {
		log.Infof("Allocated vlan %v\n", sm.tb.allocatedVlans)
		nwMap[sm.tb.allocatedVlans[i]] = 0
	}

	wloadsPerNetwork := defaultWorkloadPerHost / defaultNumNetworks

	//Keep track of number of workloads per network
	wloadsToCreate := []*workload.Workload{}

	wloads, err := sm.ListWorkloadsOnHost(h.veniceHost)
	if err != nil {
		log.Error("Error finding real workloads on hosts.")
		return nil, err
	}

	if len(wloads) == 0 {
		log.Error("No workloads created on real hosts.")
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
			log.Infof("Adding workload %v (host:%v NodeUUID:(%v) iotaNode:%v nw:%v) to create list", wload.GetName(), h.veniceHost.GetName(), h.iotaNode.GetNodeUuid(), h.iotaNode.Name, nw)

			if len(wloadsToCreate) == defaultWorkloadPerHost {
				// We have enough workloads already for this host.
				break
			}
		}
	}

	for _, wload := range wloadsToCreate {
		wrk, err := sm.createWorkload(wload, sm.tb.Topo.WorkloadType, sm.tb.Topo.WorkloadImage, h)
		if err != nil {
			log.Errorf("Error creating workload %v. Err: %v", wload.GetName(), err)
			return nil, err
		}
		wc.workloads = append(wc.workloads, wrk)
	}

	return &wc, nil

}

// SetupDefaultConfig sets up a default config for the system
func (sm *SysModel) SetupDefaultConfig(ctx context.Context, scale, scaleData bool) error {
	var wc WorkloadCollection

	log.Infof("Setting up default config...")

	// build host list for configuration
	var naplesNodes []*TestNode
	for _, nr := range sm.tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM || nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
			naplesNodes = append(naplesNodes, nr)
		}
	}

	// generate scale configuration if required
	err := sm.populateConfig(ctx, scale)
	if err != nil {
		return fmt.Errorf("Error generating scale config: %s", err)
	}

	err = sm.AssociateHosts()
	if err != nil {
		return fmt.Errorf("Error associating hosts: %s", err)
	}

	hosts, err := sm.ListRealHosts()
	if err != nil {
		log.Error("Error finding real hosts to run traffic tests")
		return err
	}

	if len(hosts) == 0 {
		//msg := "No real hosts run traffic tests"
		//return errors.New(msg)
	}

	for _, sw := range sm.tb.DataSwitches {
		_, err := sm.createSwitch(sw)
		if err != nil {
			log.Errorf("Error creating switch: %#v. Err: %v", sw, err)
			return err
		}

	}

	if scale {
		wloads, err := sm.GetWorkloadsForScale(hosts, sm.DefaultNetworkSecurityPolicy(), "tcp")
		if err != nil {
			log.Errorf("Error finding scale workloads Err: %v", err)
			return err
		}
		wc.workloads = append(wc.workloads, wloads.workloads...)
	} else {
		for _, h := range hosts {
			hwc, err := sm.SetupWorkloadsOnHost(h)

			if err != nil {
				return err
			}

			wc.workloads = append(wc.workloads, hwc.workloads...)
		}
	}

	// if we are skipping setup we dont need to bringup the workload
	if sm.tb.skipSetup {
		// first get a list of all existing workloads from iota
		gwlm := &iota.WorkloadMsg{
			ApiResponse: &iota.IotaAPIResponse{},
			WorkloadOp:  iota.Op_GET,
		}
		topoClient := iota.NewTopologyApiClient(sm.tb.iotaClient.Client)
		getResp, err := topoClient.GetWorkloads(context.Background(), gwlm)
		log.Debugf("Got get workload resp: %+v, err: %v", getResp, err)
		if err != nil {
			log.Errorf("Failed to instantiate Apps. Err: %v", err)
			return fmt.Errorf("Error creating IOTA workload. err: %v", err)
		} else if getResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Failed to instantiate Apps. resp: %+v.", getResp.ApiResponse)
			return fmt.Errorf("Error creating IOTA workload. Resp: %+v", getResp.ApiResponse)
		}

		// check if all the workloads are already running
		allFound := true
		for _, wrk := range wc.workloads {
			found := false
			for _, gwrk := range getResp.Workloads {
				if gwrk.WorkloadName == wrk.iotaWorkload.WorkloadName {
					wrk.iotaWorkload.MgmtIp = gwrk.MgmtIp
					wrk.iotaWorkload.Interface = gwrk.GetInterface()
					found = true
				}
			}
			if !found {
				allFound = false
			}
		}

		if !allFound {
			log.Infof("not all workloads found")
			getResp.WorkloadOp = iota.Op_DELETE
			delResp, err := topoClient.DeleteWorkloads(context.Background(), getResp)
			log.Debugf("Got get workload resp: %+v, err: %v", delResp, err)
			if err != nil {
				log.Errorf("Failed to delete old Apps. Err: %v", err)
				return fmt.Errorf("Error deleting IOTA workload. err: %v", err)
			}

			// bringup the workloads
			err = wc.Bringup()
			if err != nil {
				return err
			}
		} else {
			//Every workload is found, just send arping so that they can get discovered again by datapath
			sm.Action().WorkloadsSayHelloToDataPath()
		}
	} else {
		// bringup the workloads
		err := wc.Bringup()
		if err != nil {
			return err
		}
	}

	// update workload with management ip in the meta
	for _, wr := range wc.workloads {
		wr.veniceWorkload.ObjectMeta.Labels = map[string]string{"MgmtIp": wr.iotaWorkload.MgmtIp}
		if err := sm.CreateWorkload(wr.veniceWorkload); err != nil {
			log.Errorf("unable to update the workload label")
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

func timeTrack(start time.Time, name string) time.Duration {
	elapsed := time.Since(start)
	log.Infof("%s took %s\n", name, elapsed)
	return elapsed
}

//ResetConfigStats Config Stats
func (sm *SysModel) ResetConfigStats() {
	var rawData veniceRawData
	sm.resetConfigStats(&rawData)
}

//ReadConfigStats read config stats
func (sm *SysModel) ReadConfigStats() {

	writeMeasuredStat := func() error {

		var configStat veniceConfigStat
		var rawData veniceRawData
		err := sm.PullConfigStats(&rawData)
		if err != nil {
			log.Infof("Config  stat Failed %v", err)
			return err
		}

		err = json.Unmarshal([]byte(rawData.Diagnostics.String), &configStat)
		if err != nil {
			log.Infof("Config state marshalling Failed %v", err)
			return err
		}

		ofile, err := os.OpenFile(ConfigStatsFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
		if err != nil {
			panic(err)
		}
		j, err := json.MarshalIndent(&configStat, "", "  ")
		ofile.Write(j)
		ofile.Close()

		return nil
	}

	writeMeasuredStat()
}

func (sm *SysModel) ClearConfigPushStat() {
	os.Remove(ConfigPushStatsFile)
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

// populateConfig creates scale configuration based on some predetermined parameters
// TBD: we can enhance this to take the scale parameters fromt he user
func (sm *SysModel) populateConfig(ctx context.Context, scale bool) error {

	cfg := cfgen.DefaultCfgenParams
	cfg.NetworkSecurityPolicyParams.NumPolicies = 1

	//Reset config stats so that we can start fresh

	if scale {
		/* 10K * 4*4 * 3 == 640K hal rules */
		cfg.NetworkSecurityPolicyParams.NumRulesPerPolicy = 10000
		cfg.NetworkSecurityPolicyParams.NumIPPairsPerRule = 4
		cfg.NetworkSecurityPolicyParams.NumAppsPerRules = 2 //1 rule added by gen
		cfg.WorkloadParams.WorkloadsPerHost = 32
		cfg.AppParams.NumApps = 1200
	} else {
		cfg.NetworkSecurityPolicyParams.NumRulesPerPolicy = 10
		cfg.NetworkSecurityPolicyParams.NumIPPairsPerRule = 4
		cfg.NetworkSecurityPolicyParams.NumAppsPerRules = 3
		cfg.WorkloadParams.WorkloadsPerHost = 16
		cfg.AppParams.NumApps = 4
	}
	// TBD - override default-policy
	// cfg.NetworkSecurityPolicyParams.NetworkSecurityPolicyTemplate.ObjectMeta.Name = "default-policy"

	smartnics := []*cluster.DistributedServiceCard{}
	realHostNames := make(map[string]bool)
	for _, naples := range sm.naples {
		smartnics = append(smartnics, naples.smartNic)
	}

	//Add sim naples too.
	for _, naples := range sm.fakeNaples {
		smartnics = append(smartnics, naples.smartNic)
	}

	cfg.Smartnics = smartnics

	generateConfig := func() error {
		defer timeTrack(time.Now(), "Config generation")
		// generate the configuration now
		cfg.Do()
		return nil
	}

	updateScaleConfig := func() error {

		genWorkloadPairs := func(wloads []*workload.Workload, count int) *WorkloadPairCollection {
			var pairs WorkloadPairCollection
			for i := 0; i < len(wloads); i++ {
				for j := i + 1; j <= len(wloads)-1; j++ {
					//pairs = append(pairs, []string{content[i], content[j]})
					if wloads[i].Spec.Interfaces[0].ExternalVlan == wloads[j].Spec.Interfaces[0].ExternalVlan {
						pairs.pairs = append(pairs.pairs, &WorkloadPair{first: &Workload{veniceWorkload: wloads[i]},
							second: &Workload{veniceWorkload: wloads[j]}})
						if count > 0 && len(pairs.pairs) == count {
							return &pairs
						}
					}
				}
			}
			return &pairs
		}

		//Get all real workloads first
		wloads := []*workload.Workload{}
		for _, o := range cfg.ConfigItems.Hosts {
			for _, realNaples := range sm.naples {
				//Get Real host first
				if o.Spec.GetDSCs()[0].MACAddress == realNaples.smartNic.Status.PrimaryMAC {
					for _, wload := range cfg.ConfigItems.Workloads {
						if wload.Spec.HostName == o.Name {
							wloads = append(wloads, wload)
						}
					}
				}
			}
		}
		pairs := genWorkloadPairs(wloads, 0)
		localPairs := WorkloadPairCollection{}
		remotePairs := WorkloadPairCollection{}

		rand.Shuffle(len(localPairs.pairs), func(i, j int) {
			localPairs.pairs[i], localPairs.pairs[j] = localPairs.pairs[j], localPairs.pairs[i]
		})
		rand.Shuffle(len(remotePairs.pairs), func(i, j int) {
			remotePairs.pairs[i], remotePairs.pairs[j] = remotePairs.pairs[j], remotePairs.pairs[i]
		})

		for _, pair := range pairs.pairs {
			if pair.first.veniceWorkload.Spec.HostName == pair.second.veniceWorkload.Spec.HostName {
				localPairs.pairs = append(localPairs.pairs, pair)
			} else {
				remotePairs.pairs = append(remotePairs.pairs, pair)
			}
		}

		localIndex := 0
		remoteIndex := 0
		for _, pol := range cfg.ConfigItems.SGPolicies {
			for _, rule := range pol.Spec.Rules {
				for index := range rule.FromIPAddresses {
					if index%2 == 0 && localIndex < len(localPairs.pairs) {
						rule.FromIPAddresses[index] = localPairs.pairs[localIndex].first.veniceWorkload.Spec.Interfaces[0].IpAddresses[0]
						rule.ToIPAddresses[index] = localPairs.pairs[localIndex].second.veniceWorkload.Spec.Interfaces[0].IpAddresses[0]
						localIndex++
					} else if remoteIndex < len(remotePairs.pairs) {
						rule.FromIPAddresses[index] = remotePairs.pairs[remoteIndex].first.veniceWorkload.Spec.Interfaces[0].IpAddresses[0]
						rule.ToIPAddresses[index] = remotePairs.pairs[remoteIndex].second.veniceWorkload.Spec.Interfaces[0].IpAddresses[0]
						remoteIndex++
					}
				}
			}

		}
		return nil
	}

	configFile := "/tmp/scale-cfg.json"

	writeConfig := func() {
		ofile, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
		if err != nil {
			panic(err)
		}
		j, err := json.MarshalIndent(&cfg.ConfigItems, "", "  ")
		ofile.Write(j)
		ofile.Close()
	}

	readConfig := func() {
		jsonFile, err := os.OpenFile(configFile, os.O_RDONLY, 0755)
		if err != nil {
			panic(err)
		}
		byteValue, _ := ioutil.ReadAll(jsonFile)

		err = json.Unmarshal(byteValue, &cfg.ConfigItems)
		if err != nil {
			panic(err)
		}
		jsonFile.Close()
	}

	if !sm.tb.skipSetup {
		//Generate fresh config if not skip setup
		generateConfig()
		// verify and keep the data in some file

		if scale {
			err := updateScaleConfig()
			if err != nil {
				log.Errorf("Error updating scaling config %v", err)
				return err
			}
		}
		writeConfig()
	} else {
		readConfig()
	}

	for _, o := range cfg.ConfigItems.Networks {
		sm.fakeSubnets[o.ObjectMeta.Name] = &Network{veniceNetwork: o}
	}

	cfgPushTime := ConfigPushTime{}
	cfgPushStats := ConfigPushStats{}

	readStatConfig := func() {
		jsonFile, err := os.OpenFile(ConfigPushStatsFile, os.O_RDONLY, 0755)
		if err != nil {
			//file not created yet. ignore.
			return
		}
		byteValue, _ := ioutil.ReadAll(jsonFile)

		err = json.Unmarshal(byteValue, &cfgPushStats)
		if err != nil {
			panic(err)
		}
		jsonFile.Close()
	}

	writeStatConfig := func() {
		ofile, err := os.OpenFile(ConfigPushStatsFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
		if err != nil {
			panic(err)
		}
		j, err := json.MarshalIndent(&cfgPushStats, "", "  ")
		ofile.Write(j)
		ofile.Close()
	}

	configPushCheck := func(done chan error) {
		startTime := time.Now()
		iter := 1
		for ; iter <= 1500 && ctx.Err() == nil; iter++ {
			//Check every second
			time.Sleep(time.Second * time.Duration(iter))
			complete, err := sm.IsConfigPushComplete()
			if complete && err == nil {
				cfgPushTime.Config = timeTrack(startTime, "Config Push").String()
				done <- nil
			}
		}
		done <- fmt.Errorf("Config push incomplete")
	}

	setupConfigs := func() {
		for _, o := range cfg.ConfigItems.Hosts {
			h := &Host{veniceHost: o}
			sm.fakeHosts[o.ObjectMeta.Name] = h
		}
		for _, pol := range cfg.ConfigItems.SGPolicies {
			sm.fakeSGPolicies[pol.ObjectMeta.Name] = &NetworkSecurityPolicy{venicePolicy: pol}
		}
		for _, app := range cfg.ConfigItems.Apps {
			sm.fakeApps[app.ObjectMeta.Name] = &App{veniceApp: app}
		}
		nwMap := make(map[uint32]uint32)
		tbVlans := make([]uint32, len(sm.tb.allocatedVlans))
		copy(tbVlans, sm.tb.allocatedVlans)

		for _, o := range cfg.ConfigItems.Workloads {
			w := &Workload{veniceWorkload: o}
			if _, ok := realHostNames[o.Spec.HostName]; ok {
				//sm.hosts[o.ObjectMeta.Name] = &Host{veniceHost: o}
			} else {
				//
				sm.fakeWorkloads[o.ObjectMeta.Name] = w
			}

			if wireVlan, ok := nwMap[w.veniceWorkload.Spec.Interfaces[0].ExternalVlan]; ok {
				w.veniceWorkload.Spec.Interfaces[0].ExternalVlan = wireVlan
			} else {
				if len(tbVlans) == 0 {
					//continue
					//return errors.New("Not enough vlans in the testbed for the config")
				} else {
					nwMap[w.veniceWorkload.Spec.Interfaces[0].ExternalVlan] = tbVlans[0]
					tbVlans = tbVlans[1:]
				}
			}
		}
	}

	setupConfigs()

	pushConfigUsingStagingBuffer := func() error {
		defer timeTrack(time.Now(), "Committing Via Config buffer")
		stagingBuf, err := sm.NewStagingBuffer()
		if err != nil {
			return err
		}

		for _, o := range cfg.ConfigItems.Hosts {
			h := &Host{veniceHost: o}
			err := stagingBuf.AddHost(h.veniceHost)
			if err != nil {
				log.Errorf("Error creating host: %+v. Err: %v", h, err)
				return err
			}
		}

		for _, w := range cfg.ConfigItems.Workloads {
			err := stagingBuf.AddWorkload(w)
			if err != nil {
				log.Errorf("Error creating workload Err: %v", err)
				return err
			}
		}

		for _, app := range cfg.ConfigItems.Apps {
			err := stagingBuf.AddApp(app)
			if err != nil {
				log.Errorf("Error creating app Err: %v", err)
				return err
			}
		}

		for _, pol := range cfg.ConfigItems.SGPolicies {
			err := stagingBuf.AddNetowrkSecurityPolicy(pol)
			if err != nil {
				log.Errorf("Error creating app Err: %v", err)
				return err
			}
		}

		//Finally commit it
		return stagingBuf.Commit()
	}

	pushConfig := func() error {
		createHosts := func() error {
			defer timeTrack(time.Now(), "Creating hosts")
			for _, o := range cfg.ConfigItems.Hosts {
				h := &Host{veniceHost: o}
				err := sm.CreateHost(h.veniceHost)
				if err != nil {
					log.Errorf("Error creating host: %+v. Err: %v", h, err)
					return err
				}
				// TBD: push the workloads and host when we simulate Naples
			}
			return nil
		}

		if err := createHosts(); err != nil {
			return err
		}

		createWorkloads := func() error {
			defer timeTrack(time.Now(), "Create workloads")
			err := sm.CreateWorkloads(cfg.ConfigItems.Workloads)
			if err != nil {
				return err
			}
			return nil
		}

		if err := createWorkloads(); err != nil {
			return err
		}

		for _, o := range cfg.ConfigItems.Apps {
			if err := sm.CreateApp(o); err != nil {
				return fmt.Errorf("error creating app: %s", err)
			}
		}

		if len(cfg.ConfigItems.SGPolicies) > 1 {
			panic("can't have more than one sgpolicy")
		}
		for _, o := range cfg.ConfigItems.SGPolicies {
			createSgPolicy := func() error {
				defer timeTrack(time.Now(), "Create Sg Policy")
				if err := sm.CreateNetworkSecurityPolicy(o); err != nil {
					return fmt.Errorf("error creating sgpolicy: %s", err)
				}
				return nil
			}
			if err := createSgPolicy(); err != nil {
				return err
			}
		}
		return nil
	}

	var err error
	if !sm.tb.skipConfigSetup {
		err = sm.CleanupAllConfig()
		if err != nil {
			return err
		}
		if os.Getenv("USE_STAGING_BUFFER") != "" {
			err = pushConfigUsingStagingBuffer()
		} else {
			err = pushConfig()
		}
		if err != nil {
			return err
		}
		policyPushCheck := func(done chan error) {
			for _, o := range cfg.ConfigItems.SGPolicies {
				// verify that sgpolicy object has reached all naples
				startTime := time.Now()
				iter := 1
				for ; iter <= 2000 && ctx.Err() == nil; iter++ {
					time.Sleep(time.Second * time.Duration(iter))
					retSgp, err := sm.GetNetworkSecurityPolicy(&o.ObjectMeta)
					if err != nil {
						done <- fmt.Errorf("error getting back policy %s %v", o.ObjectMeta.Name, err.Error())
						return
					} else if retSgp.Status.PropagationStatus.Updated == int32(len(smartnics)) {
						log.Infof("got back policy satus %+v", retSgp.Status.PropagationStatus)
						duration := timeTrack(startTime, "Sg Policy Push").String()
						cfgPushTime.Object.SgPolicy = duration
						done <- nil
						return
					}
					log.Warnf("Propagation stats did not match for policy %v. %+v", o.ObjectMeta.Name, retSgp.Status.PropagationStatus)
				}
				done <- fmt.Errorf("unable to update policy '%s' on all naples %+v ctx.Err() is %v",
					o.ObjectMeta.Name, o.Status.PropagationStatus, ctx.Err())
			}
		}
		doneChan := make(chan error, 2)
		go policyPushCheck(doneChan)
		go configPushCheck(doneChan)

		for i := 0; i < 2; i++ {
			retErr := <-doneChan
			if retErr != nil {
				err = retErr
			}
		}

		if scale {
			readStatConfig()
			cfgPushStats.Stats = append(cfgPushStats.Stats, cfgPushTime)
			writeStatConfig()
		}
	}

	//Append default Sg polcies
	for _, sgPolicy := range sm.fakeSGPolicies {
		sgPolicy.sm = sm
		log.Infof("Setting up default sg policicies........... \n")
		sm.defaultSgPolicies = append(sm.defaultSgPolicies, sm.NewVeniceNetworkSecurityPolicy(sgPolicy))
	}

	return err
}

// allocMacAddress allocates a unique mac address
func (sm *SysModel) allocMacAddress() (string, error) {
	// Fixed seed to generate repeatable IP Addresses
	var retryCount = 1000

	for i := 0; i < retryCount; i++ {
		b := make([]byte, 8)
		num := uint64(rand.Int63n(math.MaxInt64))
		binary.BigEndian.PutUint64(b, num)
		// This will ensure that we have unicast MAC
		b[0] = (b[0] | 2) & 0xfe
		mac := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])

		// see if this mac addr was already used
		if _, ok := sm.allocatedMac[mac]; !ok {
			sm.allocatedMac[mac] = true
			return mac, nil
		}
	}

	return "", fmt.Errorf("Could not allocate a mac address")
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
	nodes, err := sm.tb.AddNodes(iota.PersonalityType_PERSONALITY_NAPLES, names)
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

	log.Infof("Bringing up workloads naples nodes : %v", names)
	var wc WorkloadCollection
	for _, h := range sm.hosts {
		for _, node := range nodes {
			if node.iotaNode == h.iotaNode {
				hwc, err := sm.SetupWorkloadsOnHost(h)
				if err != nil {
					return err
				}
				wc.workloads = append(wc.workloads, hwc.workloads...)
			}
		}
	}

	return wc.Bringup()
}

// DeleteNaplesNodes nodes on the fly
func (sm *SysModel) DeleteNaplesNodes(names []string) error {
	//First add to testbed.

	nodes := []*TestNode{}
	naplesMap := make(map[string]bool)
	for _, name := range names {
		log.Infof("Deleting naples node : %v", name)
		naples, ok := sm.naples[name]
		if !ok {
			return errors.New("naples not found to delete")
		}

		if _, ok := naplesMap[naples.testNode.iotaNode.Name]; ok {
			//Node already added
			continue
		}
		naplesMap[naples.testNode.iotaNode.Name] = true
		nodes = append(nodes, naples.testNode)
	}

	err := sm.tb.DeleteNodes(nodes)
	if err != nil {
		return err
	}

	for _, name := range names {

		delete(sm.naples, name)
	}
	//Reassociate hosts as new naples is added now.
	return sm.AssociateHosts()
}

// DeleteVeniceNodes nodes on the fly
func (sm *SysModel) DeleteVeniceNodes(names []string) error {
	//First add to testbed.

	clusterNodes, err := sm.ListClusterNodes()
	if err != nil {
		return err
	}

	nodes := []*TestNode{}
	veniceMap := make(map[string]bool)
	for _, name := range names {
		log.Infof("Deleting venice node : %v", name)
		venice, ok := sm.veniceNodes[name]
		if !ok {
			return errors.New("venice not found to delete")
		}

		if _, ok := veniceMap[venice.iotaNode.Name]; ok {
			//Node already added
			continue
		}
		veniceMap[venice.iotaNode.Name] = true
		nodes = append(nodes, venice.testNode)
	}

	clusterDelNodes := []*cluster.Node{}
	for _, node := range nodes {
		added := false
		for _, cnode := range clusterNodes {
			if cnode.ObjectMeta.Name == node.iotaNode.IpAddress {
				clusterDelNodes = append(clusterDelNodes, cnode)
				added = true
				break
			}
		}
		if !added {
			return fmt.Errorf("Node %v not found in the cluster", node.iotaNode.Name)
		}
	}

	//Remove from the cluster
	for _, node := range clusterDelNodes {

		//Remember the cluster node if we want to create again
		for name, vnode := range sm.veniceNodes {
			if vnode.iotaNode.IpAddress == node.Name {
				veniceNode, _ := sm.VeniceNodes().Select("name=" + vnode.iotaNode.Name)
				if err != nil {
					log.Errorf("Error finding venice node .%v", "name="+vnode.iotaNode.Name)
					return err
				}
				//Disconnect node before deleting
				err = sm.Action().DisconnectVeniceNodesFromCluster(veniceNode, sm.Naples())
				if err != nil {
					log.Errorf("Error disonnecting venice node.")
					return err
				}
				vnode.cnode = node
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
	err = sm.tb.DeleteNodes(nodes)
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

	return nil
}

// AddVeniceNodes node on the fly
func (sm *SysModel) AddVeniceNodes(names []string) error {
	//First add to testbed.
	log.Infof("Adding venice nodes : %v", names)
	nodes, err := sm.tb.AddNodes(iota.PersonalityType_PERSONALITY_VENICE, names)
	if err != nil {
		return err
	}

	//Add to cluster
	for _, node := range nodes {
		added := false
		for name, vnode := range sm.veniceNodesDisconnected {
			if vnode.iotaNode.IpAddress == node.iotaNode.IpAddress {
				err := sm.AddClusterNode(vnode.cnode)
				if err != nil {
					return fmt.Errorf("Node add failed %v", err)
				}
			}
			added = true
			//Add to connected nodes
			sm.veniceNodes[name] = vnode
		}

		if !added {
			return fmt.Errorf("Node %v not added to cluster", node.iotaNode.Name)
		}

	}

	for _, name := range names {
		delete(sm.veniceNodesDisconnected, name)
	}

	//Sleep for a while to for the cluster
	time.Sleep(120 * time.Second)
	//Setup venice nodes again.
	sm.SetupVeniceNodes()

	return nil
}

//IsConfigPushComplete checks whether config push is complete.
func (sm *SysModel) IsConfigPushComplete() (bool, error) {

	var configStatus veniceConfigStatus
	var rawData veniceRawData
	err := sm.PullConfigStatus(&rawData)
	if err != nil {
		log.Infof("Config  Failed %v", err)
		return false, err
	}

	err = json.Unmarshal([]byte(rawData.Diagnostics.String), &configStatus)
	if err != nil {
		log.Infof("Config marshalling Failed %v", err)
		return false, err
	}

	workloads, err := sm.ListWorkload()
	if err != nil {
		return false, err
	}
	if configStatus.KindObjects.Endpoint != len(workloads) {
		log.Infof("Endpoints not synced with NPM yet. %v %v", configStatus.KindObjects.Endpoint, len(workloads))
		return false, nil
	}

	policies, err := sm.ListNetworkSecurityPolicy()
	if err != nil {
		return false, err
	}
	if configStatus.KindObjects.NetworkSecurityPolicy != len(policies) {
		log.Infof("policies not synced with NPM yet.")
		return false, nil
	}

	for _, node := range configStatus.NodesStatus {
		if !node.KindStatus.App.Status.Create {
			log.Infof("App Creates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.App.Status.Update {
			log.Infof("App updates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.App.Status.Delete {
			//log.Infof("App deletes not synced for node %v", node.NodeID)
			//return false, nil
		}

		if !node.KindStatus.Endpoint.Status.Create {
			log.Infof("Endpoint Creates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.Endpoint.Status.Update {
			log.Infof("Endpoint updates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.Endpoint.Status.Delete {
			log.Infof("Endpoint deletes not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.SgPolicy.Status.Create {
			log.Infof("SgPolicy Creates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.SgPolicy.Status.Update {
			log.Infof("SgPolicy updates not synced for node %v", node.NodeID)
			return false, nil
		}

		if !node.KindStatus.SgPolicy.Status.Delete {
			log.Infof("SgPolicy deletes not synced for node %v", node.NodeID)
			return false, nil
		}
	}

	return true, nil
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

	if sm.tb.mockMode {
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
	trig := sm.tb.NewTrigger()
	for _, node := range sm.tb.Nodes {
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

	for _, node := range sm.tb.Nodes {
		switch node.Personality {
		case iota.PersonalityType_PERSONALITY_VENICE:
			sm.tb.CopyFromVenice(node.NodeName, []string{fmt.Sprintf("%s_venice.tar", node.NodeName)}, "logs")
		}
	}

	// get token ao authenticate to agent
	veniceCtx, err := sm.VeniceLoggedInCtx(context.Background())
	// get token ao authenticate to agent
	trig = sm.tb.NewTrigger()
	if err == nil {
		ctx, cancel := context.WithTimeout(veniceCtx, 5*time.Second)
		defer cancel()
		token, err := utils.GetNodeAuthToken(ctx, sm.GetVeniceURL()[0], []string{"*"})
		if err == nil {
			// collect tech-support on
			for _, node := range sm.tb.Nodes {
				switch node.Personality {
				case iota.PersonalityType_PERSONALITY_NAPLES:
					cmd := fmt.Sprintf("echo \"%s\" > %s", token, agentAuthTokenFile)
					trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
					cmd = fmt.Sprintf("NAPLES_URL=%s %s/entities/%s_host/%s/%s system tech-support -a %s -b %s-tech-support", penctlNaplesURL, hostToolsDir, node.NodeName, penctlPath, penctlLinuxBinary, agentAuthTokenFile, node.NodeName)
					trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
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
			for _, node := range sm.tb.Nodes {
				switch node.Personality {
				case iota.PersonalityType_PERSONALITY_NAPLES:
					sm.tb.CopyFromHost(node.NodeName, []string{fmt.Sprintf("%s-tech-support.tar.gz", node.NodeName)}, "logs")
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
