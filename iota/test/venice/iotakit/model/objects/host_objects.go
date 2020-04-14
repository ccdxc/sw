package objects

import (
	"fmt"

	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/willf/bitset"
)

// Host represents a host where workload are running
type Host struct {
	iotaNode    *iota.Node
	VeniceHost  *cluster.Host
	Naples      *Naples
	vlanBitmask *bitset.BitSet
	maxVlans    uint32
}

// HostCollection is collection of hosts
type HostCollection struct {
	CollectionCommon
	Hosts     []*Host
	FakeHosts []*Host
}

// HostWorkloadCollection is collection of host and its workloads
type HostWorkloadCollection struct {
	CollectionCommon
	host      *Host
	workloads *WorkloadCollection
}

func NewHost(host *cluster.Host, iotaNode *iota.Node, naples *Naples) *Host {
	return &Host{
		iotaNode:   iotaNode,
		VeniceHost: host,
		Naples:     naples,
		maxVlans:   4094,
	}
}

func (h *Host) Name() string {
	return h.iotaNode.Name
}

func (h *Host) GetIotaNode() *iota.Node {
	return h.iotaNode
}

func (h *Host) PersonalityType() iota.PersonalityType {
	return h.iotaNode.Type
}

// Any number of hosts
func (hc *HostCollection) Any(number int) *HostCollection {

	ret := &HostCollection{}
	for _, host := range hc.Hosts {
		ret.Hosts = append(ret.Hosts, host)
		if len(ret.Hosts) == number {
			break
		}
	}

	for _, host := range hc.FakeHosts {
		ret.FakeHosts = append(ret.FakeHosts, host)
		if len(ret.Hosts)+len(ret.FakeHosts) >= number {
			break
		}
	}

	return ret
}

//Names  return names of nodes.
func (hc *HostCollection) Names() []string {

	var names []string
	for _, host := range hc.Hosts {
		names = append(names, host.Name())
	}

	for _, host := range hc.FakeHosts {
		names = append(names, host.Name())
	}

	return names
}

func NewHostCollection(client objClient.ObjClient, testbed *testbed.TestBed) *HostCollection {
	return &HostCollection{
		CollectionCommon: CollectionCommon{Client: client,
			Testbed: testbed},
	}
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
		for _, host := range hc.Hosts {
			if host.iotaNode.Name == name {
				ret.Hosts = append(ret.Hosts, host)
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

// SelectByPercentage returns a collection with the specified hosts based on percentage.
func (hc *HostCollection) SelectByPercentage(percent int) (*HostCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if hc.err != nil {
		return nil, fmt.Errorf("host collection error (%s)", hc.err)
	}

	ret := &HostCollection{}
	for _, host := range hc.Hosts {
		ret.Hosts = append(ret.Hosts, host)
		if (len(ret.Hosts)) >= len(hc.Hosts)*percent/100 {
			break
		}
	}

	for _, host := range hc.FakeHosts {
		ret.FakeHosts = append(ret.FakeHosts, host)
		if (len(ret.Hosts) + len(ret.FakeHosts)) >= (len(hc.FakeHosts)+len(hc.Hosts))*percent/100 {
			break
		}
	}

	if len(ret.Hosts)+len(ret.FakeHosts) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
	return ret, nil
}

func NewHostWorkloadCollection(host *Host, client objClient.ObjClient, testbed *testbed.TestBed) *HostWorkloadCollection {
	return &HostWorkloadCollection{
		host: host,
		CollectionCommon: CollectionCommon{Client: client,
			Testbed: testbed},
		workloads: &WorkloadCollection{CollectionCommon: CollectionCommon{Client: client, Testbed: testbed}},
	}
}

func (hc *HostWorkloadCollection) AddWorkload(w *Workload) {
	hc.workloads.Workloads = append(hc.workloads.Workloads, w)
}

func (hc *HostWorkloadCollection) Host() *HostCollection {
	h := NewHostCollection(hc.Client, hc.Testbed)
	h.Hosts = []*Host{hc.host}
	return h
}

func (hc *HostWorkloadCollection) Workloads() *WorkloadCollection {
	return hc.workloads
}
