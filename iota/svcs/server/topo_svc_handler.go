package server

import (
	"context"
	"io"
	"log"
	"os"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

type TopoServer struct {
	server
	topoCtx TopoContext
}

// NewTopologyService Topo service handle
func NewTopologyService() *TopoServer {
	var topoServer TopoServer
	topoServer.init()
	topoServer.topoCtx = NewTopoContext(topoServer.logger)
	return &topoServer
}

func (ts *TopoServer) init() {

	_ = os.Mkdir(LogDir, 0777)
	// Init logger
	file, err := os.OpenFile(LogDir+"/"+"Topo.log", os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
	if err != nil {
		log.Fatalln("Failed to open log file", "file.txt", ":", err)
	}
	multi := io.MultiWriter(file, os.Stdout)
	ts.logger = log.New(multi, "Topology-Service:", log.Ldate|log.Ltime)

}

// InitTestBed does initiates a test bed
func (ts *TopoServer) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {

	var entities []Entity
	for _, ip := range req.IpAddress {
		entities = append(entities, NewNodeEntity(ip, ip, ts.logger))
	}
	if err := ts.topoCtx.InitTestBed(entities); err != nil {
		ts.log("Error in intialzing test bed : " + err.Error())
		return &iota.TestBedMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.IotaAPIResponse_API_STATUS_OK}}, nil
	}

	ts.log("Succesfully initialized test bed")
	return &iota.TestBedMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.IotaAPIResponse_API_STATUS_OK}}, nil
}

// CleanUpTestBed cleans up a testbed
func (ts *TopoServer) CleanUpTestBed(ctx context.Context, req *iota.TestBedMsg) (*iota.TestBedMsg, error) {
	var entities []Entity
	for _, ip := range req.IpAddress {
		entities = append(entities, NewNodeEntity(ip, ip, ts.logger))
	}
	if err := ts.topoCtx.CleanUpTestBed(entities); err != nil {
		ts.log("Error in cleaning up test bed : " + err.Error())
		return &iota.TestBedMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.IotaAPIResponse_API_STATUS_OK}}, nil
	}

	ts.log("Succesfully cleaned up test bed")
	return &iota.TestBedMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.IotaAPIResponse_API_STATUS_OK}}, nil
}

// AddNodes adds nodes to the topology
func (ts *TopoServer) AddNodes(ctx context.Context, req *iota.NodeMsg) (resp *iota.NodeMsg, err error) {
	return
}

// DeleteNodes deletes a node from the topology
func (ts *TopoServer) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (resp *iota.NodeMsg, err error) {
	return
}

// GetNodes returns the current topology information
func (ts *TopoServer) GetNodes(ctx context.Context, req *iota.NodeMsg) (resp *iota.NodeMsg, err error) {
	return
}

// AddWorkloads adds a workload on a given node
func (ts *TopoServer) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (resp *iota.WorkloadMsg, err error) {
	return
}

// DeleteWorkloads deletes a workload
func (ts *TopoServer) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (resp *iota.WorkloadMsg, err error) {
	return
}

// Trigger triggers a workload
func (ts *TopoServer) Trigger(ctx context.Context, req *iota.TriggerMsg) (resp *iota.TriggerMsg, err error) {
	return
}

// CheckClusterHealth checks the e2e cluster health
func (ts *TopoServer) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (resp *iota.ClusterHealthMsg, err error) {
	return
}
