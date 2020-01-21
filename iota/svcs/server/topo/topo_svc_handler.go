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
	Nodes            map[string]*testbed.TestNode
	Workloads        map[string]*iota.Workload // list of workloads
	ProvisionedNodes map[string]*testbed.TestNode
	downloadedImages bool
}

type testBedInfo struct {
	resp           *iota.TestBedMsg //server
	allocatedVlans []uint32
	id             uint32
	switches       []*iota.DataSwitch
}

// NewTopologyServiceHandler Topo service handle
func NewTopologyServiceHandler() *TopologyService {
	topoServer := TopologyService{
		Workloads: make(map[string]*iota.Workload),
	}
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
				nodeOs := "esx"
				switch node.Os {
				case iota.TestBedNodeOs_TESTBED_NODE_OS_ESX:
					nodeOs = "esx"
				case iota.TestBedNodeOs_TESTBED_NODE_OS_LINUX:
					nodeOs = "linux"
				case iota.TestBedNodeOs_TESTBED_NODE_OS_FREEBSD:
					nodeOs = "freebsd"
				}
				cmd := fmt.Sprintf("%s/iota/scripts/boot_naples_v2.py", wsdir)
				cmd += fmt.Sprintf(" --console-ip %s", node.NicConsoleIpAddress)
				cmd += fmt.Sprintf(" --console-port %s", node.NicConsolePort)
				cmd += fmt.Sprintf(" --mnic-ip 169.254.0.1")
				cmd += fmt.Sprintf(" --host-ip %s", node.IpAddress)
				cmd += fmt.Sprintf(" --oob-ip %s", node.NicIpAddress)
				cmd += fmt.Sprintf(" --cimc-ip %s", node.CimcIpAddress)
				cmd += fmt.Sprintf(" --image %s/nic/naples_fw.tar", wsdir)
				cmd += fmt.Sprintf(" --mode hostpin")
				if node.ServerType != "" {
					cmd += fmt.Sprintf("  --server %v", node.ServerType)
				}
				cmd += fmt.Sprintf(" --drivers-pkg %s/platform/gen/drivers-%s-eth.tar.xz", wsdir, nodeOs)
				cmd += fmt.Sprintf(" --gold-firmware-image %s/platform/goldfw/naples/naples_fw.tar", wsdir)
				cmd += fmt.Sprintf(" --uuid %s", node.NicUuid)
				cmd += fmt.Sprintf(" --os %s", nodeOs)

				latestGoldDriver := fmt.Sprintf("%s/platform/hosttools/x86_64/%s/goldfw/latest/drivers-%s-eth.tar.xz", wsdir, nodeOs, nodeOs)
				oldGoldDriver := fmt.Sprintf("%s/platform/hosttools/x86_64/%s/goldfw/old/drivers-%s-eth.tar.xz", wsdir, nodeOs, nodeOs)
				realPath, _ := filepath.EvalSymlinks(latestGoldDriver)
				latestGoldDriverVer := filepath.Base(filepath.Dir(realPath))
				realPath, _ = filepath.EvalSymlinks(oldGoldDriver)
				oldGoldDriverVer := filepath.Base(filepath.Dir(realPath))
				cmd += fmt.Sprintf(" --gold-firmware-latest-version %s", latestGoldDriverVer)
				cmd += fmt.Sprintf(" --gold-drivers-latest-pkg %s", latestGoldDriver)
				cmd += fmt.Sprintf(" --gold-firmware-old-version %s", oldGoldDriverVer)
				cmd += fmt.Sprintf(" --gold-drivers-old-pkg %s", oldGoldDriver)

				if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
					cmd += fmt.Sprintf(" --esx-script %s/iota/bin/iota_esx_setup", wsdir)
					cmd += fmt.Sprintf(" --host-username %s", node.EsxUsername)
					cmd += fmt.Sprintf(" --host-password %s", node.EsxPassword)

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
			if vlans, err = testbed.SetUpTestbedSwitch(req.DataSwitches, req.TestbedId); err != nil {
				log.Errorf("TOPO SVC | InitTestBed | Could not initialize switch id: %d, Err: %v", req.TestbedId, err)
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Switch configuration failed %d. Err: %v", req.TestbedId, err)
				return req, nil
			}
			ts.tbInfo.allocatedVlans = vlans
		}
		ts.tbInfo.resp.AllocatedVlans = ts.tbInfo.allocatedVlans
		ts.tbInfo.id = req.TestbedId
	}

	ts.SSHConfig = testbed.InitSSHConfig(ts.tbInfo.resp.Username, ts.tbInfo.resp.Password)

	// Run init
	ts.ProvisionedNodes = make(map[string]*testbed.TestNode)
	ts.Nodes = make(map[string]*testbed.TestNode)
	// split init nodes into pools of upto 'n' each
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
			n := testbed.TestNode{
				Node: &iota.Node{
					IpAddress: node.IpAddress,
					Name:      node.NodeName,
					//Used if node OS is ESX
					EsxConfig: &iota.VmwareESXConfig{
						IpAddress: node.IpAddress,
						Username:  node.EsxUsername,
						Password:  node.EsxPassword,
					},
					Os: node.Os,
				},
				Os:           node.Os,
				SSHCfg:       ts.SSHConfig,
				CimcIP:       node.CimcIpAddress,
				CimcPassword: node.CimcPassword,
				CimcUserName: node.CimcUsername,
			}

			ts.Nodes[node.IpAddress] = &n
			commonCopyArtifacts := []string{
				ts.tbInfo.resp.VeniceImage,
				ts.tbInfo.resp.NaplesImage,
				ts.tbInfo.resp.NaplesSimImage,
			}

			pool.Go(func() error {
				n := n
				return n.InitNode(req.RebootNodes, ts.SSHConfig, commonCopyArtifacts)
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
				req.Nodes[index].EsxCtrlNodeIpAddress = node.Node.IpAddress
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

func (ts *TopologyService) initTestNodes(ctx context.Context, cfg *ssh.ClientConfig, reboot bool, nodes []*iota.TestBedNode) error {
	pool, ctx := errgroup.WithContext(ctx)

	for _, node := range nodes {
		n := testbed.TestNode{
			Node: &iota.Node{
				IpAddress: node.IpAddress,
				EsxConfig: &iota.VmwareESXConfig{
					IpAddress: node.IpAddress,
					Username:  node.EsxUsername,
					Password:  node.EsxPassword,
				},
			},
			Os:           node.GetOs(),
			SSHCfg:       ts.SSHConfig,
			CimcIP:       node.CimcIpAddress,
			CimcPassword: node.CimcPassword,
			CimcUserName: node.CimcUsername,
		}

		ts.Nodes[node.IpAddress] = &n
		commonCopyArtifacts := []string{
			ts.tbInfo.resp.VeniceImage,
			ts.tbInfo.resp.NaplesImage,
		}

		pool.Go(func() error {
			n := n
			return n.InitNode(reboot, ts.SSHConfig, commonCopyArtifacts)
		})
	}
	if err := pool.Wait(); err != nil {
		return err
	}

	for ip, node := range ts.Nodes {
		for index, reqnode := range nodes {
			if reqnode.IpAddress == ip && reqnode.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
				nodes[index].EsxCtrlNodeIpAddress = node.Node.IpAddress
			}
		}
	}
	return nil
}

func (ts *TopologyService) cleanUpTestNodes(ctx context.Context, cfg *ssh.ClientConfig, reboot bool, nodes []*iota.TestBedNode) error {

	pool, ctx := errgroup.WithContext(ctx)
	for _, node := range nodes {
		n := testbed.TestNode{
			Node: &iota.Node{
				IpAddress: node.IpAddress,
				EsxConfig: &iota.VmwareESXConfig{
					IpAddress: node.IpAddress,
					Username:  node.EsxUsername,
					Password:  node.EsxPassword,
				},
			},
			Os:           node.GetOs(),
			CimcIP:       node.CimcIpAddress,
			CimcPassword: node.CimcPassword,
			CimcUserName: node.CimcUsername,
			SSHCfg:       cfg,
		}
		n.Node.Os = node.GetOs()
		pool.Go(func() error {
			n := n
			return n.CleanUpNode(cfg, reboot)
		})
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

	if len(req.Username) == 0 || len(req.Password) == 0 {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Request must include a user name and password")
		return req, nil
	}
	//ts.TestBedInfo.Username = req.Username
	//ts.TestBedInfo.Password = req.Password

	err := ts.cleanUpTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes())

	if err != nil {
		log.Errorf("TOPO SVC | InitNodes | cleanup Node Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitNodes | InitNodes Call Failed. %s", err.Error())
		return req, nil
	}

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// InitNodes initializes list of nodes
func (ts *TopologyService) InitNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {

	err := ts.cleanUpTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes())

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
	err = ts.initTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes())
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

	err := ts.cleanUpTestNodes(ctx, testbed.InitSSHConfig(req.Username, req.Password), req.RebootNodes, req.GetNodes())

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
	newNodes := []*testbed.TestNode{}
	for _, n := range req.Nodes {

		if _, ok := ts.Nodes[n.IpAddress]; !ok {
			err := fmt.Errorf("TOPO SVC | AddNodes failed, unknown node with IP %v found", n.IpAddress)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Could not get agent URL. Err: %v", err)
			return req, nil
		}

		svcName := n.Name

		tbNode := ts.Nodes[n.IpAddress]

		agentURL, err := tbNode.GetAgentURL()
		if err != nil {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", n.Name, err)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Could not get agent URL. Err: %v", err)
			return req, nil
		}

		c, err := common.CreateNewGRPCClient(svcName, agentURL, common.GrpcMaxMsgSize)
		if err != nil {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", n.Name, err)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Could not create GRPC Connection to IOTA Agent. Err: %v", err)
			return req, nil
		}
		tbNode.AgentClient = iota.NewIotaAgentApiClient(c.Client)

		if _, ok := ts.ProvisionedNodes[n.Name]; ok {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed as node already provisoned : %v", n.Name)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | AddNodes | AddNodes call failed as node already provisoned : %v", n.Name)
			return req, nil
		}

        tbNode.CimcIP = n.CimcIpAddress
        tbNode.CimcUserName = n.CimcUsername
        tbNode.CimcPassword = n.CimcPassword

		ts.ProvisionedNodes[n.Name] = tbNode
		tbNode.Node = n
		log.Infof("Adding provisioned node %v : %v\n", n.Name, tbNode.Node.Name)
		newNodes = append(newNodes, ts.ProvisionedNodes[n.Name])
	}

	// Add nodes
	addNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range newNodes {
			node := node
			pool.Go(func() error {
				return node.AddNode()
			})
		}
		return pool.Wait()
	}
	err := addNodes(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | AddNodes |AddNodes Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	} else {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	}

	for idx, node := range newNodes {
		req.Nodes[idx] = node.RespNode
		if node.RespNode.GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = "Node :" + node.RespNode.GetName() + " : " + node.RespNode.GetNodeStatus().ErrorMsg + "\n"
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
		ApiResponse: &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_STATUS_OK,
		},
	}

	for name, node := range ts.ProvisionedNodes {
		log.Infof("Returning provisioned node %v : %v\n", name, node.RespNode.Name)
		resp.Nodes = append(resp.Nodes, node.RespNode)
	}

	log.Infof("TOPO SVC | DEBUG | GetNodes Returned: %v", resp)
	return &resp, nil
}

