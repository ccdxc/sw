package objects

import (
	"errors"
	"fmt"
	"math/rand"

	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// Node represents a smart-nic
type Node struct {
	name      string
	iotaNode  *iota.Node
	testNode  *testbed.TestNode
	IPAddress string
	UUID      string
}

// NaplesInstance represents a naples instance
type NaplesInstance struct {
	Dsc        *cluster.DistributedServiceCard
	EntityName string
	LoopbackIP string
}

// Naples represents a smart-nic
type Naples struct {
	Node
	Instances []*NaplesInstance
}

// ThirdPartyNode represents non-naples
type ThirdPartyNode struct {
	Node
}

// NewNaplesNode create a new naples node obj
func NewNaplesNode(name string, node *testbed.TestNode) *Naples {
	return &Naples{
		Node: Node{
			testNode: node,
			name:     name,
			iotaNode: node.GetIotaNode(),
		},
	}
}

// AddDSC add a dsc instance
func (naples *Naples) AddDSC(name string, sn *cluster.DistributedServiceCard) {

	naples.Instances = append(naples.Instances, &NaplesInstance{
		Dsc: sn, EntityName: name,
	})
}

// NewThirdPartyNode create a new thrid party node obj
func NewThirdPartyNode(name string, node *testbed.TestNode) *ThirdPartyNode {

	return &ThirdPartyNode{
		Node: Node{
			testNode: node,
			name:     name,
			iotaNode: node.GetIotaNode(),
		},
	}
}

// IP get mgmtIP
func (n *Node) IP() string {
	if n.IPAddress != "" {
		return n.IPAddress
	}
	return n.testNode.InstanceParams().NicMgmtIP
}

// SetIP set up IP address for node
func (n *Node) SetIP(ip string) {
	n.IPAddress = ip
}

// Name get node name
func (n *Node) Name() string {
	return n.name
}

// NodeName get iota node name
func (n *Node) NodeName() string {
	return n.iotaNode.Name
}

// GetIotaNode get iota node obj
func (n *Node) GetIotaNode() *iota.Node {
	return n.iotaNode
}

// GetTestNode get test node obj
func (n *Node) GetTestNode() *testbed.TestNode {
	return n.testNode
}

// Personality returns iota personality type
func (n *Node) Personality() iota.PersonalityType {
	return n.testNode.Personality
}

// NaplesCollection contains a list of naples nodes
type NaplesCollection struct {
	CollectionCommon
	Nodes     []*Naples
	FakeNodes []*Naples
}

// ThirdPartyCollection contains a list of 3rd party nodes
type ThirdPartyCollection struct {
	CollectionCommon
	Nodes []*ThirdPartyNode
}

// Names returns names of all naples in the collection
func (npc *NaplesCollection) Names() []string {
	var ret []string
	for _, n := range npc.Nodes {
		for _, inst := range n.Instances {
			ret = append(ret, inst.Dsc.ObjectMeta.Name)
		}
	}
	for _, n := range npc.FakeNodes {
		for _, inst := range n.Instances {
			ret = append(ret, inst.Dsc.ObjectMeta.Name)
		}
	}

	return ret
}

// Any returns the requested number of naples from collection in random
func (npc *NaplesCollection) Any(num int) *NaplesCollection {
	if npc.HasError() || len(npc.Nodes) <= num {
		return npc
	}

	newNpc := &NaplesCollection{Nodes: []*Naples{},
		CollectionCommon: CollectionCommon{
			Client:  npc.Client,
			Testbed: npc.Testbed},
	}

	tmpArry := make([]*Naples, len(npc.Nodes))
	copy(tmpArry, npc.Nodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newNpc.Nodes = append(newNpc.Nodes, sn)
	}

	return newNpc
}

// Any returns the requested number of naples from collection in random
func (npc *NaplesCollection) AnyFakeNodes(num int) *NaplesCollection {
	if npc.HasError() || len(npc.FakeNodes) <= num {
		return npc
	}

	newNpc := &NaplesCollection{FakeNodes: []*Naples{},
		CollectionCommon: CollectionCommon{
			Client:  npc.Client,
			Testbed: npc.Testbed},
	}

	tmpArry := make([]*Naples, len(npc.FakeNodes))
	copy(tmpArry, npc.FakeNodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newNpc.FakeNodes = append(newNpc.FakeNodes, sn)
	}

	return newNpc
}

// Pop returns popped element and removes from currnet collection
func (npc *NaplesCollection) Pop(num int) *NaplesCollection {
	if npc.HasError() || len(npc.Nodes) <= num {
		return npc
	}

	newNpc := &NaplesCollection{Nodes: []*Naples{}}
	tmpArry := make([]*Naples, len(npc.Nodes))
	copy(tmpArry, npc.Nodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newNpc.Nodes = append(newNpc.Nodes, sn)
	}

	npc.Nodes = npc.Nodes[num:]

	return newNpc
}

// SetDscProfile sets DSC profile for the given naples
func (npc *NaplesCollection) SetDscProfile(profile *DscProfile) error {

	return profile.AttachNaples(npc)
}

// ResetProfile resets profile
func (npc *NaplesCollection) ResetProfile() error {

	for _, node := range npc.Nodes {
		for _, inst := range node.Instances {
			dsc := inst.Dsc
			curDsc, err := npc.CollectionCommon.Client.GetSmartNIC(inst.Dsc.ObjectMeta.Name)
			if err != nil {
				log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
				return err
			}
			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta

			dsc.Spec.DSCProfile = "default"
			err = npc.CollectionCommon.Client.UpdateSmartNIC(dsc)
			if err != nil {
				log.Errorf("Error updating smartnic with profile %v", err.Error())
				return err
			}
		}

	}

	for _, node := range npc.FakeNodes {
		for _, inst := range node.Instances {
			dsc := inst.Dsc
			curDsc, err := npc.CollectionCommon.Client.GetSmartNIC(dsc.ObjectMeta.Name)
			if err != nil {
				log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
				return err
			}
			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta

			dsc.Spec.DSCProfile = "default"
			err = npc.CollectionCommon.Client.UpdateSmartNIC(dsc)
			if err != nil {
				log.Errorf("Error updating smartnic with profile %v", err.Error())
				return err
			}
		}
	}

	return nil
}

// Decommission decomission naples
func (npc *NaplesCollection) Decommission() error {

	for _, naples := range npc.Nodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			curDsc, err := npc.Client.GetSmartNIC(dsc.Status.PrimaryMAC)
			if err != nil {
				log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
				return err
			}

			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta

			log.Infof("Decommissioning naples %v", dsc.Status.PrimaryMAC)
			err = npc.Client.DecommissionSmartNIC(dsc)
			if err != nil {
				log.Infof("Error decommissioning smart nic %v", err.Error())
				return err
			}
		}
	}

	for _, naples := range npc.FakeNodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			curDsc, err := npc.Client.GetSmartNIC(dsc.Status.PrimaryMAC)
			if err != nil {
				log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
				return err
			}

			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta
			err = npc.Client.DecommissionSmartNIC(dsc)
			if err != nil {
				log.Infof("Error decommissioning smart nic %v", err.Error())
				return err
			}
		}
	}

	return nil
}

