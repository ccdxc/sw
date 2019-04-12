package topo

import (
	"context"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"

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
	TestBedInfo      *iota.TestBedMsg //server
	Nodes            []*testbed.TestNode
	Workloads        map[string]*iota.Workload // list of workloads
	ProvisionedNodes map[string]*testbed.TestNode
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
			log.Errorf("Error executing boot_naples_v2.py script. Err: %v", err)
			stdout, _ := exec.Command("sh", "-c", "tail -n 100 *upgrade.log").CombinedOutput()
			fmt.Println(stdout)
			return nil, err
		}
	}

	log.Infof("Recovering naples nodes complete...")
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	return req, nil
}

// InitTestBed does initiates a test bed
func (ts *TopologyService) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | InitTestBed. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | InitTestBed Returned: %v", req)
	var vlans []uint32
	var err error
	ts.TestBedInfo = req
	ts.Nodes = []*testbed.TestNode{}

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

	if ts.TestBedInfo.DataSwitches != nil && ts.TestBedInfo.TestbedId != 0 {
		// Allocate VLANs for the test bed
		if vlans, err = testbed.SetUpTestbedSwitch(ts.TestBedInfo.DataSwitches, ts.TestBedInfo.TestbedId); err != nil {
			log.Errorf("TOPO SVC | InitTestBed | Could not initialize switch id: %d, Err: %v", ts.TestBedInfo.TestbedId, err)
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Switch configuration failed %d. Err: %v", ts.TestBedInfo.TestbedId, err)
			return req, nil
		}
		ts.TestBedInfo.AllocatedVlans = vlans
	}

	ts.SSHConfig = testbed.InitSSHConfig(ts.TestBedInfo.Username, ts.TestBedInfo.Password)

	// Run init
	ts.ProvisionedNodes = make(map[string]*testbed.TestNode)
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
				},
				Os:     node.Os,
				SSHCfg: ts.SSHConfig,
			}

			ts.Nodes = append(ts.Nodes, &n)
			commonCopyArtifacts := []string{
				ts.TestBedInfo.VeniceImage,
				ts.TestBedInfo.NaplesImage,
			}

			pool.Go(func() error {
				n := n
				return n.InitNode(ts.SSHConfig, common.DstIotaAgentDir, commonCopyArtifacts)
			})
		}
		err = pool.Wait()
		if err != nil {
			break
		}
	}
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Init Test Bed Call Failed. %v", err)
		ts.TestBedInfo.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		ts.TestBedInfo.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC InitTestBed | Init Test Bed Call Failed. %s", err.Error())
		return ts.TestBedInfo, nil
	}
	for index, node := range ts.Nodes {
		if node.Os == iota.TestBedNodeOs_TESTBED_NODE_OS_ESX {
			req.Nodes[index].EsxCtrlNodeIpAddress = node.Node.IpAddress
		}
	}

	log.Infof("Init testbed successful")
	ts.TestBedInfo.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return ts.TestBedInfo, nil
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

	ts.SSHConfig = testbed.InitSSHConfig(req.Username, req.Password)
	// Run clean up
	cleanupTestBed := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range req.Nodes {
			n := testbed.TestNode{
				Node: &iota.Node{
					IpAddress: node.IpAddress,
					EsxConfig: &iota.VmwareESXConfig{
						IpAddress: node.IpAddress,
						Username:  node.EsxUsername,
						Password:  node.EsxPassword,
					},
				},
				Os: node.GetOs(),
			}

			pool.Go(func() error {
				n := n
				return n.CleanUpNode(ts.SSHConfig)
			})
		}
		return pool.Wait()
	}
	err := cleanupTestBed(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | CleanupTestBed | Cleanup Test Bed Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Topo SVC CleanupTestBed | Clean up Test Bed Call Failed. %s", err.Error())
		return req, nil
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
		svcName := n.Name

		tbNode := &testbed.TestNode{
			Node:   n,
			SSHCfg: ts.SSHConfig,
		}

		if agentURL, err := tbNode.GetAgentURL(); err != nil {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", n.Name, err)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Could not get agent URL. Err: %v", err)
			return req, nil
		} else {
			c, err := common.CreateNewGRPCClient(svcName, agentURL, common.GrpcMaxMsgSize)
			if err != nil {
				log.Errorf("TOPO SVC | AddNodes | AddNodes call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", n.Name, err)
				req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
				req.ApiResponse.ErrorMsg = fmt.Sprintf("Could not create GRPC Connection to IOTA Agent. Err: %v", err)
				return req, nil
			}
			tbNode.AgentClient = iota.NewIotaAgentApiClient(c.Client)
		}

		if _, ok := ts.ProvisionedNodes[n.Name]; ok {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed as node already provisoned : %v", n.Name)
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | AddNodes | AddNodes call failed as node already provisoned : %v", n.Name)
			return req, nil
		}

		ts.ProvisionedNodes[n.Name] = tbNode
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
	defer log.Infof("TOPO SVC | DEBUG | GetNodes Returned: %v", req)

	resp := iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_STATUS_OK,
		},
	}

	for _, node := range ts.ProvisionedNodes {
		resp.Nodes = append(resp.Nodes, node.Node)
	}

	return &resp, nil
}

