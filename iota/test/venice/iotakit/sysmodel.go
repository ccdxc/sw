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
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfgen"
	"github.com/pensando/sw/venice/utils/log"
)

// default number of workloads per host
const defaultWorkloadPerHost = 4

// default number of networks in the model
const defaultNumNetworks = 2

// SysModel represents a model of the system under test
type SysModel struct {
	hosts          map[string]*Host          // hosts
	naples         map[string]*Naples        // Naples instances
	workloads      map[string]*Workload      // workloads
	subnets        []*Network                // subnets
	sgpolicies     map[string]*SGPolicy      // security policies
	msessions      map[string]*MirrorSession // mirror sessions
	veniceNodes    map[string]*VeniceNode    // Venice nodes
	fakeHosts      map[string]*Host          // simulated hosts
	fakeNaples     map[string]*Naples        // simulated Naples instances
	fakeWorkloads  map[string]*Workload      // simulated workloads
	fakeSubnets    map[string]*Network       // simulated subnets
	fakeApps       map[string]*App           // simulated apps
	fakeSGPolicies map[string]*SGPolicy      // simulated security policies

	tb *TestBed // testbed

	allocatedMac map[string]bool // allocated mac addresses
}

// NewSysModel creates a sysmodel for a testbed
func NewSysModel(tb *TestBed) (*SysModel, error) {
	sm := SysModel{
		tb:             tb,
		hosts:          make(map[string]*Host),
		naples:         make(map[string]*Naples),
		veniceNodes:    make(map[string]*VeniceNode),
		workloads:      make(map[string]*Workload),
		sgpolicies:     make(map[string]*SGPolicy),
		msessions:      make(map[string]*MirrorSession),
		fakeHosts:      make(map[string]*Host),
		fakeNaples:     make(map[string]*Naples),
		fakeWorkloads:  make(map[string]*Workload),
		fakeSubnets:    make(map[string]*Network),
		fakeApps:       make(map[string]*App),
		fakeSGPolicies: make(map[string]*SGPolicy),
		allocatedMac:   make(map[string]bool),
	}

	// build naples and venice nodes
	for _, nr := range sm.tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM || nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
			err := sm.createNaples(nr)
			if err != nil {
				return nil, err
			}
		} else if nr.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			// create
			err := sm.createVeniceNode(nr.iotaNode)
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

