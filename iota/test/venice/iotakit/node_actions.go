// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"
	"regexp"
	"strconv"
	"strings"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// ReloadHosts reloads a host
func (act *ActionCtx) ReloadHosts(hc *HostCollection) error {
	if hc.err != nil {
		return hc.err
	}

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	for _, hst := range hc.hosts {
		nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: hst.iotaNode.Name})
		hostNames += hst.iotaNode.Name + " "
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	log.Infof("Reloading hosts: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload hosts %+v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload hosts %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

// ReloadVeniceNodes reloads a venice node
func (act *ActionCtx) ReloadVeniceNodes(vnc *VeniceNodeCollection) error {
	if vnc.err != nil {
		return vnc.err
	}

	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	// add each node
	for _, node := range vnc.nodes {
		log.Infof("Reloading venice node %v", node.iotaNode.Name)
		nodeMsg.Nodes = append(nodeMsg.Nodes, node.iotaNode)
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	// Trigger reload
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload venice nodes %v. | Err: %v", reloadMsg.NodeMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload node %v. API Status: %+v | Err: %v", reloadMsg.NodeMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

// ReloadNaples reloads naples nodes
func (act *ActionCtx) ReloadNaples(npc *NaplesCollection) error {
	if npc.err != nil {
		return npc.err
	}

	var hostNames string
	nodeMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	for _, node := range npc.nodes {
		nodeMsg.Nodes = append(nodeMsg.Nodes, &iota.Node{Name: node.iotaNode.Name})
		hostNames += node.iotaNode.Name + " "
	}

	reloadMsg := &iota.ReloadMsg{
		NodeMsg: nodeMsg,
	}
	log.Infof("Reloading Naples: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
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
func (act *ActionCtx) DisconnectNaples(npc *NaplesCollection) error {
	if npc.err != nil {
		return npc.err
	}

	trig := act.model.tb.NewTrigger()

	// ifconfig down command
	for _, naples := range npc.nodes {
		cmd := fmt.Sprintf("ifconfig %s down", naples.iotaNode.GetNaplesConfig().ControlIntf)
		trig.AddCommand(cmd, naples.iotaNode.Name+"_naples", naples.iotaNode.Name)
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
func (act *ActionCtx) ConnectNaples(npc *NaplesCollection) error {
	if npc.err != nil {
		return npc.err
	}

	trig := act.model.tb.NewTrigger()

	// ifconfig up command
	for _, naples := range npc.nodes {
		cmd := fmt.Sprintf("ifconfig %s %s/16 up", naples.iotaNode.GetNaplesConfig().ControlIntf, naples.iotaNode.GetNaplesConfig().ControlIp)
		trig.AddCommand(cmd, naples.iotaNode.Name+"_naples", naples.iotaNode.Name)
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
func (act *ActionCtx) RunNaplesCommand(npc *NaplesCollection, cmd string) ([]string, error) {
	if npc.err != nil {
		return nil, npc.err
	}

	var stdout []string
	trig := act.model.tb.NewTrigger()

	for _, naples := range npc.nodes {
		trig.AddCommand(cmd, naples.iotaNode.Name+"_naples", naples.iotaNode.Name)
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

// PortFlap flaps the one of the port from each naples on the collection
func (act *ActionCtx) PortFlap(npc *NaplesCollection) error {
	if npc.err != nil {
		return npc.err
	}

	for _, naples := range npc.nodes {
		naplesName := naples.iotaNode.Name
		cmd := "/nic/bin/halctl show port status"
		out, err := act.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}

		// get port number from port status output
		portNum := getPortNum(out)
		if len(strings.TrimSpace(portNum)) == 0 {
			log.Errorf("failed to get a port number to flap on naples: %v", naplesName)
			return fmt.Errorf("failed to get a port number to flap on naples: %v", naplesName)
		}

		log.Infof("flapping port {%s} on naples {%v}", portNum, naplesName)

		// flap port
		cmd = fmt.Sprintf("/nic/bin/halctl debug port --port %s --admin-state down", portNum)
		_, err = act.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}
		cmd = fmt.Sprintf("/nic/bin/halctl debug port --port %s --admin-state up", portNum)
		_, err = act.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}
	}

	return nil
}

// GetNaplesEndpoints returns a map of map[<mac-address>]<vlan> indexed by naples name
func (act *ActionCtx) GetNaplesEndpoints(npc *NaplesCollection) (map[string]map[string]struct {
	Local bool
	Vlan  int
}, error) {
	trig := act.model.tb.NewTrigger()
	ret := make(map[string]map[string]struct {
		Local bool
		Vlan  int
	})
	for _, naples := range npc.nodes {
		cmd := "/nic/bin/halctl show endpoint brief"
		trig.AddCommand(cmd, naples.iotaNode.Name+"_naples", naples.iotaNode.Name)
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
func (act *ActionCtx) runCommandOnGivenNaples(np *Naples, cmd string) (string, error) {
	trig := act.model.tb.NewTrigger()

	trig.AddCommand(cmd, np.iotaNode.Name+"_naples", np.iotaNode.Name)
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
func (act *ActionCtx) VeniceNodeLoggedInCtx(vnc *VeniceNodeCollection) error {
	if vnc.err != nil {
		return vnc.err
	}

	nodeURL := fmt.Sprintf("%s:%s", vnc.nodes[0].iotaNode.IpAddress, globals.APIGwRESTPort)
	_, err := act.model.tb.VeniceNodeLoggedInCtx(nodeURL)
	if err != nil {
		log.Errorf("error logging in to venice node (%s): %v", nodeURL, err)
		return fmt.Errorf("error logging in to venice node (%s): %v", nodeURL, err)
	}
	return nil
}

// VeniceNodeGetCluster gets cluster obj by connecting to a specified venice node
func (act *ActionCtx) VeniceNodeGetCluster(vnc *VeniceNodeCollection) error {
	if vnc.err != nil {
		return vnc.err
	}

	nodeURL := fmt.Sprintf("%s:%s", vnc.nodes[0].iotaNode.IpAddress, globals.APIGwRESTPort)
	restcl, err := act.model.tb.VeniceNodeRestClient(nodeURL)
	if err != nil {
		return err
	}
	_, err = act.model.tb.GetClusterWithRestClient(restcl)
	if err != nil {
		log.Errorf("error getting cluster obj from node (%s): %v", nodeURL, err)
		return fmt.Errorf("error getting cluster obj from node (%s): %v", nodeURL, err)
	}
	return nil
}

// FlapDataSwitchPorts flaps data ports
func (act *ActionCtx) FlapDataSwitchPorts(ports *SwitchPortCollection, downTime time.Duration) error {
	switchMsg := &iota.SwitchMsg{
		ApiResponse:  &iota.IotaAPIResponse{},
		DataSwitches: []*iota.DataSwitch{},
	}

	for _, port := range ports.ports {
		log.Infof("Flapping port %v(%v)", port.port, port.sw.dataSwitch.GetIp())

		added := false
		for _, sw := range switchMsg.DataSwitches {
			if sw.GetIp() == port.sw.dataSwitch.GetIp() {
				added = true
				sw.Ports = append(sw.Ports, port.port)
			}
		}
		if !added {
			switchMsg.DataSwitches = append(switchMsg.DataSwitches, &iota.DataSwitch{Ip: port.sw.dataSwitch.GetIp(),
				Password: port.sw.dataSwitch.GetPassword(), Username: port.sw.dataSwitch.GetUsername(), Ports: []string{port.port}})
		}
	}

	switchMsg.Op = iota.SwitchOp_SHUT_PORTS

	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	switchResp, err := topoClient.DoSwitchOperation(context.Background(), switchMsg)
	if err != nil {
		return fmt.Errorf("Failed to shut ports | Err: %v", err)
	} else if switchResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to shut ports  API Status: %+v | Err: %v", switchResp.ApiResponse, err)
	}

	log.Debugf("Got switch port shut resp: %+v", switchResp)

	time.Sleep(downTime * time.Second)

	switchMsg.Op = iota.SwitchOp_NO_SHUT_PORTS

	topoClient = iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	switchResp, err = topoClient.DoSwitchOperation(context.Background(), switchMsg)
	if err != nil {
		return fmt.Errorf("Failed to shut ports | Err: %v", err)
	} else if switchResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to shut ports  API Status: %+v | Err: %v", switchResp.ApiResponse, err)
	}

	log.Debugf("Got switch port not shut resp: %+v", switchResp)

	return nil
}

// RemoveAddNaples remove and add naples
func (act *ActionCtx) RemoveAddNaples(naples *NaplesCollection) error {

	names := []string{}
	for _, obj := range naples.nodes {
		names = append(names, obj.iotaNode.Name)

	}

	if err := act.model.DeleteNaplesNodes(names); err != nil {
		log.Errorf("Failed to delete naples nodes %v", names)
		return err
	}

	if err := act.model.AddNaplesNodes(names); err != nil {
		log.Errorf("Failed to add naples node %v", names)
		return err
	}

	return nil
}
