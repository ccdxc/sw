// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"
	"strings"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// ReloadHosts reloads a host
func (act *ActionCtx) ReloadHosts(hc *HostCollection) error {
	var hostNames string
	reloadMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	for _, hst := range hc.hosts {
		reloadMsg.Nodes = append(reloadMsg.Nodes, &iota.Node{Name: hst.iotaNode.Name})
		hostNames += hst.iotaNode.Name + " "
	}

	log.Infof("Reloading hosts: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload hosts %+v. | Err: %v", reloadMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload hosts %v. API Status: %+v | Err: %v", reloadMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

// ReloadVeniceNodes reloads a venice node
func (act *ActionCtx) ReloadVeniceNodes(vnc *VeniceNodeCollection) error {
	reloadMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	// add each node
	for _, node := range vnc.nodes {
		log.Infof("Reloading venice node %v", node.iotaNode.Name)
		reloadMsg.Nodes = append(reloadMsg.Nodes, node.iotaNode)
	}

	// Trigger reload
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload venice nodes %v. | Err: %v", reloadMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload node %v. API Status: %+v | Err: %v", reloadMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

// ReloadNaples reloads naples nodes
func (act *ActionCtx) ReloadNaples(npc *NaplesCollection) error {
	var hostNames string
	reloadMsg := &iota.NodeMsg{
		ApiResponse: &iota.IotaAPIResponse{},
		Nodes:       []*iota.Node{},
	}

	for _, node := range npc.nodes {
		reloadMsg.Nodes = append(reloadMsg.Nodes, &iota.Node{Name: node.iotaNode.Name})
		hostNames += node.iotaNode.Name + " "
	}

	log.Infof("Reloading Naples: %v", hostNames)

	// Trigger App
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	reloadResp, err := topoClient.ReloadNodes(context.Background(), reloadMsg)
	if err != nil {
		return fmt.Errorf("Failed to reload Naples %+v. | Err: %v", reloadMsg.Nodes, err)
	} else if reloadResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to reload Naples %v. API Status: %+v | Err: %v", reloadMsg.Nodes, reloadResp.ApiResponse, err)
	}

	log.Debugf("Got reload resp: %+v", reloadResp)

	return nil
}

// DisconnectNaples disconnects naples by bringing down its control interface
func (act *ActionCtx) DisconnectNaples(npc *NaplesCollection) error {
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
	for _, naples := range npc.nodes {
		naplesName := naples.iotaNode.Name
		cmd := "/naples/nic/bin/halctl show port status"
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
		cmd = fmt.Sprintf("/naples/nic/bin/halctl debug port --port %s --admin-state down", portNum)
		_, err = act.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}
		cmd = fmt.Sprintf("/naples/nic/bin/halctl debug port --port %s --admin-state up", portNum)
		_, err = act.runCommandOnGivenNaples(naples, cmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", cmd, naplesName, err)
			return err
		}
	}

	return nil
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
