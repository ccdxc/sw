// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package testbed

import (
	"fmt"
	"io/ioutil"
	"os"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
	"gopkg.in/yaml.v2"
)

// TopoNode contains info about a node in the topology
type TopoNode struct {
	NodeName     string               // node name specific to this topology
	Type         iota.TestBedNodeType // node type
	Personality  iota.PersonalityType // node topology
	HostOS       string               // OS on the host
	NumInstances int                  // Number of instances (used for sim for now)
	MangedNodes  []string             //nodes managed by this node
}

//ModelType type
type ModelType int

const (
	//DefaultModel for GS
	DefaultModel ModelType = 0
	//VcenterModel for Vcenter
	VcenterModel = 1
	//CloudModel for equinix
	CloudModel = 2
	//BaseNetModel for classic
	BaseNetModel = 3
)

type WorkloadInfo struct {
	WorkloadType  iota.WorkloadType // workload type
	WorkloadImage string            // image name for the workload
}

// Topology describes the topology of the testbed
type Topology struct {
	NaplesImage    string                  // naples image
	NaplesSimImage string                  // naples sim image
	VeniceImage    string                  // venice image
	Nodes          []TopoNode              // nodes in the topology
	NumVlans       int                     // list of vlans
	Model          ModelType               // model cfg type
	WkldInfo       map[string]WorkloadInfo //workload info
}

type TopoMeta struct {
	Model string `yaml:"model"`
	Nodes struct {
		Naples struct {
			Image     string `yaml:"image"`
			Instances int    `yaml:"instances"`
		} `yaml:"naples"`
		ThirdParty struct {
			Instances int `yaml:"instances"`
		} `yaml:"third-party"`
		Venice struct {
			Image     string `yaml:"image"`
			Instances int    `yaml:"instances"`
		} `yaml:"venice"`
		NaplesSimScale struct {
			Image            string `yaml:"image"`
			Instances        int    `yaml:"instances"`
			PerNodeInstances int    `yaml:"per-node-instances"`
		} `yaml:"naples-sim-scale"`
		NaplesSim struct {
			Image     string `yaml:"image"`
			Instances int    `yaml:"instances"`
		} `yaml:"naples-sim"`
		Vcenter struct {
			Instances int `yaml:"instances"`
		} `yaml:"vcenter"`
	} `yaml:"nodes"`
	Workload struct {
		Esx struct {
			Image  string `yaml:"image"`
			Memory int    `yaml:"memory"`
			CPU    int    `yaml:"cpu"`
			Type   string `yaml:"type"`
		} `yaml:"esx"`
		Linux struct {
			Image  string `yaml:"image"`
			Memory int    `yaml:"memory"`
			CPU    int    `yaml:"cpu"`
			Type   string `yaml:"type"`
		} `yaml:"linux"`
	} `yaml:"workload"`
}

//ParseTopology parse topology file to get topo instance file
func ParseTopology(fileName string) (*Topology, error) {

	yamlFile, err := os.OpenFile(fileName, os.O_RDONLY, 0755)
	if err != nil {
		//file not created yet. ignore.
		return nil, err
	}
	byteValue, _ := ioutil.ReadAll(yamlFile)

	var topoMeta TopoMeta
	if err := yaml.Unmarshal(byteValue, &topoMeta); err != nil {
		log.Errorf("Failed to unmarshal topo meta file")
		return nil, err
	}
	yamlFile.Close()

	var topo Topology

	topo.WkldInfo = make(map[string]WorkloadInfo)

	switch topoMeta.Model {
	case "Default":
		topo.Model = DefaultModel
	case "Vcenter":
		topo.Model = VcenterModel
	case "Basenet":
		topo.Model = BaseNetModel
	case "Cloud":
		topo.Model = CloudModel
	default:
		return nil, fmt.Errorf("Unknown model type %v", topoMeta.Model)
	}

	if topoMeta.Nodes.Naples.Instances != 0 {
		log.Infof("Number of naples instances : %v", topoMeta.Nodes.Naples.Instances)
		for i := 0; i < topoMeta.Nodes.Naples.Instances; i++ {
			topo.Nodes = append(topo.Nodes, TopoNode{
				NodeName:    "naples-" + fmt.Sprintf("%v", i+1),
				Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_HW,
				Personality: iota.PersonalityType_PERSONALITY_NAPLES,
			})
		}
		topo.NaplesImage = topoMeta.Nodes.Naples.Image
	}

	if topoMeta.Nodes.Venice.Instances != 0 {
		for i := 0; i < topoMeta.Nodes.Venice.Instances; i++ {
			topo.Nodes = append(topo.Nodes, TopoNode{
				NodeName:    "venice-" + fmt.Sprintf("%v", i+1),
				Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
				Personality: iota.PersonalityType_PERSONALITY_VENICE,
			})
		}
		topo.VeniceImage = topoMeta.Nodes.Venice.Image
	}

	if topoMeta.Nodes.NaplesSimScale.Instances != 0 {
		for i := 0; i < topoMeta.Nodes.NaplesSimScale.Instances; i++ {
			topo.Nodes = append(topo.Nodes, TopoNode{
				NodeName:     "naples-sim-scale" + fmt.Sprintf("%v", i+1),
				NumInstances: topoMeta.Nodes.NaplesSimScale.PerNodeInstances,
				Type:         iota.TestBedNodeType_TESTBED_NODE_TYPE_MULTI_SIM,
				Personality:  iota.PersonalityType_PERSONALITY_NAPLES_MULTI_SIM,
			})
		}
		topo.NaplesSimImage = topoMeta.Nodes.NaplesSimScale.Image
	}

	if topoMeta.Nodes.NaplesSim.Instances != 0 {
		for i := 0; i < topoMeta.Nodes.NaplesSim.Instances; i++ {
			topo.Nodes = append(topo.Nodes, TopoNode{
				NodeName:    "naples-sim-" + fmt.Sprintf("%v", i+1),
				Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_SIM,
				Personality: iota.PersonalityType_PERSONALITY_NAPLES_CONTROL_SIM,
			})
		}
		topo.NaplesSimImage = topoMeta.Nodes.NaplesSim.Image
	}

	for i := 0; i < topoMeta.Nodes.Vcenter.Instances; i++ {
		topo.Nodes = append(topo.Nodes, TopoNode{
			NodeName:    "vcenter-" + fmt.Sprintf("%v", i+1),
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_VCENTER,
			Personality: iota.PersonalityType_PERSONALITY_VCENTER_NODE,
		})
	}

	for i := 0; i < topoMeta.Nodes.ThirdParty.Instances; i++ {
		topo.Nodes = append(topo.Nodes, TopoNode{
			NodeName:    "third-party-" + fmt.Sprintf("%v", i+1),
			Type:        iota.TestBedNodeType_TESTBED_NODE_TYPE_THIRD_PARTY,
			Personality: iota.PersonalityType_PERSONALITY_THIRD_PARTY_NIC,
		})
	}

	if topoMeta.Workload.Esx.Image != "" {
		topo.WkldInfo["esx"] = WorkloadInfo{
			WorkloadImage: topoMeta.Workload.Esx.Image,
			WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_VM,
		}
	}

	if topoMeta.Workload.Linux.Image != "" {
		topo.WkldInfo["linux"] = WorkloadInfo{
			WorkloadImage: topoMeta.Workload.Linux.Image,
			WorkloadType:  iota.WorkloadType_WORKLOAD_TYPE_CONTAINER,
		}
	}

	return &topo, nil

}
