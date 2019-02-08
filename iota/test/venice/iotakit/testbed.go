// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/onsi/gomega"
	"github.com/pensando/sw/api/generated/apiclient"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

const defaultIotaServerURL = "localhost:60000"
const maxTestbedIntRetry = 3

// DataNetworkParams contains switch port info for each port
type DataNetworkParams struct {
	Name           string // switch port's name
	SwitchPort     int    // port number on the switch
	SwitchIP       string // IP address of the switch
	SwitchUsername string // switch user name
	SwitchPassword string // switch password
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
	Resource       struct {
		NICType    string // NIC type (naples, intel, mellanox etc)
		NICUuid    string // NIC's mac address
		ServerType string // baremetal server type (server-a or server-d)
	}
	DataNetworks []DataNetworkParams // data networks
}

// TestBedParams to be parsed from warmd.json file
type TestBedParams struct {
	ID        string // testbed identifier
	Provision struct {
		Username string            // user name for SSH login
		Password string            // password for SSH login
		Vars     map[string]string // custom variables passed from .job.yml
	}
	Instances []InstanceParams // nodes in the testbed
}

// TestNode contains state of a node in the testbed
type TestNode struct {
	NodeName     string               // node name specific to this topology
	NodeUUID     string               // node UUID
	Type         iota.TestBedNodeType // node type
	Personality  iota.PersonalityType // node topology
	NodeMgmtIP   string
	VeniceConfig iota.VeniceConfig // venice specific configuration
	NaplesConfig iota.NaplesConfig // naples specific config
	iotaNode     *iota.Node        // node info we got from iota
	instParams   *InstanceParams   // instance params we got from warmd.json
	topoNode     *TopoNode         // node info from topology
}

// TestBed is the state of the testbed
type TestBed struct {
	Topo                 Topology             // testbed topology
	Params               TestBedParams        // testbed params - provided by warmd
	Nodes                []*TestNode          // nodes in the testbed
	mockMode             bool                 // mock iota server and venice node for testing purposes
	mockIota             *mockIotaServer      // mock iota server
	skipSetup            bool                 // skip setting up the cluster
	hasNaplesSim         bool                 // has Naples sim nodes in the topology
	allocatedVlans       []uint32             // VLANs allocated for this testbed
	veniceLoggedinCtx    context.Context      // venice logged in context
	veniceRestClient     []apiclient.Services // Venice REST API client
	unallocatedInstances []*InstanceParams    // currently unallocated instances

	// cached message responses from iota server
	iotaClient      *common.GRPCClient   // iota grpc client
	initTestbedResp *iota.TestBedMsg     // init testbed resp from iota server
	addNodeResp     *iota.NodeMsg        // add node resp from iota server
	makeClustrResp  *iota.MakeClusterMsg // resp to make cluster message
	authCfgResp     *iota.AuthMsg        // auth response
}

// NewTestBed initializes a new testbed and returns a testbed handler
func NewTestBed(topoName string, paramsFile string) (*TestBed, error) {
	var params TestBedParams

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

	// setup test params
	gomega.SetDefaultEventuallyTimeout(time.Minute * 3)
	gomega.SetDefaultEventuallyPollingInterval(time.Second * 10)

	// create a testbed instance
	tb := TestBed{
		Topo:   *topo,
		Params: params,
		Nodes:  make([]*TestNode, len(topo.Nodes)),
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

	// skip setup when we are re-running the tests
	skipSetup := os.Getenv("SKIP_SETUP")
	if skipSetup != "" {
		log.Infof("Skipping setup...")
		tb.skipSetup = true
	}

	// recover tetsbed if required
	if !tb.mockMode && !tb.skipSetup {
		skipRecovery := os.Getenv("SKIP_RECOVERY")
		if skipRecovery == "" {
			err = tb.recoverTestbed()
			if err != nil {
				log.Errorf("Error recovering testbed. Err: %v", err)
				return nil, err
			}
		}
	}

	// connect to iota server
	err = tb.connectToIotaServer()
	if err != nil {
		return nil, err
	}

	// init testbed and add nodes
	for i := 0; i < maxTestbedIntRetry; i++ {
		log.Infof("Trying to initialize testbed..")
		err = tb.setupTestBed()
		if err == nil {
			break
		}
		log.Warnf("Error while setting up testbed. Retrying...")
	}
	if err != nil {
		return nil, err
	}

	// check iota cluster health
	err = tb.CheckIotaClusterHealth()
	if err != nil {
		return nil, err
	}

	// return testbed instance
	return &tb, nil
}

// IsSimTestbed returns true if testbed is a Naples sim testbed
func (tb *TestBed) IsSimTestbed() bool {
	return tb.hasNaplesSim
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
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_HW:
			if inst.Type == "bm" {
				tb.unallocatedInstances = append(tb.unallocatedInstances[:idx], tb.unallocatedInstances[idx+1:]...)
				return inst
			}
		}
	}

	log.Fatalf("Could not find any instances of type: %v", instType)
	return nil
}

