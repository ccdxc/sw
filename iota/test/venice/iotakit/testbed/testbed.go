package testbed

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
	"sort"
	"strconv"
	"strings"
	"text/tabwriter"
	"time"

	"github.com/onsi/ginkgo"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const defaultIotaServerURL = "localhost:60000"
const maxTestbedInitRetry = 1
const agentAuthTokenFile = "/tmp/auth_token"
const naplesTBFile = "/tmp/naples.dat"

// DataNetworkParams contains switch port info for each port
type DataNetworkParams struct {
	Name           string // switch port's name
	SwitchPort     int    // port number on the switch
	SwitchIP       string // IP address of the switch
	SwitchUsername string // switch user name
	SwitchPassword string // switch password
}

//Port of nic
type Port struct {
	Name           string
	IP             string
	MAC            string
	SwitchIP       string
	SwitchPort     int
	SwitchUsername string
	SwitchPassword string
}

//Nic def
type Nic struct {
	MgmtIP          string
	Type            string
	ConsoleIP       string
	ConsolePort     string
	ConsolePassword string
	Ports           []Port
}

// InstanceParams contains information about vm/baremetal nodes
type InstanceParams struct {
	Name           string // node name
	Type           string // node type vm or baremetal
	ID             int    // node identifier as provided by warmd
	idx            int    // node index within the testbed
	NodeMgmtIP     string // mgmt ip of the node
	NicConsoleIP   string // NIC's console server IP
	NicConsolePort string // NIC's console server port
	NicMgmtIP      string // NIC's oob mgmt port address
	NodeCimcIP     string // CIMC ip address of the server
	NodeServer     string // NodeServer whether server is ucs/hpe
	Tag            string // Tag to have some clue of the instance
	Resource       struct {
		NICType    string // NIC type (naples, intel, mellanox etc)
		NICUuid    string // NIC's mac address
		ServerType string // baremetal server type (server-a or server-d)
		InbandMgmt bool
		Network    struct {
			Address string
			Gateway string
			IPRange string
		}
	}

	DataNetworks []DataNetworkParams // data networks
	Nics         []Nic
}

// Params to be parsed from warmd.json file
type Params struct {
	ID        string // testbed identifier
	Provision struct {
		Username string            // user name for SSH login
		Password string            // password for SSH login
		Vars     map[string]string // custom variables passed from .job.yml
	}
	Instances []InstanceParams // nodes in the testbed
	Network   struct {
		VlanID int
	}
}

func (n Nic) isNaples() bool {
	switch n.Type {
	case "naples":
		fallthrough
	case "pensando":
		return true
	}

	return false
}

const (
	naplesInband  = "inb_mnic0"
	naplesOutband = "oob_mnic0"
)

func (inst *InstanceParams) getNicMgmtIP() (string, error) {

	useInband := false
	if inst.Resource.InbandMgmt {
		useInband = true
	}

	for _, nic := range inst.Nics {
		if nic.isNaples() {
			for _, port := range nic.Ports {
				if useInband && port.Name == naplesInband {
					return port.IP, nil
				}
				if !useInband && port.Name == naplesOutband {
					return port.IP, nil
				}
			}
		}
	}

	return inst.NicMgmtIP, nil
}

// TestNode contains state of a node in the testbed
type TestNode struct {
	NodeName            string               // node name specific to this topology
	NodeUUID            []string             // node UUID
	Type                iota.TestBedNodeType // node type
	Personality         iota.PersonalityType // node topology
	NodeMgmtIP          string
	VeniceConfig        iota.VeniceConfig         // venice specific configuration
	NaplesConfigs       iota.NaplesConfigs        // naples specific config
	NaplesMultSimConfig iota.NaplesMultiSimConfig // naples multiple sim specific config
	VcenterConfig       iota.VcenterConfig        // vcenter config
	iotaNode            *iota.Node                // node info we got from iota
	instParams          *InstanceParams           // instance params we got from warmd.json
	topoNode            *TopoNode                 // node info from topology
}

//InstanceParams return inst params
func (t *TestNode) InstanceParams() *InstanceParams {
	return t.instParams
}

func (t *TestNode) GetIotaNode() *iota.Node {
	return t.iotaNode
}

//IsNaplesHW is naples HW node
func IsNaplesHW(personality iota.PersonalityType) bool {
	return personality == iota.PersonalityType_PERSONALITY_NAPLES ||
		personality == iota.PersonalityType_PERSONALITY_NAPLES_DVS
}

//IsThirdParty is naples HW node
func IsThirdParty(personality iota.PersonalityType) bool {
	return personality == iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC ||
		personality == iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC_DVS
}

//IsHW is HW node
func IsHWNode(personality iota.PersonalityType) bool {
	return IsNaplesHW(personality) ||
		personality == iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC ||
		personality == iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC_DVS
}

//IsNaples node
func IsNaples(personality iota.PersonalityType) bool {
	return IsNaplesHW(personality) || personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM
}

//IsNaplesHW is naples HW node
func (n *TestNode) IsNaplesHW() bool {
	return IsNaplesHW(n.Personality)
}

type naplesData struct {
	Naples string `json:"naples"`
	ID     int    `json:"id"`
}

// TestBed is the state of the testbed
type TestBed struct {
	Topo                 Topology                   // testbed topology
	Params               Params                     // testbed params - provided by warmd
	Nodes                []*TestNode                // nodes in the testbed
	mockMode             bool                       // mock iota server and venice node for testing purposes
	mockIota             *mockIotaServer            // mock iota server
	skipSetup            bool                       // skip setting up the cluster
	skipConfigSetup      bool                       // skip config setup
	hasNaplesSim         bool                       // has Naples sim nodes in the topology
	hasNaplesHW          bool                       // testbed has Naples HW in the topology
	allocatedVlans       []uint32                   // VLANs allocated for this testbed
	unallocatedInstances []*InstanceParams          // currently unallocated instances
	testResult           map[string]bool            // test result
	taskResult           map[string]error           // sub task result
	caseResult           map[string]*TestCaseResult // test case result counts
	DataSwitches         []*iota.DataSwitch         // data switches associated to this testbed
	naplesDataMap        map[string]naplesData      //naples name data map
	dcName               string

	// cached message responses from iota server
	iotaClient      *common.GRPCClient   // iota grpc client
	initTestbedResp *iota.TestBedMsg     // init testbed resp from iota server
	addNodeResp     *iota.NodeMsg        // add node resp from iota server
	makeClustrResp  *iota.MakeClusterMsg // resp to make cluster message
	authCfgResp     *iota.AuthMsg        // auth response
	switchName      string
	cleanup         bool //clean up testbed after done
	tbMsg           *iota.TestBedMsg
}

func (tb *TestBed) GetSwitch() string {
	return tb.switchName
}

//Client returns client
func (t *TestBed) Client() *common.GRPCClient {
	return t.iotaClient
}

// TestCaseResult stores test case results
type TestCaseResult struct {
	passCount int
	failCount int
	duration  time.Duration
}

// NewTestBed initializes a new testbed and returns a testbed handler
func NewTestBed(topoName string, paramsFile string) (*TestBed, error) {
	// skip setup when we are re-running the tests
	skipSetup := os.Getenv("SKIP_SETUP")
	if skipSetup != "" {
		log.Infof("Skipping setup...")
		return newTestBed(topoName, paramsFile, true)
	}
	return newTestBed(topoName, paramsFile, false)
}

// GetTestbed returns a testbed handler without initializing the testbed.
func GetTestbed(topoName string, paramsFile string) (*TestBed, error) {
	return newTestBed(topoName, paramsFile, true)
}

