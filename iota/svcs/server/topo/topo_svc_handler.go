package topo

import (
	"context"
	"fmt"
	"net"

	"github.com/pensando/sw/iota/svcs/common"

	"golang.org/x/crypto/ssh"

	"golang.org/x/sync/errgroup"

	log "github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/server/topo/testbed"
)

// TopologyService implements topology service API
type TopologyService struct {
	//pool errgroup.Group
	SSHConfig   *ssh.ClientConfig
	TestBedInfo *iota.TestBedMsg //server
	Nodes       []*testbed.Node
	//topoCtx TopoContext
}

// NewTopologyServiceHandler Topo service handle
func NewTopologyServiceHandler() *TopologyService {
	var topoServer TopologyService
	return &topoServer
}

// InitTestBed does initiates a test bed
func (ts *TopologyService) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	var vlans []uint32
	var err error
	ts.TestBedInfo = req

	// Allocate VLANs for the test bed
	if vlans, err = testbed.AllocateVLANS(ts.TestBedInfo.SwitchPortId); err != nil {
		log.Errorf("TOPO SVC | InitTestBed | Could not allocate VLANS from the switchport id: %d, Err: %v", ts.TestBedInfo.SwitchPortId, err)
		return nil, fmt.Errorf("could not allocate VLANS from the switchport id: %d. Err: %v", ts.TestBedInfo.SwitchPortId, err)
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
			n := testbed.Node{
				NodeName:  nodeName,
				IpAddress: ipAddress,
			}
			copyArtifacts := []string{
				common.IotaAgentBinaryPath,
				ts.TestBedInfo.VeniceImage,
				ts.TestBedInfo.NaplesImage,
				//ts.TestBedInfo.DriverSources,
			}

			pool.Go(func() error {
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
	resp := iota.TestBedMsg{}
	//var entities []Entity
	//for _, ip := range req.IpAddress {
	//	entities = append(entities, NewNodeEntity(ip, ip, ts.logger))
	//}
	//if err := ts.topoCtx.CleanUpTestBed(entities); err != nil {
	//	ts.log("Error in cleaning up test bed : " + err.Error())
	//	return &iota-server.TestBedMsg{ApiResponse: &iota-server.IotaAPIResponse{ApiStatus: iota-server.IotaAPIResponse_API_STATUS_OK}}, nil
	//}
	//
	//ts.log("Succesfully cleaned up test bed")
	//return &iota-server.TestBedMsg{ApiResponse: &iota-server.IotaAPIResponse{ApiStatus: iota-server.IotaAPIResponse_API_STATUS_OK}}, nil
	return &resp, nil
}

// AddNodes adds nodes to the topology
func (ts *TopologyService) AddNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	resp := &iota.NodeMsg{}

	return resp, nil
}

// DeleteNodes deletes a node from the topology
func (ts *TopologyService) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	resp := &iota.NodeMsg{}
	return resp, nil
}

// GetNodes returns the current topology information
func (ts *TopologyService) GetNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	resp := &iota.NodeMsg{}
	return resp, nil
}

// AddWorkloads adds a workload on a given node
func (ts *TopologyService) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	resp := &iota.WorkloadMsg{}

	return resp, nil
}

// DeleteWorkloads deletes a workload
func (ts *TopologyService) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	resp := &iota.WorkloadMsg{}

	return resp, nil
}

// Trigger triggers a workload
func (ts *TopologyService) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	resp := &iota.TriggerMsg{}

	return resp, nil
}

// CheckClusterHealth checks the e2e cluster health
func (ts *TopologyService) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (*iota.ClusterHealthMsg, error) {
	resp := &iota.ClusterHealthMsg{}

	return resp, nil
}