// Admit decomission naples
func (npc *NaplesCollection) Admit() error {

	for _, naples := range npc.Nodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			log.Infof("Admit naples %v", dsc.Status.PrimaryMAC)

			curDsc, err := npc.Client.GetSmartNIC(dsc.Status.PrimaryMAC)
			if err != nil {
				log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
				return err
			}

			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta

			err = npc.Client.AdmitSmartNIC(dsc)
			if err != nil {
				log.Infof("Error Admit smart nic %v", err.Error())
				return err
			}
		}
	}

	for _, naples := range npc.FakeNodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			curDsc, err := npc.Client.GetSmartNIC(dsc.Status.PrimaryMAC)
			if err != nil {
				log.Errorf("Error reading smartnic for profile uppate %v", err.Error())
				return err
			}

			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta

			err = npc.Client.AdmitSmartNIC(dsc)
			if err != nil {
				log.Infof("Error Admit smart nic %v", err.Error())
				return err
			}
		}
	}

	return nil
}

//IsAdmitted returns true if all snics are admitted
func (npc *NaplesCollection) IsAdmitted() error {

	admitted := true
	for _, naples := range npc.Nodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			snic, err := npc.Client.GetSmartNICByName(dsc.Name)
			if err != nil || snic.Status.AdmissionPhase != "admitted" {
				admitted = false
				log.Infof("Snic not admitted  %v", snic)
				msg := fmt.Sprintf("Snic not admitted %v %v %v", snic, dsc.Name, err)
				log.Infof(msg)
			}
		}
	}

	for _, naples := range npc.FakeNodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			snic, err := npc.Client.GetSmartNICByName(dsc.Name)
			if err != nil || !snic.Spec.Admit {
				admitted = false
				msg := fmt.Sprintf("Snic not admitted %v %v %v", snic, dsc.Name, err)
				log.Infof(msg)
			}
		}
	}

	if !admitted {
		return errors.New("Some naples are not admitted")
	}

	return nil
}

