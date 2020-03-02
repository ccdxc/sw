// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package base

import (
	"context"
	"errors"
	"fmt"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	utils "github.com/pensando/sw/iota/test/venice/iotakit/model/utils"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// CollectLogs collects all logs files from the testbed

// SetUpNaplesAuthenticationOnHosts changes naples to managed mode
func (sm *SysModel) SetUpNaplesAuthenticationOnHosts(hc *objects.HostCollection) error {

	testNodes := []*testbed.TestNode{}

	for _, node := range hc.Hosts {
		for _, testNode := range sm.Tb.Nodes {
			if node.GetIotaNode().Name == testNode.GetIotaNode().Name {
				testNodes = append(testNodes, testNode)
			}
		}
	}

	return sm.SetUpNaplesPostCluster(testNodes)
}

// ReloadHosts reloads a host
func (sm *SysModel) ReloadHosts(hc *objects.HostCollection) error {

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	for _, hst := range hc.Hosts {
		nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: hst.Name()})
		hostNames += hst.Name() + " "
	}

	iotaNodes := make(map[string]bool)
	for _, hst := range hc.FakeHosts {
		if _, ok := iotaNodes[hst.Name()]; ok {
			continue
		}
		iotaNodes[hst.Name()] = true
		nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: hst.Name()})
		hostNames += hst.Name() + " "
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	log.Infof("Reloading.Hosts: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload.Hosts %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload.Hosts %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)
	return sm.SetUpNaplesAuthenticationOnHosts(hc)
}

// ReloadHosts reloads a host
func (sm *SysModel) ReloadFakeHosts(hc *objects.HostCollection) error {

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	iotaNodes := make(map[string]bool)
	for _, hst := range hc.FakeHosts {
		if _, ok := iotaNodes[hst.Name()]; ok {
			continue
		}
		iotaNodes[hst.Name()] = true
		nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: hst.Name()})
		hostNames += hst.Name() + " "
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	log.Infof("Reloading hosts: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload hosts %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload hosts %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)
	return sm.SetUpNaplesAuthenticationOnHosts(hc)
}

// ReloadVeniceNodes reloads a venice node
func (sm *SysModel) ReloadVeniceNodes(vnc *objects.VeniceNodeCollection) error {

	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	// add each node
	for _, node := range vnc.Nodes {
		log.Infof("Reloading venice node %v", node.Name())
		nodeMsg.Nodes = append(nodeMsg.Nodes, node.GetTestNode().GetIotaNode())
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	// Trigger reload
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload venice.Nodes %v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload node %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	nodes := []*testbed.TestNode{}
	for _, node := range vnc.Nodes {
		nodes = append(nodes, node.GetTestNode())
	}
	return sm.RestoreVeniceDefaults(nodes)
}

//DisconnectVeniceNodesFromCluster disconnect venice.Nodes from cluster.
func (sm *SysModel) DisconnectVeniceNodesFromCluster(vnc *objects.VeniceNodeCollection, naples *objects.NaplesCollection) error {

	trig := sm.Tb.NewTrigger()

	for _, venice := range vnc.Nodes {

		if _, ok := sm.VeniceNodesMapDisconnected[venice.Name()]; ok {
			log.Errorf("Venice node %v alreadt disconnected", venice.Name)
			continue
		}
		for otherNode := range sm.VeniceNodeMap {
			if venice.Name() == otherNode {
				continue
			}
			cmd := fmt.Sprintf("sudo iptables -A INPUT -s %v -j DROP", otherNode)
			trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
				venice.Name(), 3)
		}

		for otherNode := range sm.VeniceNodesMapDisconnected {
			if venice.Name() == otherNode {
				continue
			}
			cmd := fmt.Sprintf("sudo iptables -A INPUT -s %v -j DROP", otherNode)
			trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
				venice.Name(), 3)
		}

	}

	// run the trigger
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running disonnect command on venice. Err: %v", resp)
		return fmt.Errorf("Error running disonnect command on venice. Err: %v", resp)
	}

	for _, cmd := range resp {
		if cmd.ExitCode != 0 {
			log.Errorf("Error running disonnect command on venice. Err: %v", cmd)
			return fmt.Errorf("Error running disonnect command on venice. Err: %v", cmd)
		}
	}

	for _, venice := range vnc.Nodes {
		for name, otherNode := range sm.VeniceNodeMap {
			if venice.Name() == name {
				sm.VeniceNodesMapDisconnected[name] = otherNode
				delete(sm.VeniceNodeMap, name)
			}
		}
	}

	//Disconnect naples.Nodes too
	if naples != nil && (len(naples.Nodes) != 0 || len(naples.FakeNodes) != 0) {
		sm.DenyVeniceNodesFromNaples(vnc, naples)
	}
	return sm.InitCfgModel()
}