func newTestBed(topoName string, paramsFile string, skipSetup bool) (*TestBed, error) {
	var params Params

	// find the topology by name
	topo, ok := Topologies[topoName]
	if !ok {
		log.Errorf("Can not find topo name: %v", topoName)
		return nil, fmt.Errorf("Topology not found")
	}

	// read the testbed params
	jsonFile, err := os.Open(paramsFile)
	if err != nil {
		log.Errorf("Error opening file %v. Err: %v", paramsFile, err)
		return nil, err
	}
	defer jsonFile.Close()
	byteValue, err := ioutil.ReadAll(jsonFile)
	if err != nil {
		log.Errorf("Error reading from file %v. Err: %v", paramsFile, err)
		return nil, err
	}

	// parse the json file
	err = json.Unmarshal(byteValue, &params)
	if err != nil {
		log.Errorf("Error parsing JSON from %v. Err: %v", string(byteValue), err)
		return nil, err
	}

	// perform some checks on the parsed JSON
	if params.ID == "" || params.Provision.Username == "" || params.Provision.Password == "" {
		log.Errorf("Not sufficient params in JSON: %+v", params)
		return nil, fmt.Errorf("Not sufficient params in JSON")
	}

	if len(params.Instances) < len(topo.Nodes) {
		log.Errorf("Topology requires atleast %d nodes. Testbed has only %d nodes", len(topo.Nodes), len(params.Instances))
		return nil, fmt.Errorf("Not enough nodes in testbed")
	}

	// create a testbed instance
	tb := TestBed{
		Topo:       *topo,
		Params:     params,
		Nodes:      make([]*TestNode, len(topo.Nodes)),
		testResult: make(map[string]bool),
		taskResult: make(map[string]error),
		caseResult: make(map[string]*TestCaseResult),
	}

	// initialize node state
	err = tb.initNodeState()
	if err != nil {
		return nil, err
	}

	// see if we need to mock iota server
	mockMode := os.Getenv("MOCK_IOTA")
	if mockMode != "" {
		tb.mockMode = true

		// create a mock iota server
		tb.mockIota, err = newMockIotaServer(defaultIotaServerURL, &tb)
		if err != nil {
			return nil, err
		}
	}

	tb.skipSetup = skipSetup

	if os.Getenv("SKIP_CONFIG") != "" {
		log.Infof("Skipping config push ...")
		tb.skipConfigSetup = true
	}

	// connect to iota server
	err = tb.connectToIotaServer()
	if err != nil {
		return nil, err
	}

	// init testbed and add nodes
	for i := 0; i < maxTestbedInitRetry; i++ {
		log.Infof("Trying to initialize testbed..")
		err = tb.setupTestBed()
		if err == nil {
			break
		}
		log.Warnf("Error while setting up testbed. Retrying...")
	}
	if err != nil {
		log.Errorf("Setting up testbed failed after retries. collecting logs")
		tb.CollectLogs()
		return nil, err
	}

	// check iota cluster health
	err = tb.CheckIotaClusterHealth()
	if err != nil {
		log.Errorf("Error during Checking cluster health. collecting logs")
		tb.CollectLogs()
		return nil, err
	}

	// return testbed instance
	return &tb, nil
}

//AllocatedVlans returns allocated vlan
func (tb *TestBed) AllocatedVlans() []uint32 {
	return tb.allocatedVlans
}

// HasNaplesSim returns true if testbed is a Naples sim testbed
func (tb *TestBed) HasNaplesSim() bool {
	return tb.hasNaplesSim
}

//DoSkipSetup skip setup
func (tb *TestBed) DoSkipSetup() {
	tb.skipSetup = true
}

// HasNaplesHW returns true if testbed has Naples HW
func (tb *TestBed) HasNaplesHW() bool {
	return tb.hasNaplesHW
}

// numNaples returns number of naples nodes in the topology
func (tb *TestBed) numNaples() int {
	numNaples := 0
	for _, node := range tb.Nodes {
		if node.IsNaplesHW() {
			numNaples++
		} else if node.Personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM {
			numNaples++
		}
	}

	return numNaples
}

// IsMockMode returns true if test is running in mock mode
func (tb *TestBed) IsMockMode() bool {
	return tb.mockMode
}

// getAvailableInstance returns next instance of a given type
func (tb *TestBed) getAvailableInstance(instType iota.TestBedNodeType) *InstanceParams {

	for idx, inst := range tb.unallocatedInstances {
		switch instType {
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM:
			if inst.Type == "vm" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_THIRD_PARTY:
			if inst.Type == "bm" && inst.Resource.NICType != "pensando" && inst.Resource.NICType != "naples" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_HW:
			if inst.Type == "bm" && inst.Resource.Network.Address == "" && inst.Tag != "venice" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM:
			if inst.Type == "bm" && inst.Resource.Network.Address != "" && inst.Tag != "venice" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_VENICE_BM:
			if inst.Type == "bm" && inst.Resource.Network.Address != "" && inst.Tag == "venice" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_VCENTER:
			if inst.Type == "vm" && inst.Tag == "vcenter" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		}
	}

	//This should be removed when vcenter is provisioned from jobd
	if instType == iota.TestBedNodeType_TESTBED_NODE_TYPE_VCENTER &&
		tb.Params.Provision.Vars["VcenterIP"] != "" {
		inst := &InstanceParams{Type: "vm", Tag: "vcenter",
			NodeMgmtIP: tb.Params.Provision.Vars["VcenterIP"],
			Name:       tb.Params.Provision.Vars["VcenterIP"]}
		return inst
	}

	log.Fatalf("Could not find any instances of type: %v", instType)
	return nil
}

