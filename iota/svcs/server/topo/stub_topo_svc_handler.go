package topo

import (
	"context"

	"github.com/pensando/sw/venice/utils/log"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

// StubTopologyService implements topology service API
type StubTopologyService struct {
}

// NewStubTopologyService Topo service handle
func NewStubTopologyService() *StubTopologyService {
	var topoServer StubTopologyService
	return &topoServer
}

// InstallImage recovers the Naples nodes and installs an image
func (ts *StubTopologyService) InstallImage(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | InstallImage. Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// InitTestBed does initiates a test bed
func (ts *StubTopologyService) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | InitTestBed. Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// GetTestBed get testbed info
func (ts *StubTopologyService) GetTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | InitTestBed. Received Request Msg: %v", req)
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// CleanUpTestBed cleans up a testbed
func (ts *StubTopologyService) CleanUpTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	log.Infof("TOPO SVC | DEBUG | CleanUpTestBed. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK

	return req, nil
}

// InitNodes initializes test nodes
func (ts *StubTopologyService) InitNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {
	return nil, nil
}

// CleanNodes cleans up list of nodes and removes association
func (ts *StubTopologyService) CleanNodes(ctx context.Context, req *iota.TestNodesMsg) (*iota.TestNodesMsg, error) {
	return nil, nil
}

// AddNodes adds nodes to the topology
func (ts *StubTopologyService) AddNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddNodes. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// IpmiNodeAction runs any ipmi power command on node
func (*StubTopologyService) IpmiNodeAction(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	resp := &iota.ReloadMsg{}
	return resp, nil
}

// ReloadNodes saves and loads node personality
func (*StubTopologyService) ReloadNodes(ctx context.Context, req *iota.ReloadMsg) (*iota.ReloadMsg, error) {
	resp := &iota.ReloadMsg{}
	return resp, nil
}

// MoveWorkloads move workloads
func (*StubTopologyService) MoveWorkloads(ctx context.Context, req *iota.WorkloadMoveMsg) (*iota.WorkloadMoveMsg, error) {
	resp := &iota.WorkloadMoveMsg{}
	return resp, nil
}

// DeleteNodes deletes a node from the topology
func (ts *StubTopologyService) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DeleteNodes. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// GetNodes returns the current topology information
func (ts *StubTopologyService) GetNodes(ctx context.Context, req *iota.NodeMsg) (*iota.NodeMsg, error) {
	log.Infof("TOPO SVC | DEBUG | GetNodes. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// AddWorkloads adds a workload on a given node
func (ts *StubTopologyService) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | AddWorkloads. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	// TODO return fully formed resp here
	return req, nil
}

// GetWorkloads gets current list of workloads
func (ts *StubTopologyService) GetWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | GetWorkloads. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	// TODO return fully formed resp here
	return req, nil
}

// GetWorkloads gets current list of workloads
func (ts *StubTopologyService) RemoveNetworks(ctx context.Context, req *iota.NetworksMsg) (*iota.NetworksMsg, error) {
	log.Infof("TOPO SVC | DEBUG | RemoveNetworks Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	// TODO return fully formed resp here
	return req, nil
}

// DeleteWorkloads deletes a workload
func (ts *StubTopologyService) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	log.Infof("TOPO SVC | DEBUG | DeleteWorkloads. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// CheckClusterHealth checks the e2e cluster health
func (ts *StubTopologyService) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (*iota.ClusterHealthMsg, error) {

	resp := &iota.ClusterHealthMsg{}

	resp.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return resp, nil
}

// EntityCopy does copy of items to/from entity.
func (ts *StubTopologyService) EntityCopy(ctx context.Context, req *iota.EntityCopyMsg) (*iota.EntityCopyMsg, error) {

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// Trigger triggers a workload
func (ts *StubTopologyService) Trigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	log.Infof("TOPO SVC | DEBUG | Trigger. Received Request Msg: %v", req)

	for _, cmd := range req.Commands {
		cmd.Handle = "dummy"
	}
	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// DoSwitchOperation do switch operation
func (ts *StubTopologyService) DoSwitchOperation(ctx context.Context, req *iota.SwitchMsg) (*iota.SwitchMsg, error) {
	log.Infof("TOPO SVC | DEBUG | SwitchMsg. Received Request Msg: %v", req)

	req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
	return req, nil
}

// DownloadAssets pulls assets
func (ts *StubTopologyService) DownloadAssets(ctx context.Context, req *iota.DownloadAssetsMsg) (*iota.DownloadAssetsMsg, error) {
    log.Infof("TOPO SVC | DEBUG | DownloadAssets. Received Request Msg: %v", req)

    req.ApiResponse.ApiStatus = iota.APIResponseType_API_STATUS_OK
    return req, nil
}