// initNodeState merges topology and testbed params to create node state
func (tb *TestBed) initNodeState() error {
	// add all instances to unallocated list
	for i := 0; i < len(tb.Params.Instances); i++ {
		pinst := &tb.Params.Instances[i]
		pinst.idx = i
		tb.unallocatedInstances = append(tb.unallocatedInstances, pinst)
	}

	// setup all nodes
	for i := 0; i < len(tb.Nodes); i++ {
		tnode := &tb.Topo.Nodes[i]
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

		// check if testbed node can take this personality
		switch tnode.Type {
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM:
			switch tnode.Personality {
			case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
				if pinst.Type != "vm" {
					log.Errorf("Incompatible testbed node %v for personality %v/%v", pinst.Type, tnode.Type, tnode.Personality)
					return fmt.Errorf("Incompatible testbed node")
				}

				tb.hasNaplesSim = true
				node.NaplesConfig = iota.NaplesConfig{
					ControlIntf:     "eth1",
					ControlIp:       fmt.Sprintf("172.16.100.%d", i+1),
					DataIntfs:       []string{"eth2", "eth3"},
					NaplesIpAddress: "169.254.0.1",
					NaplesUsername:  "root",
					NaplesPassword:  "pen123",
					NicType:         "pensando-sim",
				}
			case iota.PersonalityType_PERSONALITY_VENICE:
				if pinst.Type != "vm" {
					log.Errorf("Incompatible testbed node %v for personality %v/%v", pinst.Type, tnode.Type, tnode.Personality)
					return fmt.Errorf("Incompatible testbed node")
				}

				node.VeniceConfig = iota.VeniceConfig{
					ControlIntf: "eth1",
					ControlIp:   fmt.Sprintf("172.16.100.%d", i+1),
					VenicePeers: []*iota.VenicePeer{},
				}
			default:
				return fmt.Errorf("unsupported node personality %v", tnode.Personality)
			}
		case iota.TestBedNodeType_TESTBED_NODE_TYPE_HW:
			switch tnode.Personality {
			case iota.PersonalityType_PERSONALITY_NAPLES:
				if pinst.Type != "bm" {
					log.Errorf("Incompatible testbed node %v for personality %v/%v", pinst.Type, tnode.Type, tnode.Personality)
					return fmt.Errorf("Incompatible testbed node")
				}

				// we need Esx credentials if this is running esx
				if tnode.HostOS == "esx" {
					_, uok := tb.Params.Provision.Vars["EsxUsername"]
					_, pok := tb.Params.Provision.Vars["EsxPassword"]
					if !uok || !pok {
						log.Errorf("Esx login credentials are not provided in testbed params. %+v", tb.Params.Provision)
						return fmt.Errorf("Esx login credentials are not provided")
					}
				}

				node.NaplesConfig = iota.NaplesConfig{
					ControlIntf:     "eth1",
					ControlIp:       fmt.Sprintf("172.16.100.%d", i+1),
					DataIntfs:       []string{"eth2", "eth3"},
					NaplesIpAddress: "169.254.0.1",
					NaplesUsername:  "root",
					NaplesPassword:  "pen123",
					NicType:         "naples",
				}
			default:
				return fmt.Errorf("unsupported node personality %v for Type: %v", tnode.Personality, pinst.Type)
			}
		default:
			return fmt.Errorf("invalid testbed node type %v", tnode.Type)
		}

		tb.Nodes[i] = &node
	}

	// setup venice ips
	for _, node := range tb.Nodes {
		switch node.Personality {
		case iota.PersonalityType_PERSONALITY_NAPLES:
			fallthrough
		case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
			var veniceIps []string
			for _, vn := range tb.Nodes {
				if vn.Personality == iota.PersonalityType_PERSONALITY_VENICE {
					veniceIps = append(veniceIps, vn.VeniceConfig.ControlIp)
				}
			}
			node.NaplesConfig.VeniceIps = veniceIps
		case iota.PersonalityType_PERSONALITY_VENICE:
			if tb.hasNaplesSim {
				for _, vn := range tb.Nodes {
					if vn.Personality == iota.PersonalityType_PERSONALITY_VENICE {
						peer := iota.VenicePeer{
							HostName:  vn.NodeName,
							IpAddress: vn.VeniceConfig.ControlIp, // in Sim setups venice-naples use control network
						}
						node.VeniceConfig.VenicePeers = append(node.VeniceConfig.VenicePeers, &peer)
					}
				}
			} else {
				for _, vn := range tb.Nodes {
					if vn.Personality == iota.PersonalityType_PERSONALITY_VENICE {
						peer := iota.VenicePeer{
							HostName:  vn.NodeName,
							IpAddress: vn.NodeMgmtIP, // HACK: in HW setups, Venice-Naples use mgmt network
						}
						node.VeniceConfig.VenicePeers = append(node.VeniceConfig.VenicePeers, &peer)
					}
				}
			}
		}

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

	// allocate some Vlans
	for i := 0; i < tb.Topo.NumVlans; i++ {
		tb.allocatedVlans = append(tb.allocatedVlans, uint32(baseVlan+i))
	}

	return nil
}

// recoverTestbed recovers testbed even if its in a bad state
// FIXME: this basically runs a python script to install golden fw on naples and recover it.
//        There is got to be a better way of doing this!!
func (tb *TestBed) recoverTestbed() error {
	gopath := os.Getenv("GOPATH")
	if gopath == "" {
		log.Errorf("GOPATH not defined in the environment")
		return fmt.Errorf("GOPATH not defined in the environment")
	}
	wsdir := gopath + "/src/github.com/pensando/sw"

	pool, _ := errgroup.WithContext(context.Background())

	for _, node := range tb.Nodes {
		if node.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
			cmd := fmt.Sprintf("%s/iota/scripts/boot_naples_v2.py", wsdir)
			cmd += fmt.Sprintf(" --console-ip %s", node.instParams.NicConsoleIP)
			cmd += fmt.Sprintf(" --console-port %s", node.instParams.NicConsolePort)
			cmd += fmt.Sprintf(" --mnic-ip 169.254.0.1")
			cmd += fmt.Sprintf(" --host-ip %s", node.instParams.NodeMgmtIP)
			cmd += fmt.Sprintf(" --cimc-ip %s", node.instParams.NodeCimcIP)
			cmd += fmt.Sprintf(" --image %s/nic/naples_fw.tar", wsdir)
			cmd += fmt.Sprintf(" --mode hostpin")
			cmd += fmt.Sprintf(" --drivers-pkg %s/platform/gen/drivers-%s-eth.tar.xz", wsdir, node.topoNode.HostOS)
			cmd += fmt.Sprintf(" --gold-firmware-image %s/platform/goldfw/naples/naples_fw.tar", wsdir)
			cmd += fmt.Sprintf(" --uuid %s", node.instParams.Resource.NICUuid)
			cmd += fmt.Sprintf(" --os %s", node.topoNode.HostOS)

			latestGoldDriver := fmt.Sprintf("%s/platform/hosttools/x86_64/%s/goldfw/latest/drivers-%s-eth.tar.xz", wsdir, node.topoNode.HostOS, node.topoNode.HostOS)
			oldGoldDriver := fmt.Sprintf("%s/platform/hosttools/x86_64/%s/goldfw/old/drivers-%s-eth.tar.xz", wsdir, node.topoNode.HostOS, node.topoNode.HostOS)
			realPath, _ := filepath.EvalSymlinks(latestGoldDriver)
			latestGoldDriverVer := filepath.Base(filepath.Dir(realPath))
			realPath, _ = filepath.EvalSymlinks(oldGoldDriver)
			oldGoldDriverVer := filepath.Base(filepath.Dir(realPath))
			cmd += fmt.Sprintf(" --gold-firmware-latest-version %s", latestGoldDriverVer)
			cmd += fmt.Sprintf(" --gold-drivers-latest-pkg %s", latestGoldDriver)
			cmd += fmt.Sprintf(" --gold-firmware-old-version %s", oldGoldDriverVer)
			cmd += fmt.Sprintf(" --gold-drivers-old-pkg %s", oldGoldDriver)

			if node.topoNode.HostOS == "esx" {
				cmd += fmt.Sprintf(" --esx-script %s/iota/bin/iota_esx_setup", wsdir)
				cmd += fmt.Sprintf(" --host-username %s", tb.Params.Provision.Vars["EsxUsername"])
				cmd += fmt.Sprintf(" --host-password %s", tb.Params.Provision.Vars["EsxPassword"])

			}
			nodeName := node.NodeName

			// add the command to pool to be executed in parallel
			pool.Go(func() error {
				command := exec.Command("sh", "-c", cmd)
				log.Infof("Running command: %s", cmd)

				// open the out file for writing
				outfile, err := os.Create(fmt.Sprintf("%s/iota/%s-firmware-upgrade.log", wsdir, nodeName))
				if err != nil {
					log.Errorf("Error creating log file. Err: %v", err)
					return err
				}
				defer outfile.Close()
				command.Stdout = outfile
				command.Stderr = outfile
				err = command.Start()
				if err != nil {
					log.Errorf("Error running command %s. Err: %v", cmd, err)
					return err
				}

				return command.Wait()
			})

		}
	}

	err := pool.Wait()
	if err != nil {
		log.Errorf("Error executing pool. Err: %v", err)
		return err
	}

	log.Infof("Recovering naples nodes complete...")

	return nil
}

