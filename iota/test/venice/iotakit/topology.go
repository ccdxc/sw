// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	iota "github.com/pensando/sw/iota/protos/gogen"
)

// TopoNode contains info about a node in the topology
type TopoNode struct {
	NodeName    string               // node name specific to this topology
	Type        iota.TestBedNodeType // node type
	Personality iota.PersonalityType // node topology
	HostOS      string               // OS on the host
}

// Topology describes the topology of the testbed
type Topology struct {
	NaplesImage   string            // naples image
	VeniceImage   string            // venice image
	WorkloadType  iota.WorkloadType // workload type
	WorkloadImage string            // image name for the workload
	Nodes         []TopoNode        // nodes in the topology
	NumVlans      int               // list of vlans
}

// Topologies contains list of topologies
var Topologies = map[string]*Topology{
	"3Venice_3NaplesSim": &topo3Venice3NaplesSim,
	"3Venice_Nodes":      &topo3VeniceNodes,
	"5Venice_Nodes":      &topo5VeniceNodes,
	"1Venice_2NaplesSim": &topo1Venice2NaplesSim,
	"1Venice_2Naples":    &topo1Venice2Naples,
	"3Venice_1Naples":    &topo3Venice1Naples,
	"3Venice_2Naples":    &topo3Venice2Naples,
}

// 3 Venice Nodes and 3 Naples Sim nodes
var topo3Venice3NaplesSim = Topology{
	NaplesImage:   "../nic/obj/images/naples-release-v1.tgz",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_CONTAINER,
	WorkloadImage: "registry.test.pensando.io:5000/pensando/iota/centos:1.1",
	NumVlans:      10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "naples1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			HostOS:      "linux",
		},
		{
			NodeName:    "naples2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			HostOS:      "linux",
		},
		{
			NodeName:    "naples3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			HostOS:      "linux",
		},
	},
}

// just 3 venice nodes (no naples)
var topo3VeniceNodes = Topology{
	NaplesImage: "../nic/obj/images/naples-release-v1.tgz",
	VeniceImage: "../bin/venice.tgz",
	NumVlans:    10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// 5 venice nodes (no naples)
var topo5VeniceNodes = Topology{
	NaplesImage: "../nic/obj/images/naples-release-v1.tgz",
	VeniceImage: "../bin/venice.tgz",
	NumVlans:    10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// 1 venice node and 1 naples sim node
var topo1Venice2NaplesSim = Topology{
	NaplesImage:   "../nic/obj/images/naples-release-v1.tgz",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_CONTAINER,
	WorkloadImage: "registry.test.pensando.io:5000/pensando/iota/centos:1.1",
	NumVlans:      10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "naples1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			HostOS:      "linux",
		},
		{
			NodeName:    "naples2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_SIM,
			HostOS:      "linux",
		},
	},
}

var topo3Venice1Naples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-130",
	NumVlans:      10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naples1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// one venice node and two Naples baremetal nodes
var topo1Venice2Naples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-130",
	NumVlans:      10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naples1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

var topo3Venice2Naples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-130",
	NumVlans:      10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naples1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}