// SaveNode save node personality for reboot
func (ts *TopologyService) SaveNode(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	resp := &iota.NodeMsg{}
	return resp, nil
}

// ReloadNode saves and loads node personality
func (ts *TopologyService) ReloadNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | ReloadNodes. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | ReloadNodes Returned: %v", req)

	rNodes := []*testbed.TestNode{}

	for _, node := range req.Nodes {
		if n, ok := ts.ProvisionedNodes[node.Name]; !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Reload on unprovisioned node : %v", node.GetName())
			return req, nil
		} else {
			rNodes = append(rNodes, n)
		}
	}

	reloadNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range rNodes {
			node := node
			pool.Go(func() error {
				return node.ReloadNode()
			})
		}
		return pool.Wait()
	}

	err := reloadNodes(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | ReloadNodes | ReloadNodes Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
	} else {
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	}

	for idx, node := range rNodes {
		req.Nodes[idx] = node.RespNode
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
	for index, cmd := range req.GetCommands() {
		node, _ := ts.ProvisionedNodes[cmd.GetNodeName()]
		if node.TriggerInfo == nil {
			node.TriggerInfo = &iota.TriggerMsg{Commands: []*iota.Command{cmd},
				TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
		} else {
			node.TriggerInfo.Commands = append(node.TriggerInfo.Commands, cmd)
		}
		node.CmdIndexes = append(node.CmdIndexes, index)
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
	triggers := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)
		for _, node := range triggerNodes {
			node := node
			pool.Go(func() error {
				if err := node.Trigger(); err != nil {
					return err
				}
				return nil
			})

		}
		return pool.Wait()
	}

	resetTriggers := func() {
		for _, node := range triggerNodes {
			node.TriggerInfo = nil
			node.TriggerResp = nil
			node.CmdIndexes = []int{}
		}
	}

	err := triggers(context.Background())
	defer resetTriggers()
	if err != nil {
		log.Errorf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		return req, nil

	}

	for _, node := range triggerNodes {
		for index, cmd := range node.TriggerResp.GetCommands() {
			realIndex := node.CmdIndexes[index]
			triggerResp.Commands[realIndex] = cmd
		}
	}

	triggerResp.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}

	return triggerResp, nil
}

func (ts *TopologyService) runSerialTrigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	for cidx, cmd := range req.GetCommands() {
		node, _ := ts.ProvisionedNodes[cmd.GetNodeName()]
		node.TriggerInfo = &iota.TriggerMsg{Commands: []*iota.Command{cmd},
			TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
		node.TriggerResp = nil
		if err := node.Trigger(); err != nil {

			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | Trigger | RunSerialTrigger Call Failed. %v", err)
			//clean up
			node.TriggerInfo = nil
			node.TriggerResp = nil
			return req, nil
		}
		/* Only one command sent anyway */
		req.Commands[cidx] = node.TriggerResp.GetCommands()[0]
		node.TriggerInfo = nil
		node.TriggerResp = nil
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

	for _, reqNode := range req.Nodes {
		if node, ok := ts.ProvisionedNodes[reqNode.Name]; ok {
			nodeHealth := &iota.NodeHealth{NodeName: reqNode.Name}
			nodeResp, err := node.AgentClient.CheckHealth(ctx, nodeHealth)
			if err == nil {
				resp.Health = append(resp.Health, nodeResp)
			}
		} else {
			resp.Health = append(resp.Health, &iota.NodeHealth{NodeName: reqNode.Name, HealthCode: iota.NodeHealth_NOT_PROVISIONED})
		}
	}

	resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return resp, nil
}

// WorkloadCopy does copy of items to/from entity.
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
			log.Errorf("TOPO SVC | EntityCopy | Failed to copy files to entity:  %v on node : %v",
				req.GetEntityName(), node.Node.Name)
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
			log.Errorf("TOPO SVC | EntityCopy | Failed to copy files from entity:  %v on node : %v",
				req.GetEntityName(), node.Node.Name)
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