// getNaplesInstanceByName which was allocated before
func (tb *TestBed) getNaplesInstanceByName(name string) *InstanceParams {

	for _, naples := range tb.naplesDataMap {
		for idx, inst := range tb.unallocatedInstances {
			if inst.Type == "bm" && naples.Naples == name && inst.ID == naples.ID {
				log.Infof("Choosing  %v %v %v %v", inst.ID, name, naples.ID, naples.Naples)
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		}
	}

	//Did not find int allocated before instances, try to allocate a fresh one
	return tb.getAvailableInstance(iota.TestBedNodeType_TESTBED_NODE_TYPE_HW)
}

func (tb *TestBed) addAvailableInstance(instance *InstanceParams) error {
	for _, inst := range tb.unallocatedInstances {
		if inst.ID == instance.ID {
			return nil
		}
	}

	//Add element to beginning to make sure we can reuse
	tb.unallocatedInstances = append([]*InstanceParams{instance}, tb.unallocatedInstances...)
	return nil
}

func (tb *TestBed) preapareNodeParams(nodeType iota.TestBedNodeType, personality iota.PersonalityType, node *TestNode) error {

	// check if testbed node can take this personality
	switch nodeType {
	case iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM:
		if node.instParams.Type != "bm" {
			log.Errorf("Incompatible testbed node %v for personality %v/%v", node.instParams.Type, nodeType, personality)
			return fmt.Errorf("Incompatible testbed node")
		}

		//tb.hasNaplesSim = true
		node.NaplesMultSimConfig = iota.NaplesMultiSimConfig{
			Network:      node.instParams.Resource.Network.Address,
			Gateway:      node.instParams.Resource.Network.Gateway,
			IpAddrRange:  node.instParams.Resource.Network.IPRange,
			NumInstances: (uint32)(node.topoNode.NumInstances),
			VeniceIps:    nil,
			NicType:      node.instParams.Resource.NICType,
			Parent:       "eno1",
		}
	case iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM:
		switch personality {
		case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
			if node.instParams.Type != "vm" {
				log.Errorf("Incompatible testbed node %v for personality %v/%v", nodeType, personality, personality)
				return fmt.Errorf("Incompatible testbed node")
			}

			tb.hasNaplesSim = true
			node.NaplesConfigs = iota.NaplesConfigs{
				Configs: []*iota.NaplesConfig{
					&iota.NaplesConfig{
						ControlIntf:     "eth1",
						ControlIp:       fmt.Sprintf("172.16.100.%d", len(tb.Nodes)+1), //FIXME
						DataIntfs:       []string{"eth2", "eth3"},
						NaplesIpAddress: "169.254.0.1",
						NaplesUsername:  "root",
						NaplesPassword:  "pen123",
						NicType:         "pensando-sim",
						Name:            "pensando-sim",
					},
				},
			}
		case iota.PersonalityType_PERSONALITY_VENICE:
			if node.instParams.Type != "vm" {
				log.Errorf("Incompatible testbed node %v for personality %v/%v", node.instParams.Type, nodeType, personality)
				return fmt.Errorf("Incompatible testbed node")
			}

			node.VeniceConfig = iota.VeniceConfig{
				ControlIntf: "eth1",
				ControlIp:   fmt.Sprintf("172.16.100.%d", len(tb.Nodes)+1), //FIXME
				VenicePeers: []*iota.VenicePeer{},
			}
		default:
			return fmt.Errorf("unsupported node personality %v", personality)
		}

	case iota.TestBedNodeType_TESTBED_NODE_TYPE_VCENTER:
		switch personality {
		case iota.PersonalityType_PERSONALITY_VCENTER_NODE:
		}
	case iota.TestBedNodeType_TESTBED_NODE_TYPE_THIRD_PARTY:
		// we need Esx credentials if this is running esx
		if node.topoNode.HostOS == "esx" {
			_, uok := tb.Params.Provision.Vars["EsxUsername"]
			_, pok := tb.Params.Provision.Vars["EsxPassword"]
			if !uok || !pok {
				log.Errorf("Esx login credentials are not provided in testbed params. %+v", tb.Params.Provision)
				return fmt.Errorf("Esx login credentials are not provided")
			}
		}
	case iota.TestBedNodeType_TESTBED_NODE_TYPE_HW:
		switch personality {
		case iota.PersonalityType_PERSONALITY_NAPLES_DVS:
			fallthrough
		case iota.PersonalityType_PERSONALITY_NAPLES:
			if node.instParams.Type != "bm" {
				log.Errorf("Incompatible testbed node %v for personality %v/%v", node.instParams.Type, nodeType, personality)
				return fmt.Errorf("Incompatible testbed node")
			}
			tb.hasNaplesHW = true
			// we need Esx credentials if this is running esx
			if node.topoNode.HostOS == "esx" {
				_, uok := tb.Params.Provision.Vars["EsxUsername"]
				_, pok := tb.Params.Provision.Vars["EsxPassword"]
				if !uok || !pok {
					log.Errorf("Esx login credentials are not provided in testbed params. %+v", tb.Params.Provision)
					return fmt.Errorf("Esx login credentials are not provided")
				}
			}

			node.NaplesConfigs = iota.NaplesConfigs{Configs: []*iota.NaplesConfig{}}
			for nicID, nic := range node.instParams.Nics {
				for _, port := range nic.Ports {

					config := &iota.NaplesConfig{
						ControlIntf:     "eth1",
						ControlIp:       fmt.Sprintf("172.16.100.%d", len(tb.Nodes)+1), //FIXME
						DataIntfs:       []string{"eth2", "eth3"},
						NaplesIpAddress: "169.254.0.1", //Default
						NaplesUsername:  "root",
						NaplesPassword:  "pen123",
						NicType:         "naples",
						Name:            node.NodeName + "_naples" + "-" + strconv.Itoa(nicID),
						//Enable this with BRAD PR
						NicHint: port.MAC,
					}
					node.NaplesConfigs.Configs = append(node.NaplesConfigs.Configs, config)
					break
				}
			}

		default:
			return fmt.Errorf("unsupported node personality %v for Type: %v", personality, node.instParams.Type)
		}
	case iota.TestBedNodeType_TESTBED_NODE_TYPE_VENICE_BM:
		switch personality {
		case iota.PersonalityType_PERSONALITY_VENICE_BM:
			node.VeniceConfig = iota.VeniceConfig{
				ControlIntf: "eth1",
				ControlIp:   fmt.Sprintf("172.16.100.%d", len(tb.Nodes)+1), //FIXME
				VenicePeers: []*iota.VenicePeer{},
			}
		default:
			return fmt.Errorf("unsupported node personality %v", personality)
		}
	default:
		return fmt.Errorf("invalid testbed node type %v", nodeType)
	}

	return nil
}

//Cleanup clean up testbed
func (tb *TestBed) Cleanup() {
	if tb.cleanup && tb.iotaClient != nil {
		log.Infof("Cleaning up testbed")
		client := iota.NewTopologyApiClient(tb.iotaClient.Client)
		client.CleanUpTestBed(context.Background(), tb.tbMsg)
	}
}

func (tb *TestBed) GetDC() string {
	return tb.dcName
}

func (tb *TestBed) setupVcenterNode(node *TestNode) error {

	uid := os.Getenv("USER")
	if uid == "" {
		uid = os.Getenv("SUDO_USER")
		if uid == "" {
			if os.Getenv("JOB_ID") == "" {
				return fmt.Errorf("DC name cannot be derived, please run as non-sudo user")
			}
			tb.cleanup = true
			uid = "default-" + os.Getenv("HOSTNAME")
		}
	}

	log.Info("Setting up vcenter node...")
	switch node.Personality {
	case iota.PersonalityType_PERSONALITY_VCENTER_NODE:
		node.VcenterConfig.DcName = uid + "-iota-dc"
		node.VcenterConfig.ClusterName = uid + "-iota-cluster"
		node.VcenterConfig.DistributedSwitch = "#Pen-DVS-" + node.VcenterConfig.DcName
		tb.switchName = node.VcenterConfig.DistributedSwitch
		tb.dcName = node.VcenterConfig.DcName
		node.VcenterConfig.EsxConfigs = []*iota.VmwareESXConfig{}
		for _, mn := range node.topoNode.MangedNodes {
			for _, tbn := range tb.Nodes {
				if tbn.NodeName == mn {
					node.VcenterConfig.EsxConfigs = append(node.VcenterConfig.EsxConfigs,
						&iota.VmwareESXConfig{Name: mn, IpAddress: tbn.NodeMgmtIP,
							Username: tb.Params.Provision.Vars["EsxUsername"],
							Password: tb.Params.Provision.Vars["EsxPassword"]})
				}
			}

		}
	}
	return nil
}

func (tb *TestBed) setupVeniceIPs(node *TestNode) error {

	switch node.Personality {
	case iota.PersonalityType_PERSONALITY_NAPLES:
		fallthrough
	case iota.PersonalityType_PERSONALITY_NAPLES_DVS:
		fallthrough
	case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
		fallthrough
	case iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM:
		var veniceIps []string
		for _, vn := range tb.Nodes {
			if vn.Personality == iota.PersonalityType_PERSONALITY_VENICE ||
				vn.Personality == iota.PersonalityType_PERSONALITY_VENICE_BM {
				veniceIps = append(veniceIps, vn.NodeMgmtIP) // in HW setups, use venice mgmt ip
			}
		}
		if node.Personality == iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM {
			node.NaplesMultSimConfig.VeniceIps = veniceIps
		} else {
			for _, naplesConfig := range node.NaplesConfigs.Configs {
				naplesConfig.VeniceIps = veniceIps
			}
		}
	case iota.PersonalityType_PERSONALITY_VENICE:
		fallthrough
	case iota.PersonalityType_PERSONALITY_VENICE_BM:
		if tb.hasNaplesSim {
			for _, vn := range tb.Nodes {
				if vn.Personality == iota.PersonalityType_PERSONALITY_VENICE ||
					vn.Personality == iota.PersonalityType_PERSONALITY_VENICE_BM {
					peer := iota.VenicePeer{
						HostName:  vn.NodeName,
						IpAddress: vn.VeniceConfig.ControlIp, // in Sim setups venice-naples use control network
					}
					node.VeniceConfig.VenicePeers = append(node.VeniceConfig.VenicePeers, &peer)
				}
			}
		} else {
			for _, vn := range tb.Nodes {
				if vn.Personality == iota.PersonalityType_PERSONALITY_VENICE ||
					vn.Personality == iota.PersonalityType_PERSONALITY_VENICE_BM {
					peer := iota.VenicePeer{
						HostName:  vn.NodeName,
						IpAddress: vn.NodeMgmtIP, // HACK: in HW setups, Venice-Naples use mgmt network
					}
					node.VeniceConfig.VenicePeers = append(node.VeniceConfig.VenicePeers, &peer)
				}
			}
		}

	}
	return nil
}

func (tb *TestBed) setupNode(node *TestNode) error {
	var err error
	client := iota.NewTopologyApiClient(tb.iotaClient.Client)
	tbn := iota.TestBedNode{
		Type:                node.Type,
		IpAddress:           node.NodeMgmtIP,
		NicConsoleIpAddress: node.instParams.NicConsoleIP,
		NicConsolePort:      node.instParams.NicConsolePort,
		NicIpAddress:        node.instParams.NicMgmtIP,
		CimcIpAddress:       node.instParams.NodeCimcIP,
		ServerType:          node.instParams.NodeServer,
		CimcUsername:        common.DefaultCimcUserName,
		CimcPassword:        common.DefaultCimcPassword,
		NicUuid:             node.instParams.Resource.NICUuid,
		NodeName:            node.NodeName,
	}

	tbn.NicIpAddress, err = node.instParams.getNicMgmtIP()
	if err != nil {
		return err
	}
	node.instParams.NicMgmtIP = tbn.NicIpAddress

	// set esx user name when required
	if node.topoNode.HostOS == "vcenter" {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER
		tbn.EsxUsername = tb.Params.Provision.Vars["VcenterUsername"]
		tbn.EsxPassword = tb.Params.Provision.Vars["VcenterPassword"]
		tbn.License = tb.Params.Provision.Vars["VcenterLicense"]
	} else if node.topoNode.HostOS == "esx" {
		tbn.EsxUsername = tb.Params.Provision.Vars["EsxUsername"]
		tbn.EsxPassword = tb.Params.Provision.Vars["EsxPassword"]
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_ESX
	} else if node.topoNode.HostOS == "freebsd" {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD
	} else {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX
	}

	initNodeMsg := &iota.TestNodesMsg{
		Username:    tb.Params.Provision.Username,
		Password:    tb.Params.Provision.Password,
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.TestBedNode{&tbn},
	}

	if tb.Params.Provision.Vars["VcenterUsername"] != "" {
		initNodeMsg.Licenses = append(initNodeMsg.Licenses,
			&iota.License{
				Username: tb.Params.Provision.Vars["VcenterUsername"],
				Password: tb.Params.Provision.Vars["VcenterPassword"],
				Key:      tb.Params.Provision.Vars["VcenterLicense"],
				Type:     iota.License_LICENSE_VCENTER.String(),
			})
	}

	if err := tb.setupVcenterNode(node); err != nil {
		log.Errorf("Error in setting up vcenter  nodes: ApiResp: %+v. ", err.Error())
		return err
	}
	resp, err := client.InitNodes(context.Background(), initNodeMsg)
	if err != nil {
		log.Errorf("Error initing nodes:  Err %v", err)
		return fmt.Errorf("Error while adding nodes in testbed")
	} else if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Error initing nodes: ApiResp: %+v. ", resp.ApiResponse)
		return fmt.Errorf("Error while initing nodes in testbed")
	}

	if err := tb.setupVeniceIPs(node); err != nil {
		log.Errorf("Error in setting up venice nodes: ApiResp: %+v. ", resp.ApiResponse)
		return err
	}

	return nil
}

