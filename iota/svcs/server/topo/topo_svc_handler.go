package topo

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/iota/svcs/common"

	"golang.org/x/crypto/ssh"

	"golang.org/x/sync/errgroup"

	"github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/server/topo/testbed"
)

// TopologyService implements topology service API
type TopologyService struct {
	SSHConfig        *ssh.ClientConfig
	tbInfo           testBedInfo
	Nodes            map[string]testbed.TestNodeInterface
	ProvisionedNodes map[string]testbed.TestNodeInterface
	downloadedImages bool
}

type testBedInfo struct {
	resp           *iota.TestBedMsg //server
	allocatedVlans []uint32
	id             uint32
	switches       []*iota.DataSwitch
}

type FirmwareInfo struct {
	FwImage                   string `json:"image"`
	GoldFirmwareImage         string `json:"gold_fw_img"`
	GoldFirmwareLatestVersion string `json:"gold_fw_latest_ver"`
	GoldFirmwareOldVersion    string `json:"gold_fw_old_ver"`
}

type DriverInfo struct {
	OS                      string `json:"OS"`
	DriversPackage          string `json:"drivers_pkg"`
	GoldDriverLatestPackage string `json:"gold_drv_latest_pkg"`
	GoldDriverOldPackage    string `json:"gold_drv_old_pkg"`
	FileType                string `json:"pkg_file_type"`
}

// NewTopologyServiceHandler Topo service handle
func NewTopologyServiceHandler() *TopologyService {
	topoServer := TopologyService{}
	return &topoServer
}

// downloadImages downloads image
func (ts *TopologyService) downloadImages() error {

	if ts.downloadedImages {
		return nil
	}
	ctrlVMDir := common.ControlVMImageDirectory + "/" + common.EsxControlVMImage
	imageName := common.EsxControlVMImage + ".ova"
	cwd, _ := os.Getwd()
	mkdir := []string{"mkdir", "-p", ctrlVMDir}
	if stdout, err := exec.Command(mkdir[0], mkdir[1:]...).CombinedOutput(); err != nil {
		return errors.Wrap(err, string(stdout))
	}

	os.Chdir(ctrlVMDir)
	defer os.Chdir(cwd)

	buildIt := []string{common.BuildItBinary, "-t", common.BuildItURL, "image", "pull", "-o", imageName, common.EsxControlVMImage}
	if stdout, err := exec.Command(buildIt[0], buildIt[1:]...).CombinedOutput(); err != nil {
		return errors.Wrap(err, string(stdout))
	}

	tarCmd := []string{"tar", "-xvf", imageName}
	if stdout, err := exec.Command(tarCmd[0], tarCmd[1:]...).CombinedOutput(); err != nil {
		return errors.Wrap(err, string(stdout))
	}

	ts.downloadedImages = true
	return nil
}

