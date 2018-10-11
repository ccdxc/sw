package topo

import (
	"context"
	"fmt"
	"net"

	"github.com/pensando/sw/iota/svcs/common"

	"golang.org/x/crypto/ssh"

	"golang.org/x/sync/errgroup"

	"github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/server/topo/testbed"
)

// TopologyService implements topology service API
type TopologyService struct {
	SSHConfig   *ssh.ClientConfig
	TestBedInfo *iota.TestBedMsg //server
	Nodes       []*testbed.TestNode
}

// NewTopologyServiceHandler Topo service handle
func NewTopologyServiceHandler() *TopologyService {
	var topoServer TopologyService
	return &topoServer
}

// InitTestBed does initiates a test bed
func (ts *TopologyService) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | InitTestBed. Received Request Msg: %v", req)
	var vlans []uint32
	var err error
	ts.TestBedInfo = req

	// Preflight checks
	if len(req.IpAddress) == 0 {
		log.Errorf("TOPO SVC | InitTestBed | No IP Addresses present. Err: %v", ts.TestBedInfo.SwitchPortId, err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("request message doesn't have any ip addresses")
		return req, nil
	}

	if len(req.User) == 0 || len(req.Passwd) == 0 {
		log.Errorf("TOPO SVC | InitTestBed | User creds to access the vms are missing.")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = "user creds are missing to access the VMs"
		return req, nil
	}

	if len(req.ControlIntf) == 0 {
		log.Errorf("")
	}

	// Allocate VLANs for the test bed
	if vlans, err = testbed.AllocateVLANS(ts.TestBedInfo.SwitchPortId); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Could not allocate VLANS from the switchport id: %d, Err: %v", ts.TestBedInfo.SwitchPortId, err)
		req.ApiResponse.ErrorMsg = fmt.Sprintf("could not allocate VLANS from the switchport id: %d. Err: %v", ts.TestBedInfo.SwitchPortId, err)
		return req, nil
	}
	ts.TestBedInfo.AllocatedVlans = vlans

	ts.SSHConfig = testbed.InitSSHConfig(ts.TestBedInfo.User, ts.TestBedInfo.Passwd)

	// Run init
	initTestBed := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for idx, ipAddress := range req.IpAddress {
			nodeName := fmt.Sprintf("iota-node-%d", idx)
			ipAddr := net.ParseIP(ipAddress)
			if len(ipAddr) == 0 {
				log.Errorf("TOPO SVC | InitTestBed | Invalid IP Address format. %v", ipAddress)
				return fmt.Errorf("invalid ip address format. %v", ipAddress)
			}

			n := testbed.TestNode{
				Node: &iota.Node{
					IpAddress: ipAddress,
					Name:      nodeName,
				},
			}

			ts.Nodes = append(ts.Nodes, &n)
			copyArtifacts := []string{
				common.IotaAgentBinaryPath,
				ts.TestBedInfo.VeniceImage,
				ts.TestBedInfo.NaplesImage,
			}

			pool.Go(func() error {
				n := n
				return n.InitNode(ts.SSHConfig, copyArtifacts)
			})
		}
		return pool.Wait()
	}
	err = initTestBed(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Init Test Bed Call Failed. %v", err)
		return nil, err
	}
	return ts.TestBedInfo, err
}

// CleanUpTestBed cleans up a testbed
func (ts *TopologyService) CleanUpTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | CleanUpTestBed. Received Request Msg: %v", req)

	resp := iota.TestBedMsg{}
	return &resp, nil
}

// AddNodes adds nodes to the topology
func (ts *TopologyService) AddNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddNodes. Received Request Msg: %v", req)

	if req.NodeOp != iota.Op_ADD {
		log.Errorf("TOPO SVC | AddNodes | AddNodes call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("AddNodes API must specify Add operation. Found: %v", req.NodeOp)
		return req, nil
	}

	// Prep Topo
	for idx, n := range req.Nodes {
		svcName := n.Name
		agentURL := fmt.Sprintf("%s:%d", n.IpAddress, common.IotaAgentPort)
		c, err := common.CreateNewGRPCClient(svcName, agentURL)

		if err != nil {
			log.Errorf("TOPO SVC | AddNodes | AddNodes call failed to establish GRPC Connection to Agent running on Node: %v. Err: %v", n.Name, err)
			return nil, err
		}

		ts.Nodes[idx] = &testbed.TestNode{
			Node:        n,
			AgentClient: iota.NewIotaAgentApiClient(c.Client),
		}
	}

	// Add nodes
	addNodes := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range ts.Nodes {
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
		return nil, err
	}

	// TODO return fully formed resp here
	return req, nil
}

// DeleteNodes deletes a node from the topology
func (ts *TopologyService) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DeleteNodes. Received Request Msg: %v", req)

	resp := &iota.NodeMsg{}
	return resp, nil
}

// GetNodes returns the current topology information
func (ts *TopologyService) GetNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | GetNodes. Received Request Msg: %v", req)

	resp := &iota.NodeMsg{}
	return resp, nil
}

// AddWorkloads adds a workload on a given node
func (ts *TopologyService) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddWorkloads. Received Request Msg: %v", req)

	if req.WorkloadOp != iota.Op_ADD {
		log.Errorf("TOPO SVC | AddWorkloads | AddWorkloads call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("AddWorkloads must specify Op_Add for workload op. Found: %v", req.WorkloadOp)
		return req, nil
	}

	for _, w := range req.Workloads {
		for idx, n := range ts.Nodes {
			if w.NodeName == n.Node.Name {
				ts.Nodes[idx].Workloads = append(ts.Nodes[idx].Workloads, w)
			}
		}
	}

	// Add workloads
	addWorkloads := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range ts.Nodes {
			node := node
			//for range node.Workloads {
			pool.Go(func() error {
				return node.AddWorkload()
			})
		}
		return pool.Wait()
	}
	err := addWorkloads(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | AddWorkloads |AddWorkloads Call Failed. %v", err)
		return nil, err
	}

	// TODO return fully formed resp here
	return req, nil
}

// DeleteWorkloads deletes a workload
func (ts *TopologyService) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DeleteWorkloads. Received Request Msg: %v", req)

	resp := &iota.WorkloadMsg{}

	return resp, nil
}

// Trigger triggers a workload
func (ts *TopologyService) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	log.Infof("TOPO SVC | DEBUG | Trigger. Received Request Msg: %v", req)

	if req.TriggerOp == iota.TriggerOp_TYPE_NONE {
		log.Errorf("TOPO SVC | Trigger | Trigger call failed")
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
		req.ApiResponse.ErrorMsg = fmt.Sprintf("Trigger must specify TriggerOp for workload op. Found: %v", req.TriggerOp)
		return req, nil
	}

	for idx, n := range ts.Nodes {
		if req.NodeName == n.Node.Name {
			ts.Nodes[idx].TriggerInfo = req
		}
	}

	// Triggers
	triggers := func(ctx context.Context) error {
		pool, ctx := errgroup.WithContext(ctx)

		for _, node := range ts.Nodes {
			node := node
			if node.Node.Name == req.NodeName {
				pool.Go(func() error {
					return node.Trigger()
				})
			}

		}
		return pool.Wait()
	}
	err := triggers(context.Background())
	if err != nil {
		log.Errorf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		return nil, err
	}

	// TODO return fully formed resp here
	return req, nil
}

// CheckClusterHealth checks the e2e cluster health
func (ts *TopologyService) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (*iota.ClusterHealthMsg, error) {
	resp := &iota.ClusterHealthMsg{}

	return resp, nil
}