//ConnectVeniceNodesToCluster  reconnect venice.Nodes to cluster.
func (sm *SysModel) ConnectVeniceNodesToCluster(vnc *objects.VeniceNodeCollection, naples *objects.NaplesCollection) error {
	trig := sm.Tb.NewTrigger()

	for _, venice := range vnc.Nodes {
		if _, ok := sm.VeniceNodesMapDisconnected[venice.Name()]; !ok {
			log.Errorf("Venice node %v not disconnected to be connected", venice.Name())
			continue
		}
		for otherNode := range sm.VeniceNodeMap {
			if venice.Name() == otherNode {
				continue
			}
			cmd := fmt.Sprintf("sudo iptables -L INPUT --line-numbers | grep  -w %v | grep -i drop | awk '{print $1}' | sort -n -r | xargs -n 1 sudo iptables -D INPUT $1", otherNode)
			trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
				venice.Name(), 3)
		}

		//Make sure remove entry disonnected node too.
		for otherNode := range sm.VeniceNodesMapDisconnected {
			if venice.Name() == otherNode {
				continue
			}
			cmd := fmt.Sprintf("sudo iptables -L INPUT --line-numbers | grep -w %v | grep -i drop | awk '{print $1}' | sort -n -r | xargs -n 1 sudo iptables -D INPUT $1", otherNode)
			trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
				venice.Name(), 3)
		}
	}

	// run the trigger , iptables should be serial.
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running disonnect command on venice. Err: %v", resp)
		//Ignore error as the rule may not be there.
	}

	for _, cmd := range resp {
		if cmd.ExitCode != 0 {
			log.Errorf("Error running connect venice. Err: %v", cmd)
			return fmt.Errorf("Error running connect venice. Err: %v", cmd)
		}
	}

	//Node is back to be connected.
	for _, venice := range vnc.Nodes {
		for name, otherNode := range sm.VeniceNodesMapDisconnected {
			if venice.Name() == name {
				sm.VeniceNodeMap[name] = otherNode
				delete(sm.VeniceNodesMapDisconnected, name)
			}
		}
	}
	//Allow naples.Nodes too
	if naples != nil && (len(naples.Nodes) != 0 || len(naples.FakeNodes) != 0) {
		sm.AllowVeniceNodesFromNaples(vnc, naples)
	}

	return sm.InitCfgModel()
}

func getRuleCookie(naples *objects.NaplesCollection) (string, error) {
	//For later deletion
	cookie := ""
	if len(naples.Nodes) == 0 {
		if len(naples.FakeNodes) == 0 {
			return "", errors.New("No node to add deny rules")
		}
		cookie = "rule-id-" + naples.FakeNodes[0].Name()
	} else {
		cookie = "rule-id-" + naples.Nodes[0].Name()

	}

	return cookie, nil
}

//DenyVeniceNodesFromNaples denies venice node from list of naples
func (sm *SysModel) DenyVeniceNodesFromNaples(vnc *objects.VeniceNodeCollection,
	naples *objects.NaplesCollection) error {

	trig := sm.Tb.NewTrigger()

	//For later deletion
	cookie, err := getRuleCookie(naples)
	if err != nil {
		return err
	}

	for _, venice := range vnc.Nodes {

		for _, n := range naples.Nodes {
			cmd := fmt.Sprintf("sudo iptables -A INPUT -s %v -j DROP  -m comment --comment %s",
				strings.Split(n.SmartNic.GetStatus().IPConfig.IPAddress, "/")[0], cookie)
			trig.AddCommand(cmd, venice.Name()+"_venice", venice.Name())
			trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
				venice.Name(), 3)
		}
		for _, n := range naples.FakeNodes {
			cmd := fmt.Sprintf("sudo iptables -A INPUT -s %v -j DROP  -m comment --comment %s",
				strings.Split(n.SmartNic.GetStatus().IPConfig.IPAddress, "/")[0], cookie)
			trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
				venice.Name(), 3)
		}
	}

	// run the trigger
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running disonnect naples command on venice. Err: %v", resp)
		return fmt.Errorf("Error running disonnect  naples command on venice. Err: %v", resp)
	}

	for _, cmd := range resp {
		if cmd.ExitCode != 0 {
			log.Errorf("Error running deny on venice for naples . Err: %v", cmd)
			return fmt.Errorf("Error running deny on venice naples . Err: %v", cmd)
		}
	}

	return nil
}

