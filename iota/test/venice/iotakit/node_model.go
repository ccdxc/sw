// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"fmt"
	"math/rand"
	"strings"

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
	naples      *Naples
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
	// find the naples
	naples, ok := sm.naples[n.NodeName]
	if !ok || naples.smartNic == nil {
		return nil, fmt.Errorf("Could not find the naples object for snic: %v", n.NodeName)
	}
	host := cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: n.NodeName,
		},
		Spec: cluster.HostSpec{
			SmartNICs: []cluster.SmartNICID{
				{
					MACAddress: naples.smartNic.Status.PrimaryMAC,
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
		naples:      naples,
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
	snic, err := sm.tb.GetSmartNICByName(node.NodeName)
	if sm.tb.mockMode {
		snic, err = sm.tb.GetSmartNICInMacRange(node.NodeUUID)
	}
	if err != nil {
		err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", node.NodeName, err)
		log.Errorf("%v", err)
		return err
	}
	naples := Naples{
		iotaNode: node.iotaNode,
		testNode: node,
		smartNic: snic,
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

// Naples returns all naples in the model
func (sm *SysModel) Naples() *NaplesCollection {
	var naples []*Naples
	for _, np := range sm.naples {
		naples = append(naples, np)
	}
	return &NaplesCollection{nodes: naples}
}

// Any returns the requested number of naples from collection in random
func (npc *NaplesCollection) Any(num int) *NaplesCollection {
	if npc.err != nil || len(npc.nodes) <= num {
		return npc
	}

	newNpc := &NaplesCollection{nodes: []*Naples{}}
	tmpArry := make([]*Naples, len(npc.nodes))
	copy(tmpArry, npc.nodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newNpc.nodes = append(newNpc.nodes, sn)
	}

	return newNpc
}

// Error returns the error in collection
func (npc *NaplesCollection) Error() error {
	return npc.err
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
		} else if cl.Status.Leader == node.iotaNode.IpAddress {
			return &VeniceNodeCollection{
				nodes: []*VeniceNode{node},
				sm:    vnc.sm,
			}
		}
	}

	return &VeniceNodeCollection{err: fmt.Errorf("Could not find a leader node")}

}

type selectParams struct {
	names []string
}

func parseSelectorString(str string) (selectParams, error) {
	ret := selectParams{}
	// Only handling Spaces. not all space characters.
	str = strings.Replace(str, " ", "", -1)
	parts := strings.SplitN(str, "=", 2)
	if len(parts) != 2 {
		return ret, fmt.Errorf("failed to parse selector string")
	}
	if parts[0] != "name" {
		return ret, fmt.Errorf("only name selector supported")
	}
	ret.names = strings.Split(parts[1], ",")
	return ret, nil
}

// Select returns a collection with the specified venice nodes, error if any of the specified nodes is not found
func (vnc *VeniceNodeCollection) Select(sel string) (*VeniceNodeCollection, error) {
	if vnc.err != nil {
		return nil, fmt.Errorf("node collection error (%s)", vnc.err)
	}
	ret := &VeniceNodeCollection{sm: vnc.sm}
	params, err := parseSelectorString(sel)
	if err != nil {
		return ret, fmt.Errorf("could not parse selector")
	}
	var notFound []string
nodeLoop:
	for _, name := range params.names {
		for _, node := range vnc.nodes {
			if node.iotaNode.Name == name {
				ret.nodes = append(ret.nodes, node)
				continue nodeLoop
			}
		}
		notFound = append(notFound, name)
	}
	if len(notFound) != 0 {
		return nil, fmt.Errorf("%v not found", notFound)
	}
	return ret, nil
}

// Select returns a collection with the specified hosts, error if any of the specified hosts is not found
func (hc *HostCollection) Select(sel string) (*HostCollection, error) {
	if hc.err != nil {
		return nil, fmt.Errorf("host collection error (%s)", hc.err)
	}
	ret := &HostCollection{}
	params, err := parseSelectorString(sel)
	if err != nil {
		return ret, fmt.Errorf("could not parse selector")
	}
	var notFound []string
hostLoop:
	for _, name := range params.names {
		for _, host := range hc.hosts {
			if host.iotaNode.Name == name {
				ret.hosts = append(ret.hosts, host)
				continue hostLoop
			}
		}
		notFound = append(notFound, name)
	}
	if len(notFound) != 0 {
		return nil, fmt.Errorf("%v not found", notFound)
	}
	return ret, nil
}