//DeleteNodes add node dynamically
func (tb *TestBed) DeleteNodes(nodes []*TestNode) error {

	client := iota.NewTopologyApiClient(tb.iotaClient.Client)
	cnt := 0
	for i := 0; i < len(tb.Nodes); i++ {
		n := tb.Nodes[i]
		toDelete := false
		for _, node := range nodes {
			if node.NodeName == n.NodeName {
				toDelete = true
				break
			}

		}
		if !toDelete {
			tb.Nodes[cnt] = n
			cnt++
		}
	}

	if cnt == len(tb.Nodes) {
		log.Errorf("Nodes not presnt to delete:  Err %v", nodes)
		return errors.New("Node not presnt to delete")
	}

	tb.Nodes = tb.Nodes[:cnt]

	// move naples to managed mode
	err := tb.cleanUpNaplesConfig(nodes)
	if err != nil {
		log.Errorf("clean up naples failed. Err: %v", err)
		return err
	}

	cleanNodeMsg := &iota.TestNodesMsg{
		Username:    tb.Params.Provision.Username,
		Password:    tb.Params.Provision.Password,
		ApiResponse: &iota.IotaAPIResponse{},
	}
	for _, node := range nodes {
		tb.addAvailableInstance(node.instParams)
		tbn := iota.TestBedNode{
			Type:                node.Type,
			IpAddress:           node.NodeMgmtIP,
			NicConsoleIpAddress: node.instParams.NicConsoleIP,
			NicConsolePort:      node.instParams.NicConsolePort,
			NicIpAddress:        node.instParams.NicMgmtIP,
			CimcIpAddress:       node.instParams.NodeCimcIP,
			CimcUsername:        common.DefaultCimcUserName,
			CimcPassword:        common.DefaultCimcPassword,
			NicUuid:             node.instParams.Resource.NICUuid,
			NodeName:            node.NodeName,
		}
		tbn.NicIpAddress, err = node.instParams.getNicMgmtIP()
		if err != nil {
			return err
		}
		node.instParams.NicMgmtIP = tbn.NicIpAddress
		// set esx user name when required
		if node.topoNode.HostOS == "esx" {
			tbn.EsxUsername = tb.Params.Provision.Vars["EsxUsername"]
			tbn.EsxPassword = tb.Params.Provision.Vars["EsxPassword"]
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_ESX
		} else if node.topoNode.HostOS == "freebsd" {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD
		} else {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX
		}

		cleanNodeMsg.Nodes = append(cleanNodeMsg.Nodes, &tbn)

	}

	resp, err := client.CleanNodes(context.Background(), cleanNodeMsg)
	if err != nil {
		log.Errorf("Error cleaning up nodes:  Err %v", err)
		return fmt.Errorf("Error while removing nodes in testbed")
	} else if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Error removing nodes: ApiResp: %+v. ", resp.ApiResponse)
		return fmt.Errorf("Error while removing nodes in testbed")
	}

	return nil
}

//AddNodes add node dynamically
func (tb *TestBed) AddNodes(personality iota.PersonalityType, names []string) ([]*TestNode, error) {

	for i := 0; i < len(tb.Nodes); i++ {
		n := tb.Nodes[i]
		for _, name := range names {
			if n.NodeName == name {
				msg := fmt.Sprintf("Node name %v already present", name)
				return nil, errors.New(msg)
			}
		}
	}

	// Build Topology Object after parsing warmd.json
	nodes := &iota.NodeMsg{
		NodeOp:      iota.Op_ADD,
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
		MakeCluster: true,
	}
	client := iota.NewTopologyApiClient(tb.iotaClient.Client)

	newNodes := []*TestNode{}
	var pinst *InstanceParams
	for _, name := range names {

		nodeType := iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM
		if IsNaplesHW(personality) {
			nodeType = iota.TestBedNodeType_TESTBED_NODE_TYPE_HW
			pinst = tb.getNaplesInstanceByName(name)
		} else if personality == iota.PersonalityType_PERSONALITY_VENICE ||
			personality == iota.PersonalityType_PERSONALITY_VENICE_BM {
			nodeType = iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM
			pinst = tb.getAvailableInstance(iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM)
		}

		// setup node state
		node := &TestNode{
			NodeName:    name,
			Type:        nodeType,
			Personality: personality,
			NodeMgmtIP:  pinst.NodeMgmtIP,
			instParams:  pinst,
			topoNode: &TopoNode{NodeName: name,
				Personality: personality,
				Type:        nodeType},
		}

		if IsHWNode(personality) {
			node.topoNode.HostOS = tb.Params.Provision.Vars["BmOs"]
		}
		if err := tb.preapareNodeParams(nodeType, personality, node); err != nil {
			return nil, err
		}

		if err := tb.setupNode(node); err != nil {
			return nil, err
		}

		newNodes = append(newNodes, node)

		iotaNode := tb.getIotaNode(node)

		nodes.Nodes = append(nodes.Nodes, iotaNode)
	}

	log.Infof("Adding nodes to the testbed...")
	log.Infof("Adding nodes: %+v", nodes)
	addNodeResp, err := client.AddNodes(context.Background(), nodes)
	if err != nil {
		log.Errorf("Error adding nodes:  Err %v", err)
		return nil, fmt.Errorf("Error while adding nodes to testbed")
	} else if addNodeResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Error adding nodes: ApiResp: %+v. ", addNodeResp.ApiResponse)
		return nil, fmt.Errorf("Error while adding nodes to testbed")
	}

	// save node uuid
	for index, node := range newNodes {
		if node.IsNaplesHW() {
			tb.addToNaplesCache(node.NodeName, node.instParams.ID)
			for iter, naplesConfig := range addNodeResp.Nodes[index].GetNaplesConfigs().GetConfigs() {
				node.NodeUUID = append(node.NodeUUID, naplesConfig.NodeUuid)
				node.NaplesConfigs.Configs[iter].NodeUuid = naplesConfig.NodeUuid
				node.NaplesConfigs.Configs[iter].NaplesIpAddress = naplesConfig.NaplesIpAddress
			}
		}

		node.iotaNode = addNodeResp.Nodes[index]
		tb.Nodes = append(tb.Nodes, node)
	}

	//Save the context
	tb.saveNaplesCache()

	return newNodes, nil
}