//AllowVeniceNodesFromNaples allows venice to be connected from naples
func (sm *SysModel) AllowVeniceNodesFromNaples(vnc *objects.VeniceNodeCollection,
	naples *objects.NaplesCollection) error {
	trig := sm.Tb.NewTrigger()

	cookie, err := getRuleCookie(naples)
	if err != nil {
		return err
	}

	for _, venice := range vnc.Nodes {
		cmd := fmt.Sprintf("sudo iptables -L INPUT --line-numbers | grep  -w %v | grep -i drop | awk '{print $1}' | sort -n -r | xargs -n 1 sudo iptables -D INPUT $1",
			cookie)
		trig.AddCommandWithRetriesOnFailures(cmd, venice.Name()+"_venice",
			venice.Name(), 3)
	}

	// run the trigger should serial as we are modifying ipables.
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running disonnect command on venice. Err: %v", resp)
		//Ignore error as the rule may not be there.
	}

	for _, cmd := range resp {
		if cmd.ExitCode != 0 {
			log.Errorf("Error running allow on venice for naples . Err: %v", cmd)
			return fmt.Errorf("Error running allow on venice naples . Err: %v", cmd)
		}
	}

	return nil
}

// RunCommandOnVeniceNodes runs given command on venice node, not output
func (sm *SysModel) RunCommandOnVeniceNodes(vnc *objects.VeniceNodeCollection, cmd string) error {

	trig := sm.Tb.NewTrigger()
	// add each node
	for _, node := range vnc.Nodes {
		entity := node.Name() + "_venice"
		trig.AddCommand(cmd, entity, node.Name())
	}

	// trigger commands
	_, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to check cmd/etcd service status. Err: %v", err)
		return err
	}

	return nil
}

