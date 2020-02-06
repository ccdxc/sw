package base

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/enterprise/base"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	modelUtils "github.com/pensando/sw/iota/test/venice/iotakit/model/utils"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/willf/bitset"
)

// SysModel represents a objects.of the system under test
type SysModel struct {
	Type common.ModelType
	enterprise.CfgModel
	NaplesHosts                map[string]*objects.Host           // naples tp hosts map
	WorkloadsObjs              map[string]*objects.Workload       // workloads
	ThirdPartyHosts            map[string]*objects.Host           // naples tp hosts map
	Switches                   map[string]*objects.Switch         // switches in test
	SwitchPortsList            []*objects.SwitchPort              // switches in test
	NaplesNodes                map[string]*objects.Naples         // Naples instances
	FakeHosts                  map[string]*objects.Host           // simulated hosts
	FakeNaples                 map[string]*objects.Naples         // simulated Naples instances
	ThirdPartyNodes            map[string]*objects.ThirdPartyNode // Naples instances
	VeniceNodeMap              map[string]*objects.VeniceNode     // Venice nodes
	VeniceNodesMapDisconnected map[string]*objects.VeniceNode     // Venice which are not part of cluster
	AuthToken                  string                             // authToken obtained after logging in

	Tb *testbed.TestBed // testbed

}

func (sm *SysModel) Init(tb *testbed.TestBed, cfgType enterprise.CfgType) error {
	sm.Tb = tb
	sm.NaplesHosts = make(map[string]*objects.Host)
	sm.ThirdPartyHosts = make(map[string]*objects.Host)
	sm.Switches = make(map[string]*objects.Switch)
	sm.NaplesNodes = make(map[string]*objects.Naples)
	sm.ThirdPartyNodes = make(map[string]*objects.ThirdPartyNode)
	sm.VeniceNodeMap = make(map[string]*objects.VeniceNode)
	sm.VeniceNodesMapDisconnected = make(map[string]*objects.VeniceNode)
	sm.FakeNaples = make(map[string]*objects.Naples)
	sm.FakeHosts = make(map[string]*objects.Host)
	sm.WorkloadsObjs = make(map[string]*objects.Workload)

	return nil
}

func (sm *SysModel) ConfigClient() objClient.ObjClient {
	return sm.CfgModel.ObjClient()
}

// SetupConfig sets up the venice cluster and basic config (like auth etc)
func (sm *SysModel) SetupConfig(ctx context.Context) error {

	// build venice nodes
	for _, nr := range sm.Tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			// create
			sm.VeniceNodeMap[nr.NodeName] = objects.NewVeniceNode(nr)
		}
	}

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
		err := sm.DoModeSwitchOfNaples(sm.Tb.Nodes)
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
	return sm.JoinNaplesToVenice(sm.Tb.Nodes)

}

func (sm *SysModel) InitCfgModel() error {

	veniceCtx, err := sm.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return err
	}
	veniceUrls := sm.GetVeniceURL()

	sm.InitClient(veniceCtx, veniceUrls)
	return nil

}

// createNaples creates a naples instance
func (sm *SysModel) CreateNaples(node *testbed.TestNode) error {

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
					sm.FakeNaples[simName] = objects.NewNaplesNode(simName, node, snic)
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

//SetupVeniceNaples setups venice and naples
func (sm *SysModel) SetupVeniceNaples() error {
	ctx, cancel := context.WithTimeout(context.TODO(), 30*time.Minute)
	defer cancel()

	// build venice nodes
	for _, nr := range sm.Tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_VENICE {
			// create
			sm.VeniceNodeMap[nr.NodeName] = objects.NewVeniceNode(nr)
		}
	}

	// make cluster & setup auth
	err := sm.SetupConfig(ctx)
	if err != nil {
		sm.Tb.CollectLogs()
		return err
	}

	return nil
}