// InstallImage recovers the Naples nodes and installs an image
func (ts *TopologyService) InstallImage(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	resp := *req
	log.Infof("TOPO SVC | DEBUG | InstallImage. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | InstallImage Returned: %v", resp)

	gopath := os.Getenv("GOPATH")
	if gopath == "" {
		log.Errorf("GOPATH not defined in the environment")

		err := fmt.Errorf("GOPATH not defined in the environment")
		resp.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		resp.ApiResponse.ErrorMsg = err.Error()
		return &resp, err
	}
	wsdir := gopath + "/src/github.com/pensando/sw"

	// split updates into pools of upto 'n' each
	for nodeIdx := 0; nodeIdx < len(req.Nodes); {
		poolNodes := []*iota.TestBedNode{}
		for nodeIdx < len(req.Nodes) {
			poolNodes = append(poolNodes, req.Nodes[nodeIdx])
			nodeIdx++
			if len(poolNodes) >= common.MaxConcurrentNaplesToBringup {
				break
			}
		}

		pool, _ := errgroup.WithContext(context.Background())
		// walk each node
		for _, node := range poolNodes {
			if node.Type == iota.TestBedNodeType_TESTBED_NODE_TYPE_HW {
				cmd := fmt.Sprintf("%s/iota/scripts/boot_naples_v2.py", wsdir)
				cmd += fmt.Sprintf(" --mnic-ip 169.254.0.1")
				cmd += fmt.Sprintf(" --instance-name %v", node.InstanceName)
				cmd += fmt.Sprintf(" --testbed %v", req.TestbedJsonFile)
				// naples_type should come from topology or testbed
				cmd += fmt.Sprintf(" --naples capri")
				if filepath.Base(req.NaplesImage) != "naples_fw.tar" {
					cmd += fmt.Sprintf(" --pipeline apulu --image-build equinix")
				}
				cmd += fmt.Sprintf(" --mode hostpin")
				if node.MgmtIntf != "" {
					cmd += fmt.Sprintf(" --mgmt-intf %v", node.MgmtIntf)
				}
				cmd += fmt.Sprintf(" --uuid %s", node.NicUuid)

				cmd += fmt.Sprintf(" --wsdir %s", wsdir)
				cmd += fmt.Sprintf(" --image-manifest %s/images/latest.json", wsdir)

				if node.NoMgmt {
					cmd += fmt.Sprintf(" --no-mgmt")
				}
				if node.AutoDiscoverOnInstall {
					cmd += fmt.Sprintf(" --auto-discover-on-install")
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
			log.Errorf("Error executing boot_naples_v2.py script. Err: %s", err)
			stdout, _ := exec.Command("sh", "-c", "tail -n 100 *upgrade.log").CombinedOutput()
			fmt.Println(stdout)
			return nil, err
		}
	}

	log.Infof("Recovering naples nodes complete...")
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	return req, nil
}

func (ts *TopologyService) switchProgrammingRequired(req *iota.TestBedMsg) bool {

	if req.TestbedId == 0 {
		return false
	}
	if ts.tbInfo.id != req.TestbedId || len(ts.tbInfo.allocatedVlans) == 0 {
		return true
	}
	sliceEqual := func(X, Y []string) bool {
		m := make(map[string]int)

		for _, y := range Y {
			m[y]++
		}

		for _, x := range X {
			if m[x] > 0 {
				m[x]--
				continue
			}
			//not present or execess
			return false
		}

		return len(m) == 0
	}

	for _, reqSw := range req.DataSwitches {
		for _, sw := range ts.tbInfo.switches {
			if sw.GetIp() != reqSw.GetIp() || reqSw.GetSpeed() != sw.GetSpeed() ||
				!sliceEqual(reqSw.Ports, sw.Ports) {
				return true
			}
		}
	}

	return false
}

// InitTestBed does initiates a test bed
func (ts *TopologyService) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | InitTestBed. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | InitTestBed Returned: %v", req)
	var vlans []uint32
	var err error
	ts.tbInfo.resp = req

	if err := ts.downloadImages(); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Download images failed. Err: %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Download images failed")
		return req, nil

	}
	// Preflight checks
	if len(req.Nodes) == 0 {
		log.Errorf("TOPO SVC | InitTestBed | No Nodes present. Err: %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("request message doesn't have any nodes.")
		return req, nil
	}

	if len(req.Username) == 0 || len(req.Password) == 0 {
		log.Errorf("TOPO SVC | InitTestBed | User creds to access the vms are missing.")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = "user creds are missing to access the VMs"
		return req, nil
	}

	if req.DataSwitches != nil && req.TestbedId != 0 {
		//Allocate only if TB ID chabged or vlan not allocated
		if ts.switchProgrammingRequired(req) {
			if vlans, err = testbed.SetUpTestbedSwitch(req.DataSwitches, req.TestbedId, req.NativeVlan); err != nil {
				log.Errorf("TOPO SVC | InitTestBed | Could not initialize switch id: %d, Err: %v", req.TestbedId, err)
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Switch configuration failed %d. Err: %v", req.TestbedId, err)
				return req, nil
			}
			ts.tbInfo.allocatedVlans = vlans
		} else {
			log.Infof("Skipping switch programming")
		}
		ts.tbInfo.resp.AllocatedVlans = ts.tbInfo.allocatedVlans
		ts.tbInfo.id = req.TestbedId
	}

	ts.SSHConfig = testbed.InitSSHConfig(ts.tbInfo.resp.Username, ts.tbInfo.resp.Password)

	// Run init
	ts.ProvisionedNodes = make(map[string]testbed.TestNodeInterface)
	ts.Nodes = make(map[string]testbed.TestNodeInterface)
	// split init nodes into pools of upto 'n' each
	restoreAgentFiles := false
	for nodeIdx := 0; nodeIdx < len(req.Nodes); {
		poolNodes := []*iota.TestBedNode{}
		for nodeIdx < len(req.Nodes) {
			poolNodes = append(poolNodes, req.Nodes[nodeIdx])
			nodeIdx++
			if len(poolNodes) >= common.MaxConcurrentNaplesToBringup {
				break
			}
		}
		pool, _ := errgroup.WithContext(context.Background())

		for _, node := range poolNodes {
			nodeInfo := testbed.NodeInfo{
				CimcIP:       node.CimcIpAddress,
				CimcNcsiIp:   node.CimcNcsiIp,
				CimcPassword: node.CimcPassword,
				CimcUserName: node.CimcUsername,
				Os:           node.Os,
				SSHCfg:       ts.SSHConfig,
				Username:     req.Username,
				Password:     req.Password,
				Name:         node.NodeName,
				IPAddress:    node.IpAddress,
				License:      node.License,
			}

			if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX ||
				node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER {
				nodeInfo.Username = node.EsxUsername
				nodeInfo.Password = node.EsxPassword
			}
			n := testbed.NewTestNode(nodeInfo)
			ts.Nodes[node.IpAddress] = n

			commonCopyArtifacts := []string{
				ts.tbInfo.resp.VeniceImage,
				ts.tbInfo.resp.NaplesImage,
				ts.tbInfo.resp.NaplesSimImage,
			}
			if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER {
				n.SetDC(node.DcName)
				n.SetSwitch(node.Switch)
			}

			pool.Go(func() error {
				n := n
				return n.InitNode(req.RebootNodes, restoreAgentFiles, ts.SSHConfig, commonCopyArtifacts)
			})
		}
		err = pool.Wait()
		if err != nil {
			break
		}
	}
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Init Test Bed Call Failed. %v", err)
		ts.tbInfo.resp.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		ts.tbInfo.resp.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitTestBed | Init Test Bed Call Failed. %s", err.Error())
		return ts.tbInfo.resp, nil
	}
	for ip, node := range ts.Nodes {
		for index, reqnode := range req.Nodes {
			if reqnode.IpAddress == ip && reqnode.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
				if ip, err := node.GetNodeIP(); err == nil {
					req.Nodes[index].EsxCtrlNodeIpAddress = ip
				}
			}
		}
	}

	log.Infof("Init testbed successful")
	ts.tbInfo.resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return ts.tbInfo.resp, nil
}

// GetTestBed gets testbed state
func (ts *TopologyService) GetTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {

	if ts.tbInfo.resp == nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Testbed not initialized")
		return req, nil

	}

	return ts.tbInfo.resp, nil
}