// ReloadNaples reloads naples.Nodes
func (sm *SysModel) ReloadNaples(npc *objects.NaplesCollection) error {

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	for _, node := range npc.Nodes {
		nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: node.NodeName()})
		hostNames += node.NodeName() + " "
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	log.Infof("Reloading Naples: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload Naples %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload Naples %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

// DisconnectNaples disconnects naples by bringing down its control interface
func (sm *SysModel) DisconnectNaples(npc *objects.NaplesCollection) error {

	trig := sm.Tb.NewTrigger()

	// ifconfig down command
	for _, naples := range npc.Nodes {
		for _, naplesConfig := range naples.GetIotaNode().GetNaplesConfigs().Configs {
			if naplesConfig.NodeUuid == naples.Nodeuuid {
				cmd := fmt.Sprintf("ifconfig %s down", naplesConfig.ControlIntf)
				trig.AddCommand(cmd, naplesConfig.Name, naples.NodeName())
			}
		}
	}

	// run the trigger
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error bringing down control interface on naples. Err: %v", err)
		return fmt.Errorf("Error bringing down control interface. Err: %v", err)
	}

	log.Debugf("Got Disconnect naples trigger resp: %+v", resp)

	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("ifconfig down failed. %+v", cmdResp)
			return fmt.Errorf("ifconfig down failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// ConnectNaples connects naples back to venice by bringing up control interface
func (sm *SysModel) ConnectNaples(npc *objects.NaplesCollection) error {

	trig := sm.Tb.NewTrigger()

	// ifconfig up command
	for _, naples := range npc.Nodes {
		for _, naplesConfig := range naples.GetIotaNode().GetNaplesConfigs().Configs {
			if naplesConfig.NodeUuid == naples.Nodeuuid {
				cmd := fmt.Sprintf("ifconfig %s %s/16 up", naplesConfig.ControlIntf, naplesConfig.ControlIp)
				trig.AddCommand(cmd, naplesConfig.Name, naples.NodeName())
			}
		}
	}

	// run the trigger
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error bringing up control interface on naples. Err: %v", err)
		return fmt.Errorf("Error bringing up control interface. Err: %v", err)
	}

	log.Debugf("Got connect naples trigger resp: %+v", resp)

	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("ifconfig up failed. %+v", cmdResp)
			return fmt.Errorf("ifconfig up failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// RunNaplesCommand runs the given naples command on the collection, returns []stdout
func (sm *SysModel) RunNaplesCommand(npc *objects.NaplesCollection, cmd string) ([]string, error) {

	if len(npc.Nodes) == 0 {
		return nil, nil
	}

	var stdout []string
	trig := sm.Tb.NewTrigger()

	for _, naples := range npc.Nodes {
		trig.AddCommand(cmd, naples.Name(), naples.NodeName())
	}

	// run the trigger
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("Error running command, Err: %v", err)
		return []string{}, fmt.Errorf("Error running command, Err: %v", err)
	}

	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("command failed. %+v", cmdResp)
			return []string{}, fmt.Errorf("command failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}

		stdout = append(stdout, cmdResp.Stdout)
	}

	return stdout, nil
}

// RunFakeNaplesBackgroundCommand runs the given fake naples command on the collection, return contxt
func (sm *SysModel) RunFakeNaplesBackgroundCommand(npc *objects.NaplesCollection, cmd string) (interface{}, error) {

	trig := sm.Tb.NewTrigger()

	for _, naples := range npc.FakeNodes {
		trig.AddBackgroundCommand(cmd, naples.Name(), naples.NodeName())
	}

	// run the trigger
	resp, err := trig.RunParallel()
	if err != nil {
		log.Errorf("Error running command, Err: %v", err)
		return []string{}, fmt.Errorf("Error running command, Err: %v", err)
	}

	return resp, nil
}

// StopCommands stops commands
func (sm *SysModel) StopCommands(cmdCtx interface{}) ([]string, error) {

	trig := sm.Tb.NewTrigger()

	cmd, _ := cmdCtx.([]*iota.Command)
	resp, _ := trig.StopCommands(cmd)

	var stdout []string
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("command failed. %+v", cmdResp)
			return []string{}, fmt.Errorf("command failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}

		stdout = append(stdout, cmdResp.Stdout)
	}

	return stdout, nil
}

// RunFakeNaplesCommand runs the given fake naples command on the collection, returns []stdout
func (sm *SysModel) RunFakeNaplesCommand(npc *objects.NaplesCollection, cmd string) ([]string, error) {

	var stdout []string
	trig := sm.Tb.NewTrigger()

	for _, naples := range npc.FakeNodes {
		trig.AddCommand(cmd, naples.Name(), naples.NodeName())
	}

	// run the trigger
	resp, err := trig.RunParallel()
	if err != nil {
		log.Errorf("Error running command, Err: %v", err)
		return []string{}, fmt.Errorf("Error running command, Err: %v", err)
	}

	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("command failed. %+v", cmdResp)
			return []string{}, fmt.Errorf("command failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}

		stdout = append(stdout, cmdResp.Stdout)
	}

	return stdout, nil
}

// PortFlap flaps the one of the port from each naples on the collection
func (sm *SysModel) PortFlap(npc *objects.NaplesCollection) error {

	for _, naples := range npc.Nodes {
		naplesName := naples.NodeName()
		cmd := "/nic/bin/halctl show port status"
		out, err := sm.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}

		// get port number from port status output
		portNum := utils.GetPortNum(out)
		if len(strings.TrimSpace(portNum)) == 0 {
			log.Errorf("failed to get a port number to flap on naples: %v", naplesName)
			return fmt.Errorf("failed to get a port number to flap on naples: %v", naplesName)
		}

		log.Infof("flapping port {%s} on naples {%v}", portNum, naplesName)

		// flap port
		cmd = fmt.Sprintf("/nic/bin/halctl debug port --port %s --admin-state down", portNum)
		_, err = sm.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}
		cmd = fmt.Sprintf("/nic/bin/halctl debug port --port %s --admin-state up", portNum)
		_, err = sm.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}
	}

	return nil
}