// SaveNode save node personality for reboot
func (ts *TopologyService) SaveNode(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	resp := &iota.NodeMsg{}
	return resp, nil
}

// ReloadNodes saves and loads node personality
func (ts *TopologyService) ReloadNodes(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | ReloadNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | ReloadNodes Returned: %v", req)

	rNodes := []*testbed.TestNode{}

	for _, node := range req.NodeMsg.Nodes {
		n, ok := ts.ProvisionedNodes[node.Name]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Reload on unprovisioned node : %v", node.GetName())
			return req, nil
		}
        if node.ApcInfo != nil {
            n.ApcInfo = &iota.ApcInfo {
                Ip : node.ApcInfo.Ip,
                Port : node.ApcInfo.Port,
                Username : node.ApcInfo.Username,
                Password : node.ApcInfo.Password,
            }
        }
        n.RestartMethod = req.RestartMethod
		rNodes = append(rNodes, n)
	}

	reloadNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range rNodes {
			node := node
			pool.Go(func() error {
				return node.ReloadNode(!req.SkipRestore)
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

	for idx, node := range rNodes {
		req.NodeMsg.Nodes[idx] = node.RespNode
		if node.RespNode.GetNodeStatus().ApiStatus != iota.APIResponseType_API_STATUS_OK {
			req.ApiResponse.ErrorMsg = "Node :" + node.RespNode.GetName() + " : " + node.RespNode.GetNodeStatus().ErrorMsg + "\n"
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

	workloadNodes := []*testbed.TestNode{}
	for _, w := range req.Workloads {

		node, ok := ts.ProvisionedNodes[w.NodeName]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("AddWorkloads found to unprovisioned node : %v", w.NodeName)
			return req, nil
		}
		if node.WorkloadInfo == nil {
			node.WorkloadInfo = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
		}
		node.WorkloadInfo.Workloads = append(node.WorkloadInfo.Workloads, w)
		added := false
		for _, workloadNode := range workloadNodes {
			if workloadNode.Node.Name == node.Node.Name {
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
				return node.AddWorkloads(node.WorkloadInfo)
			})

		}
		return pool.Wait()
	}

	resetAddWorkloads := func() {
		for _, node := range workloadNodes {
			node.WorkloadInfo = nil
			node.WorkloadResp = nil
		}
	}

	defer resetAddWorkloads()
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
		for _, respWload := range node.WorkloadResp.Workloads {
			for index, reqWload := range req.Workloads {
				if reqWload.GetNodeName() == node.Node.GetName() && reqWload.WorkloadName == respWload.WorkloadName {
					req.Workloads[index] = respWload
					if respWload.WorkloadStatus != nil && respWload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
						req.ApiResponse.ErrorMsg = respWload.WorkloadStatus.ErrorMsg
						log.Errorf("TOPO SVC | AddWorkloads | Workload add %v failed with  %v", respWload.GetWorkloadName(), req.ApiResponse.ErrorMsg)
					}
					break
				}
			}
		}

	}

	// save workload info
	for _, w := range req.Workloads {
		ts.Workloads[w.WorkloadName] = w
	}

	return req, nil
}

