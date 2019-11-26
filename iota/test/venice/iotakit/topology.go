// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	iota "github.com/pensando/sw/iota/protos/gogen"
)

// TopoNode contains info about a node in the topology
type TopoNode struct {
	NodeName     string               // node name specific to this topology
	Type         iota.TestBedNodeType // node type
	Personality  iota.PersonalityType // node topology
	HostOS       string               // OS on the host
	NumInstances int                  // Number of instances (used for sim for now)
}

// Topology describes the topology of the testbed
type Topology struct {
	NaplesImage    string            // naples image
	NaplesSimImage string            // naples sim image
	VeniceImage    string            // venice image
	WorkloadType   iota.WorkloadType // workload type
	WorkloadImage  string            // image name for the workload
	Nodes          []TopoNode        // nodes in the topology
	NumVlans       int               // list of vlans
}

// Topologies contains list of topologies
var Topologies = map[string]*Topology{
	"3Venice_3NaplesSim":                  &topo3Venice3NaplesSim,
	"3Venice_Nodes":                       &topo3VeniceNodes,
	"5Venice_Nodes":                       &topo5VeniceNodes,
	"1Venice_2NaplesSim":                  &topo1Venice2NaplesSim,
	"1Venice_2Naples":                     &topo1Venice2Naples,
	"1Venice_1Naples":                     &topo1Venice1Naples,
	"3Venice_1Naples":                     &topo3Venice1Naples,
	"3Venice_2Naples":                     &topo3Venice2Naples,
	"3VeniceBM_2Naples":                   &topo3VeniceBM2Naples,
	"1Venice_ScaleNaples":                 &topo1VeniceScaleNaples,
	"1Venice_2Naples_1SimScale1Naples":    &topo1Venice2Naples1NaplesSim3Scale,
	"3Venice_2Naples_1SimScale100Naples":  &topo3Venice2Naples1NaplesSim100Scale,
	"3Venice_2Naples_3SimScale100Naples":  &topo3Venice2Naples3NaplesSim100Scale,
	"1Venice_2Naples_3SimScale200Naples":  &topo1Venice2Naples3NaplesSim200Scale,
	"1Venice_15Naples_3SimScale200Naples": &topo1Venice2Naples15NaplesSim200Scale,
	"3Venice_9Naples_3SimScale100Naples":  &topo3Venice9Naples3NaplesSim100Scale,
	"3Venice_9Naples_7SimScale100Naples":  &topo3Venice9Naples7NaplesSim100Scale,
	"3Venice_3SimScale200Naples":          &topo3Venice3NaplesSim200Scale,
	"3Venice_1SimScale200Naples":          &topo3Venice1NaplesSim200Scale,
	"1Venice_1SimScale200Naples":          &topo1Venice1NaplesSim200Scale,
	"1Venice_3SimScale200Naples":          &topo1Venice3NaplesSim200Scale,
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
	WorkloadImage: "build-160",
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

// one venice node and one Naples baremetal nodes
var topo1Venice1Naples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-160",
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
	},
}

// one venice node and two Naples baremetal nodes
var topo1Venice2Naples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-160",
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

// one venice node and two Naples baremetal nodes and naples sim scale
var topo1Venice2Naples1NaplesSim3Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
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
			NodeName:    "naplesSimScale",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 3,
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// three venice node and two Naples baremetal nodes and naples sim scale with 100 instances
var topo3Venice2Naples1NaplesSim100Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
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
			NodeName:    "naplesSimScale",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
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

// three venice node and two Naples baremetal nodes and naples sim scale with 100 instances
var topo3Venice2Naples3NaplesSim100Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
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
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
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

// one  venice node and 15 Naples baremetal nodes and naples sim scale with 100 instances
var topo1Venice2Naples15NaplesSim200Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
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
			NodeName:    "naples3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples6",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples7",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples8",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples9",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 100,
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// one  venice node and 15 Naples baremetal nodes and naples sim scale with 100 instances
var topo3Venice9Naples3NaplesSim100Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
	Nodes: []TopoNode{
		/*{
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
			NodeName:    "naples3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples6",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples7",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples8",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples9",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},*/
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
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

var topo3Venice9Naples7NaplesSim100Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
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
			NodeName:    "naples3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples6",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples7",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples8",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples9",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
		},
		{
			NodeName:    "naplesSimScale4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
		},
		{
			NodeName:    "naplesSimScale5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
		},
		{
			NodeName:    "naplesSimScale6",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
		},
		{
			NodeName:    "naplesSimScale7",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 220,
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

// one  venice node and two Naples baremetal nodes and naples sim scale with 100 instances
var topo1Venice2Naples3NaplesSim200Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
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
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// three venice node and two Naples baremetal nodes and naples sim scale with 100 instances
var topo3Venice3NaplesSim200Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
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

// three venice node and two Naples baremetal nodes and naples sim scale with 200 instances
var topo3Venice1NaplesSim200Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
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

// One venice node and two Naples baremetal nodes and naples sim scale with 200 instances
var topo1Venice1NaplesSim200Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}

// One venice node and three naples sim scale with 200 instances
var topo1Venice3NaplesSim200Scale = Topology{
	NaplesImage:    "../nic/naples_fw.tar",
	VeniceImage:    "../bin/venice.tgz",
	NaplesSimImage: "../nic/obj/images/naples-release-v1.tgz",
	WorkloadType:   iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage:  "build-160",
	NumVlans:       10, // FIXME: what should this be??
	Nodes: []TopoNode{
		{
			NodeName:    "naplesSimScale1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
		},
		{
			NodeName:    "naplesSimScale3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			//This should be linux
			HostOS:       "linux",
			NumInstances: 200,
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
	WorkloadImage: "build-160",
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

var topo3VeniceBM2Naples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-160",
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
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_VENICE_BM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE_BM,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice2",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_VENICE_BM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE_BM,
			HostOS:      "linux",
		},
		{
			NodeName:    "venice3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_VENICE_BM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE_BM,
			HostOS:      "linux",
		},
	},
}

// bm-scale topology: one venice and many baremetal Naples
var topo1VeniceScaleNaples = Topology{
	NaplesImage:   "../nic/naples_fw.tar",
	VeniceImage:   "../bin/venice.tgz",
	WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
	WorkloadImage: "build-160",
	NumVlans:      10,
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
			NodeName:    "naples3",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples4",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples5",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples6",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples7",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples8",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples9",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		/*{
			NodeName:    "naples10",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples11",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples12",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples13",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples14",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		},
		{
			NodeName:    "naples15",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
			Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			HostOS:      "esx",
		}, */
		{
			NodeName:    "venice1",
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
			Personality: iota.PersonalityType_PERSONALITY_VENICE,
			HostOS:      "linux",
		},
	},
}