//SetupNodes setups up nodes
func (sm *SysModel) SetupNodes() error {

	clusterNodes, err := sm.ListClusterNodes()
	if err != nil {
		return err
	}

	// setup nodes
	for _, nr := range sm.Tb.Nodes {
		if testbed.IsNaples(nr.Personality) {
			err := sm.CreateNaples(nr)
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
					vnode := sm.VeniceNodeMap[nr.NodeName]
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

// AssociateHosts gets all real hosts from venice cluster
func (sm *SysModel) AssociateHosts() error {
	objs, err := sm.ListHost()
	if err != nil {
		return err
	}

	if sm.Tb.IsMockMode() {
		//One on One association if mock mode
		convertMac := func(s string) string {
			mac := strings.Replace(s, ".", "", -1)
			var buffer bytes.Buffer
			var l1 = len(mac) - 1
			for i, rune := range mac {
				buffer.WriteRune(rune)
				if i%2 == 1 && i != l1 {
					buffer.WriteRune(':')
				}
			}
			return buffer.String()
		}
		//In mockmode we retrieve mac from the snicList, so we should convert to format that venice likes.
		for _, naples := range sm.NaplesNodes {
			naples.Nodeuuid = convertMac(naples.Nodeuuid)
		}
	}

	for k := range sm.NaplesHosts {
		delete(sm.NaplesHosts, k)
	}
	for _, n := range sm.NaplesNodes {
		n.SmartNic.Labels = make(map[string]string)
		nodeMac := strings.Replace(n.Nodeuuid, ":", "", -1)
		nodeMac = strings.Replace(nodeMac, ".", "", -1)
		for _, obj := range objs {
			objMac := strings.Replace(obj.GetSpec().DSCs[0].MACAddress, ".", "", -1)
			if objMac == nodeMac {
				log.Infof("Associating host %v(ip:%v) with %v(ip:%v)\n", obj.GetName(),
					n.IP(), n.Name(),
					n.SmartNic.GetStatus().IPConfig.IPAddress)
				bs := bitset.New(uint(4096))
				bs.Set(0).Set(1).Set(4095)

				h := objects.NewHost(obj, n.GetIotaNode(), n)
				sm.NaplesHosts[n.GetIotaNode().Name] = h

				//Add BM type to support upgrade
				n.SmartNic.Labels["type"] = "bm"
				if err := sm.UpdateSmartNIC(n.SmartNic); err != nil {
					log.Infof("Error updating smart nic object %v", err)
					return err
				}
				break
			}
		}
	}

	for k := range sm.FakeNaples {
		delete(sm.FakeHosts, k)
	}
	sm.ListSmartNIC()
	for simName, n := range sm.FakeNaples {
		n.SmartNic.Labels = make(map[string]string)
		for _, simNaples := range n.GetIotaNode().GetNaplesMultiSimConfig().GetSimsInfo() {
			if simNaples.GetName() == simName {
				nodeMac := strings.Replace(simNaples.GetNodeUuid(), ":", "", -1)
				nodeMac = strings.Replace(nodeMac, ".", "", -1)
				for _, obj := range objs {
					objMac := strings.Replace(obj.GetSpec().DSCs[0].MACAddress, ".", "", -1)
					if objMac == nodeMac {
						log.Infof("Associating host %v(ip:%v) with %v(%v on %v)\n", obj.GetName(),
							n.GetIotaNode().GetIpAddress(), simName, simNaples.GetIpAddress(), n.GetIotaNode().Name)
						bs := bitset.New(uint(4096))
						bs.Set(0).Set(1).Set(4095)

						// add it to database
						h := objects.NewHost(obj, n.GetIotaNode(), n)
						sm.FakeHosts[obj.GetName()] = h
						//Add BM type to support upgrade
						n.SmartNic.Labels["type"] = "sim"
						if err := sm.UpdateSmartNIC(n.SmartNic); err != nil {
							log.Infof("Error updating smart nic object %v", err)
							return err
						}
						break
					}
				}
			}
		}
	}

	for k := range sm.ThirdPartyHosts {
		delete(sm.ThirdPartyHosts, k)
	}
	for _, n := range sm.ThirdPartyNodes {
		for _, obj := range objs {
			if obj.GetSpec().DSCs[0].MACAddress == n.Node.Nodeuuid {
				h := objects.NewHost(obj, n.GetIotaNode(), nil)
				log.Infof("Associating third party host %v(ip:%v) with %v(\n", obj.GetName(),
					n.IP(), n.Name())
				sm.ThirdPartyHosts[n.Name()] = h
			}
		}

	}

	log.Infof("Total number of hosts associated %v\n", len(sm.NaplesHosts)+len(sm.FakeHosts))
	log.Infof("Total number of 3rd Party hosts associated %v\n", len(sm.ThirdPartyHosts))
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

	for _, naples := range sm.FakeNaples {
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

	return nil
}

// createHost creates a new host instance
func (sm *SysModel) CreateSwitch(sw *iota.DataSwitch) (*objects.Switch, error) {

	smSw := objects.NewSwitch(sw)

	getHostName := func(ip, port string) (string, error) {
		for _, node := range sm.Tb.Nodes {
			for _, dn := range node.InstanceParams().DataNetworks {
				if dn.Name == port && dn.SwitchIP == ip {
					return node.NodeName, nil
				}
			}
		}
		return "", fmt.Errorf("Node name not found for switch %v %v", port, ip)
	}
	for _, p := range sw.GetPorts() {

		hostName, err := getHostName(smSw.IP(), p)
		if err != nil {
			return nil, err
		}
		swPort := objects.NewSwitchPort(hostName, smSw, p)

		sm.SwitchPortsList = append(sm.SwitchPortsList, swPort)
	}
	sm.Switches[sw.GetIp()] = smSw

	return smSw, nil
}

func (sm *SysModel) Cleanup() error {
	// collect all log files
	sm.CollectLogs()
	return nil
}

//GetOrchestrator Default objects.has no orchestrator
func (sm *SysModel) GetOrchestrator() (*objects.Orchestrator, error) {
	return nil, nil
}

// NetworkSecurityPolicy finds an SG policy by name
func (sm *SysModel) NetworkSecurityPolicy(name string) *objects.NetworkSecurityPolicyCollection {
	return nil
}

// DefaultNetworkSecurityPolicy resturns default-policy that prevails across tests cases in the system
func (sm *SysModel) DefaultNetworkSecurityPolicy() *objects.NetworkSecurityPolicyCollection {
	return nil
}

// Networks returns a list of subnets
func (sm *SysModel) Networks() *objects.NetworkCollection {
	return nil
}

func (sm *SysModel) NewMirrorSession(name string) *objects.MirrorSessionCollection {
	return nil
}

func (sm *SysModel) NewNetworkSecurityPolicy(name string) *objects.NetworkSecurityPolicyCollection {
	return nil
}

func (sm *SysModel) QueryDropMetricsForWorkloadPairs(wpc *objects.WorkloadPairCollection, timestr string) error {
	return errors.New("Not implemented")
}

func (sm *SysModel) VerifyRuleStats(timestr string, spc *objects.NetworkSecurityPolicyCollection, minCounts []map[string]float64) error {
	return errors.New("Not implemented")
}