func (ts *TopologyService) initTestNodes(ctx context.Context, req *iota.TestNodesMsg) error {
	pool, ctx := errgroup.WithContext(ctx)

	restoreAgentFiles := false
	for _, node := range req.GetNodes() {

		nodeInfo := testbed.NodeInfo{
			CimcIP:       node.CimcIpAddress,
			CimcNcsiIp:   node.CimcNcsiIp,
			CimcPassword: node.CimcPassword,
			CimcUserName: node.CimcUsername,
			Os:           node.Os,
			SSHCfg:       ts.SSHConfig,
			Username:     req.Username,
			Password:     req.Password,
			Name:         node.NodeName,
			IPAddress:    node.IpAddress,
			License:      node.License,
		}
		if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX ||
			node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER {
			nodeInfo.Username = node.EsxUsername
			nodeInfo.Password = node.EsxPassword
		}
		n := testbed.NewTestNode(nodeInfo)
		ts.Nodes[node.IpAddress] = n
		commonCopyArtifacts := []string{
			ts.tbInfo.resp.VeniceImage,
			ts.tbInfo.resp.NaplesImage,
		}
		if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER {
			n.SetDC(node.DcName)
			n.SetSwitch(node.Switch)
		}

		pool.Go(func() error {
			n := n
			return n.InitNode(req.RebootNodes, restoreAgentFiles, ts.SSHConfig, commonCopyArtifacts)
		})
	}
	if err := pool.Wait(); err != nil {
		return err
	}

	for ip, node := range ts.Nodes {
		for index, reqnode := range req.GetNodes() {
			if reqnode.IpAddress == ip && reqnode.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
				if ip, err := node.GetNodeIP(); err != nil {
					req.GetNodes()[index].EsxCtrlNodeIpAddress = ip
				}
			}
		}
	}
	return nil
}

func (ts *TopologyService) cleanUpTestNodes(ctx context.Context, cfg *ssh.ClientConfig, reboot bool,
	nodes []*iota.TestBedNode, globalLicenses []*iota.License) error {

	pool, ctx := errgroup.WithContext(ctx)
	cnodes := []testbed.TestNodeInterface{}
	for _, node := range nodes {
		nodeInfo := testbed.NodeInfo{
			CimcIP:         node.CimcIpAddress,
			CimcNcsiIp:     node.CimcNcsiIp,
			CimcPassword:   node.CimcPassword,
			CimcUserName:   node.CimcUsername,
			Os:             node.Os,
			SSHCfg:         cfg,
			Username:       ts.tbInfo.resp.Username,
			Password:       ts.tbInfo.resp.Password,
			Name:           node.NodeName,
			IPAddress:      node.IpAddress,
			License:        node.License,
			GlobalLicenses: globalLicenses,
			ApcInfo:        node.ApcInfo,
		}
		if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX ||
			node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER {
			nodeInfo.Username = node.EsxUsername
			nodeInfo.Password = node.EsxPassword
		}
		n := testbed.NewTestNode(nodeInfo)
		if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_VCENTER {
			n.SetDC(node.DcName)
			n.SetSwitch(node.Switch)
		}
		cnodes = append(cnodes, n)

	}

	//First try to clean up nodes
	for _, n := range cnodes {
		if !n.IsOrchesratorNode() {
			n := n
			pool.Go(func() error {
				return n.CleanUpNode(cfg, reboot)
			})
		}
	}

	pool.Wait()

	pool, ctx = errgroup.WithContext(ctx)
	//Now clean up orchestrator node
	for _, n := range cnodes {
		if n.IsOrchesratorNode() {
			n := n
			pool.Go(func() error {
				return n.CleanUpNode(cfg, reboot)
			})
		}
	}

	return pool.Wait()
}