//IsNotAdmitted returns true if all snics are admitted
func (npc *NaplesCollection) IsNotAdmitted() error {

	admitted := false
	for _, naples := range npc.Nodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			snic, err := npc.Client.GetSmartNICByName(dsc.Name)
			if err == nil && snic.Status.AdmissionPhase == "admitted" {
				admitted = true
				log.Infof("DSC admitted  %v", snic)
				msg := fmt.Sprintf("DSC admitted %v %v %v", snic, dsc.Name, err)
				log.Infof(msg)
			}
		}
	}

	for _, naples := range npc.FakeNodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			snic, err := npc.Client.GetSmartNICByName(dsc.Name)
			if err == nil && snic.Status.AdmissionPhase == "admitted" {
				admitted = true
				log.Infof("DSC admitted  %v", snic)
				msg := fmt.Sprintf("DSC admitted %v %v %v", snic, dsc.Name, err)
				log.Infof(msg)
			}
		}
	}

	if admitted {
		return errors.New("Some naples are still admitted")
	}

	return nil
}

//Delete deletes smartnic
func (npc *NaplesCollection) Delete() error {

	for _, naples := range npc.Nodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			err := npc.Client.DeleteSmartNIC(dsc)
			if err != nil {
				log.Infof("Error deleting smart nic %v", err.Error())
				return err
			}
		}

	}

	for _, naples := range npc.FakeNodes {
		for _, inst := range naples.Instances {
			dsc := inst.Dsc
			err := npc.Client.DeleteSmartNIC(dsc)
			if err != nil {
				log.Infof("Error deleting smart nic %v", err.Error())
				return err
			}
		}
	}

	return nil
}

// SelectByPercentage returns a collection with the specified napls based on percentage.
func (npc *NaplesCollection) SelectByPercentage(percent int) (*NaplesCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if npc.err != nil {
		return nil, fmt.Errorf("naples collection error (%s)", npc.err)
	}

	ret := &NaplesCollection{}
	for _, entry := range npc.Nodes {
		ret.Nodes = append(ret.Nodes, entry)
		if (len(ret.Nodes)) >= (len(npc.Nodes)+len(npc.FakeNodes))*percent/100 {
			break
		}
	}

	for _, entry := range npc.FakeNodes {

		if (len(ret.Nodes) + len(ret.FakeNodes)) >= (len(npc.Nodes)+len(npc.FakeNodes))*percent/100 {
			break
		}
		ret.FakeNodes = append(ret.FakeNodes, entry)
	}

	if (len(ret.Nodes) + len(ret.FakeNodes)) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
	return ret, nil
}

//RunCommand runs command on the naples nodes
func (npc *NaplesCollection) RunCommand(node *Naples, cmd string) (string, string, int32, error) {

	//Derivce the container ID
	trig := npc.Testbed.NewTrigger()

	entity := node.iotaNode.Name + "_naples"

	trig.AddCommand(cmd, entity, node.iotaNode.Name)

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		return "", "", -1, err
	}

	return triggerResp[0].Stdout, triggerResp[0].Stderr, triggerResp[0].ExitCode, nil
}

func (npc *NaplesCollection) Refresh() error {
	if npc.HasError() {
		return fmt.Errorf("naples collection error (%s)", npc.err)
	}

	for _, node := range npc.Nodes {
		for _, inst := range node.Instances {
			dsc := inst.Dsc
			log.Infof("Get DSC %v", dsc.GetName())

			curDsc, err := npc.Client.GetSmartNIC(dsc.GetName())
			if err != nil {
				log.Errorf("Error reading smartnic %v", err.Error())
				return err
			}
			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta
		}
	}

	for _, node := range npc.FakeNodes {
		for _, inst := range node.Instances {
			dsc := inst.Dsc
			log.Infof("Get DSC %v", dsc.GetName())

			curDsc, err := npc.Client.GetSmartNIC(dsc.GetName())
			if err != nil {
				log.Errorf("Error reading smartnic %v", err.Error())
				return err
			}
			dsc.Spec = curDsc.Spec
			dsc.Status = curDsc.Status
			dsc.ObjectMeta = curDsc.ObjectMeta
		}
	}

	return nil
}