// initNodeState merges topology and testbed params to create node state
func (tb *TestBed) initNodeState() error {
	// add all instances to unallocated list
	for i := 0; i < len(tb.Params.Instances); i++ {
		pinst := &tb.Params.Instances[i]
		pinst.idx = i
		tb.unallocatedInstances = append(tb.unallocatedInstances, pinst)
	}

	naplesInst := os.Getenv("NAPLES_INSTANCES")
	bringUpNaples := math.MaxInt32
	if naplesInst != "" {
		naplesInst, err := strconv.Atoi(naplesInst)
		if err != nil || naplesInst < 0 {
			log.Debugf("Invalid number if naples Instances")
		}
		bringUpNaples = naplesInst
		log.Infof("Bringing up naples instances %v", bringUpNaples)
	} else {
		log.Infof("Bringing up all naples instances")
	}

	// setup all nodes
	count := 0
	for i := 0; i < len(tb.Nodes); i++ {
		tnode := &tb.Topo.Nodes[i]
		if IsNaplesHW(tnode.Personality) {
			if bringUpNaples <= 0 {
				continue
			}
			bringUpNaples = bringUpNaples - 1
		}

		pinst := tb.getAvailableInstance(tnode.Type)

		// setup node state
		node := TestNode{
			NodeName:    tnode.NodeName,
			Type:        tnode.Type,
			Personality: tnode.Personality,
			NodeMgmtIP:  pinst.NodeMgmtIP,
			instParams:  pinst,
			topoNode:    tnode,
		}

		if err := tb.preapareNodeParams(tnode.Type, tnode.Personality, &node); err != nil {
			return err
		}

		tb.Nodes[count] = &node
		count++
	}

	tb.Nodes = tb.Nodes[:count]
	for _, node := range tb.Nodes {
		tb.setupVeniceIPs(node)
		log.Debugf("Testbed Node: %+v", node)
	}

	// determine base vlan
	baseVlan := 2
	for _, inst := range tb.Params.Instances {
		if inst.ID != 0 {
			baseVlan += inst.ID
			break
		}
	}

	return nil
}

// Start logging naples
func (tb *TestBed) StartNaplesConsoleLogging() error {

	if os.Getenv("NO_CONSOLE_LOG") != "" {
		log.Infof("Skipping console log capturing.")
		return nil
	}
	naplesNodes := 0
	for _, node := range tb.Nodes {
		if IsNaplesHW(node.Personality) {
			naplesNodes++
		}
	}

	if naplesNodes != 0 {
		waitCh := make(chan error, naplesNodes)

		startNaplesLogging := func(name, consoleIP, consolePort string) {
			// create logs directory if it doesnt exists
			cmdStr := fmt.Sprintf("PYTHONPATH=%s/src/github.com/pensando/sw/iota  %s/src/github.com/pensando/sw/iota/scripts/lab_tools/console_clear.py --console-ip %s --console-port %s",
				os.Getenv("GOPATH"), os.Getenv("GOPATH"), consoleIP, consolePort)
			command := exec.Command("sh", "-c", cmdStr)
			err := command.Start()
			if err != nil {
				log.Errorf("Error running command %s. Err: %v", cmdStr, err)
				waitCh <- err
				return
			}
			command.Wait()
			logFile := fmt.Sprintf("%s/src/github.com/pensando/sw/iota/%s_console.log", os.Getenv("GOPATH"), name)
			cmdStr = fmt.Sprintf("telnet %s %s | tee %s ", consoleIP, consolePort, logFile)

			command = exec.Command("sh", "-c", cmdStr)
			command.StdinPipe()
			err = command.Start()
			if err != nil {
				log.Errorf("Error starting loggin %s\n", err)
				waitCh <- err
				return
			}
			waitCh <- nil
			go func() {
				command.Wait()
				log.Infof("Exiting consile log for %v:%v", consoleIP, consolePort)
			}()
		}

		for _, node := range tb.Nodes {
			if IsNaplesHW(node.Personality) {
				go startNaplesLogging(node.NodeName, node.instParams.NicConsoleIP, node.instParams.NicConsolePort)
			}
		}

		for i := 0; i < naplesNodes; i++ {
			err := <-waitCh
			if err != nil {
				return err
			}
		}
	}

	return nil
}

// connectToIotaServer connects to iota server
func (tb *TestBed) connectToIotaServer() error {
	srvURL := os.Getenv("IOTA_SERVER_URL")
	if srvURL == "" {
		srvURL = defaultIotaServerURL
	}

	// connect to iota server
	iotc, err := common.CreateNewGRPCClient("iota-server", srvURL, 0)
	if err != nil {
		return err
	}

	tb.iotaClient = iotc

	return nil
}

func (tb *TestBed) getIotaNode(node *TestNode) *iota.Node {

	tbn := iota.Node{
		Name:      node.NodeName,
		Type:      node.Personality,
		IpAddress: node.NodeMgmtIP,
	}

	// set esx user name when required
	if node.topoNode.HostOS == "vcenter" {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER
	} else if node.topoNode.HostOS == "esx" {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_ESX
		tbn.EsxConfig = &iota.VmwareESXConfig{
			Username:  tb.Params.Provision.Vars["EsxUsername"],
			Password:  tb.Params.Provision.Vars["EsxPassword"],
			IpAddress: node.NodeMgmtIP,
		}
	} else if node.topoNode.HostOS == "freebsd" {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD
	} else {
		tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX
	}

	switch node.Personality {
	case iota.PersonalityType_PERSONALITY_NAPLES_DVS:
		fallthrough
	case iota.PersonalityType_PERSONALITY_NAPLES:
		tbn.StartupScript = "/naples/nodeinit.sh"
		fallthrough
	case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
		tbn.Image = filepath.Base(tb.Topo.NaplesImage)
		tbn.NodeInfo = &iota.Node_NaplesConfigs{
			NaplesConfigs: &node.NaplesConfigs,
		}
		tbn.Entities = []*iota.Entity{
			{
				Type: iota.EntityType_ENTITY_TYPE_HOST,
				Name: node.NodeName + "_host",
			},
		}
		for _, naplesConfig := range tbn.GetNaplesConfigs().Configs {
			tbn.Entities = append(tbn.Entities, &iota.Entity{
				Type: iota.EntityType_ENTITY_TYPE_NAPLES,
				Name: naplesConfig.Name,
			})
		}
	case iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC:
		fallthrough
	case iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC_DVS:
		tbn.NodeInfo = &iota.Node_ThirdPartyNicConfig{
			ThirdPartyNicConfig: &iota.ThirdPartyNicConfig{
				NicType: node.instParams.Resource.NICType,
			},
		}
		tbn.Entities = []*iota.Entity{
			{
				Type: iota.EntityType_ENTITY_TYPE_HOST,
				Name: node.NodeName + "_host",
			},
		}
	case iota.PersonalityType_PERSONALITY_VENICE:
		tbn.Image = filepath.Base(tb.Topo.VeniceImage)
		fallthrough
	case iota.PersonalityType_PERSONALITY_VENICE_BM:
		tbn.NodeInfo = &iota.Node_VeniceConfig{
			VeniceConfig: &node.VeniceConfig,
		}
		tbn.Entities = []*iota.Entity{
			{
				Type: iota.EntityType_ENTITY_TYPE_HOST,
				Name: node.NodeName + "_venice",
			},
		}
	}

	return &tbn
}

func (tb *TestBed) initNaplesCache() {
	tb.naplesDataMap = make(map[string]naplesData)
	if jdata, err := ioutil.ReadFile(naplesTBFile); err == nil {
		if err := json.Unmarshal(jdata, &tb.naplesDataMap); err != nil {
			log.Fatal("Failed to read naples TB file")
		}
	}
}

func (tb *TestBed) resetNaplesCache() {
	tb.naplesDataMap = make(map[string]naplesData)
	os.Remove(naplesTBFile)
}

