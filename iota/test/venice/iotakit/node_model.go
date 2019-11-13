// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"bytes"
	"context"
	"fmt"
	"math/rand"
	"strings"
	"time"

	"github.com/willf/bitset"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/events/generated/eventtypes"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
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
	name     string
	iotaNode *iota.Node
	testNode *TestNode
	smartNic *cluster.DistributedServiceCard
	sm       *SysModel // pointer back to the model
}

// Switch represents a data-switch

type Switch struct {
	dataSwitch *iota.DataSwitch
}

// SwitchPort represents a port in a data switch
type SwitchPort struct {
	sw       *Switch
	hostName string
	port     string
}

// VeniceNode represents a venice node
type VeniceNode struct {
	cnode    *cluster.Node
	iotaNode *iota.Node
	testNode *TestNode
	sm       *SysModel // pointer back to the model
}

// HostCollection is collection of hosts
type HostCollection struct {
	err       error
	hosts     []*Host
	fakeHosts []*Host
}

// SwitchPortCollection ports
type SwitchPortCollection struct {
	err   error
	ports []*SwitchPort
}

// VeniceNodeCollection is collection of venice nodes
type VeniceNodeCollection struct {
	err   error
	sm    *SysModel // pointer back to the model
	nodes []*VeniceNode
}

// NaplesCollection contains a list of naples nodes
type NaplesCollection struct {
	err       error
	nodes     []*Naples
	fakeNodes []*Naples
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
func (sm *SysModel) createSwitch(sw *iota.DataSwitch) (*Switch, error) {

	smSw := &Switch{dataSwitch: sw}

	getHostName := func(sw *SwitchPort) (string, error) {
		for _, node := range sm.tb.Nodes {
			for _, dn := range node.instParams.DataNetworks {
				if dn.Name == sw.port && dn.SwitchIP == sw.sw.dataSwitch.Ip {
					return node.NodeName, nil
				}
			}
		}
		return "", fmt.Errorf("Node name not found for switch %v %v", sw.port, sw.sw.dataSwitch.Ip)
	}
	var err error
	for _, p := range sw.GetPorts() {
		swPort := &SwitchPort{port: p, sw: smSw}
		swPort.hostName, err = getHostName(swPort)
		if err != nil {
			return nil, err
		}
		sm.switchPorts = append(sm.switchPorts, swPort)
	}
	sm.switches[sw.GetIp()] = smSw

	return smSw, nil
}

// createHost creates a new host instance
func (sm *SysModel) createHost(n *TestNode) (*Host, error) {
	// find the naples
	naples, ok := sm.naples[n.NodeName]
	if !ok || naples.smartNic == nil {
		return nil, fmt.Errorf("Could not find the naples object for snic: %v", n.NodeName)
	}
	host := &cluster.Host{
		TypeMeta: api.TypeMeta{Kind: "Host"},
		ObjectMeta: api.ObjectMeta{
			Name: n.NodeName,
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: naples.smartNic.Status.PrimaryMAC,
				},
			},
		},
	}

	// create the host in venice
	err := sm.CreateHost(host)
	if err != nil {
		log.Errorf("Error creating host: %+v. Err: %v", host, err)
		//Check whether host is already ADMITTED
		hosts, _ := sm.ListHost()
		for _, curHost := range hosts {
			if curHost.ObjectMeta.Name == n.NodeName {
				host = curHost
				break
			}
		}
		if host == nil {
			return nil, err
		}
	}

	bs := bitset.New(uint(4096))
	bs.Set(0).Set(1).Set(4095)

	h := Host{
		iotaNode:    n.iotaNode,
		veniceHost:  host,
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

	for _, hst := range sm.fakeHosts {
		hc.fakeHosts = append(hc.fakeHosts, hst)
	}

	return &hc
}

