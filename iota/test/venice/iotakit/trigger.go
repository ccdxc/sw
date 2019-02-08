// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

// Trigger is an instance of trigger
type Trigger struct {
	cmds       []*iota.Command
	tb         *TestBed
	iotaClient *common.GRPCClient
}

// NewTrigger returns a new trigger instance
func (tb *TestBed) NewTrigger() *Trigger {
	return &Trigger{
		cmds:       []*iota.Command{},
		tb:         tb,
		iotaClient: tb.iotaClient,
	}
}

// AddCommand adds a command to trigger
func (tr *Trigger) AddCommand(command, entity, node string) error {
	cmd := iota.Command{
		Mode:       iota.CommandMode_COMMAND_FOREGROUND,
		Command:    command,
		EntityName: entity,
		NodeName:   node,
	}
	tr.cmds = append(tr.cmds, &cmd)
	return nil
}

// AddBackgroundCommand adds a background command
func (tr *Trigger) AddBackgroundCommand(command, entity, node string) error {
	cmd := iota.Command{
		Mode:       iota.CommandMode_COMMAND_BACKGROUND,
		Command:    command,
		EntityName: entity,
		NodeName:   node,
	}
	tr.cmds = append(tr.cmds, &cmd)
	return nil
}

// Run runs trigger commands in parallel
func (tr *Trigger) Run() ([]*iota.Command, error) {
	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_PARALLEL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    tr.cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(tr.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return nil, fmt.Errorf("Trigger failed. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got Trigger resp: %+v", triggerResp)

	return triggerResp.Commands, nil
}

// RunSerial runs commands serially
func (tr *Trigger) RunSerial() ([]*iota.Command, error) {
	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_SERIAL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    tr.cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(tr.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return nil, fmt.Errorf("Trigger failed. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got Trigger resp: %+v", triggerResp)

	return triggerResp.Commands, nil
}

// StopCommands stop all commands using previously returned command handle
func (tr *Trigger) StopCommands(cmds []*iota.Command) ([]*iota.Command, error) {
	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_TERMINATE_ALL_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_PARALLEL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(tr.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return nil, fmt.Errorf("Trigger failed. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got StopCommands Trigger resp: %+v", triggerResp)

	return triggerResp.Commands, nil
}