// DoSwitchOperation do switch operation
func (ts *TopologyService) DoSwitchOperation(ctx context.Context, req *iota.SwitchMsg) (*iota.SwitchMsg, error) {
	log.Infof("TOPO SVC | DEBUG | SwitchMsg. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | SwitchMsg Returned: %v", req)

	if err := testbed.DoSwitchOperation(ctx, req); err != nil {
		msg := fmt.Sprintf("TOPO SVC | SwitchMsg | Could not initialize switch Err: %v", err)
		req.ApiResponse.ErrorMsg = msg
		return req, nil
	}
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// CleanUpTestBed cleans up a testbed
func (ts *TopologyService) CleanUpTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | CleanUpTestBed. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | CleanUpTestBed Returned: %v", req)

	var vlans []uint32
	var err error
	if len(req.Username) == 0 || len(req.Password) == 0 {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Request must include a user name and password")
		return req, nil
	}

	ts.tbInfo.resp = req

	err = ts.cleanUpTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes(), req.GetLicenses())

	if err != nil {
		log.Errorf("TOPO SVC | InitNodes | cleanup Node Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitNodes | InitNodes Call Failed. %s", err.Error())
		return req, nil
	}

	if ts.switchProgrammingRequired(req) {
		if vlans, err = testbed.SetUpTestbedSwitch(req.DataSwitches, req.TestbedId, req.NativeVlan); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Could not initialize switch id: %d, Err: %v", req.TestbedId, err)
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Switch configuration failed %d. Err: %v", req.TestbedId, err)
			return req, nil
		}
		ts.tbInfo.allocatedVlans = vlans
	} else {
		log.Infof("Skipping switch programming")
	}
	ts.tbInfo.resp.AllocatedVlans = ts.tbInfo.allocatedVlans
	ts.tbInfo.id = req.TestbedId

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// InitNodes initializes list of nodes
func (ts *TopologyService) InitNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {

	err := ts.cleanUpTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes(), req.GetLicenses())

	if err != nil {
		log.Errorf("TOPO SVC | InitNodes | cleanup Node Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitNodes | InitNodes Call Failed. %s", err.Error())
		return req, nil
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	//Avoid one other reboot
	if req.RebootNodes {
		req.RebootNodes = false
	}
	err = ts.initTestNodes(ctx, req)
	if err != nil {
		log.Errorf("TOPO SVC | InitNodes | initnode Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitNodes | InitNodes Call Failed. %s", err.Error())
		return req, nil
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// CleanNodes cleans up list of nodes and removes association
func (ts *TopologyService) CleanNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {

	err := ts.cleanUpTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes(), req.GetLicenses())

	if err != nil {
		log.Errorf("TOPO SVC | InitNodes | cleanup Node Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitNodes | InitNodes Call Failed. %s", err.Error())
		return req, nil
	}

	log.Info("TOPO SVC | Clean Nodes , cleaning up node")
	for _, n := range req.Nodes {
		_, ok := ts.ProvisionedNodes[n.GetNodeName()]
		if ok {
			delete(ts.ProvisionedNodes, n.GetNodeName())
			log.Infof("TOPO SVC | Clean Nodes , cleaning up node %v", n.GetNodeName())
		}
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	return req, nil
}

// AddNodes adds nodes to the topology
func (ts *TopologyService) AddNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | AddNodes Returned: %v", req)

	if req.NodeOp != iota.Op_ADD {
		log.Errorf("TOPO SVC | AddNodes | AddNodes call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("AddNodes API must specify Add operation. Found: %v", req.NodeOp)
		return req, nil
	}

	// Prep Topo
	newNodes := []testbed.TestNodeInterface{}
	for _, n := range req.Nodes {

		if _, ok := ts.Nodes[n.IpAddress]; !ok {
			err := fmt.Errorf("TOPO SVC | AddNodes failed, unknown node with IP %v found", n.IpAddress)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Could not get agent URL. Err: %v", err)
			return req, nil
		}

		tbNode := ts.Nodes[n.IpAddress]

		if _, ok := ts.ProvisionedNodes[n.Name]; ok {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed as node already provisoned : %v", n.Name)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | AddNodes | AddNodes call failed as node already provisoned : %v", n.Name)
			return req, nil
		}

		ts.ProvisionedNodes[n.Name] = tbNode
		tbNode.SetNodeMsg(n)
		err := tbNode.SetupNode()
		if err != nil {
			log.Errorf("TOPO SVC | AddNodes | Setup node failed: %v", n.Name)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | AddNodes | Setup node failed: : %v", n.Name)
			return req, nil
		}
		log.Infof("Adding provisioned node %v : %v\n", n.Name, tbNode)
		newNodes = append(newNodes, ts.ProvisionedNodes[n.Name])
	}

	//First to add nodes which is not orchestrator
	pool, ctx := errgroup.WithContext(ctx)
	for _, n := range newNodes {
		if !n.IsOrchesratorNode() {
			n := n
			pool.Go(func() error {
				return n.AddNode()
			})
		}
	}

	err := pool.Wait()
	if err != nil {
		log.Errorf("TOPO SVC | AddNodes |AddNodes Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	} else {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	}

	//Associate nodes to orchestrator
	for _, node := range newNodes {
		//After successful provisioning, get managed do
		for _, mnode := range node.GetManagedNodes() {
			//All managed node references will be to the manager from now on
			ts.ProvisionedNodes[mnode.GetNodeInfo().Name] = node
			for _, indepNode := range newNodes {
				//If managed node is independently provisioned also add independent node
				if indepNode.GetNodeInfo().Name == mnode.GetNodeInfo().Name {
					node.AssocaiateIndependentNode(indepNode)
				}
			}
		}

	}

	//Now add orchestration node
	pool, ctx = errgroup.WithContext(ctx)
	for _, n := range newNodes {
		if n.IsOrchesratorNode() {
			n := n
			pool.Go(func() error {
				return n.AddNode()
			})
		}
	}

	err = pool.Wait()
	if err != nil {
		log.Errorf("TOPO SVC | AddNodes |AddNodes Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	} else {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	}

	for idx, node := range newNodes {
		req.Nodes[idx] = node.GetNodeMsg(node.GetNodeInfo().Name)
		if req.Nodes[idx].GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = "Node :" + req.Nodes[idx].GetName() + " : " + req.Nodes[idx].GetNodeStatus().ErrorMsg + "\n"
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		}
	}

	return req, nil
}

// DeleteNodes deletes a node from the topology
func (ts *TopologyService) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DeleteNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | DeleteNodes Returned: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// GetNodes returns the current topology information
func (ts *TopologyService) GetNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | GetNodes. Received Request Msg: %v", req)

	resp := iota.NodeMsg{
		AllocatedVlans: ts.tbInfo.allocatedVlans,
		ApiResponse: &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_STATUS_OK,
		},
	}

	for name, node := range ts.ProvisionedNodes {
		respMsg := node.GetNodeMsg(name)
		log.Infof("Returning provisioned node %v : %v\n", name, respMsg.Name)
		resp.Nodes = append(resp.Nodes, respMsg)
	}

	log.Infof("TOPO SVC | DEBUG | GetNodes Returned: %v", resp)
	return &resp, nil
}

// SaveNode save node personality for reboot
func (ts *TopologyService) SaveNode(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	resp := &iota.NodeMsg{}
	return resp, nil
}

// IpmiNodeAction issues ipmi command. also saves and loads node personality if needed
func (ts *TopologyService) IpmiNodeAction(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | IpmiNodeAction. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | IpmiNodeAction Returned: %v", req)

	type reloadReq struct {
		name    string
		node    testbed.TestNodeInterface
		method  string
		useNcsi bool
	}
	rNodes := []reloadReq{}

	for _, node := range req.NodeMsg.Nodes {
		n, ok := ts.ProvisionedNodes[node.Name]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Reload on unprovisioned node : %v", node.GetName())
			return req, nil
		}
		rNodes = append(rNodes, reloadReq{name: node.Name, node: n, method: req.RestartMethod, useNcsi: req.UseNcsi})
	}

	reloadNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, rnode := range rNodes {
			node := rnode.node
			name := rnode.name
			pool.Go(func() error {
				return node.IpmiNodeControl(name, !req.SkipRestore, rnode.method, rnode.useNcsi)
			})
		}
		return pool.Wait()
	}

	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	err := reloadNodes(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | IpmiNodeAction | IpmiNodeActions Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = err.Error()
	}

	for idx, rnode := range rNodes {
		req.NodeMsg.Nodes[idx] = rnode.node.GetNodeMsg(rnode.name)
		if req.NodeMsg.Nodes[idx].GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = "Node :" + req.NodeMsg.Nodes[idx].GetName() + " : " + req.NodeMsg.Nodes[idx].GetNodeStatus().ErrorMsg + "\n"
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		}
	}
	return req, nil
}