func (tb *TestBed) addToNaplesCache(name string, ID int) {
	tb.naplesDataMap[name] = naplesData{Naples: name, ID: ID}
}

func (tb *TestBed) saveNaplesCache() error {
	//Remember which naples was used for which ID
	if jdata, err := json.Marshal(tb.naplesDataMap); err != nil {
		return fmt.Errorf("Error in marshaling naples map")
	} else if err := ioutil.WriteFile(naplesTBFile, jdata, 0644); err != nil {
		return fmt.Errorf("unable to write naples data file: %s", err)
	}

	return nil
}

// setupTestBed sets up testbed
func (tb *TestBed) setupTestBed() error {

	var err error
	tb.initNaplesCache()
	client := iota.NewTopologyApiClient(tb.iotaClient.Client)

	// Allocate VLANs
	testBedMsg := &iota.TestBedMsg{
		NaplesImage:    tb.Topo.NaplesImage,
		VeniceImage:    tb.Topo.VeniceImage,
		NaplesSimImage: tb.Topo.NaplesSimImage,
		Username:       tb.Params.Provision.Username,
		Password:       tb.Params.Provision.Password,
		ApiResponse:    &iota.IotaAPIResponse{},
		Nodes:          []*iota.TestBedNode{},
		DataSwitches:   []*iota.DataSwitch{},
		NativeVlan:     uint32(tb.Params.Network.VlanID),
	}

	tb.tbMsg = testBedMsg

	if tb.Params.Provision.Vars["VcenterUsername"] != "" {
		testBedMsg.Licenses = append(testBedMsg.Licenses,
			&iota.License{
				Username: tb.Params.Provision.Vars["VcenterUsername"],
				Password: tb.Params.Provision.Vars["VcenterPassword"],
				Key:      tb.Params.Provision.Vars["VcenterLicense"],
				Type:     iota.License_LICENSE_VCENTER.String(),
			})
	}

	dsmap := make(map[string]*iota.DataSwitch)
	for _, node := range tb.Nodes {
		tbn := iota.TestBedNode{
			Type:                node.Type,
			IpAddress:           node.NodeMgmtIP,
			NicConsoleIpAddress: node.instParams.NicConsoleIP,
			NicConsolePort:      node.instParams.NicConsolePort,
			NicIpAddress:        node.instParams.NicMgmtIP,
			CimcIpAddress:       node.instParams.NodeCimcIP,
			ServerType:          node.instParams.NodeServer,
			CimcUsername:        common.DefaultCimcUserName,
			CimcPassword:        common.DefaultCimcPassword,
			NicUuid:             node.instParams.Resource.NICUuid,
			NodeName:            node.NodeName,
		}
		tbn.NicIpAddress, err = node.instParams.getNicMgmtIP()
		if err != nil {
			return err
		}
		node.instParams.NicMgmtIP = tbn.NicIpAddress

		//For now hack up until we get the vendor information
		if node.instParams.Resource.ServerType == "hpe" {
			tbn.ServerType = "hpe"
		}
		// set esx user name when required
		if node.topoNode.HostOS == "vcenter" {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER
			tbn.EsxUsername = tb.Params.Provision.Vars["VcenterUsername"]
			tbn.EsxPassword = tb.Params.Provision.Vars["VcenterPassword"]
			tbn.License = tb.Params.Provision.Vars["VcenterLicense"]
			err := tb.setupVcenterNode(node)
			if err != nil {
				return err
			}
			tbn.DcName = node.VcenterConfig.DcName
			tbn.Switch = node.VcenterConfig.DistributedSwitch
		} else if node.topoNode.HostOS == "esx" {
			tbn.EsxUsername = tb.Params.Provision.Vars["EsxUsername"]
			tbn.EsxPassword = tb.Params.Provision.Vars["EsxPassword"]
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_ESX
		} else if node.topoNode.HostOS == "freebsd" {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD
		} else {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX
		}

		testBedMsg.Nodes = append(testBedMsg.Nodes, &tbn)

		// set testbed id if available
		if IsNaplesHW(node.Personality) && node.instParams.ID != 0 && testBedMsg.TestbedId == 0 {
			testBedMsg.TestbedId = uint32(node.instParams.ID)
		}

		// add switch port
		for _, dn := range node.instParams.DataNetworks {
			// create switch if it doesnt exist
			ds, ok := dsmap[dn.SwitchIP]
			if !ok {
				ds = &iota.DataSwitch{}
				ds.Ip = dn.SwitchIP
				ds.Username = dn.SwitchUsername
				ds.Password = dn.SwitchPassword
				ds.Speed = iota.DataSwitch_Speed_auto
				dsmap[dn.SwitchIP] = ds
			}

			ds.Ports = append(ds.Ports, dn.Name)
		}
	}
	for _, ds := range dsmap {
		testBedMsg.DataSwitches = append(testBedMsg.DataSwitches, ds)
		tb.DataSwitches = append(tb.DataSwitches, ds)
	}

	skipInstall := os.Getenv("SKIP_INSTALL") != ""
	if !tb.skipSetup {
		// first cleanup testbed as we have to clean up all vcenter related stuff
		client.CleanUpTestBed(context.Background(), testBedMsg)

		// install image if required
		if !skipInstall && tb.hasNaplesHW {
			//we are reinstalling, reset current mapping
			tb.resetNaplesCache()
			log.Infof("Installing images on testbed. This may take 10s of minutes...")
			ctx, cancel := context.WithTimeout(context.Background(), time.Duration(time.Hour))
			defer cancel()
			instResp, err := client.InstallImage(ctx, testBedMsg)
			if err != nil {
				nerr := fmt.Errorf("Error during installing image: %v", err)
				log.Errorf("%v", nerr)
				return nerr
			}
			if instResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
				log.Errorf("Error during InitTestBed(). ApiResponse: %+v Err: %v", instResp.ApiResponse, err)
				return fmt.Errorf("Error during install image: %v", instResp.ApiResponse)
			}
			//Image installted, no need to resinstall on retry
			os.Setenv("SKIP_INSTALL", "1")
		}

		// then, init testbed
		log.Debugf("Initializing testbed with params: %+v", testBedMsg)
		testBedMsg.RebootNodes = (os.Getenv("REBOOT_ONLY") != "")
		if testBedMsg.RebootNodes {
			log.Infof("Cleaning testbed with reboot...")
		} else {
			log.Infof("Cleaning testbed without reboot...")
		}

		// first cleanup testbed
		client.CleanUpTestBed(context.Background(), testBedMsg)

		testBedMsg.RebootNodes = false
		resp, err := client.InitTestBed(context.Background(), testBedMsg)
		if err != nil {
			log.Errorf("Error during InitTestBed(). Err: %v", err)
			return fmt.Errorf("Error during init testbed")
		}
		if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Error during InitTestBed(). ApiResponse: %+v Err: %v", resp.ApiResponse, err)
			return fmt.Errorf("Error during init testbed")
		}

		log.Debugf("Got InitTestBed resp: %+v", resp)
		tb.initTestbedResp = resp
	} else {
		resp, err := client.GetTestBed(context.Background(), testBedMsg)
		if err != nil {
			log.Errorf("Error during GetTestBed(). Err: %v", err)
			return fmt.Errorf("Error during init testbed")
		}
		if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Error during GetTestBed(). ApiResponse: %+v Err: %v", resp.ApiResponse, err)
			return fmt.Errorf("Error during init testbed")
		}

		log.Debugf("Got GetTestBed resp: %+v", resp)
		tb.initTestbedResp = resp
	}

	//Copy the vlans from the testbed
	for _, vlan := range tb.initTestbedResp.AllocatedVlans {
		tb.allocatedVlans = append(tb.allocatedVlans, vlan)
	}
	//Ignore the first one as it is native vlan
	if len(tb.allocatedVlans) > 1 {
		tb.allocatedVlans = tb.allocatedVlans[1:]
	}

	// Build Topology Object after parsing warmd.json
	nodes := &iota.NodeMsg{
		NodeOp:      iota.Op_ADD,
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
		MakeCluster: true,
	}

	for i := 0; i < len(tb.Nodes); i++ {
		node := tb.Nodes[i]
		tbn := iota.Node{
			Name:      node.NodeName,
			Type:      node.Personality,
			IpAddress: node.NodeMgmtIP,
		}

		// set esx user name when required
		if node.topoNode.HostOS == "esx" {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_ESX
			tbn.EsxConfig = &iota.VmwareESXConfig{
				Username:  tb.Params.Provision.Vars["EsxUsername"],
				Password:  tb.Params.Provision.Vars["EsxPassword"],
				IpAddress: node.NodeMgmtIP,
			}
		} else if node.topoNode.HostOS == "freebsd" {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD
		} else {
			tbn.Os = iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX
		}

		switch node.Personality {
		case iota.PersonalityType_PERSONALITY_NAPLES_DVS:
			fallthrough
		case iota.PersonalityType_PERSONALITY_NAPLES:
			tbn.StartupScript = "/naples/nodeinit.sh"
			fallthrough
		case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
			tbn.Image = filepath.Base(tb.Topo.NaplesImage)
			tbn.NodeInfo = &iota.Node_NaplesConfigs{
				NaplesConfigs: &node.NaplesConfigs,
			}
			tbn.Entities = []*iota.Entity{
				{
					Type: iota.EntityType_ENTITY_TYPE_HOST,
					Name: node.NodeName + "_host",
				},
			}
			for _, naplesConfig := range tbn.GetNaplesConfigs().Configs {
				tbn.Entities = append(tbn.Entities, &iota.Entity{
					Type: iota.EntityType_ENTITY_TYPE_NAPLES,
					Name: naplesConfig.Name,
				})
			}
		case iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC:
			fallthrough
		case iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC_DVS:
			tbn.NodeInfo = &iota.Node_ThirdPartyNicConfig{
				ThirdPartyNicConfig: &iota.ThirdPartyNicConfig{
					NicType: node.instParams.Resource.NICType,
				},
			}

			tbn.Entities = []*iota.Entity{
				{
					Type: iota.EntityType_ENTITY_TYPE_HOST,
					Name: node.NodeName + "_host",
				},
			}

		case iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM:
			tbn.Image = filepath.Base(tb.Topo.NaplesSimImage)
			tbn.NodeInfo = &iota.Node_NaplesMultiSimConfig{
				NaplesMultiSimConfig: &node.NaplesMultSimConfig,
			}
			tbn.Entities = []*iota.Entity{
				{
					Type: iota.EntityType_ENTITY_TYPE_HOST,
					Name: node.NodeName + "_host",
				},
			}
		case iota.PersonalityType_PERSONALITY_VCENTER_NODE:
			if err := tb.setupVcenterNode(node); err != nil {
				log.Errorf("Error in setting up vcenter nodes %v", err.Error())
				return err
			}
			tbn.Image = filepath.Base(tb.Topo.VeniceImage)
			tbn.NodeInfo = &iota.Node_VcenterConfig{
				VcenterConfig: &node.VcenterConfig}

		case iota.PersonalityType_PERSONALITY_VENICE:
			tbn.Image = filepath.Base(tb.Topo.VeniceImage)
			tbn.NodeInfo = &iota.Node_VeniceConfig{
				VeniceConfig: &node.VeniceConfig,
			}
			tbn.Entities = []*iota.Entity{
				{
					Type: iota.EntityType_ENTITY_TYPE_HOST,
					Name: node.NodeName + "_venice",
				},
			}
		case iota.PersonalityType_PERSONALITY_VENICE_BM:
			tbn.NodeInfo = &iota.Node_VeniceConfig{
				VeniceConfig: &node.VeniceConfig,
			}
			tbn.Entities = []*iota.Entity{
				{
					Type: iota.EntityType_ENTITY_TYPE_HOST,
					Name: node.NodeName + "_venice",
				},
			}
		}

		nodes.Nodes = append(nodes.Nodes, &tbn)
	}

	if !tb.skipSetup {
		// add all nodes
		log.Infof("Adding nodes to the testbed...")
		log.Debugf("Adding nodes: %+v", nodes)
		addNodeResp, err := client.AddNodes(context.Background(), nodes)
		if err != nil {
			log.Errorf("Error adding nodes:  Err %v", err)
			return fmt.Errorf("Error while adding nodes to testbed")
		} else if addNodeResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Error adding nodes: ApiResp: %+v. ", addNodeResp.ApiResponse)
			return fmt.Errorf("Error while adding nodes to testbed")
		}

		log.Debugf("Got add node resp: %+v", addNodeResp)
		tb.addNodeResp = addNodeResp

	} else {
		log.Infof("Skipping setup of the testbed...")
		getNodeResp, err := client.GetNodes(context.Background(), nodes)
		if err != nil {
			log.Errorf("Error getting nodes: ApiResp: %+v. Err %v", getNodeResp.ApiResponse, err)
			return fmt.Errorf("Error while getting nodes from testbed")
		}

		log.Debugf("Got node resp: %+v", getNodeResp)
		tb.addNodeResp = getNodeResp
	}

	// save node uuid
	addedNodes := []*TestNode{}

	for i := 0; i < len(tb.Nodes); i++ {
		nodeAdded := false
		node := tb.Nodes[i]
		for _, nr := range tb.addNodeResp.Nodes {
			log.Debugf("Checking %v %v", node.NodeName, nr.Name)
			if node.NodeName == nr.Name {
				log.Debugf("Adding node %v as used", node.topoNode.NodeName)
				node.iotaNode = nr
				nodeAdded = true
				addedNodes = append(addedNodes, node)
				//Update Instance to be used.
				if IsNaplesHW(node.Personality) {
					tb.addToNaplesCache(node.NodeName, node.instParams.ID)
					for iter, naplesConfig := range nr.GetNaplesConfigs().GetConfigs() {
						node.NodeUUID = append(node.NodeUUID, naplesConfig.NodeUuid)
						node.NaplesConfigs.Configs[iter].NodeUuid = naplesConfig.NodeUuid
						node.NaplesConfigs.Configs[iter].NaplesIpAddress = naplesConfig.NaplesIpAddress
					}
				}
			}
		}
		//Probably node not added, release the instance
		if !nodeAdded {
			tb.addAvailableInstance(node.instParams)
		}
	}

	if err := tb.saveNaplesCache(); err != nil {
		log.Errorf("Failed to save to naples cache Err: %v", err)
		return err
	}

	tb.Nodes = addedNodes

	//Start naples console logging for debugging
	return tb.StartNaplesConsoleLogging()
}