// CleanupAllConfig cleans up any configuration present in the system
// this function would query all Venice objects and delete them from Venice
// - it does not clean up any state left out in Naples or inernal components of Venice
func (sm *SysModel) CleanupAllConfig() error {
	var err error

	if !sm.tb.skipSetup {
		return nil
	}

	// get all venice configs
	veniceHosts, err := sm.tb.ListHost()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceSGPolicies, err := sm.tb.ListSGPolicy()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceNetworks, err := sm.tb.ListNetwork()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceApps, err := sm.tb.ListApp()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}
	veniceWorkloads, err := sm.tb.ListWorkload()
	if err != nil {
		log.Errorf("err: %s", err)
		return err
	}

	log.Infof("Cleanup: hosts %d, sgpolicy %d workloads %d hosts %d networks %d",
		len(veniceHosts), len(veniceSGPolicies), len(veniceWorkloads), len(veniceHosts), len(veniceNetworks))

	// delete venice objects
	for _, obj := range veniceSGPolicies {
		if err := sm.tb.DeleteSGPolicy(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	for _, obj := range veniceApps {
		if err := sm.tb.DeleteApp(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	for _, obj := range veniceWorkloads {
		if err := sm.tb.DeleteWorkload(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}
	for _, obj := range veniceHosts {
		if err := sm.tb.DeleteHost(obj); err != nil {
			err = fmt.Errorf("Error deleting obj %+v. Err: %s", obj, err)
			log.Errorf("%s", err)
			return err
		}
	}

	return nil
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
	err := sm.populateConfig(ctx, sm.tb.GetBaseVlan(), scale)
	if err != nil {
		return fmt.Errorf("Error generating scale config: %s", err)
	}

	err = sm.AssociateHosts()
	if err != nil {
		return fmt.Errorf("Error associating hosts: %s", err)
	}
	// create some networks
	/*for i := 0; i < defaultNumNetworks; i++ {
		vlanID := sm.tb.allocatedVlans[i]
		err := sm.createNetwork(vlanID, fmt.Sprintf("192.168.%d.0/24", i+2))
		if err != nil {
			log.Errorf("Error creating network: vlan %v. Err: %v", vlanID, err)
			return err
		}
	}

	//snc := sm.Networks() */

	hosts, err := sm.ListRealHosts()
	if err != nil {
		log.Error("Error finding real hosts to run traffic tests")
		return err
	}

	if len(hosts) == 0 {
		msg := "No real hosts run traffic tests"
		return errors.New(msg)
	}

	nwMap := make(map[uint32]uint32)

	for _, h := range hosts {

		for i := 2 + sm.tb.GetBaseVlan(); i <= defaultNumNetworks+sm.tb.GetBaseVlan()+1; i++ {
			nwMap[((uint32)(i))] = 0
		}
		wloadsPerNetwork := defaultWorkloadPerHost / defaultNumNetworks

		//Keep track of number of workloads per network
		wloadsToCreate := []*workload.Workload{}

		wloads, err := sm.ListWorkloadsOnHost(h.veniceHost)
		if err != nil {
			log.Error("Error finding real workloads on hosts.")
			return err
		}

		if len(wloads) == 0 {
			log.Error("No workloads created on real hosts.")
			return err
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
				return err
			}
			wc.workloads = append(wc.workloads, wrk)
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
		if err := sm.tb.CreateWorkload(wr.veniceWorkload); err != nil {
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
	err = sm.DefaultSGPolicy().Restore()
	if err != nil {
		log.Errorf("Error creating default policy. Err: %v", err)
		return err
	}

	return nil
}

func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
	log.Infof("%s took %s\n", name, elapsed)
}

// populateConfig creates scale configuration based on some predetermined parameters
// TBD: we can enhance this to take the scale parameters fromt he user
func (sm *SysModel) populateConfig(ctx context.Context, vlanBase uint32, scale bool) error {
	cfg := cfgen.DefaultCfgenParams
	cfg.SGPolicyParams.NumPolicies = 1

	if scale {
		cfg.SGPolicyParams.NumRulesPerPolicy = 50000
		cfg.WorkloadParams.WorkloadsPerHost = 100
		cfg.AppParams.NumApps = 5000
	} else {
		cfg.SGPolicyParams.NumRulesPerPolicy = 5
		cfg.WorkloadParams.WorkloadsPerHost = 50
		cfg.AppParams.NumApps = 4
	}
	// TBD - override default-policy
	// cfg.SGPolicyParams.SGPolicyTemplate.ObjectMeta.Name = "default-policy"

	smartnics := []*cluster.SmartNIC{}
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

	configFile := "/tmp/scale.json"

	writeConfig := func() {
		ofile, err := os.OpenFile(configFile, os.O_RDWR|os.O_CREATE, 0755)
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
		writeConfig()
	} else {
		readConfig()
	}

	for _, o := range cfg.ConfigItems.Networks {
		sm.fakeSubnets[o.ObjectMeta.Name] = &Network{veniceNetwork: o}
	}

	createHosts := func() error {
		defer timeTrack(time.Now(), "Creating hosts")
		for _, o := range cfg.ConfigItems.Hosts {
			h := &Host{veniceHost: o}
			sm.fakeHosts[o.ObjectMeta.Name] = h

			err := sm.tb.CreateHost(h.veniceHost)
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
		for _, o := range cfg.ConfigItems.Workloads {
			w := &Workload{veniceWorkload: o}
			if _, ok := realHostNames[o.Spec.HostName]; ok {
				//sm.hosts[o.ObjectMeta.Name] = &Host{veniceHost: o}
			} else {
				//
				sm.fakeWorkloads[o.ObjectMeta.Name] = w
			}

			w.veniceWorkload.Spec.Interfaces[0].ExternalVlan += vlanBase
			err := sm.tb.CreateWorkload(w.veniceWorkload)
			if err != nil {
				return err
			}
		}
		return nil
	}

	if err := createWorkloads(); err != nil {
		return err
	}

	for _, o := range cfg.ConfigItems.Apps {

		sm.fakeApps[o.ObjectMeta.Name] = &App{veniceApp: o}
		if err := sm.tb.CreateApp(o); err != nil {
			return fmt.Errorf("error creating app: %s", err)
		}
	}

	if len(cfg.ConfigItems.SGPolicies) > 1 {
		panic("can't have more than one sgpolicy")
	}

	for _, o := range cfg.ConfigItems.SGPolicies {

		createSgPolicy := func() error {
			defer timeTrack(time.Now(), "Create Sg Policy")

			sm.fakeSGPolicies[o.ObjectMeta.Name] = &SGPolicy{venicePolicy: o}
			if err := sm.tb.CreateSGPolicy(o); err != nil {
				return fmt.Errorf("error creating sgpolicy: %s", err)
			}

			return nil
		}

		if err := createSgPolicy(); err != nil {
			return err
		}

		// verify that sgpolicy object has reached all naples
		policyPushCheck := func() error {
			defer timeTrack(time.Now(), "Sg Policy Push")
			iter := 1
			for ; iter <= 2000 && ctx.Err() == nil; iter++ {
				time.Sleep(time.Second * time.Duration(iter))
				if retSgp, err := sm.tb.GetSGPolicy(&o.ObjectMeta); err != nil {
					return fmt.Errorf("error getting back policy %s %v", o.ObjectMeta.Name, err.Error())
				} else if retSgp.Status.PropagationStatus.Updated == int32(len(smartnics)) {
					log.Infof("got back policy satus %+v", o.Status.PropagationStatus)
					return nil
				}
			}
			return fmt.Errorf("unable to update policy '%s' on all naples %+v ctx.Err() is %v",
				o.ObjectMeta.Name, o.Status.PropagationStatus, ctx.Err())
		}

		if err := policyPushCheck(); err != nil {
			return err
		}

	}

	return nil
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

	return sm.tb.UpdateFirewallProfile(&fwp)
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
	err := sm.tb.CreateApp(&ftpApp)
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
	err = sm.tb.CreateApp(&icmpApp)
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
	err = sm.tb.CreateApp(&icmpApp)
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
	err = sm.tb.CreateApp(&dnsApp)
	if err != nil {
		return fmt.Errorf("Error creating DNS app. Err: %v", err)
	}

	return nil
}

// AddNaplesNode node on the fly
func (sm *SysModel) AddNaplesNode(name string) error {
	//First add to testbed.
	node, err := sm.tb.AddNode(iota.PersonalityType_PERSONALITY_NAPLES, name)
	if err != nil {
		return err
	}

	return sm.createNaples(node)
}

// DeleteNaplesNode node on the fly
func (sm *SysModel) DeleteNaplesNode(name string) error {
	//First add to testbed.

	naples, ok := sm.naples[name]

	if !ok {
		return errors.New("naples not found to delete")
	}

	err := sm.tb.DeleteNode(naples.testNode)
	if err != nil {
		return err
	}

	delete(sm.naples, name)
	return nil
}
