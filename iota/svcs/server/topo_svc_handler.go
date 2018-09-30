package server

import (
	"context"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

// TopologyService implements topology service API
type TopologyService struct {
	//server
	//topoCtx TopoContext
}

// NewTopologyServiceHandler Topo service handle
func NewTopologyServiceHandler() *TopologyService {
	var topoServer TopologyService
	//topoServer.init()
	//topoServer.topoCtx = NewTopoContext(topoServer.logger)
	return &topoServer
}

//func (ts *TopologyService) init() {
//
//	_ = os.Mkdir(LogDir, 0777)
//	// Init logger
//	file, err := os.OpenFile(LogDir+"/"+"Topo.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
//	if err != nil {
//		log.Fatalln("Failed to open log file", "file.txt", ":", err)
//	}
//	multi := io.MultiWriter(file, os.Stdout)
//	ts.logger = log.New(multi, "Topology-Service:", log.Ldate|log.Ltime)
//
//}

// InitTestBed does initiates a test bed
func (ts *TopologyService) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	resp := iota.TestBedMsg{}
	//var entities []Entity
	//for _, ip := range req.IpAddress {
	//	entities = append(entities, NewNodeEntity(ip, ip, ts.logger))
	//}
	//if err := ts.topoCtx.InitTestBed(entities); err != nil {
	//	ts.log("Error in intialzing test bed : " + err.Error())
	//	return &iota-server.TestBedMsg{ApiResponse: &iota-server.IotaAPIResponse{ApiStatus: iota-server.IotaAPIResponse_API_STATUS_OK}}, nil
	//}
	//
	//ts.log("Succesfully initialized test bed")
	//return &iota-server.TestBedMsg{ApiResponse: &iota-server.IotaAPIResponse{ApiStatus: iota-server.IotaAPIResponse_API_STATUS_OK}}, nil
	return &resp, nil
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