// ReloadNodes saves and loads node personality
func (ts *TopologyService) ReloadNodes(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | ReloadNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | ReloadNodes Returned: %v", req)

	type reloadReq struct {
		name    string
		node    testbed.TestNodeInterface
		method  string
		useNcsi bool
	}
	rNodes := []reloadReq{}

	for _, node := range req.NodeMsg.Nodes {
		n, ok := ts.ProvisionedNodes[node.Name]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Reload on unprovisioned node : %v", node.GetName())
			return req, nil
		}
		rNodes = append(rNodes, reloadReq{name: node.Name, node: n, method: req.RestartMethod, useNcsi: req.UseNcsi})
	}

	reloadNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, rnode := range rNodes {
			node := rnode.node
			name := rnode.name
			pool.Go(func() error {
				return node.ReloadNode(name, !req.SkipRestore, rnode.method, rnode.useNcsi)
			})
		}
		return pool.Wait()
	}

	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	err := reloadNodes(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | ReloadNodes | ReloadNodes Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = err.Error()
	}

	for idx, rnode := range rNodes {
		req.NodeMsg.Nodes[idx] = rnode.node.GetNodeMsg(rnode.name)
		if req.NodeMsg.Nodes[idx].GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = "Node :" + req.NodeMsg.Nodes[idx].GetName() + " : " + req.NodeMsg.Nodes[idx].GetNodeStatus().ErrorMsg + "\n"
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		}
	}
	return req, nil
}

// AddWorkloads adds a workload on a given node
func (ts *TopologyService) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddWorkloads. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | AddWorkloads Returned: %v", req)

	if req.WorkloadOp != iota.Op_ADD {
		log.Errorf("TOPO SVC | AddWorkloads | AddWorkloads call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("AddWorkloads must specify Op_Add for workload op. Found: %v", req.WorkloadOp)
		return req, nil
	}

	log.Infof("TOPO SVC | DEBUG | STATE | %v", ts.Nodes)

	workloadNodes := []testbed.TestNodeInterface{}
	var wlNode map[string]*iota.WorkloadMsg
	var wlRespNode map[string]*iota.WorkloadMsg
	wlNode = make(map[string]*iota.WorkloadMsg)
	var m sync.Mutex
	wlRespNode = make(map[string]*iota.WorkloadMsg)

	for _, w := range req.Workloads {

		node, ok := ts.ProvisionedNodes[w.NodeName]
		nodeName := node.GetNodeInfo().Name
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("AddWorkloads found to unprovisioned node : %v", w.NodeName)
			return req, nil
			//}
		}
		if _, ok := wlNode[nodeName]; !ok {
			wlNode[nodeName] = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
		}
		workloadInfo := wlNode[nodeName]

		workloadInfo.Workloads = append(workloadInfo.Workloads, w)
		added := false
		for _, workloadNode := range workloadNodes {
			if workloadNode.GetNodeInfo().Name == node.GetNodeInfo().Name {
				added = true
				break
			}
		}
		if !added {
			workloadNodes = append(workloadNodes, node)
		}
	}

	// Add workloads
	addWorkloads := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)
		for _, node := range workloadNodes {
			node := node
			pool.Go(func() error {
				workloadInfo, _ := wlNode[node.GetNodeInfo().Name]
				log.Infof("Triggering workload add for %v", node.GetNodeInfo().Name)
				resp, err := node.AddWorkloads(workloadInfo)
				m.Lock()
				wlRespNode[node.GetNodeInfo().Name] = resp
				m.Unlock()
				return err
			})

		}
		return pool.Wait()
	}

	err := addWorkloads(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | AddWorkloads |AddWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | AddWorkloads |AddWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		return req, nil
	}
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	//Assoicate response
	for _, node := range workloadNodes {
		resp, ok := wlRespNode[node.GetNodeInfo().Name]
		if !ok {
			continue
		}
		if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = resp.ApiResponse.ErrorMsg
			req.ApiResponse.ApiStatus = resp.ApiResponse.ApiStatus
			log.Errorf("TOPO SVC | AddWorkloads | Workload  failed with  %v", resp.ApiResponse.ErrorMsg)
			break
		}
		for _, respWload := range resp.Workloads {
			for index, reqWload := range req.Workloads {
				if reqWload.GetNodeName() == node.GetNodeInfo().Name && reqWload.WorkloadName == respWload.WorkloadName {
					req.Workloads[index] = respWload
					if respWload.WorkloadStatus != nil && respWload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
						req.ApiResponse.ErrorMsg = respWload.WorkloadStatus.ErrorMsg
						req.ApiResponse.ApiStatus = respWload.WorkloadStatus.ApiStatus
						log.Errorf("TOPO SVC | AddWorkloads | Workload add %v failed with  %v", respWload.GetWorkloadName(), req.ApiResponse.ErrorMsg)
					}
					break
				}
			}
		}

	}

	return req, nil
}