// GetNaplesEndpoints returns a map of map[<mac-address>]<vlan> indexed by naples name
func (sm *SysModel) GetNaplesEndpoints(npc *objects.NaplesCollection) (map[string]map[string]struct {
	Local bool
	Vlan  int
}, error) {
	trig := sm.Tb.NewTrigger()
	ret := make(map[string]map[string]struct {
		Local bool
		Vlan  int
	})
	for _, naples := range npc.Nodes {
		cmd := "/nic/bin/halctl show endpoint brief"
		trig.AddCommand(cmd, naples.Name(), naples.NodeName())
	}
	resp, err := trig.Run()
	if err != nil {
		log.Errorf("failed to run halctl commends on naples (%s)", err)
		return ret, fmt.Errorf("failed to run halctl commands on naples (%s)", err)
	}
	re := regexp.MustCompile(`^(\d+)\s+([0-9a-f]+\:[0-9a-f]+\:[0-9a-f]+\:[0-9a-f]+\:[0-9a-f]+\:[0-9a-f]+)\s+(\S+)`)
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("failed to run halctl command on naples [%v] (%d) [ %v]", cmdResp.NodeName, cmdResp.ExitCode, cmdResp.Stderr)
			return ret, fmt.Errorf("failed to run halctl command on naples [%v](%d)", cmdResp.NodeName, cmdResp.ExitCode)
		}
		macmap := make(map[string]struct {
			Local bool
			Vlan  int
		})
		for _, line := range strings.Split(cmdResp.Stdout, "\n") {
			if re.Match([]byte(line)) {
				strs := re.FindStringSubmatch(line)

				vlan, err := strconv.ParseInt(strs[1], 10, 32)
				if err != nil {
					vlan = -1
				}
				mac := strs[2]
				local := true
				if strings.HasPrefix(strs[3], "Uplink") {
					local = false
				}
				macmap[mac] = struct {
					Local bool
					Vlan  int
				}{local, int(vlan)}
			}
		}
		ret[cmdResp.NodeName] = macmap
	}
	return ret, nil
}

// runCommandOnGivenNaples runs the given command on given naples and returns stdout
func (sm *SysModel) runCommandOnGivenNaples(np *objects.Naples, cmd string) (string, error) {
	trig := sm.Tb.NewTrigger()

	trig.AddCommand(cmd, np.Name(), np.NodeName())
	resp, err := trig.Run()
	if err != nil {
		return "", err
	}

	cmdResp := resp[0]
	if cmdResp.ExitCode != 0 {
		return "", fmt.Errorf("command failed: %+v", cmdResp)
	}
	return cmdResp.Stdout, nil
}

// VeniceNodeLoggedInCtx creates logged in context by connecting to a specified venice node
func (sm *SysModel) VeniceNodeLoggedInCtx(vnc *objects.VeniceNodeCollection) error {

	nodeURL := fmt.Sprintf("%s:%s", vnc.Nodes[0].IP(), globals.APIGwRESTPort)
	_, err := sm.veniceNodeLoggedInCtxWithURL(nodeURL)
	if err != nil {
		log.Errorf("error logging in to venice node (%s): %v", nodeURL, err)
		return fmt.Errorf("error logging in to venice node (%s): %v", nodeURL, err)
	}
	return nil
}

// VeniceNodeCreateSnapshotConfig creates the default snapshot configuration on the cluster.
func (sm *SysModel) VeniceNodeCreateSnapshotConfig(vnc *objects.VeniceNodeCollection) error {

	return sm.CfgModel.ConfigureSnapshot()

}

// VeniceNodeCreateSnapshotConfig creates the default snapshot configuration on the cluster.
func (sm *SysModel) VeniceNodeTakeSnapshot(vnc *objects.VeniceNodeCollection) (string, error) {

	return sm.CfgModel.TakeConfigSnapshot("IotaTest")
}

// VeniceNodeCreateSnapshotConfig creates the default snapshot configuration on the cluster.
func (sm *SysModel) VeniceNodeRestoreConfig(vnc *objects.VeniceNodeCollection, name string) error {

	return sm.CfgModel.RestoreConfig(name)
}

