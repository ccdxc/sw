// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/willf/bitset"
)

// Host represents a host where workload are running
type Host struct {
	iotaNode    *iota.Node
	veniceHost  *cluster.Host
	vlanBitmask *bitset.BitSet
	maxVlans    uint32
	sm          *SysModel // pointer back to the model
}

// Naples represents a smart-nic
type Naples struct {
	iotaNode *iota.Node
	testNode *TestNode
	smartNic *cluster.SmartNIC
	sm       *SysModel // pointer back to the model
}

// VeniceNode represents a venice node
type VeniceNode struct {
	iotaNode *iota.Node
	sm       *SysModel // pointer back to the model
}

// HostCollection is collection of hosts
type HostCollection struct {
	err   error
	hosts []*Host
}

// VeniceNodeCollection is collection of venice nodes
type VeniceNodeCollection struct {
	err   error
	sm    *SysModel // pointer back to the model
	nodes []*VeniceNode
}

// NaplesCollection contains a list of naples nodes
type NaplesCollection struct {
	err   error
	nodes []*Naples
}

//  iterator functions
type hostIteratorFn func(*HostCollection) error
type veniceNodeIteratorFn func(*VeniceNodeCollection) error
type naplesIteratorFn func(*NaplesCollection) error

const (
	hostToolsDir        = "/pensando/iota"
	penctlPath          = "nic/bin"
	penctlPkgName       = "../nic/host.tar"
	penctlNaplesURL     = "http://169.254.0.1"
	penctlLinuxBinary   = "penctl.linux"
	penctlFreebsdBinary = "penctl.freebsd"
)

// createHost creates a new host instance
func (sm *SysModel) createHost(n *TestNode) (*Host, error) {
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: n.NodeName,
		},
		Spec: cluster.HostSpec{
			SmartNICs: []cluster.SmartNICID{
				{
					Name:       n.NodeName,
					MACAddress: n.NodeUUID,
				},
			},
		},
	}

	// create the host in venice
	err := sm.tb.CreateHost(&host)
	if err != nil {
		log.Errorf("Error creating host: %+v. Err: %v", host, err)
		return nil, err
	}

	bs := bitset.New(uint(4096))
	bs.Set(0).Set(1).Set(4095)

	h := Host{
		iotaNode:    n.iotaNode,
		veniceHost:  &host,
		maxVlans:    4094,
		vlanBitmask: bs,
		sm:          sm,
	}

	// add it to database
	sm.hosts[n.NodeName] = &h

	return &h, nil
}

// Hosts returns list of all hosts in the system
func (sm *SysModel) Hosts() *HostCollection {
	var hc HostCollection
	for _, hst := range sm.hosts {
		hc.hosts = append(hc.hosts, hst)
	}

	return &hc
}

// ForEachHost calls a function for each host
func (sm *SysModel) ForEachHost(fn hostIteratorFn) error {
	for _, hst := range sm.hosts {
		err := fn(&HostCollection{hosts: []*Host{hst}})
		if err != nil {
			return err
		}
	}

	return nil
}

func (hst *Host) allocUsegVlan() (uint32, error) {
	// allocate next available address
	vlanBit, ok := hst.vlanBitmask.NextClear(0)
	if !ok || vlanBit >= uint(hst.maxVlans) {
		log.Errorf("Could not find a free bit in bitset for vlans %v", hst.veniceHost.Name)
		return 0, fmt.Errorf("Could not find a free vlan")
	}

	hst.vlanBitmask.Set(uint(vlanBit))
	return uint32(vlanBit), nil
}

// NewWorkload creates new workload on the host in each specified subnet
func (hc *HostCollection) NewWorkload(namePrefix string, snc *NetworkCollection) *WorkloadCollection {
	if snc.err != nil {
		return &WorkloadCollection{err: snc.err}
	}
	if hc.err != nil {
		return &WorkloadCollection{err: hc.err}
	}

	var wc WorkloadCollection
	for _, host := range hc.hosts {
		for _, subnet := range snc.subnets {
			name := fmt.Sprintf("%s_%s_%s", namePrefix, host.veniceHost.Name, subnet.Name)
			w, err := host.sm.createWorkload(host.sm.tb.Topo.WorkloadType, host.sm.tb.Topo.WorkloadImage, name, host, subnet)
			if err != nil {
				return &WorkloadCollection{err: err}
			}
			wc.workloads = append(wc.workloads, w)
			if w == nil {
				panic("Invalid workload object")
			}
		}
	}

	// bringup the workloads
	err := wc.Bringup()
	if err != nil {
		return &WorkloadCollection{err: err}
	}

	return &wc
}

// createNaples creates a naples instance
func (sm *SysModel) createNaples(node *TestNode) error {
	naples := Naples{
		iotaNode: node.iotaNode,
		testNode: node,
		sm:       sm,
	}

	sm.naples[node.NodeName] = &naples

	return nil
}

// ForEachNaples calls an iterator for each naples in the model
func (sm *SysModel) ForEachNaples(fn naplesIteratorFn) error {
	for _, node := range sm.naples {
		err := fn(&NaplesCollection{nodes: []*Naples{node}})
		if err != nil {
			return err
		}
	}

	return nil
}

func (sm *SysModel) createVeniceNode(iotaNode *iota.Node) error {
	vn := VeniceNode{
		iotaNode: iotaNode,
		sm:       sm,
	}

	sm.veniceNodes[iotaNode.Name] = &vn

	return nil
}

// ForEachVeniceNode runs an iterator function on each venice node
func (sm *SysModel) ForEachVeniceNode(fn veniceNodeIteratorFn) error {
	for _, node := range sm.veniceNodes {
		err := fn(&VeniceNodeCollection{nodes: []*VeniceNode{node}, sm: sm})
		if err != nil {
			return err
		}
	}

	return nil
}

// VeniceNodes returns a collection of venice nodes
func (sm *SysModel) VeniceNodes() *VeniceNodeCollection {
	vnc := VeniceNodeCollection{sm: sm}
	for _, node := range sm.veniceNodes {
		vnc.nodes = append(vnc.nodes, node)
	}

	return &vnc
}

// Leader returns the leader node
func (vnc *VeniceNodeCollection) Leader() *VeniceNodeCollection {
	if vnc.err != nil {
		return vnc
	}
	// get the cluster from venice
	cl, err := vnc.sm.tb.GetCluster()
	if err != nil {
		return &VeniceNodeCollection{err: fmt.Errorf("Failed to get cluster. Err: %v", err)}
	}

	for _, node := range vnc.nodes {
		if cl.Status.Leader == node.iotaNode.Name {
			return &VeniceNodeCollection{
				nodes: []*VeniceNode{node},
				sm:    vnc.sm,
			}
		}
	}

	return &VeniceNodeCollection{err: fmt.Errorf("Could not find a leader node")}

}
