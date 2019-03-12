// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"context"
	"fmt"
	"strings"

	"github.com/pensando/sw/api"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/strconv"
)

// number of times to retry netcat client command
const maxNetcatRetries = 3

// PingPairs action verifies ping works between collection of workload pairs
func (act *ActionCtx) PingPairs(wpc *WorkloadPairCollection) error {
	if wpc.HasError() {
		return wpc.Error()
	}

	cmds := []*iota.Command{}
	var pairNames []string
	for _, pair := range wpc.pairs {
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.second.iotaWorkload.WorkloadName, pair.first.iotaWorkload.WorkloadName))
		ipAddr := strings.Split(pair.second.iotaWorkload.IpPrefix, "/")[0]
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("ping -c 5 -i 1 %v", ipAddr),
			EntityName: pair.first.iotaWorkload.WorkloadName,
			NodeName:   pair.first.iotaWorkload.NodeName,
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Testing ping between workloads %v ", pairNames)

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !act.model.tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got ping Trigger resp: %+v", triggerResp)

	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Ping trigger failed. Resp: %#v", cmdResp)
			return fmt.Errorf("PingPairs trigger failed on %s. code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// PingFails action verifies ping fails between collection of workload pairs
func (act *ActionCtx) PingFails(wpc *WorkloadPairCollection) error {
	if wpc.HasError() {
		return wpc.Error()
	}

	cmds := []*iota.Command{}
	var pairNames []string
	for _, pair := range wpc.pairs {
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.second.iotaWorkload.WorkloadName, pair.first.iotaWorkload.WorkloadName))
		ipAddr := strings.Split(pair.second.iotaWorkload.IpPrefix, "/")[0]
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("ping -c 5 -i 1 %v", ipAddr),
			EntityName: pair.first.iotaWorkload.WorkloadName,
			NodeName:   pair.first.iotaWorkload.NodeName,
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Verifying ping failure between workloads %v ", pairNames)

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !act.model.tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(act.model.tb.iotaClient.Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got ping Trigger resp: %+v", triggerResp)

	// in mock mode, pretend ping failed
	if act.model.tb.mockMode {
		return nil
	}

	// verify ping failed on each pair
	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode == 0 {
			log.Errorf("Ping suceeded while expecting to fail. Resp: %#v", cmdResp)
			return fmt.Errorf("PingFails: ping suceeded while expecting to fail on %s. code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// netcatTrigger runs netcat triggers between pair of workloads
func (act *ActionCtx) netcatTrigger(wpc *WorkloadPairCollection, serverOpt, clientOpt string, port int, expFail bool, expClientExitCode int32, expOutput string) error {
	if wpc.err != nil {
		return wpc.err
	}
	srvWorkloads := make(map[string]*iota.Workload)
	for _, pair := range wpc.pairs {
		srvWorkloads[pair.first.iotaWorkload.WorkloadName] = pair.first.iotaWorkload
	}

	// run netcat servers on first workload
	trig := act.model.tb.NewTrigger()
	for _, srvWrkld := range srvWorkloads {
		trig.AddBackgroundCommand(fmt.Sprintf("nc -lk %s -p %d", serverOpt, port), srvWrkld.WorkloadName, srvWrkld.NodeName)
	}
	srvResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run netcat server. Err: %v, Trigger Req: %+v, Resp: %+v", err, trig, srvResp)
		return fmt.Errorf("Failed to run netcat server. Err: %v", err)
	}
	log.Debugf("Got netcat server resp: %+v", srvResp)

	// verify server cmd was successful
	for _, cmdResp := range srvResp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Netcat server command failed. %+v", cmdResp)
			return fmt.Errorf("Netcat server command failed on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	// run netcat from second host and verify it was successful
	var clientErr error
	for i := 0; i < maxNetcatRetries; i++ {
		trig = act.model.tb.NewTrigger()
		var pairNames []string
		for _, pair := range wpc.pairs {
			toIP := strings.Split(pair.first.iotaWorkload.IpPrefix, "/")[0]
			trig.AddCommand(fmt.Sprintf("nc -z %s %s %d", clientOpt, toIP, port), pair.second.iotaWorkload.WorkloadName, pair.second.iotaWorkload.NodeName)
			pairNames = append(pairNames, fmt.Sprintf("%s -> %s", pair.second.iotaWorkload.WorkloadName, pair.first.iotaWorkload.WorkloadName))
		}

		log.Infof("Netcat testing workload pairs %v", pairNames)

		// run trigger
		clientResp, terr := trig.RunParallel()
		if terr != nil {
			log.Errorf("Failed to run netcat client. Err: %v, Trigger Req: %+v, Resp: %+v", terr, trig, clientResp)
			clientErr = fmt.Errorf("Failed to run netcat client. Err: %v", terr)
		} else {
			log.Debugf("Got netcat client resp: %+v", clientResp)

			// verify client was what we expected
			for _, cmdResp := range clientResp {
				if expFail {
					if cmdResp.ExitCode != expClientExitCode {
						log.Errorf("Netcat client command suceeded while expecting to fail. %+v", cmdResp)
						clientErr = fmt.Errorf("Netcat client command suceeded while expecting failure on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
					}
					if expOutput != "" && strings.Contains(cmdResp.Stdout, expOutput) {
						log.Errorf("Not Expecting netcat output %s. %+v", expOutput, cmdResp)
						clientErr = fmt.Errorf("Not Expecting netcat output %s on %s. exit code %v, Out: %v, StdErr: %v", expOutput, cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
					}
				} else {
					if cmdResp.ExitCode != 0 {
						log.Errorf("Netcat client command failed. %+v", cmdResp)
						clientErr = fmt.Errorf("Netcat client command failed on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
					}
					if expOutput != "" && !strings.Contains(cmdResp.Stdout, expOutput) {
						log.Errorf("Invalid netcat output. Expecting %s. %+v", expOutput, cmdResp)
						clientErr = fmt.Errorf("Invalid netcat output. Expecting %s on %s. exit code %v, Out: %v, StdErr: %v", expOutput, cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
					}
				}

			}
		}
		if clientErr == nil {
			break
		}
	}

	// stop the netcat server
	trig = act.model.tb.NewTrigger()
	stopResp, err := trig.StopCommands(srvResp)
	if err != nil {
		log.Errorf("Error stopping netcat servers. Err: %v. trig: %+v, resp: %+v", err, srvResp, stopResp)
		return fmt.Errorf("Error stopping netcat servers. Err: %v", err)
	}

	// verify stop was successful
	for _, cmdResp := range stopResp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Netcat stopping server failed. %+v", cmdResp)
		}
	}

	return clientErr
}

// TCPSession runs TCP session between pair of workloads
func (act *ActionCtx) TCPSession(wpc *WorkloadPairCollection, port int) error {
	if wpc.err != nil {
		return wpc.err
	}

	return act.netcatTrigger(wpc, "", "", port, false, 0, "")
}

// UDPSession runs UDP session between pair of workloads
func (act *ActionCtx) UDPSession(wpc *WorkloadPairCollection, port int) error {
	if wpc.err != nil {
		return wpc.err
	}

	if act.model.tb.mockMode {
		return nil
	}

	return act.netcatTrigger(wpc, "-u --sh-exec 'echo test'", "-u", port, false, 0, "test")
}

// TCPSessionFails verifies TCP session fails between pair of workloads
func (act *ActionCtx) TCPSessionFails(wpc *WorkloadPairCollection, port int) error {
	if wpc.err != nil {
		return wpc.err
	}

	if act.model.tb.mockMode {
		return nil
	}

	return act.netcatTrigger(wpc, "", "", port, true, 1, "")
}

// UDPSessionFails verifies  UDP session fails between pair of workloads
func (act *ActionCtx) UDPSessionFails(wpc *WorkloadPairCollection, port int) error {
	if wpc.err != nil {
		return wpc.err
	}

	if act.model.tb.mockMode {
		return nil
	}

	return act.netcatTrigger(wpc, "-u --sh-exec 'echo test'", "-u", port, true, 0, "test")
}

// VerifyWorkloadStatus verifies workload status in venice
func (act *ActionCtx) VerifyWorkloadStatus(wc *WorkloadCollection) error {
	if wc.HasError() {
		return wc.Error()
	}

	for _, wr := range wc.workloads {
		wsts, err := act.model.tb.GetWorkload(&wr.veniceWorkload.ObjectMeta)
		if err != nil {
			log.Errorf("Could not get workload %v. Err: %v", wr.veniceWorkload.Name, err)
			return err
		}

		log.Debugf("Got workload status: %+v", wsts)

		if wsts.Spec.HostName != wr.veniceWorkload.Spec.HostName {
			return fmt.Errorf("Invalid hostname on workload %v. Exp: %v, Got: %v", wr.veniceWorkload.Name, wr.veniceWorkload.Spec.HostName, wsts.Spec.HostName)
		}
		if len(wsts.Spec.Interfaces) != len(wr.veniceWorkload.Spec.Interfaces) {
			return fmt.Errorf("Unexpected number of interfaces on workload %v. Exp: %+v, Got: %+v", wr.veniceWorkload.Name, wr.veniceWorkload.Spec.Interfaces, wsts.Spec.Interfaces)
		}

		name, err := strconv.ParseMacAddr(wr.veniceWorkload.Spec.Interfaces[0].MACAddress)
		if err != nil {
			name = wr.veniceWorkload.Spec.Interfaces[0].MACAddress
		}
		epMeta := api.ObjectMeta{
			Tenant:    wr.veniceWorkload.Tenant,
			Namespace: wr.veniceWorkload.Namespace,
			Name:      wr.veniceWorkload.Name + "-" + name,
		}

		// get the endpoints for the workload
		ep, err := act.model.tb.GetEndpoint(&epMeta)
		if err != nil {
			log.Errorf("Could not get endpoint %v. Err: %v", epMeta.Name, err)
			return err
		}

		if ep.Status.MacAddress != wr.veniceWorkload.Spec.Interfaces[0].MACAddress {
			return fmt.Errorf("Invalid mac address %v for workload %v", ep.Status.MacAddress, wr.veniceWorkload.Name)
		}
		if ep.Status.HomingHostName != wr.veniceWorkload.Spec.HostName {
			return fmt.Errorf("Invalid host name %v for endpoint on workload %v", ep.Status.HomingHostName, wr.veniceWorkload.Name)
		}
		if ep.Status.NodeUUID != wr.host.naples.smartNic.Status.PrimaryMAC {
			return fmt.Errorf("Invalid node uuid %v for endpoint on workload %v", ep.Status.NodeUUID, wr.veniceWorkload.Name)
		}
		if ep.Status.Network != wr.subnet.Name {
			return fmt.Errorf("Invalid subnet %v for endpoint on workload %v", ep.Status.Network, wr.veniceWorkload.Name)
		}
	}

	return nil
}

func (act *ActionCtx) startFTPServer(wpc *WorkloadPairCollection) error {
	if wpc.HasError() {
		return wpc.Error()
	}

	srvWorkloads := make(map[string]*Workload)
	for _, pair := range wpc.pairs {
		srvWorkloads[pair.first.iotaWorkload.WorkloadName] = pair.first
	}

	trig := act.model.tb.NewTrigger()
	for _, wr := range srvWorkloads {
		// if the server is not running already, start it
		if !wr.isFTPServerRunning {
			trig.AddCommand(fmt.Sprintf("systemctl stop vsftpd"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
			trig.AddCommand(fmt.Sprintf("useradd -m -c admin -s /bin/bash admin"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
			trig.AddCommand(fmt.Sprintf("echo admin | tee -a /etc/vsftpd.userlist"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
			trig.AddCommand(fmt.Sprintf("echo password | passwd --stdin admin"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
			trig.AddCommand(fmt.Sprintf("systemctl start vsftpd"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
			trig.AddCommand(fmt.Sprintf("systemctl enable vsftpd"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
			trig.AddCommand(fmt.Sprintf("echo test > /tmp/test.txt"), wr.iotaWorkload.WorkloadName, wr.iotaWorkload.NodeName)
		}
	}

	// run the trigger
	srvResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to start FTP server. Err: %v, Trigger Req: %+v, Resp: %+v", err, trig, srvResp)
		return fmt.Errorf("Failed to start FTP server. Err: %v", err)
	}
	log.Debugf("Got FTP server start resp: %+v", srvResp)

	// verify server cmd was successful
	for _, cmdResp := range srvResp {
		if cmdResp.ExitCode != 0 && !strings.Contains(cmdResp.Stdout, "already exists") {
			log.Errorf("FTP server start failed. %+v", cmdResp)
			return fmt.Errorf("FTP server start failed on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	// remember server is running already
	for _, wr := range srvWorkloads {
		wr.isFTPServerRunning = true
	}

	return nil
}

func (act *ActionCtx) runFTPClient(wpc *WorkloadPairCollection, ftpPort, expClientExitCode int32, ftpCmd string) error {
	if wpc.HasError() {
		return wpc.Error()
	}

	trig := act.model.tb.NewTrigger()

	// add ftp client command for each pair
	var pairNames []string
	for _, pair := range wpc.pairs {
		toIP := strings.Split(pair.first.iotaWorkload.IpPrefix, "/")[0]
		trig.AddCommand(fmt.Sprintf("lftp -u admin,password  %s -p %d -e 'set net:timeout 5; set net:max-retries 3; set net:reconnect-interval-base 5; set xfer:clobber yes; %s; bye' ", toIP, ftpPort, ftpCmd), pair.second.iotaWorkload.WorkloadName, pair.second.iotaWorkload.NodeName)
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s;", pair.second.iotaWorkload.WorkloadName, pair.first.iotaWorkload.WorkloadName))
	}

	log.Infof("running FTP gets %v", pairNames)

	resp, err := trig.RunParallel()
	if err != nil {
		log.Errorf("Failed to run FTP client. Err: %v, Trigger Req: %+v, Resp: %+v", err, trig, resp)
		return fmt.Errorf("Failed to run FTP client. Err: %v", err)
	}
	log.Debugf("Got FTP client resp: %+v", resp)

	// verify client cmd was successful
	for _, cmdResp := range resp {
		if !strings.HasPrefix(cmdResp.Command, "rm -f") && cmdResp.ExitCode != expClientExitCode {
			log.Errorf("FTP client failed. %+v", cmdResp)
			return fmt.Errorf("FTP client failed on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// FTPGet runs FTP get trigger between each workload pair
func (act *ActionCtx) FTPGet(wpc *WorkloadPairCollection) error {
	// start ftp server
	err := act.startFTPServer(wpc)
	if err != nil {
		return err
	}

	// run ftp get
	err = act.runFTPClient(wpc, 21, 0, "get /tmp/test.txt")
	if err != nil {
		return fmt.Errorf("FTP Get failed. Err: %v", err)
	}

	return nil
}

// FTPGetFails verifies FTP get fails for each workload pair
func (act *ActionCtx) FTPGetFails(wpc *WorkloadPairCollection) error {
	// start ftp server
	err := act.startFTPServer(wpc)
	if err != nil {
		return err
	}

	if act.model.tb.mockMode {
		return nil
	}

	// run ftp get
	err = act.runFTPClient(wpc, 21, 1, "get /tmp/test.txt")
	if err != nil {
		return fmt.Errorf("FTP Get suceeded while expecting to fail. Err: %v", err)
	}

	return nil
}