// cleanUpNaplesConfig cleans up naples config
func (tb *TestBed) cleanUpNaplesConfig(nodes []*TestNode) error {
	log.Infof("Removing Naples in self managed mode")

	// set date, untar penctl and trigger mode switch
	cmds := 0
	trig := tb.NewTrigger()
	for _, node := range nodes {
		if IsNaplesHW(node.Personality) {
			for _, naples := range node.NaplesConfigs.Configs {
				trig.AddCommand(fmt.Sprintf("rm -rf /sysconfig/config0/*.db"), naples.Name, node.NodeName)
				trig.AddCommand(fmt.Sprintf("rm -rf %s", globals.NaplesTrustRootsFile), naples.Name, node.NodeName)
				cmds++

			}
			// cleaning up db is good for now.
		}
	}

	//No naples, return
	if cmds == 0 {
		return nil
	}
	resp, err := trig.Run()
	if err != nil {
		return fmt.Errorf("Error running command on naples Err: %v", err)
	}
	log.Debugf("Got trigger resp: %+v", resp)

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Cleaning naples mode failed. %+v", cmdResp)
			return fmt.Errorf("Cleaning naples mode failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}
	for _, node := range nodes {
		if IsNaplesHW(node.Personality) {
			nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: node.iotaNode.Name})
			hostNames += node.iotaNode.Name + ", "

		}
	}
	log.Infof("Reloading Naples after unset: %v", hostNames)

	reloadMsg := &iota.ReloadMsg{
		NodeMsg:     nodeMsg,
		SkipRestore: true,
	}
	// Trigger App
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload Naples %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload Naples %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