// GetWorkloads returns list of workloads
func (ts *TopologyService) GetWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	var workloads []*iota.Workload
	log.Infof("TOPO SVC | DEBUG | GetWorkloads. Received Request Msg: %#v", req)

	for _, w := range ts.Workloads {
		workloads = append(workloads, w)
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

	workloadNodes := []*testbed.TestNode{}
	for _, w := range req.Workloads {

		node, ok := ts.ProvisionedNodes[w.NodeName]
		if !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("DeleteWorkloads found to unprovisioned node : %v", w.NodeName)
			return req, nil
		}
		if node.WorkloadInfo == nil {
			node.WorkloadInfo = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
			node.WorkloadResp = &iota.WorkloadMsg{WorkloadOp: iota.Op_ADD, Workloads: []*iota.Workload{}}
		}
		node.WorkloadInfo.Workloads = append(node.WorkloadInfo.Workloads, w)
		added := false
		for _, workloadNode := range workloadNodes {
			if workloadNode.Node.Name == node.Node.Name {
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
				return node.DeleteWorkloads(node.WorkloadInfo)
			})

		}
		return pool.Wait()
	}

	resetDeleteWorkloads := func() {
		for _, node := range workloadNodes {
			node.WorkloadInfo = nil
			node.WorkloadResp = nil
		}
	}

	defer resetDeleteWorkloads()

	err := deleteWorkloads(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | DeleteWorkloads |DeleteWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | DeleteWorkloads |DeleteWorkloads Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	} else {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	}

	//Assoicate response
	for _, node := range workloadNodes {
		for _, respWload := range node.WorkloadResp.Workloads {
			for index, reqWload := range req.Workloads {
				if reqWload.GetNodeName() == node.Node.GetName() && reqWload.WorkloadName == respWload.WorkloadName {
					req.Workloads[index] = respWload
					if respWload.WorkloadStatus != nil && respWload.WorkloadStatus.ApiStatus != iota.APIResponseType_API_STATUS_OK {
						req.ApiResponse.ErrorMsg = respWload.WorkloadStatus.ErrorMsg
						log.Errorf("TOPO SVC | DeleteWorkloads | Workload add %v failed with  %v", respWload.GetWorkloadName(), req.ApiResponse.ErrorMsg)
					}
					break
				}
			}
		}

	}

	for _, w := range req.Workloads {
		delete(ts.Workloads, w.WorkloadName)
	}

	return req, nil
}