// GetWorkloads returns list of workloads
func (ts *TopologyService) GetWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	var workloads []*iota.Workload
	log.Infof("TOPO SVC | DEBUG | GetWorkloads. Received Request Msg: %#v", req)

	for name, node := range ts.ProvisionedNodes {
		wloads := node.GetWorkloads(name)
		for _, wl := range wloads {
			workloads = append(workloads, wl)
		}
	}

	wmsg := &iota.WorkloadMsg{
		ApiResponse: &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_STATUS_OK,
		},
		WorkloadOp: iota.Op_GET,
		Workloads:  workloads,
	}
	log.Infof("TOPO SVC | DEBUG | GetWorkloads Returned: %#v", wmsg)

	return wmsg, nil
}

// DeleteWorkloads deletes a workload
func (ts *TopologyService) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DeleteWorkloads. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | DeleteWorkloads Returned: %v", req)

	if req.WorkloadOp != iota.Op_DELETE {
		log.Errorf("TOPO SVC | DeleteWorkloads | DeleteWorkloads call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("DeleteWorkloads must specify Op_Add for workload op. Found: %v", req.WorkloadOp)
		return req, nil
	}

	log.Infof("TOPO SVC | DEBUG | STATE | %v", ts.Nodes)

	workloadNodes := []testbed.TestNodeInterface{}

	var wlNode map[string]*iota.WorkloadMsg
	var wlRespNode map[string]*iota.WorkloadMsg
	wlNode = make(map[string]*iota.WorkloadMsg)
	var m sync.Mutex
	wlRespNode = make(map[string]*iota.WorkloadMsg)

	for _, w := range req.Workloads {

		node, ok := ts.ProvisionedNodes[w.NodeName]
		nodeName := node.GetNodeInfo().Name
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("DeleteWorkloads found to unprovisioned node : %v", w.NodeName)
			return req, nil
			//}
		}
		if _, ok := wlNode[nodeName]; !ok {
			wlNode[nodeName] = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
		}
		workloadInfo := wlNode[nodeName]

		workloadInfo.Workloads = append(workloadInfo.Workloads, w)
		added := false
		for _, workloadNode := range workloadNodes {
			if workloadNode.GetNodeInfo().Name == node.GetNodeInfo().Name {
				added = true
				break
			}
		}
		if !added {
			workloadNodes = append(workloadNodes, node)
		}
	}

	// Add workloads
	deleteWorkloads := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)
		for _, node := range workloadNodes {
			node := node
			pool.Go(func() error {
				workloadInfo, _ := wlNode[node.GetNodeInfo().Name]
				resp, err := node.DeleteWorkloads(workloadInfo)
				m.Lock()
				wlRespNode[node.GetNodeInfo().Name] = resp
				m.Unlock()
				return err
			})

		}
		return pool.Wait()
	}

	err := deleteWorkloads(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | DeleteWorkloads |DeleteWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | DeleteWorkloads |DeleteWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	} else {
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	}

	//Assoicate response
	for _, node := range workloadNodes {
		resp, ok := wlRespNode[node.GetNodeInfo().Name]
		if !ok {
			continue
		}
		for _, respWload := range resp.Workloads {
			for index, reqWload := range req.Workloads {
				if reqWload.GetNodeName() == node.GetNodeInfo().Name && reqWload.WorkloadName == respWload.WorkloadName {
					req.Workloads[index] = respWload
					if respWload.WorkloadStatus != nil && respWload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
						req.ApiResponse.ErrorMsg = respWload.WorkloadStatus.ErrorMsg
						log.Errorf("TOPO SVC | Delete | Workload add %v failed with  %v", respWload.GetWorkloadName(), req.ApiResponse.ErrorMsg)
					}
					break
				}
			}
		}

	}

	return req, nil
}