// FlapDataSwitchPorts flaps data ports
func (sm *SysModel) FlapDataSwitchPorts(ports *objects.SwitchPortCollection, downTime time.Duration) error {
	switchMsg := &iota.SwitchMsg{
		ApiResponse:  &iota.IotaAPIResponse{},
		DataSwitches: []*iota.DataSwitch{},
	}

	for _, port := range ports.Ports {
		log.Infof("Flapping port %v(%v)", port.Port, port.SwitchIP())

		added := false
		for _, sw := range switchMsg.DataSwitches {
			if sw.GetIp() == port.SwitchIP() {
				added = true
				sw.Ports = append(sw.Ports, port.Port)
			}
		}
		if !added {
			switchMsg.DataSwitches = append(switchMsg.DataSwitches, &iota.DataSwitch{Ip: port.SwitchIP(),
				Password: port.SwitchPassword(), Username: port.SwitchUsernme(), Ports: []string{port.Port}})
		}
	}

	log.Infof("Flapping number of ports %v", switchMsg.DataSwitches[0].Ports)
	switchMsg.Op = iota.SwitchOp_SHUT_PORTS

	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	switchResp, err := topoClient.DoSwitchOperation(context.Background(), switchMsg)
	if err != nil {
		return fmt.Errorf("Failed to shut ports | Err: %v", err)
	} else if switchResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to shut ports  API Status: %+v | Err: %v", switchResp.ApiResponse, err)
	}

	log.Infof("Got switch port shut resp: %+v", switchResp)

	time.Sleep(downTime)

	switchMsg.Op = iota.SwitchOp_NO_SHUT_PORTS

	topoClient = iota.NewTopologyApiClient(sm.Tb.Client().Client)
	switchResp, err = topoClient.DoSwitchOperation(context.Background(), switchMsg)
	if err != nil {
		return fmt.Errorf("Failed to shut ports | Err: %v", err)
	} else if switchResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to shut ports  API Status: %+v | Err: %v", switchResp.ApiResponse, err)
	}

	log.Debugf("Got switch port not shut resp: %+v", switchResp)

	return nil
}

// FlapDataSwitchPortsPeriodically until context is cancelled or timeout
func (sm *SysModel) FlapDataSwitchPortsPeriodically(ctx context.Context, ports *objects.SwitchPortCollection,
	downTime time.Duration, flapInterval time.Duration, flapCount int) error {

	for i := 0; i < int(flapCount); i++ {
		//if cancelled
		sm.FlapDataSwitchPorts(ports, downTime)
		select {
		case <-ctx.Done():
			return nil
		default:
			time.Sleep(time.Duration(flapInterval))
		}
	}

	return nil
}

// RemoveAddNaples remove and add naples
func (sm *SysModel) RemoveAddNaples(naples *objects.NaplesCollection) error {

	names := []string{}
	for _, obj := range naples.Nodes {
		names = append(names, obj.NodeName())

	}

	if err := sm.DeleteNaplesNodes(names); err != nil {
		log.Errorf("Failed to delete naples.Nodes %v", names)
		return err
	}

	if err := sm.AddNaplesNodes(names); err != nil {
		log.Errorf("Failed to add naples node %v", names)
		return err
	}

	return nil
}

// RemoveAddVenice remove and add venice.Nodes
func (sm *SysModel) RemoveAddVenice(venice *objects.VeniceNodeCollection) error {

	names := []string{}
	for _, obj := range venice.Nodes {
		names = append(names, obj.Name())

	}

	if err := sm.DeleteVeniceNodes(names); err != nil {
		log.Errorf("Failed to delete venice.Nodes %v", names)
		return err
	}

	if err := sm.AddVeniceNodes(names); err != nil {
		log.Errorf("Failed to add venice node %v", names)
		return err
	}

	return nil
}

// RemoveVenice remove venice.Nodes
func (sm *SysModel) RemoveVenice(venice *objects.VeniceNodeCollection) error {

	names := []string{}
	for _, obj := range venice.Nodes {
		names = append(names, obj.Name())

	}

	if err := sm.DeleteVeniceNodes(names); err != nil {
		log.Errorf("Failed to delete venice.Nodes %v", names)
		return err
	}

	return nil
}

// AddVenice add venice.Nodes
func (sm *SysModel) AddVenice(venice *objects.VeniceNodeCollection) error {

	names := []string{}
	for _, obj := range venice.Nodes {
		names = append(names, obj.Name())

	}

	if err := sm.AddVeniceNodes(names); err != nil {
		log.Errorf("Failed to add venice node %v, err : %v", names, err)
		return err
	}

	return nil
}