/*
// reassociateNaplestoVenice reassociated by removing cert
func (sm *SysModel) reassociateNaplestoVenice(nodes []*TestNode) error {

	// copy penctl to host if required
	cmds := 0
	trig := sm.tb.NewTrigger()
	for _, node := range nodes {
		if IsNaplesHW(node.Personality) {
			err := sm.tb.CopyToHost(node.NodeName, []string{penctlPkgName}, "")
			if err != nil {
				return fmt.Errorf("Error copying penctl package to host. Err: %v", err)
			}
			cmd := fmt.Sprintf("tar -xvf %s", filepath.Base(penctlPkgName))
			trig.AddCommand(cmd, node.NodeName+"_host", node.NodeName)
			for _, naples := range node.NaplesConfigs.Configs {
				trig.AddCommand(fmt.Sprintf("rm -rf %s", globals.NaplesTrustRootsFile), naples.Name, node.NodeName)
			}
			cmds++
		}
	}
	//No naples, return
	if cmds == 0 {
		return nil
	}
	resp, err := trig.Run()
	if err != nil {
		return fmt.Errorf("Error running command on naples Err: %v", err)
	}
	log.Debugf("Got trigger resp: %+v", resp)

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Cleaning naples mode failed. %+v", cmdResp)
			return fmt.Errorf("Cleaning naples mode failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}
	for _, node := range nodes {
		if IsNaplesHW(node.Personality) {
			nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: node.iotaNode.Name})
			hostNames += node.iotaNode.Name + ", "

		}
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload Naples %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload Naples %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	return nil
}

// SetupConfig sets up the venice cluster and basic config (like auth etc)
func (sm *SysModel) SetupConfig(ctx context.Context) error {

	if sm.tb.skipSetup {
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
		err := sm.doModeSwitchOfNaples(sm.tb.Nodes)
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
	return sm.joinNaplesToVenice(sm.tb.Nodes)

}

*/

// AfterTestCommon common handling after each test
func (tb *TestBed) AfterTestCommon() error {
	testInfo := ginkgo.CurrentGinkgoTestDescription()
	if testInfo.Failed && os.Getenv("STOP_ON_ERROR") != "" {
		fmt.Printf("\n------------------ Test Failed exiting--------------------\n")
		os.Exit(1)
	} else if testInfo.Failed {
		tb.AddTaskResult("", fmt.Errorf("test failed"))
	} else {
		tb.AddTaskResult("", nil)
	}

	// remember the result
	tb.testResult[strings.Join(testInfo.ComponentTexts, "\t")] = testInfo.Failed
	return nil
}

// AddTaskResult adds a sub task result to summary
func (tb *TestBed) AddTaskResult(taskName string, err error) {
	testInfo := ginkgo.CurrentGinkgoTestDescription()
	if err != nil && os.Getenv("STOP_ON_ERROR") != "" {
		fmt.Printf("\n ------ %v |%v ------\n", strings.Join(testInfo.ComponentTexts, "|"), taskName)
		fmt.Printf("\n------------------ Test Failed exiting--------------------\n")
		os.Exit(1)
	}

	// remember the result
	testName := strings.Join(testInfo.ComponentTexts, "\t")
	tb.taskResult[fmt.Sprintf("%s\t%s", testName, taskName)] = err
	if tb.caseResult[testName] == nil {
		tb.caseResult[testName] = &TestCaseResult{}
	}
	if err == nil {
		tb.caseResult[testName].passCount++
	} else {
		tb.caseResult[testName].failCount++
	}

	tb.caseResult[testName].duration += testInfo.Duration
}

// PrintResult prints test result summary
func (tb *TestBed) PrintResult() {
	fmt.Printf("==================================================================\n")
	fmt.Printf("                Cases \n")
	fmt.Printf("==================================================================\n")

	w := tabwriter.NewWriter(os.Stdout, 0, 0, 4, ' ', tabwriter.AlignRight|tabwriter.Debug)

	tcr := []string{}
	for k := range tb.caseResult {
		tcr = append(tcr, k)
	}
	sort.Strings(tcr)

	fmt.Fprintf(w, "Id\t Test Bundle\t Group\t Case\t Pass\t Fail\t Time\n")
	for i, key := range tcr {
		res := tb.caseResult[key]
		fmt.Fprintf(w, "%d\t %s\t    %d\t    %d\t    %v\n", i+1, key, res.passCount, res.failCount, res.duration)
	}
	w.Flush()

	// print test results
	fmt.Printf("\n\n\n")
	fmt.Printf("==================================================================\n")
	fmt.Printf("                Test Results\n")
	fmt.Printf("==================================================================\n")

	w = tabwriter.NewWriter(os.Stdout, 0, 0, 4, ' ', tabwriter.AlignRight|tabwriter.Debug)

	tcr = []string{}
	for k := range tb.testResult {
		tcr = append(tcr, k)
	}
	sort.Strings(tcr)

	for i, key := range tcr {
		failed := tb.testResult[key]

		if !failed {
			fmt.Fprintf(w, "%d\t %s\t         PASS\n", i+1, key)
		} else {
			fmt.Fprintf(w, "%d\t %s\t         FAIL\n", i+1, key)
		}
	}
	w.Flush()
	fmt.Print("==================================================================\n")
}

/*
// Cleanup cleans up the testbed
func (sm *SysModel) Cleanup() error {
	// collect all log files
	sm.CollectLogs()
	return nil
}

func NewSysModel(model ModelType, tb *TestBed) (SysModelInterface, error) {

	switch model {
	case DefaultModel:
		return NewDefaultSysModel(tb, enterprise.GsCfgType)
	case VcenterModel:
		return NewVcenterSysModel(tb, enterprise.VcenterCfgType)
	}

	return nil, errors.New("Model not defined")
}

// InitSuite initializes test suite
func InitSuite(topoName, paramsFile string, scale, scaleData bool) (*TestBed, SysModelInterface, error) {
	// create testbed

	// setup test params
	if scale {
		gomega.SetDefaultEventuallyTimeout(time.Minute * 30)
		gomega.SetDefaultEventuallyPollingInterval(time.Second * 30)
	} else {
		gomega.SetDefaultEventuallyTimeout(time.Minute * 6)
		gomega.SetDefaultEventuallyPollingInterval(time.Second * 10)
	}

	tb, err := NewTestBed(topoName, paramsFile)
	if err != nil {
		return nil, nil, err
	}

	// create sysmodel
	model, err := NewSysModel(tb.Topo.Model, tb)
	if err != nil {
		tb.CollectLogs()
		return nil, nil, err
	}

	// setup default config for the sysmodel
	ctx, cancel := context.WithTimeout(context.TODO(), 60*time.Minute)
	defer cancel()
	err = model.SetupDefaultConfig(ctx, scale, scaleData)
	if err != nil {
		model.CollectLogs()
		return nil, nil, err
	}

	return tb, model, nil
}
*/

// CollectLogs collects all logs files from the testbed
func (tb *TestBed) CollectLogs() error {

	// create logs directory if it doesnt exists
	cmdStr := fmt.Sprintf("mkdir -p %s/src/github.com/pensando/sw/iota/logs", os.Getenv("GOPATH"))
	cmd := exec.Command("bash", "-c", cmdStr)
	out, err := cmd.CombinedOutput()
	if err != nil {
		log.Errorf("creating log directory failed with: %s\n", err)
	}

	if tb.mockMode {
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

	// create a tar.gz from all log files
	cmdStr = fmt.Sprintf("pushd %s/src/github.com/pensando/sw/iota/logs && tar cvzf venice-iota.tgz ../*.log && popd", os.Getenv("GOPATH"))
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

// CheckIotaClusterHealth checks iota cluster health
func (tb *TestBed) CheckIotaClusterHealth() error {
	// check cluster health
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	tb.addNodeResp.ClusterDone = tb.makeClustrResp != nil
	healthResp, err := topoClient.CheckClusterHealth(context.Background(), tb.addNodeResp)
	if err != nil {
		log.Errorf("Failed to check health of the cluster. Err: %v", err)
		return fmt.Errorf("Cluster health check failed. %v", err)
	} else if healthResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Failed to check health of the cluster {%+v}. Err: %v", healthResp.ApiResponse, err)
		return fmt.Errorf("Cluster health check failed %v", healthResp.ApiResponse.ApiStatus)
	}

	for _, h := range healthResp.Health {
		if h.HealthCode != iota.NodeHealth_HEALTH_OK {
			log.Errorf("Testbed unhealthy. HealthCode: %v | Node: %v", h.HealthCode, h.NodeName)
			return fmt.Errorf("Cluster health check failed")
		}
	}

	log.Debugf("Got cluster health resp: %+v", healthResp)

	return nil
}