func (ts *TopologyService) runParallelTrigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	triggerNodes := []testbed.TestNodeInterface{}
	triggerResp := &iota.TriggerMsg{TriggerMode: req.GetTriggerMode(),
		TriggerOp: req.GetTriggerOp()}

	triggerInfoMap := new(sync.Map)
	triggerRespMap := new(sync.Map)
	var triggerInfo *iota.TriggerMsg
	triggerCmdIndexMap := make(map[string][]int)

	for index, cmd := range req.GetCommands() {
		node, _ := ts.ProvisionedNodes[cmd.GetNodeName()]
		if item, ok := triggerInfoMap.Load(node.GetNodeInfo().Name); !ok {
			triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
				TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
			triggerInfoMap.Store(node.GetNodeInfo().Name, triggerMsg)
			triggerInfo = triggerMsg
			triggerCmdIndexMap[node.GetNodeInfo().Name] = []int{}
		} else {
			triggerInfo = item.(*iota.TriggerMsg)
			triggerInfo.Commands = append(triggerInfo.Commands, cmd)
		}

		triggerCmdIndexMap[node.GetNodeInfo().Name] = append(triggerCmdIndexMap[node.GetNodeInfo().Name], index)
		//Just copy the request for now
		triggerResp.Commands = append(triggerResp.Commands, cmd)
		added := false
		for _, triggerNode := range triggerNodes {
			if triggerNode.GetNodeInfo().Name == node.GetNodeInfo().Name {
				added = true
				break
			}
		}
		if !added {
			triggerNodes = append(triggerNodes, node)
		}
	}
	// Triggers
	triggers := func(ctx2 context.Context) error {
		pool, ctx3 := errgroup.WithContext(ctx2)
		for _, node := range triggerNodes {
			node := node
			pool.Go(func() error {
				item, _ := triggerInfoMap.Load(node.GetNodeInfo().Name)
				triggerMsg := item.(*iota.TriggerMsg)
				triggerResp, err := node.TriggerWithContext(ctx3, triggerMsg)
				triggerRespMap.Store(node.GetNodeInfo().Name, triggerResp)
				if err != nil {
					return err
				}
				return nil
			})

		}
		return pool.Wait()
	}

	err := triggers(ctx)
	if err != nil {
		log.Errorf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		return req, nil

	}

	for _, node := range triggerNodes {
		item, _ := triggerRespMap.Load(node.GetNodeInfo().Name)
		triggerMsg := item.(*iota.TriggerMsg)
		for index, cmd := range triggerMsg.Commands {
			realIndex := triggerCmdIndexMap[node.GetNodeInfo().Name][index]
			triggerResp.Commands[realIndex] = cmd
		}
	}

	triggerResp.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}

	return triggerResp, nil
}

func (ts *TopologyService) runSerialTrigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	for cidx, cmd := range req.GetCommands() {
		node, _ := ts.ProvisionedNodes[cmd.GetNodeName()]
		triggerInfo := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
			TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
		if triggerResp, err := node.TriggerWithContext(ctx, triggerInfo); err != nil {

			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | Trigger | RunSerialTrigger Call Failed. %v", err)
			return req, nil
		} else {
			req.Commands[cidx] = triggerResp.GetCommands()[0]
		}
		/* Only one command sent anyway */
	}
	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	return req, nil
}

// Trigger triggers a workload
func (ts *TopologyService) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	log.Infof("TOPO SVC | DEBUG | Trigger. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | Trigger Returned: %v", req)

	if req.TriggerOp == iota.TriggerOp_TYPE_NONE {
		log.Errorf("TOPO SVC | Trigger | Trigger call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Trigger must specify TriggerOp for workload op. Found: %v", req.TriggerOp)
		return req, nil
	}

	for _, cmd := range req.GetCommands() {
		if _, ok := ts.ProvisionedNodes[cmd.NodeName]; !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Trigger command found to unprovisioned node : %v", cmd.NodeName)
			return req, nil
		}

	}

	if req.GetTriggerOp() == iota.TriggerOp_TERMINATE_ALL_CMDS ||
		req.GetTriggerMode() == iota.TriggerMode_TRIGGER_PARALLEL ||
		req.GetTriggerMode() == iota.TriggerMode_TRIGGER_NODE_PARALLEL {
		return ts.runParallelTrigger(ctx, req)
	}

	return ts.runSerialTrigger(ctx, req)
}

// CheckClusterHealth checks the e2e cluster health
func (ts *TopologyService) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (*iota.ClusterHealthMsg, error) {
	log.Infof("TOPO SVC | DEBUG | CheckClusterHealth. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | CheckClusterHealth Returned: %v", req)

	resp := &iota.ClusterHealthMsg{
		ApiResponse: &iota.IotaAPIResponse{},
	}

	for name, node := range ts.ProvisionedNodes {
		nodeHealth := &iota.NodeHealth{NodeName: name, ClusterDone: req.GetClusterDone()}
		nodeResp, err := node.CheckHealth(ctx, nodeHealth)
		if err == nil {
			resp.Health = append(resp.Health, nodeResp)
		}
	}

	resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return resp, nil
}

// EntityCopy does copy of items to/from entity.
func (ts *TopologyService) EntityCopy(ctx context.Context, req *iota.EntityCopyMsg) (*iota.EntityCopyMsg, error) {
	log.Infof("TOPO SVC | DEBUG | EntityCopy. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | EntityCopy Returned: %v", req)

	node, ok := ts.ProvisionedNodes[req.NodeName]
	if !ok {
		errMsg := fmt.Sprintf("Node %s  not provisioned", req.NodeName)
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
			ErrorMsg: errMsg}
		return req, nil
	}

	return node.EntityCopy(ts.SSHConfig, req)
}

// MoveWorkloads move workloads
func (ts *TopologyService) MoveWorkloads(ctx context.Context, req *iota.WorkloadMoveMsg) (*iota.WorkloadMoveMsg, error) {
	log.Infof("TOPO SVC | DEBUG | MoveWorkloads. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | MoveWorkloads Returned: %v", req)

	node, ok := ts.ProvisionedNodes[req.OrchestratorNode]
	if !ok {
		errMsg := fmt.Sprintf("Node %s  not provisioned", req.OrchestratorNode)
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
			ErrorMsg: errMsg}
		return req, nil
	}

	return node.MoveWorkloads(ctx, req)
}

// SaveNodes to save and download context to local fs
func (ts *TopologyService) SaveNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | SaveNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | SaveNodes Returned: %v", req)

	// - Call SaveNodes Agent API
	rNodes := []testbed.TestNodeInterface{}
	for _, node := range req.Nodes {
		n, ok := ts.ProvisionedNodes[node.Name]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("SaveNodes on unprovisioned node: %v", node.GetName())
			return req, nil
		}
		rNodes = append(rNodes, n)
	}

	saveNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range rNodes {
			node := node
			pool.Go(func() error {
				return node.SaveNode(ts.SSHConfig)
			})
		}
		return pool.Wait()
	}
	err := saveNodes(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | SaveNodes | SaveNodes Call failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = err.Error()
	}
	req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	return req, nil
}

