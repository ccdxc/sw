// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"
	"path/filepath"
	"time"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/venice/utils/log"
)

// used instead of infinite timeout for triggers or log copying
const maxOpTimeout = 20 * time.Minute

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

// AddCommandWithRetriesOnFailures adds a command to trigger
func (tr *Trigger) AddCommandWithRetriesOnFailures(command, entity, node string, retries uint32) error {
	cmd := iota.Command{
		Mode:             iota.CommandMode_COMMAND_FOREGROUND,
		Command:          command,
		EntityName:       entity,
		NodeName:         node,
		RetriesOnFailure: retries,
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

// Run runs trigger commands serially within a node but, parallelly across nodes
func (tr *Trigger) Run() ([]*iota.Command, error) {
	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_PARALLEL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    tr.cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(tr.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	triggerResp, err := topoClient.Trigger(ctx, trigMsg)
	cancel()
	if err != nil {
		return nil, fmt.Errorf("Trigger failed. API Status: err: %v", err)
	} else if triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return nil, fmt.Errorf("Trigger failed. API Status: resp: %+v", triggerResp.ApiResponse)
	}

	log.Debugf("Got Trigger resp: %+v", triggerResp)

	return triggerResp.Commands, nil
}

// RunParallel runs all commands parallelly
func (tr *Trigger) RunParallel() ([]*iota.Command, error) {
	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: iota.TriggerMode_TRIGGER_NODE_PARALLEL,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    tr.cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(tr.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	triggerResp, err := topoClient.Trigger(ctx, trigMsg)
	cancel()
	if err != nil {
		return nil, fmt.Errorf("Trigger failed. API Status: err: %v", err)
	} else if triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return nil, fmt.Errorf("Trigger failed. API Status: resp: %+v", triggerResp.ApiResponse)
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
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	triggerResp, err := topoClient.Trigger(ctx, trigMsg)
	cancel()
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
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	triggerResp, err := topoClient.Trigger(ctx, trigMsg)
	cancel()
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return nil, fmt.Errorf("Trigger failed. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got StopCommands Trigger resp: %+v", triggerResp)

	return triggerResp.Commands, nil
}

// CopyToHost copies a file to host
func (tb *TestBed) CopyToHost(nodeName string, files []string, destDir string) error {
	// copy message
	copyMsg := iota.EntityCopyMsg{
		Direction:   iota.CopyDirection_DIR_IN,
		NodeName:    nodeName,
		EntityName:  nodeName + "_host",
		Files:       files,
		DestDir:     destDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil {
		log.Errorf("Copy failed: Err: %v", err)
		return fmt.Errorf("Copy files failed.  Err: %v", err)
	} else if copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Copy files failed. API Status: %+v ", copyResp.ApiResponse)
	}

	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}

// CopyFromHost copies a file from host
func (tb *TestBed) CopyFromHost(nodeName string, files []string, destDir string) error {
	// copy message
	copyMsg := iota.EntityCopyMsg{
		Direction:   iota.CopyDirection_DIR_OUT,
		NodeName:    nodeName,
		EntityName:  nodeName + "_host",
		Files:       files,
		DestDir:     destDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil {
		log.Errorf("Copy failed: Err: %v", err)
		return fmt.Errorf("Copy files failed.  Err: %v", err)
	} else if copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Copy failed: Resp: %v", copyResp)
		return fmt.Errorf("Copy files failed. API Status: %+v ", copyResp.ApiResponse)
	}

	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}

// CopyFromNaples copies files from naples
func (tb *TestBed) CopyFromNaples(nodeName string, files []string, destDir string) error {
	// copy message
	copyMsg := iota.EntityCopyMsg{
		Direction:   iota.CopyDirection_DIR_OUT,
		NodeName:    nodeName,
		EntityName:  nodeName + "_naples",
		Files:       files,
		DestDir:     destDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil {
		log.Errorf("Copy failed: Err: %v", err)
		return fmt.Errorf("Copy files failed.  Err: %v", err)
	} else if copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Copy failed: Resp: %v", copyResp)
		return fmt.Errorf("Copy files failed. API Status: %+v ", copyResp.ApiResponse)
	}

	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}

// CopyToNaples copies files to naples
func (tb *TestBed) CopyToNaples(nodeName string, files []string, destDir string) error {

	//Copy to current directory where ever iota is running
	hostTmpDir := "./"
	// First copy to host
	copyMsg := iota.EntityCopyMsg{
		Direction:   iota.CopyDirection_DIR_IN,
		NodeName:    nodeName,
		EntityName:  nodeName + "_host",
		Files:       files,
		DestDir:     hostTmpDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil {
		log.Errorf("Copy failed to host (for naples): Err: %v", err)
		return fmt.Errorf("Copy files failed.  Err: %v", err)
	} else if copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Copy failed to host (for naples): Resp: %v", copyResp)
		return fmt.Errorf("Copy files failed. API Status: %+v ", copyResp.ApiResponse)
	}

	//now copy to naples from host
	naplesPasswd := "pen123"
	trig := tb.NewTrigger()
	for _, f := range files {
		fullSrcPath := hostTmpDir + filepath.Base(f)

		for _, tb := range tb.Nodes {
			if tb.NodeName == nodeName {
				//Copy to all naples
				for _, config := range tb.NaplesConfigs.Configs {
					copyCmd := fmt.Sprintf("sshpass -p %v scp -o StrictHostKeyChecking=no  %s %s@%s:%s", naplesPasswd, fullSrcPath, "root", config.NaplesIpAddress, destDir)
					trig.AddCommand(copyCmd, nodeName+"_host", nodeName)
				}
			}
		}
	}

	// trigger commands
	resp, err := trig.Run()
	if err != nil {
		msg := fmt.Sprintf("Failed to Copy to naples command Err: %v", err)
		log.Errorf(msg)
		return fmt.Errorf(msg)
	}

	// check the response
	for _, cmdResp := range resp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Copy from host to naples failed. %+v", cmdResp)
			return fmt.Errorf("Changing naples mode failed. exit code %v, Out: %v, StdErr: %v", cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}
	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}

// CopyFromVenice copies a file from venice node
func (tb *TestBed) CopyFromVenice(nodeName string, files []string, destDir string) error {
	// copy message
	copyMsg := iota.EntityCopyMsg{
		Direction:   iota.CopyDirection_DIR_OUT,
		NodeName:    nodeName,
		EntityName:  nodeName + "_venice",
		Files:       files,
		DestDir:     destDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil {
		log.Errorf("Copy failed: Err: %v", err)
		return fmt.Errorf("Copy files failed.  Err: %v", err)
	} else if copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Copy failed: Resp: %v", copyResp)
		return fmt.Errorf("Copy files failed. API Status: %+v ", copyResp.ApiResponse)
	}

	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}

// CopyToVenice copies a file to venice node
func (tb *TestBed) CopyToVenice(nodeName string, files []string, destDir string) error {
	// copy message
	copyMsg := iota.EntityCopyMsg{
		Direction:   iota.CopyDirection_DIR_IN,
		NodeName:    nodeName,
		EntityName:  nodeName + "_venice",
		Files:       files,
		DestDir:     destDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil {
		log.Errorf("Copy failed: Err: %v", err)
		return fmt.Errorf("Copy files failed.  Err: %v", err)
	} else if copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Copy failed: Resp: %v", copyResp)
		return fmt.Errorf("Copy files failed. API Status: %+v ", copyResp.ApiResponse)
	}

	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}

// CopyFromWorkload copies files from naples
func (tb *TestBed) CopyFromWorkload(nodeName, workloadName string, files []string, destDir string) error {
	return tb.CopyToFromWorkload(iota.CopyDirection_DIR_OUT, nodeName, workloadName, files, destDir)
}

// CopyToWorkload copies files from naples
func (tb *TestBed) CopyToWorkload(nodeName, workloadName string, files []string, destDir string) error {
	return tb.CopyToFromWorkload(iota.CopyDirection_DIR_IN, nodeName, workloadName, files, destDir)
}

// CopyToFromWorkload is a common routine to copy in/out from workload
func (tb *TestBed) CopyToFromWorkload(direction iota.CopyDirection, nodeName, workloadName string, files []string, destDir string) error {
	// copy message
	copyMsg := iota.EntityCopyMsg{
		Direction:   direction,
		NodeName:    nodeName,
		EntityName:  workloadName,
		Files:       files,
		DestDir:     destDir,
		ApiResponse: &iota.IotaAPIResponse{},
	}

	// send it to iota
	topoClient := iota.NewTopologyApiClient(tb.iotaClient.Client)
	ctx, cancel := context.WithTimeout(context.Background(), maxOpTimeout)
	copyResp, err := topoClient.EntityCopy(ctx, &copyMsg)
	cancel()
	if err != nil || copyResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		err := fmt.Errorf("Copy %v failed: %v, resp %+v", direction, err, copyResp)
		log.Errorf("%s", err)
		return err
	}

	log.Debugf("Got Copy resp: %+v", copyResp)

	return nil
}