// SwitchPorts returns list of all switch ports
func (sm *SysModel) SwitchPorts() *SwitchPortCollection {
	var swPc SwitchPortCollection
	for _, port := range sm.switchPorts {
		swPc.ports = append(swPc.ports, port)
	}

	return &swPc
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

/*
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
			w, err := host.sm.createWorkload(host.sm.Topo.WorkloadType, host.sm.tb.Topo.WorkloadImage, name, host, subnet)
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
*/

// createNaples creates a naples instance
func (sm *SysModel) createNaples(node *TestNode) error {
	snic, err := sm.GetSmartNICByName(node.NodeName)
	if sm.tb.mockMode {
		snic, err = sm.GetSmartNICInMacRange(node.NodeUUID)
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

// createNaples creates a naples instance
func (sm *SysModel) createMultiSimNaples(node *TestNode) error {

	numInstances := node.NaplesMultSimConfig.GetNumInstances()
	if len(node.iotaNode.GetNaplesMultiSimConfig().GetSimsInfo()) != int(numInstances) {
		err := fmt.Errorf("Number of instances mismatch in iota node and config expected (%v), actual (%v)",
			numInstances, len(node.iotaNode.GetNaplesMultiSimConfig().GetSimsInfo()))
		log.Errorf("%v", err)
		return err

	}
	log.Infof("Adding fake naples : %v", (node.NaplesMultSimConfig.GetNumInstances()))

	success := false
	var err error
	for i := 0; i < 3; i++ {
		var snicList []*cluster.DistributedServiceCard
		snicList, err = sm.ListSmartNIC()
		if err != nil {
			continue
		}
		for _, simInfo := range node.iotaNode.GetNaplesMultiSimConfig().GetSimsInfo() {
			//TODO: (iota agent is also following the same format.)
			simName := simInfo.GetName()
			success = false
			for _, snic := range snicList {
				if snic.Spec.ID == simName {
					naples := Naples{
						iotaNode: node.iotaNode,
						testNode: node,
						smartNic: snic,
						sm:       sm,
						name:     simName,
					}
					sm.fakeNaples[simName] = &naples
					success = true
				}
			}

			if !success {
				err = fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", node.NodeName, err)
				log.Errorf("%v", err)
				break
			}
		}
		//All got added, success!
		if success {
			break
		}
	}

	if !success {
		return fmt.Errorf("Errorr adding fake naples  %v", err.Error())
	}

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

// ForEachFakeNaples calls an iterator for each naples in the model
func (sm *SysModel) ForEachFakeNaples(fn naplesIteratorFn) error {
	fakeNaples := []*Naples{}
	for _, node := range sm.fakeNaples {
		fakeNaples = append(fakeNaples, node)
	}
	err := fn(&NaplesCollection{fakeNodes: fakeNaples})
	if err != nil {
		return err
	}
	return nil
}

// Naples returns all naples in the model
func (sm *SysModel) Naples() *NaplesCollection {
	var naples []*Naples
	var fakesNaples []*Naples
	for _, np := range sm.naples {
		naples = append(naples, np)
	}

	for _, np := range sm.fakeNaples {
		fakesNaples = append(fakesNaples, np)
	}
	return &NaplesCollection{nodes: naples, fakeNodes: fakesNaples}
}

// Names retruns names of all naples in the collection
func (npc *NaplesCollection) Names() []string {
	var ret []string
	for _, n := range npc.nodes {
		ret = append(ret, n.smartNic.ObjectMeta.Name)
	}

	return ret
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

func (sm *SysModel) createVeniceNode(node *TestNode) error {
	vn := VeniceNode{
		iotaNode: node.iotaNode,
		testNode: node,
		sm:       sm,
	}

	sm.veniceNodes[node.iotaNode.Name] = &vn

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

// GenVeniceIPs get venice IPs
func (vnc *VeniceNodeCollection) GenVeniceIPs() []string {

	ipAddrs := []string{}
	for _, node := range vnc.nodes {
		ipAddrs = append(ipAddrs, node.iotaNode.GetIpAddress())
	}

	return ipAddrs
}

// Leader returns the leader node
func (vnc *VeniceNodeCollection) Leader() *VeniceNodeCollection {
	if vnc.err != nil {
		return vnc
	}
	// get the cluster from venice
	cl, err := vnc.sm.GetCluster()
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

//CaptureGRETCPDump tcpdump untill ctx is done
func (vnc *VeniceNodeCollection) CaptureGRETCPDump(ctx context.Context) (string, error) {

	trig := vnc.sm.tb.NewTrigger()

	trig.AddBackgroundCommand("tcpdump -x -nni eth0 ip proto gre",
		vnc.nodes[0].iotaNode.Name+"_venice", vnc.nodes[0].iotaNode.Name)

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

// NonLeaders returns all nodes except leaders
func (vnc *VeniceNodeCollection) NonLeaders() *VeniceNodeCollection {
	if vnc.err != nil {
		return vnc
	}
	// get the cluster from venice
	cl, err := vnc.sm.GetCluster()
	if err != nil {
		return &VeniceNodeCollection{err: fmt.Errorf("Failed to get cluster. Err: %v", err)}
	}

	nonLeaders := VeniceNodeCollection{sm: vnc.sm}
	for _, node := range vnc.nodes {
		if cl.Status.Leader == node.iotaNode.Name || cl.Status.Leader == node.iotaNode.IpAddress {
			continue
		}
		nonLeaders.nodes = append(nonLeaders.nodes, node)
	}

	if len(nonLeaders.nodes) == 0 {
		nonLeaders.err = fmt.Errorf("Could not find a leader node")
	}

	return &nonLeaders
}

// Any returns the requested number of venice from collection in random
func (vnc *VeniceNodeCollection) Any(num int) *VeniceNodeCollection {
	if vnc.err != nil || len(vnc.nodes) <= num {
		return vnc
	}

	newVnc := &VeniceNodeCollection{nodes: []*VeniceNode{}}
	tmpArry := make([]*VeniceNode, len(vnc.nodes))
	copy(tmpArry, vnc.nodes)
	for i := 0; i < num; i++ {
		idx := rand.Intn(len(tmpArry))
		sn := tmpArry[idx]
		tmpArry = append(tmpArry[:idx], tmpArry[idx+1:]...)
		newVnc.nodes = append(newVnc.nodes, sn)
	}

	return newVnc
}

//GetVeniceNodeWithService  Get nodes running service
func (vnc *VeniceNodeCollection) GetVeniceNodeWithService(service string) (*VeniceNodeCollection, error) {
	if vnc.err != nil {
		return nil, vnc.err
	}
	srvVnc := VeniceNodeCollection{sm: vnc.sm}

	leader := vnc.sm.VeniceNodes().Leader()

	//There is any error
	if leader.err != nil {
		return nil, leader.err
	}

	trig := vnc.sm.tb.NewTrigger()

	entity := leader.nodes[0].iotaNode.Name + "_venice"

	cmd := `/pensando/iota/bin/kubectl get pods -a --all-namespaces -o json  | /usr/local/bin/jq-linux64 -r '.items[] | select(.metadata.labels.name == ` + fmt.Sprintf("%q", service) +
		` ) | .status.hostIP'`
	trig.AddCommand(cmd, entity, leader.nodes[0].iotaNode.Name)

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

	for _, vn := range vnc.sm.veniceNodes {
		for _, ip := range hostIP {
			if vn.iotaNode.IpAddress == ip {
				srvVnc.nodes = append(srvVnc.nodes, vn)
			}
		}
	}

	if len(srvVnc.nodes) == 0 {
		log.Errorf("Did not find node running %v", service)
		srvVnc.err = fmt.Errorf("Did not find node running %v", service)
		return nil, srvVnc.err
	}
	return &srvVnc, nil
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

// SelectByPercentage returns a collection with the specified venice nodes based on percentage.
func (vnc *VeniceNodeCollection) SelectByPercentage(percent int) (*VeniceNodeCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if vnc.err != nil {
		return nil, fmt.Errorf("venice collection error (%s)", vnc.err)
	}

	ret := &VeniceNodeCollection{sm: vnc.sm}
	for i, node := range vnc.nodes {
		ret.nodes = append(ret.nodes, node)
		if (i + 1) >= len(vnc.nodes)*percent/100 {
			break
		}
	}

	if len(ret.nodes) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
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

// ListRealHosts gets all real hosts from venice cluster
func (sm *SysModel) ListRealHosts() ([]*Host, error) {

	hosts := []*Host{}
	for _, h := range sm.hosts {
		if h.iotaNode.Type == iota.PersonalityType_PERSONALITY_NAPLES {
			hosts = append(hosts, h)
		}
	}

	return hosts, nil
}

// AssociateHosts gets all real hosts from venice cluster
func (sm *SysModel) AssociateHosts() error {
	objs, err := sm.ListHost()
	if err != nil {
		return err
	}

	if sm.tb.IsMockMode() {
		//One on One association if mock mode
		convertMac := func(s string) string {
			mac := strings.Replace(s, ".", "", -1)
			var buffer bytes.Buffer
			var l1 = len(mac) - 1
			for i, rune := range mac {
				buffer.WriteRune(rune)
				if i%2 == 1 && i != l1 {
					buffer.WriteRune(':')
				}
			}
			return buffer.String()
		}
		//In mockmode we retrieve mac from the snicList, so we should convert to format that venice likes.
		for _, naples := range sm.naples {
			naples.iotaNode.NodeUuid = convertMac(naples.iotaNode.NodeUuid)
		}
	}

	for k := range sm.hosts {
		delete(sm.hosts, k)
	}
	//TOOD:Associate fake naples too
	for _, n := range sm.naples {
		n.smartNic.Labels = make(map[string]string)
		nodeMac := strings.Replace(n.iotaNode.GetNodeUuid(), ":", "", -1)
		for _, obj := range objs {
			objMac := strings.Replace(obj.GetSpec().DSCs[0].MACAddress, ".", "", -1)
			if objMac == nodeMac {
				log.Infof("Associating host %v(ip:%v) with %v(ip:%v)\n", obj.GetName(),
					n.iotaNode.GetIpAddress(), n.iotaNode.Name,
					n.smartNic.GetStatus().IPConfig.IPAddress)
				bs := bitset.New(uint(4096))
				bs.Set(0).Set(1).Set(4095)
				h := Host{
					iotaNode:    n.iotaNode,
					veniceHost:  obj,
					naples:      n,
					maxVlans:    4094,
					vlanBitmask: bs,
					sm:          sm,
				}
				sm.hosts[obj.GetName()] = &h

				//Add BM type to support upgrade
				n.smartNic.Labels["type"] = "bm"
				if err := sm.UpdateSmartNIC(n.smartNic); err != nil {
					log.Infof("Error updating smart nic object %v", err)
					return err
				}
				break
			}
		}
	}

	for k := range sm.fakeHosts {
		delete(sm.fakeHosts, k)
	}
	sm.ListSmartNIC()
	for simName, n := range sm.fakeNaples {
		n.smartNic.Labels = make(map[string]string)
		for _, simNaples := range n.iotaNode.GetNaplesMultiSimConfig().GetSimsInfo() {
			if simNaples.GetName() == simName {
				nodeMac := strings.Replace(simNaples.GetNodeUuid(), ":", "", -1)
				for _, obj := range objs {
					objMac := strings.Replace(obj.GetSpec().DSCs[0].MACAddress, ".", "", -1)
					if objMac == nodeMac {
						log.Infof("Associating host %v(ip:%v) with %v(%v on %v)\n", obj.GetName(),
							n.iotaNode.GetIpAddress(), simName, simNaples.GetIpAddress(), n.iotaNode.Name)
						bs := bitset.New(uint(4096))
						bs.Set(0).Set(1).Set(4095)
						h := Host{
							iotaNode:    n.iotaNode,
							veniceHost:  obj,
							naples:      n,
							maxVlans:    4094,
							vlanBitmask: bs,
							sm:          sm,
						}
						sm.fakeHosts[obj.GetName()] = &h
						//Add BM type to support upgrade
						n.smartNic.Labels["type"] = "sim"
						if err := sm.UpdateSmartNIC(n.smartNic); err != nil {
							log.Infof("Error updating smart nic object %v", err)
							return err
						}
						break
					}
				}
			}
		}
	}
	log.Infof("Total number of hosts associated %v\n", len(sm.hosts)+len(sm.fakeHosts))
	return nil
}

// ListWorkloadsOnHost gets workloads on host
func (sm *SysModel) ListWorkloadsOnHost(h *cluster.Host) (objs []*workload.Workload, err error) {
	objs, err = sm.ListWorkload()
	if err != nil {
		return nil, err
	}

	i := 0
	for _, obj := range objs {
		if obj.Spec.HostName == h.ObjectMeta.Name {
			objs[i] = obj
			i++
		}
	}

	return objs[:i], nil
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
	for _, host := range hc.hosts {
		ret.hosts = append(ret.hosts, host)
		if (len(ret.hosts)) >= len(hc.hosts)*percent/100 {
			break
		}
	}

	for _, host := range hc.fakeHosts {
		ret.fakeHosts = append(ret.fakeHosts, host)
		if (len(ret.hosts) + len(ret.fakeHosts)) >= (len(hc.fakeHosts)+len(hc.hosts))*percent/100 {
			break
		}
	}

	if len(ret.hosts)+len(ret.fakeHosts) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
	return ret, nil
}

// SelectByPercentage returns a collection on switch ports based on percentage.
func (pc *SwitchPortCollection) SelectByPercentage(percent int) (*SwitchPortCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if pc.err != nil {
		return nil, fmt.Errorf("switch port collection error (%s)", pc.err)
	}

	//Get equal share from each host to be fair
	hostPortMap := make(map[string]*[]*SwitchPort)
	for _, port := range pc.ports {
		if _, ok := hostPortMap[port.hostName]; !ok {
			hostPortMap[port.hostName] = &[]*SwitchPort{}
		}
		swPorts, _ := hostPortMap[port.hostName]
		*swPorts = append(*swPorts, port)
	}

	ret := &SwitchPortCollection{}
	for _, ports := range hostPortMap {
		for i, port := range *ports {
			added := false
			for _, addedPort := range ret.ports {
				if addedPort == port {
					added = true
					break
				}
			}
			if !added {
				ret.ports = append(ret.ports, port)
				//Break if reached limit for this host
				if (i + 1) >= len(*ports)*percent/100 {
					break
				}
			}
		}
		//If adding one more will execeed limit
		if len(ret.ports) >= len(pc.ports)*percent/100 {
			break
		}
	}

	if len(ret.ports) == 0 {
		return nil, fmt.Errorf("Did not find ports matching percentage (%v)", percent)
	}
	return ret, nil
}

// SelectByPercentage returns a collection with the specified napls based on percentage.
func (naples *NaplesCollection) SelectByPercentage(percent int) (*NaplesCollection, error) {
	if percent > 100 {
		return nil, fmt.Errorf("Invalid percentage input %v", percent)
	}

	if naples.err != nil {
		return nil, fmt.Errorf("naples collection error (%s)", naples.err)
	}

	ret := &NaplesCollection{}
	for _, entry := range naples.nodes {
		ret.nodes = append(ret.nodes, entry)
		if (len(ret.nodes)) >= (len(naples.nodes)+len(naples.fakeNodes))*percent/100 {
			break
		}
	}

	for _, entry := range naples.fakeNodes {

		if (len(ret.nodes) + len(ret.fakeNodes)) >= (len(naples.nodes)+len(naples.fakeNodes))*percent/100 {
			break
		}
		ret.fakeNodes = append(ret.fakeNodes, entry)
	}

	if (len(ret.nodes) + len(ret.fakeNodes)) == 0 {
		return nil, fmt.Errorf("Did not find hosts matching percentage (%v)", percent)
	}
	return ret, nil
}

// ServiceStoppedEvents returns list of service stopped events.
func (sm *SysModel) ServiceStoppedEvents(since time.Time, npc *NaplesCollection) *EventsCollection {
	var naplesNames []string
	for _, naples := range npc.nodes {
		naplesNames = append(naplesNames, naples.iotaNode.Name)
	}

	fieldSelector := fmt.Sprintf("type=%s,meta.mod-time>=%v,object-ref.kind=DistributedServiceCard,object-ref.name in (%v)",
		eventtypes.SERVICE_STOPPED, since.Format(time.RFC3339Nano), fmt.Sprintf("%s", strings.Join(naplesNames, ",")))

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &EventsCollection{err: err}
	}

	for _, ev := range eventsList.Items {
		log.Errorf("Service stopped event %v %v", ev.Name, ev.Message)
	}
	return &EventsCollection{list: eventsList}
}
