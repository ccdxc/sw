package server

import (
	"context"

	iota "github.com/pensando/sw/iota/protos/gogen"
)

type TopoServer struct{}

//
func NewTopologyService() *TopoServer {
	var topoServer TopoServer
	return &topoServer
}

// InitTestBed does initiates a test bed
func (c *TopoServer) InitTestBed(ctx context.Context, req *iota.TestBedMsg) (resp *iota.TestBedMsg, err error) {
	return
}

// CleanUpTestBed cleans up a testbed
func (c *TopoServer) CleanUpTestBed(ctx context.Context, req *iota.TestBedMsg) (resp *iota.TestBedMsg, err error) {
	return
}

// AddNodes adds nodes to the topology
func (c *TopoServer) AddNodes(ctx context.Context, req *iota.NodeMsg) (resp *iota.NodeMsg, err error) {
	return
}

// DeleteNodes deletes a node from the topology
func (c *TopoServer) DeleteNodes(ctx context.Context, req *iota.NodeMsg) (resp *iota.NodeMsg, err error) {
	return
}

// GetNodes returns the current topology information
func (c *TopoServer) GetNodes(ctx context.Context, req *iota.NodeMsg) (resp *iota.NodeMsg, err error) {
	return
}

// AddWorkloads adds a workload on a given node
func (c *TopoServer) AddWorkloads(ctx context.Context, req *iota.WorkloadMsg) (resp *iota.WorkloadMsg, err error) {
	return
}

// DeleteWorkloads deletes a workload
func (c *TopoServer) DeleteWorkloads(ctx context.Context, req *iota.WorkloadMsg) (resp *iota.WorkloadMsg, err error) {
	return
}

// Trigger triggers a workload
func (c *TopoServer) Trigger(ctx context.Context, req *iota.TriggerMsg) (resp *iota.TriggerMsg, err error) {
	return
}

// CheckClusterHealth checks the e2e cluster health
func (c *TopoServer) CheckClusterHealth(ctx context.Context, req *iota.NodeMsg) (resp *iota.ClusterHealthMsg, err error) {
	return
}