// connectToIotaServer connects to iota server
func (tb *TestBed) connectToIotaServer() error {
	srvURL := os.Getenv("IOTA_SERVER_URL")
	if srvURL == "" {
		srvURL = defaultIotaServerURL
	}

	// connect to iota server
	iotc, err := common.CreateNewGRPCClient("iota-server", srvURL)
	if err != nil {
		return err
	}

	tb.iotaClient = iotc

	return nil
}

func (tb *TestBed) setupTestBed() error {
	client := iota.NewTopologyApiClient(tb.iotaClient.Client)

	// Allocate VLANs
	testBedMsg := &iota.TestBedMsg{
		NaplesImage: tb.Topo.NaplesImage,
		VeniceImage: tb.Topo.VeniceImage,
		Username:    tb.Params.Provision.Username,
		Password:    tb.Params.Provision.Password,
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.TestBedNode{},
	}

	for _, node := range tb.Nodes {
		tbn := iota.TestBedNode{
			Type:      node.Type,
			IpAddress: node.NodeMgmtIP,
		}

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

		testBedMsg.Nodes = append(testBedMsg.Nodes, &tbn)
	}

	if !tb.skipSetup {
		// first cleanup testbed
		client.CleanUpTestBed(context.Background(), testBedMsg)

		// then, init testbed
		log.Debugf("Initializing testbed with params: %+v", testBedMsg)
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
		case iota.PersonalityType_PERSONALITY_NAPLES:
			tbn.StartupScript = "/naples/nodeinit.sh"
			fallthrough
		case iota.PersonalityType_PERSONALITY_NAPLES_SIM:
			tbn.Image = filepath.Base(tb.Topo.NaplesImage)
			tbn.NodeInfo = &iota.Node_NaplesConfig{
				NaplesConfig: &node.NaplesConfig,
			}
			tbn.Entities = []*iota.Entity{
				{
					Type: iota.EntityType_ENTITY_TYPE_HOST,
					Name: node.NodeName + "_host",
				},
				{
					Type: iota.EntityType_ENTITY_TYPE_NAPLES,
					Name: node.NodeName + "_naples",
				},
			}
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
		}

		nodes.Nodes = append(nodes.Nodes, &tbn)
	}

	if !tb.skipSetup {
		// add all nodes
		log.Debugf("Adding nodes: %+v", nodes)
		addNodeResp, err := client.AddNodes(context.Background(), nodes)
		if err != nil || addNodeResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			log.Errorf("Error adding nodes: ApiResp: %+v. Err %v", addNodeResp.ApiResponse, err)
			return fmt.Errorf("Error while adding nodes to testbed")
		}

		log.Debugf("Got add node resp: %+v", addNodeResp)
		tb.addNodeResp = addNodeResp

	} else {
		getNodeResp, err := client.GetNodes(context.Background(), nodes)
		if err != nil {
			log.Errorf("Error getting nodes: ApiResp: %+v. Err %v", getNodeResp.ApiResponse, err)
			return fmt.Errorf("Error while getting nodes from testbed")
		}

		log.Debugf("Got get node resp: %+v", getNodeResp)
		tb.addNodeResp = getNodeResp
	}

	// save node uuid
	for _, nr := range tb.addNodeResp.Nodes {
		for i := 0; i < len(tb.Nodes); i++ {
			node := tb.Nodes[i]
			if node.NodeName == nr.Name {
				node.NodeUUID = nr.NodeUuid
				node.iotaNode = nr
			}
		}
	}

	return nil
}

// SetupConfig sets up the venice cluster and basic config (like auth etc)
func (tb *TestBed) SetupConfig() error {
	if tb.skipSetup {
		return nil
	}

	// make venice cluster
	err := tb.MakeVeniceCluster()
	if err != nil {
		log.Errorf("Error creating venice cluster. Err: %v", err)
		return err
	}

	// setup auth and wait for venice cluster to come up
	err = tb.InitVeniceConfig()
	if err != nil {
		log.Errorf("Error configuring cluster. Err: %v", err)
		return err
	}

	// setup some tooling on venice nodes
	err = tb.SetupVeniceNodes()
	if err != nil {
		log.Errorf("Error setting up venice nodes. Err: %v", err)
		return err
	}

	return nil
}

// Cleanup cleans up the testbed
func (tb *TestBed) Cleanup() error {
	return nil
}