// RestoreNodes is to initNode after external/unknown reboot event
func (ts *TopologyService) RestoreNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | RestoreNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | RestoreNodes Returned: %v", req)

	pool, ctx := errgroup.WithContext(ctx)

	rNodes := []testbed.TestNodeInterface{}
	for _, node := range req.Nodes {
		n, ok := ts.ProvisionedNodes[node.Name]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("SaveNodes on unprovisioned node: %v", node.GetName())
			return req, nil
		}
		rNodes = append(rNodes, n)
	}
	reboot := false
	restoreAgentFiles := true
	// - Prepare to upload the required files from local fs to remote node
	for _, node := range rNodes {
		// Assumption that node is alive and reachable
		commonCopyArtifacts := []string{
			ts.tbInfo.resp.VeniceImage,
			ts.tbInfo.resp.NaplesImage,
			ts.tbInfo.resp.NaplesSimImage,
		}
		pool.Go(func() error {
			node := node
			node.InitNode(reboot, restoreAgentFiles, ts.SSHConfig, commonCopyArtifacts)
			svcName := node.GetNodeInfo().Name

			agentURL, err := node.GetAgentURL()
			if err != nil {
				log.Errorf("TOPO SVC | RestoreNodes | Failed obtain AgentURL for Node: %v. Err: %v", svcName, err)
				return err
			}

			c, err := common.CreateNewGRPCClient(svcName, agentURL, common.GrpcMaxMsgSize)
			if err != nil {
				log.Errorf("TOPO SVC | RestoreNodes | CreateNewGRPCClient call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", svcName, err)
				return err
			}
			node.SetNodeAgent(iota.NewIotaAgentApiClient(c.Client))
			resp, err := node.GetNodeAgent().ReloadNode(ctx, node.GetNodeMsg(node.GetNodeInfo().Name))
			log.Infof("TOPO SVC | RestoreNodes | ReloadNode Agent . Received Response Msg: %v", resp)
			if err != nil {
				log.Errorf("Reload node %v failed. Err: %v", node.GetNodeInfo().Name, err)
				return err
			}
			return nil
		})
	}
	if err := pool.Wait(); err != nil {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		return req, nil
	}
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// DownlaodAssets pulls assets
func (ts *TopologyService) DownloadAssets(ctx context.Context, req *iota.DownloadAssetsMsg) (*iota.DownloadAssetsMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DownloadAssets. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | DownloadAssets Returned: %v", req)

	// TODO: Explore integrating with asset-build. For now invoke the cmd to asset-pull
	mkdir := []string{"mkdir", "-p", req.ParentDir}
	if stdout, err := exec.Command(mkdir[0], mkdir[1:]...).CombinedOutput(); err != nil {
		log.Errorf("TOPO SVC | DownloadAssets | Failed to create repo-folder: %v (%v, %v)",
			req.ParentDir, err.Error(), string(stdout))
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to create repo folder")
		return req, nil
	}

	cwd, _ := os.Getwd()
	os.Chdir(req.ParentDir)
	defer os.Chdir(cwd)

	destFile := req.ReleaseVersion + ".tgz"

	pullAsset := []string{common.AssetPullBin, req.AssetName, req.ReleaseVersion, destFile}
	if stdout, err := exec.Command(pullAsset[0], pullAsset[1:]...).CombinedOutput(); err != nil {
		log.Errorf("TOPO SVC | DownloadAssets | Failed to download asset: %v %v (%v %v)",
			req.AssetName, req.ReleaseVersion, err.Error(), string(stdout))
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("asset-pull failed")
		return req, nil
	}

	tarCmd := []string{"tar", "-zxvf", destFile}
	if stdout, err := exec.Command(tarCmd[0], tarCmd[1:]...).CombinedOutput(); err != nil {
		log.Errorf("TOPO SVC | DownloadAssets | Failed to extract asset: %v %v (%v %v)",
			req.AssetName, req.ReleaseVersion, err.Error(), string(stdout))
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to untar downloaded asset")
		return req, nil
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// AddNetworks add networks
func (ts *TopologyService) AddNetworks(ctx context.Context, req *iota.NetworksMsg) (*iota.NetworksMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddNetworks Received Request Msg: %v", req)

	node, ok := ts.ProvisionedNodes[req.OrchestratorNode]
	if !ok {
		errMsg := fmt.Sprintf("Node %s  not provisioned", req.OrchestratorNode)
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
			ErrorMsg: errMsg}
		return req, nil
	}

	return node.AddNetworks(ctx, req)
}

// RemoveNetworks remove networks
func (ts *TopologyService) RemoveNetworks(ctx context.Context, req *iota.NetworksMsg) (*iota.NetworksMsg, error) {
	log.Infof("TOPO SVC | DEBUG | RemoveNetworks Received Request Msg: %v", req)

	if req.Switch == "" {
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR,
			ErrorMsg: "Switch not set"}
		// TODO return fully formed resp here
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR,
			ErrorMsg: "Switch not set"}
		return req, nil
	}

	node, ok := ts.ProvisionedNodes[req.OrchestratorNode]
	if !ok {
		errMsg := fmt.Sprintf("Node %s  not provisioned", req.OrchestratorNode)
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
			ErrorMsg: errMsg}
		return req, nil
	}

	return node.RemoveNetworks(ctx, req)
}
