package objects

import (
	"context"
	"fmt"
	"math/rand"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

// VeniceNode represents a venice node
type VeniceNode struct {
	ClusterNode *cluster.Node
	iotaNode    *iota.Node
	testNode    *testbed.TestNode
}

//Name return name
func (v *VeniceNode) Name() string {
	return v.iotaNode.Name
}

//IP return IP
func (v *VeniceNode) IP() string {
	return v.iotaNode.IpAddress
}

//GetTestNode return name
func (v *VeniceNode) GetTestNode() *testbed.TestNode {
	return v.testNode
}

// VeniceNodeCollection is collection of venice nodes
type VeniceNodeCollection struct {
	CollectionCommon
	Nodes []*VeniceNode
}

func NewVeniceNodeCollection(client objClient.ObjClient, testbed *testbed.TestBed) *VeniceNodeCollection {
	return &VeniceNodeCollection{
		CollectionCommon: CollectionCommon{Client: client,
			Testbed: testbed},
	}
}

//NewVeniceNode returns venice node
func NewVeniceNode(node *testbed.TestNode) *VeniceNode {
	return &VeniceNode{
		testNode: node,
		iotaNode: node.GetIotaNode(),
	}
}

// GenVeniceIPs get venice IPs
func (vnc *VeniceNodeCollection) GenVeniceIPs() []string {

	ipAddrs := []string{}
	for _, node := range vnc.Nodes {
		ipAddrs = append(ipAddrs, node.iotaNode.GetIpAddress())
	}

	return ipAddrs
}

func (vnc *VeniceNodeCollection) Error() error {
	return vnc.err
}

// Leader returns the leader node
func (vnc *VeniceNodeCollection) Leader() *VeniceNodeCollection {
	if vnc.HasError() {
		return vnc
	}

	cl, err := vnc.Client.GetCluster()
	if err != nil {
		vnc.err = err
		return vnc
	}
	nvnc := &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	// get the cluster from venice
	for _, node := range vnc.Nodes {
		if cl.Status.Leader == node.iotaNode.Name {
			nvnc.Nodes = []*VeniceNode{node}
			return nvnc
		} else if cl.Status.Leader == node.iotaNode.IpAddress {
			nvnc.Nodes = []*VeniceNode{node}
			return nvnc
		}
	}

	vnc = &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	vnc.SetError(fmt.Errorf("Could not find a leader node"))

	return vnc
}

// NonLeaders returns all nodes except leaders
func (vnc *VeniceNodeCollection) NonLeaders() *VeniceNodeCollection {
	if vnc.HasError() {
		return vnc
	}

	cl, err := vnc.Client.GetCluster()
	if err != nil {
		vnc.err = err
		return vnc
	}

	nonLeaders := VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	for _, node := range vnc.Nodes {
		if cl.Status.Leader == node.iotaNode.Name || cl.Status.Leader == node.iotaNode.IpAddress {
			continue
		}
		nonLeaders.Nodes = append(nonLeaders.Nodes, node)
	}

	if len(nonLeaders.Nodes) == 0 {
		nonLeaders.err = fmt.Errorf("Could not find a leader node")
	}

	return &nonLeaders
}

// Any returns the requested number of venice from collection in random
func (vnc *VeniceNodeCollection) Any(num int) *VeniceNodeCollection {
	if vnc.HasError() || len(vnc.Nodes) <= num {
		return vnc
	}

	newVnc := &VeniceNodeCollection{Nodes: []*VeniceNode{}, CollectionCommon: vnc.CollectionCommon}
	tmpArry := make([]*VeniceNode, len(vnc.Nodes))
	copy(tmpArry, vnc.Nodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newVnc.Nodes = append(newVnc.Nodes, sn)
	}

	return newVnc
}

// SelectByPercentage returns a collection with the specified venice nodes based on percentage.
func (vnc *VeniceNodeCollection) SelectByPercentage(percent int) (*VeniceNodeCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if vnc.err != nil {
		return nil, fmt.Errorf("venice collection error (%s)", vnc.err)
	}

	ret := &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	for i, node := range vnc.Nodes {
		ret.Nodes = append(ret.Nodes, node)
		if (i + 1) >= len(vnc.Nodes)*percent/100 {
			break
		}
	}

	if len(ret.Nodes) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
	return ret, nil
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
	if vnc.HasError() {
		return nil, fmt.Errorf("node collection error (%s)", vnc.err)
	}
	ret := &VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}
	params, err := parseSelectorString(sel)
	if err != nil {
		return ret, fmt.Errorf("could not parse selector")
	}
	var notFound []string
nodeLoop:
	for _, name := range params.names {
		for _, node := range vnc.Nodes {
			if node.iotaNode.Name == name {
				ret.Nodes = append(ret.Nodes, node)
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

func (vnc *VeniceNodeCollection) CaptureGRETCPDump(ctx context.Context) (string, error) {

	trig := vnc.Testbed.NewTrigger()

	trig.AddBackgroundCommand("tcpdump -x -nni eth0 ip proto gre",
		vnc.Nodes[0].Name()+"_venice", vnc.Nodes[0].Name())

	resp, err := trig.Run()
	if err != nil {
		return "", fmt.Errorf("Error running command %v", err.Error())
	}

	<-ctx.Done()
	stopResp, err := trig.StopCommands(resp)
	if err != nil {
		return "", fmt.Errorf("Error stopping command %v", err.Error())
	}

	return stopResp[0].GetStdout(), nil
}

func (vnc *VeniceNodeCollection) GetGRETCPDumpCount(ctx context.Context) (int, error) {

	trig := vnc.Testbed.NewTrigger()

	trig.AddBackgroundCommand("tcpdump -x -nni eth0 ip proto gre -w test.pcap",
		vnc.Nodes[0].Name()+"_venice", vnc.Nodes[0].Name())

	resp, err := trig.Run()
	if err != nil {
		return 0, fmt.Errorf("Error running command %v", err.Error())
	}

	<-ctx.Done()
	stopResp, err := trig.StopCommands(resp)
	if err != nil {
		return 0, fmt.Errorf("Error stopping command %v", err.Error())
	}

	trig = vnc.Testbed.NewTrigger()

	trig.AddCommand("tcpdump -r test1.pcap  | wc -l",
		vnc.Nodes[0].Name()+"_venice", vnc.Nodes[0].Name())

	resp, err = trig.Run()
	if err != nil {
		return 0, fmt.Errorf("Error running command %v", err.Error())
	}

	return strconv.Atoi(strings.TrimSuffix(stopResp[0].GetStdout(), "\n"))
}

//GetVeniceNodeWithService  Get nodes running service
func (vnc *VeniceNodeCollection) GetVeniceNodeWithService(service string) (*VeniceNodeCollection, error) {
	if vnc.err != nil {
		return nil, vnc.err
	}
	srvVnc := VeniceNodeCollection{CollectionCommon: vnc.CollectionCommon}

	leader := vnc.Leader()

	//There is any error
	if leader.err != nil {
		return nil, leader.err
	}

	trig := vnc.Testbed.NewTrigger()

	entity := leader.Nodes[0].iotaNode.Name + "_venice"

	cmd := `/pensando/iota/bin/kubectl get pods -a --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | select(.metadata.labels.name == ` + fmt.Sprintf("%q", service) +
		` ) | .status.hostIP'`
	trig.AddCommand(cmd, entity, leader.Nodes[0].iotaNode.Name)

	// trigger commands
	triggerResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run command to get service node Err: %v", err)
		srvVnc.err = fmt.Errorf("Failed to run command to get service node")
		return nil, srvVnc.err
	}

	if triggerResp[0].ExitCode != 0 {
		srvVnc.err = fmt.Errorf("Failed to run command to get service node : %v",
			triggerResp[0].Stderr)
		return nil, srvVnc.err
	}

	ret := triggerResp[0].Stdout
	hostIP := strings.Split(ret, "\n")

	for _, vn := range vnc.Nodes {
		for _, ip := range hostIP {
			if vn.iotaNode.IpAddress == ip {
				srvVnc.Nodes = append(srvVnc.Nodes, vn)
			}
		}
	}

	if len(srvVnc.Nodes) == 0 {
		log.Errorf("Did not find node running %v", service)
		srvVnc.err = fmt.Errorf("Did not find node running %v", service)
		return nil, srvVnc.err
	}
	return &srvVnc, nil
}