// FlapMgmtLinkNaples flap mgmt link for naples
func (sm *SysModel) FlapMgmtLinkNaples(naples *objects.NaplesCollection) error {

	cmd := "ifconfig oob_mnic0 down && sleep 300 && ifconfig oob_mnic0 up && dhclient oob_mnic0"
	stdout, err := sm.RunNaplesCommand(naples, cmd)
	if err != nil {
		log.Errorf("Failed to flap mgmt link node %v :%v", err, stdout)
		return err
	}

	for i := 0; i < 3; i++ {
		cmd = "ifconfig eth0 down && ifconfig eth1 down && sleep 600 && ifconfig eth0 up && ifconfig eth1 up && route add default gw 172.17.0.1"
		stdout, err = sm.RunFakeNaplesCommand(naples, cmd)
		if err != nil {
			continue
		}
	}
	if err != nil {
		log.Errorf("Failed to flap mgmt link node %v :%v", err, stdout)
		return err
	}

	return nil
}

// StartEventsGenOnNaples starts event loop on naples
//For now only fake naples
func (sm *SysModel) StartEventsGenOnNaples(naples *objects.NaplesCollection, rate, count string) error {

	// Start events.
	sm.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		cmd := fmt.Sprintf("LD_LIBRARY_PATH=/naples/nic/lib/:/naples/nic/lib64/ /naples/nic/bin/gen_events -r %v -t %v -s \"scale-testing\"",
			rate, count)
		sm.RunFakeNaplesBackgroundCommand(nc, cmd)
		return nil
	})

	return nil
}

// StopEventsGenOnNaples starts event loop on naples
// For now only fake naples
func (sm *SysModel) StopEventsGenOnNaples(naples *objects.NaplesCollection) error {

	//stop events.
	sm.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		sm.RunFakeNaplesBackgroundCommand(nc,
			"pkill -9 gen_events")
		return nil
	})

	return nil
}

// StartFWLogGenOnNaples starts fwlog gen loop on naples
// For now only fake naples
func (sm *SysModel) StartFWLogGenOnNaples(naples *objects.NaplesCollection, rate, count string) error {

	// Start events.
	sm.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		cmd := fmt.Sprintf("LD_LIBRARY_PATH=/naples/nic/lib/:/naples/nic/lib64/ /naples/nic/bin/fwloggen -metrics -rate %v -num  %v",
			rate, count)
		sm.RunFakeNaplesBackgroundCommand(nc, cmd)
		return nil
	})

	return nil
}

// StopFWLogGenOnNaples stops fwlog gen on naples
// For now only fake naples
func (sm *SysModel) StopFWLogGenOnNaples(naples *objects.NaplesCollection) error {

	// stop events.
	sm.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		sm.RunFakeNaplesBackgroundCommand(nc,
			"pkill -9 fwloggen")
		return nil
	})

	return nil
}

func (sm *SysModel) DeleteNaplesNodes(names []string) error {
	//First add to testbed.

	nodes := []*testbed.TestNode{}
	naplesMap := make(map[string]bool)
	for _, name := range names {
		log.Infof("Deleting naples node : %v", name)
		naples, ok := sm.NaplesNodes[name]
		if !ok {
			return errors.New("naples not found to delete")
		}

		if _, ok := naplesMap[naples.GetIotaNode().Name]; ok {
			//Node already added
			continue
		}
		naplesMap[naples.GetIotaNode().Name] = true
		nodes = append(nodes, naples.GetTestNode())
	}

	err := sm.Tb.DeleteNodes(nodes)
	if err != nil {
		return err
	}

	for _, name := range names {

		if naples, ok := sm.NaplesNodes[name]; ok {
			delete(sm.NaplesNodes, name)
			delete(sm.NaplesHosts, naples.GetTestNode().NodeName)

		}
	}
	//Reassociate hosts as new naples is added now.
	return sm.AssociateHosts()
}