func (ts *TopologyService) runParallelTrigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	triggerNodes := []*testbed.TestNode{}
	triggerResp := &iota.TriggerMsg{TriggerMode: req.GetTriggerMode(),
		TriggerOp: req.GetTriggerOp()}

	triggerInfoMap := new(sync.Map)
	triggerRespMap := new(sync.Map)
	var triggerInfo *iota.TriggerMsg
	triggerCmdIndexMap := make(map[string][]int)

	for index, cmd := range req.GetCommands() {
		node, _ := ts.ProvisionedNodes[cmd.GetNodeName()]
		if item, ok := triggerInfoMap.Load(cmd.GetNodeName()); !ok {
			triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
				TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
			triggerInfoMap.Store(cmd.GetNodeName(), triggerMsg)
			triggerInfo = triggerMsg
			triggerCmdIndexMap[cmd.GetNodeName()] = []int{}
		} else {
			triggerInfo = item.(*iota.TriggerMsg)
			triggerInfo.Commands = append(triggerInfo.Commands, cmd)
		}

		triggerCmdIndexMap[cmd.GetNodeName()] = append(triggerCmdIndexMap[cmd.GetNodeName()], index)
		//Just copy the request for now
		triggerResp.Commands = append(triggerResp.Commands, cmd)
		added := false
		for _, triggerNode := range triggerNodes {
			if triggerNode.Node.Name == node.Node.Name {
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
				item, _ := triggerInfoMap.Load(node.Node.Name)
				triggerMsg := item.(*iota.TriggerMsg)
				triggerResp, err := node.TriggerWithContext(ctx3, triggerMsg)
				triggerRespMap.Store(node.Node.Name, triggerResp)
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
		item, _ := triggerRespMap.Load(node.Node.Name)
		triggerMsg := item.(*iota.TriggerMsg)
		for index, cmd := range triggerMsg.Commands {
			realIndex := triggerCmdIndexMap[node.Node.Name][index]
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
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
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
		nodeResp, err := node.AgentClient.CheckHealth(ctx, nodeHealth)
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

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	if req.Direction == iota.CopyDirection_DIR_IN {

		dstDir := common.DstIotaEntitiesDir + "/" + req.GetEntityName() + "/" + req.GetDestDir() + "/"
		if err := node.CopyTo(ts.SSHConfig, dstDir, req.GetFiles()); err != nil {
			log.Errorf("TOPO SVC | EntityCopy | Failed to copy files to entity:  %v on node : %v (%v)",
				req.GetEntityName(), node.Node.Name, err.Error())
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Failed to copy files to entity:  %v on node : %v",
				req.GetEntityName(), node.Node.Name)
		}
	} else if req.Direction == iota.CopyDirection_DIR_OUT {
		files := []string{}
		srcDir := common.DstIotaEntitiesDir + "/" + req.GetEntityName() + "/"
		for _, file := range req.GetFiles() {
			files = append(files, srcDir+file)
		}

		if err := node.CopyFrom(ts.SSHConfig, req.GetDestDir(), files); err != nil {
			log.Errorf("TOPO SVC | EntityCopy | Failed to copy files from entity:  %v on node : %v (%v)",
				req.GetEntityName(), node.Node.Name, err.Error())
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		}

	} else {
		errMsg := fmt.Sprintf("No direction specified for entity copy")
		req.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
			ErrorMsg: errMsg}
		return req, nil
	}

	return req, nil
}