// DeleteVeniceNodes nodes on the fly
func (sm *SysModel) DeleteVeniceNodes(names []string) error {
	//First add to testbed.

	clusterNodes, err := sm.ListClusterNodes()
	if err != nil {
		return err
	}

	nodes := []*testbed.TestNode{}
	veniceMap := make(map[string]bool)
	for _, name := range names {
		log.Infof("Deleting venice node : %v", name)
		venice, ok := sm.VeniceNodeMap[name]
		if !ok {
			return errors.New("venice not found to delete")
		}

		if _, ok := veniceMap[venice.Name()]; ok {
			//Node already added
			continue
		}
		veniceMap[venice.Name()] = true
		nodes = append(nodes, venice.GetTestNode())
	}

	clusterDelNodes := []*cluster.Node{}
	for _, node := range nodes {
		added := false
		for _, cnode := range clusterNodes {
			if cnode.ObjectMeta.Name == node.GetIotaNode().IpAddress {
				clusterDelNodes = append(clusterDelNodes, cnode)
				added = true
				break
			}
		}
		if !added {
			return fmt.Errorf("Node %v not found in the cluster", node.GetIotaNode().Name)
		}
	}

	//Remove from the cluster
	for _, node := range clusterDelNodes {

		//Remember the cluster node if we want to create again
		for name, vnode := range sm.VeniceNodeMap {
			if vnode.IP() == node.Name {
				veniceNode, _ := sm.VeniceNodes().Select("name=" + vnode.IP())
				if err != nil {
					log.Errorf("Error finding venice node .%v", "name="+vnode.Name())
					return err
				}
				//Disconnect node before deleting
				err = sm.DisconnectVeniceNodesFromCluster(veniceNode, sm.Naples())
				if err != nil {
					log.Errorf("Error disonnecting venice node.")
					return err
				}
				vnode.ClusterNode = node
				sm.VeniceNodesMapDisconnected[name] = vnode
				break
			}
		}

		//Sleep for 2 minutes to for cluster to be reformed.
		time.Sleep(120 * time.Second)
		log.Infof("Deleting venice node from cluster : %v", node.Name)
		err := sm.DeleteClusterNode(node)
		if err != nil {
			log.Errorf("Error deleting cluster venice node.%v", err)
			return err
		}

	}

	log.Infof("Deleting venice node from testbed : %v", nodes)
	err = sm.Tb.DeleteNodes(nodes)
	if err != nil {
		log.Errorf("Error cleaning up venice node.%v", err)
		return err
	}

	for _, name := range names {
		delete(sm.VeniceNodeMap, name)
	}
	//Sleep for a while to for the cluster
	time.Sleep(120 * time.Second)
	log.Infof("Deleting venice complete")

	return sm.InitCfgModel()
}

// AddVeniceNodes node on the fly
func (sm *SysModel) AddVeniceNodes(names []string) error {
	//First add to testbed.
	log.Infof("Adding venice nodes : %v", names)
	nodes, err := sm.Tb.AddNodes(iota.PersonalityType_PERSONALITY_VENICE, names)
	if err != nil {
		return err
	}

	//Add to cluster
	for _, node := range nodes {
		added := false
		for name, vnode := range sm.VeniceNodesMapDisconnected {
			if vnode.IP() == node.GetIotaNode().IpAddress {
				err := sm.AddClusterNode(vnode.ClusterNode)
				if err != nil {
					return fmt.Errorf("Node add failed %v", err)
				}
			}
			added = true
			//Add to connected nodes
			sm.VeniceNodeMap[name] = vnode
		}

		if !added {
			return fmt.Errorf("Node %v not added to cluster", node.GetIotaNode().Name)
		}

	}

	for _, name := range names {
		delete(sm.VeniceNodesMapDisconnected, name)
	}

	//Sleep for a while to for the cluster
	time.Sleep(120 * time.Second)
	//Setup venice nodes again.
	sm.SetupVeniceNodes()

	return sm.InitCfgModel()

}

// AddNaplesNodes node on the fly
func (sm *SysModel) AddNaplesNodes(names []string) error {
	//First add to testbed.
	log.Infof("Adding naples nodes : %v", names)
	nodes, err := sm.Tb.AddNodes(iota.PersonalityType_PERSONALITY_NAPLES, names)
	if err != nil {
		return err
	}

	// move naples to managed mode
	err = sm.DoModeSwitchOfNaples(nodes, sm.NoModeSwitchReboot)
	if err != nil {
		log.Errorf("Setting up naples failed. Err: %v", err)
		return err
	}

	// add venice node to naples
	err = sm.SetUpNaplesPostCluster(nodes)
	if err != nil {
		log.Errorf("Setting up naples failed. Err: %v", err)
		return err
	}

	for _, node := range nodes {
		if err := sm.CreateNaples(node); err != nil {
			return err
		}

	}

	//Reassociate hosts as new naples is added now.
	if err := sm.AssociateHosts(); err != nil {
		log.Infof("Error in host association %v", err.Error())
		return err
	}

	return nil
}

// GetExclusiveServices node on the fly
func (sm *SysModel) GetExclusiveServices() ([]string, error) {
	return []string{}, nil
}
