// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package base

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/pensando/sw/api"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/iota/tools/fuz/fuze"
	"github.com/pensando/sw/venice/utils/log"

	libstrconv "github.com/pensando/sw/venice/utils/strconv"
)

// number of times to retry netcat client command
const maxNetcatRetries = 3

type workloadPort struct {
	w    *objects.Workload
	port int
}

// PingAndCapturePackets packets runs tcpdump and gets the output on the workload
func (sm *SysModel) PingAndCapturePackets(wpc *objects.WorkloadPairCollection, wc *objects.WorkloadCollection, wlnum int) error {
	cmds := []*iota.Command{}
	// Add tcpdump command
	cmd := iota.Command{
		Mode:       iota.CommandMode_COMMAND_BACKGROUND,
		Command:    fmt.Sprintf("tcpdump -x -nni %v ip proto gre", wc.Workloads[wlnum].GetInterface()),
		EntityName: wc.Workloads[wlnum].Name(),
		NodeName:   wc.Workloads[wlnum].NodeName(),
	}
	cmds = append(cmds, &cmd)

	// Add ping command
	var pairNames []string
	for _, pair := range wpc.Pairs {
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.Second.Name(), pair.First.Name()))
		ipAddr := pair.First.GetIP()
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("ping -c 20 -i 0.2 %v", ipAddr),
			EntityName: pair.Second.Name(),
			NodeName:   pair.Second.NodeName(),
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Testing ping between workloads %v ", pairNames)
	log.Infof("Collecting tcpdump on %#v interface %v", wc.Workloads[wlnum].Name(),
		wc.Workloads[wlnum].GetInterface())

	trmode := iota.TriggerMode_TRIGGER_SERIAL
	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Infof("Terminate commands")
	trig := sm.Tb.NewTrigger()
	stopResp, err := trig.StopCommands(triggerResp.Commands)
	if err != nil {
		log.Errorf("Error stopping ping and tcpdump cmds. Err: %v, resp: %+v", err, stopResp)
		return fmt.Errorf("Error stopping ping and tcpdump cmds. Err: %v", err)
	}

	for _, cmdResp := range stopResp {
		//log.Infof("Dumping command in stopResponse: %v stdout: %v stderr: %v", cmdResp.Command, cmdResp.Stdout, cmdResp.Stderr)
		if strings.Contains(cmdResp.Command, "tcpdump") {
			//log.Infof("tcpdump stdout: %v stderr: %v", cmdResp.Stdout, cmdResp.Stderr)
			if !strings.Contains(cmdResp.Stdout, "GREv0, length") {
				log.Errorf("PingCapture trigger failed. Did not find GRE pkts in tcpdump! Resp: %#v", cmdResp)
				return fmt.Errorf("PingCapture trigger failed on %s. code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
			}
		}
	}

	return nil
}

// PingPairs sm.on verifies ping works between collection of workload pairs
func (sm *SysModel) TriggerHping(wpc *objects.WorkloadPairCollection, cmd string) error {

	cmds := []*iota.Command{}
	var pairNames []string
	for _, pair := range wpc.Pairs {
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.First.Name(), pair.Second.Name()))
		ipAddr := pair.Second.GetIP()
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("sudo hping3 %s %v", cmd, ipAddr),
			EntityName: pair.First.Name(),
			NodeName:   pair.First.NodeName(),
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Testing ping between workloads %v ", pairNames)

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !sm.Tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got ping Trigger resp: %+v", triggerResp)

	return nil
}

// PingPairs sm.on verifies ping works between collection of workload pairs
func (sm *SysModel) PingPairs(wpc *objects.WorkloadPairCollection) error {

	cmds := []*iota.Command{}
	var pairNames []string
	for _, pair := range wpc.Pairs {
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.First.Name(), pair.Second.Name()))
		ipAddr := pair.Second.GetIP()
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("ping -c 5 -i 1 %v", ipAddr),
			EntityName: pair.First.Name(),
			NodeName:   pair.First.NodeName(),
		}
		cmds = append(cmds, &cmd)

		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.First.Name(), pair.Second.Name()))
		ipAddr = pair.First.GetIP()
		otherCmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("ping -c 5 -i 1 %v", ipAddr),
			EntityName: pair.Second.Name(),
			NodeName:   pair.Second.NodeName(),
		}
		cmds = append(cmds, &otherCmd)

	}

	log.Infof("Testing ping between workloads %v ", pairNames)

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !sm.Tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
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

// PingFails sm.on verifies ping fails between collection of workload pairs
func (sm *SysModel) PingFails(wpc *objects.WorkloadPairCollection) error {

	cmds := []*iota.Command{}
	var pairNames []string
	for _, pair := range wpc.Pairs {
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s; ", pair.Second.Name(), pair.First.Name()))
		ipAddr := pair.Second.GetIP()
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("ping -c 5 -i 1 %v", ipAddr),
			EntityName: pair.First.Name(),
			NodeName:   pair.First.NodeName(),
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Verifying ping failure between workloads %v ", pairNames)

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !sm.Tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a ping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	log.Debugf("Got ping Trigger resp: %+v", triggerResp)

	// in mock mode, pretend ping failed
	if sm.Tb.IsMockMode() {
		return nil
	}

	// verify ping failed on each pair
	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode == 0 {
			log.Errorf("Ping succeeded while expecting to fail. Resp: %#v", cmdResp)
			return fmt.Errorf("PingFails: ping succeeded while expecting to fail on %s. code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
		}
	}

	return nil
}

// netcatTrigger runs netcat triggers between pair of workloads
func (sm *SysModel) netcatTrigger(wpc *objects.WorkloadPairCollection, serverOpt, clientOpt string, port int, expFail bool, expClientExitCode int32, expOutput string) error {

	srvWorkloads := make(map[string]*workloadPort)
	for _, pair := range wpc.Pairs {
		key := ""
		actualPort := port
		if port == 0 {
			key = pair.First.Name() + "-" + strconv.Itoa(pair.Ports[0])
			actualPort = pair.Ports[0]
		} else {
			key = pair.First.Name() + "-" + strconv.Itoa(port)
		}
		srvWorkloads[key] =
			&workloadPort{w: pair.First, port: actualPort}
	}

	// stop old netcat servers if its running
	trig := sm.Tb.NewTrigger()
	for _, srvWrkld := range srvWorkloads {
		trig.AddCommand("pkill nc", srvWrkld.w.Name(), srvWrkld.w.NodeName())
		trig.AddCommand("pkill fuz", srvWrkld.w.Name(), srvWrkld.w.NodeName())
	}
	trig.Run()

	// run netcat servers on.First workload
	trig = sm.Tb.NewTrigger()
	for _, srvWrkld := range srvWorkloads {
		trig.AddBackgroundCommand(fmt.Sprintf("nc -lk %s -p %d", serverOpt, srvWrkld.port), srvWrkld.w.Name(), srvWrkld.w.NodeName())
	}
	srvResp, err := trig.Run()
	if err != nil {
		log.Errorf("Failed to run netcat server. Err: %v, Trigger Req: %+v, Resp: %+v", err, trig, srvResp)
		return fmt.Errorf("Failed to run netcat server. Err: %v", err)
	}
	log.Debugf("Got netcat server resp: %+v", srvResp)

	// verify server cmd was successful
	for _, cmdResp := range srvResp {
		if cmdResp.ExitCode != 0 && !strings.Contains(cmdResp.Command, "pkill") {
			log.Errorf("Netcat server command failed. %+v", cmdResp)
			return fmt.Errorf("Netcat server command failed on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	// run netcat from.Second host and verify it was successful
	var clientErr error
	for i := 0; i < maxNetcatRetries; i++ {
		trig = sm.Tb.NewTrigger()
		var pairNames []string
		for _, pair := range wpc.Pairs {
			toIP := pair.First.GetIP()
			actualPort := port
			if port == 0 {
				actualPort = pair.Ports[0]
			}
			trig.AddCommand(fmt.Sprintf("nc -z %s %s %d", clientOpt, toIP, actualPort), pair.Second.Name(), pair.Second.NodeName())
			pairNames = append(pairNames, fmt.Sprintf("%s -> %s", pair.Second.Name(), pair.First.Name()))
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
					if expClientExitCode != 0 && cmdResp.ExitCode != expClientExitCode { // check non-zero exit codes on failure
						log.Errorf("Netcat client command succeeded while expecting to fail. %+v", cmdResp)
						clientErr = fmt.Errorf("Netcat client command succeeded while expecting failure on %s. exit code %v, Out: %v, StdErr: %v", cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)
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
	trig = sm.Tb.NewTrigger()
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

// ConnectionWithOptions runs  session between pair of workloads with options
func (sm *SysModel) ConnectionWithOptions(wpc *objects.WorkloadPairCollection, options *objects.ConnectionOptions) error {

	return sm.FuzItWithOptions(wpc, options)
}

// TCPSession runs TCP session between pair of workloads
func (sm *SysModel) TCPSession(wpc *objects.WorkloadPairCollection, port int) error {

	return sm.FuzIt(wpc, 1, "tcp", strconv.Itoa(port))
}

// UDPSession runs UDP session between pair of workloads
func (sm *SysModel) UDPSession(wpc *objects.WorkloadPairCollection, port int) error {

	if sm.Tb.IsMockMode() {
		return nil
	}

	return sm.FuzIt(wpc, 1, "udp", strconv.Itoa(port))
}

// TCPSessionFails verifies TCP session fails between pair of workloads
func (sm *SysModel) TCPSessionFails(wpc *objects.WorkloadPairCollection, port int) error {

	if sm.Tb.IsMockMode() {
		return nil
	}

	return sm.netcatTrigger(wpc, "", "", port, true, 1, "")
}

// UDPSessionFails verifies  UDP session fails between pair of workloads
func (sm *SysModel) UDPSessionFails(wpc *objects.WorkloadPairCollection, port int) error {
	if sm.Tb.IsMockMode() {
		return nil
	}

	return sm.netcatTrigger(wpc, "-u --sh-exec 'echo test'", "-u", port, true, 0, "test")
}

// Workloads returns all Workloads in the model
func (sm *SysModel) Workloads() *objects.WorkloadCollection {

	wpc := objects.WorkloadCollection{}

	for _, wf := range sm.WorkloadsObjs {
		wpc.Workloads = append(wpc.Workloads, wf)
	}

	return &wpc
}

// WorkloadsSayHelloToDataPath discover workloads so that datapath know about it
func (sm *SysModel) WorkloadsSayHelloToDataPath() error {

	cmds := []*iota.Command{}
	for _, wr := range sm.Workloads().Workloads {
		ipAddr := wr.GetIP()
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("arping -c  5 -U %s -I %s ", ipAddr, wr.GetInterface()),
			EntityName: wr.Name(),
			NodeName:   wr.NodeName(),
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Rediscovering workloads")

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !sm.Tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a arping. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode != 0 {
			//Don't return error for now.
			log.Errorf("Discovery failed. Resp: %#v", cmdResp)
		}
	}

	return nil
}

// WorkloadsGoGetIPs function to do Dhclient in workloads
func (sm *SysModel) WorkloadsGoGetIPs() error {

	cmds := []*iota.Command{}
	for _, wr := range sm.Workloads().Workloads {
		cmd := iota.Command{
			Mode:       iota.CommandMode_COMMAND_FOREGROUND,
			Command:    fmt.Sprintf("dhclient %s", wr.GetInterface()),
			EntityName: wr.Name(),
			NodeName:   wr.NodeName(),
		}
		cmds = append(cmds, &cmd)
	}

	log.Infof("Workloads getting IPs dynamically.")

	trmode := iota.TriggerMode_TRIGGER_PARALLEL
	if !sm.Tb.HasNaplesSim() {
		trmode = iota.TriggerMode_TRIGGER_NODE_PARALLEL
	}

	trigMsg := &iota.TriggerMsg{
		TriggerOp:   iota.TriggerOp_EXEC_CMDS,
		TriggerMode: trmode,
		ApiResponse: &iota.IotaAPIResponse{},
		Commands:    cmds,
	}

	// Trigger App
	topoClient := iota.NewTopologyApiClient(sm.Tb.Client().Client)
	triggerResp, err := topoClient.Trigger(context.Background(), trigMsg)
	if err != nil || triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		return fmt.Errorf("Failed to trigger a dhclient on workload. API Status: %+v | Err: %v", triggerResp.ApiResponse, err)
	}

	for _, cmdResp := range triggerResp.Commands {
		if cmdResp.ExitCode != 0 {
			//Don't return error for now.
			log.Errorf("Getting IP failed. Resp: %#v", cmdResp)
		}
	}

	return nil
}

// VerifyWorkloadStatus verifies workload status in venice
func (sm *SysModel) VerifyWorkloadStatus(wc *objects.WorkloadCollection) error {
	for _, wr := range wc.Workloads {
		wsts, err := sm.GetWorkload(&wr.VeniceWorkload.ObjectMeta)
		if err != nil {
			log.Errorf("Could not get workload %v. Err: %v", wr.VeniceWorkload.Name, err)
			return err
		}

		log.Debugf("Got workload status: %+v", wsts)

		if wsts.Spec.HostName != wr.VeniceWorkload.Spec.HostName {
			return fmt.Errorf("Invalid hostname on workload %v. Exp: %v, Got: %v", wr.VeniceWorkload.Name, wr.VeniceWorkload.Spec.HostName, wsts.Spec.HostName)
		}
		if len(wsts.Spec.Interfaces) != len(wr.VeniceWorkload.Spec.Interfaces) {
			return fmt.Errorf("Unexpected number of interfaces on workload %v. Exp: %+v, Got: %+v", wr.VeniceWorkload.Name, wr.VeniceWorkload.Spec.Interfaces, wsts.Spec.Interfaces)
		}

		name, err := libstrconv.ParseMacAddr(wr.VeniceWorkload.Spec.Interfaces[0].MACAddress)
		if err != nil {
			name = wr.VeniceWorkload.Spec.Interfaces[0].MACAddress
		}
		epMeta := api.ObjectMeta{
			Tenant:    wr.VeniceWorkload.Tenant,
			Namespace: wr.VeniceWorkload.Namespace,
			Name:      wr.VeniceWorkload.Name + "-" + name,
		}

		// get the endpoints for the workload
		ep, err := sm.GetEndpoint(&epMeta)
		if err != nil {
			log.Errorf("Could not get endpoint %v. Err: %v", epMeta.Name, err)
			return err
		}

		if ep.Status.MacAddress != wr.VeniceWorkload.Spec.Interfaces[0].MACAddress {
			return fmt.Errorf("Invalid mac address %v for workload %v", ep.Status.MacAddress, wr.VeniceWorkload.Name)
		}
		if ep.Status.HomingHostName != wr.VeniceWorkload.Spec.HostName {
			return fmt.Errorf("Invalid host name %v for endpoint on workload %v", ep.Status.HomingHostName, wr.VeniceWorkload.Name)
		}
		if ep.Status.NodeUUID != wr.NaplesUUID() {
			return fmt.Errorf("Invalid node uuid %v for endpoint on workload %v", ep.Status.NodeUUID, wr.VeniceWorkload.Name)
		}
		//No need to verify subnet
		//if ep.Status.Network != wr.subnet.Name {
		//	return fmt.Errorf("Invalid subnet %v for endpoint on workload %v", ep.Status.Network, wr.VeniceWorkload.Name)
		//}
	}

	return nil
}

func (sm *SysModel) startFTPServer(wpc *objects.WorkloadPairCollection) error {

	srvWorkloads := make(map[string]*objects.Workload)
	for _, pair := range wpc.Pairs {
		srvWorkloads[pair.First.Name()] = pair.First
	}

	trig := sm.Tb.NewTrigger()
	for _, wr := range srvWorkloads {
		// if the server is not running already, start it
		if !wr.IsFTPServerRunning {
			trig.AddCommand(fmt.Sprintf("systemctl stop vsftpd"), wr.Name(), wr.NodeName())
			trig.AddCommand(fmt.Sprintf("useradd -m -c admin -s /bin/bash admin"), wr.Name(), wr.NodeName())
			trig.AddCommand(fmt.Sprintf("echo admin | tee -a /etc/vsftpd.userlist"), wr.Name(), wr.NodeName())
			trig.AddCommand(fmt.Sprintf("echo password | passwd --stdin admin"), wr.Name(), wr.NodeName())
			trig.AddCommand(fmt.Sprintf("systemctl start vsftpd"), wr.Name(), wr.NodeName())
			trig.AddCommand(fmt.Sprintf("systemctl enable vsftpd"), wr.Name(), wr.NodeName())
			trig.AddCommand(fmt.Sprintf("echo test > /tmp/test.txt"), wr.Name(), wr.NodeName())
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
		if cmdResp.ExitCode != 0 && !strings.Contains(cmdResp.Stdout, "already exists") &&
			!strings.Contains(cmdResp.Stderr, "already exists") {
			log.Errorf("FTP server start failed. %+v", cmdResp)
			return fmt.Errorf("FTP server start failed on %s. exit code %v, Out: %v, StdErr: %v",
				cmdResp.EntityName, cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr)

		}
	}

	// remember server is running already
	for _, wr := range srvWorkloads {
		wr.IsFTPServerRunning = true
	}

	return nil
}

func (sm *SysModel) runFTPClient(wpc *objects.WorkloadPairCollection, ftpPort, expClientExitCode int32, ftpCmd string) error {

	trig := sm.Tb.NewTrigger()

	// add ftp client command for each pair
	var pairNames []string
	for _, pair := range wpc.Pairs {
		toIP := pair.First.GetIP()
		trig.AddCommand(fmt.Sprintf("lftp -u admin,password  %s -p %d -e 'set net:timeout 5; set net:max-retries 3; set net:reconnect-interval-base 5; set xfer:clobber yes; %s; bye' ", toIP, ftpPort, ftpCmd), pair.Second.Name(), pair.Second.NodeName())
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s;", pair.Second.Name(), pair.First.Name()))
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
func (sm *SysModel) FTPGet(wpc *objects.WorkloadPairCollection) error {
	// start ftp server
	err := sm.startFTPServer(wpc)
	if err != nil {
		return err
	}

	// run ftp get
	err = sm.runFTPClient(wpc, 21, 0, "get /tmp/test.txt")
	if err != nil {
		return fmt.Errorf("FTP Get failed. Err: %v", err)
	}

	return nil
}

// FTPGetFails verifies FTP get fails for each workload pair
func (sm *SysModel) FTPGetFails(wpc *objects.WorkloadPairCollection) error {
	// start ftp server
	err := sm.startFTPServer(wpc)
	if err != nil {
		return err
	}

	if sm.Tb.IsMockMode() {
		return nil
	}

	// run ftp get
	err = sm.runFTPClient(wpc, 21, 1, "get /tmp/test.txt")
	if err != nil {
		return fmt.Errorf("FTP Get succeeded while expecting to fail. Err: %v", err)
	}

	return nil
}

// FuzIt Establish large number (numConns) of connections from client workload to server workload
func (sm *SysModel) FuzIt(wpc *objects.WorkloadPairCollection, numConns int, proto, port string) error {
	options := &objects.ConnectionOptions{Port: port,
		NumConns:          numConns,
		Duration:          "10s",
		ReconnectAttempts: 5,
		Proto:             proto,
	}
	return sm.FuzItWithOptions(wpc, options)
}

// FuzItWithOptions Establish large number (numConns) of connections from client workload to server workload with some options
func (sm *SysModel) FuzItWithOptions(wpc *objects.WorkloadPairCollection, options *objects.ConnectionOptions) error {
	var pairNames []string
	// no need to perform connection scale in sim run
	if sm.Tb.HasNaplesSim() {
		return nil
	}

	numConns := options.NumConns
	if numConns == 0 {
		numConns = 1
	}
	cps := options.Cps
	if cps == 0 {
		cps = 100
	}

	srvWorkloads := make(map[string]*workloadPort)
	workloads := make(map[string]*objects.Workload)
	serverInput := make(map[string]fuze.Input)
	clientInput := make(map[string]fuze.Input)

	// walk wc pairs to collect list of workloads, and create fuz input for client and server
	for _, pair := range wpc.Pairs {
		wfName := pair.First.Name()
		workloads[wfName] = pair.First

		key := ""
		actualPort := options.Port

		if options.Port == "0" || options.Port == "" {
			key = wfName + "-" + strconv.Itoa(pair.Ports[0])
			actualPort = strconv.Itoa(pair.Ports[0])
		} else {
			key = wfName + "-" + options.Port
		}

		//Add only if added before
		if _, ok := srvWorkloads[key]; !ok {
			srvWorkloads[key] = &workloadPort{w: pair.First}
			conns := []*fuze.Connection{}
			if _, ok := serverInput[wfName]; ok {
				conns = serverInput[wfName].Connections
			}
			conns = append(conns, &fuze.Connection{ServerIPPort: ":" + actualPort, Proto: options.Proto})
			serverInput[wfName] = fuze.Input{Connections: conns}
		}

		wfName = pair.Second.Name()
		conns := []*fuze.Connection{}
		if _, ok := clientInput[wfName]; ok {
			conns = clientInput[wfName].Connections
		}
		destIPAddr := pair.First.GetIP()
		for ii := 0; ii < numConns; ii++ {
			conns = append(conns, &fuze.Connection{ServerIPPort: destIPAddr + ":" + actualPort, Proto: options.Proto})
		}
		workloads[wfName] = pair.Second
		clientInput[wfName] = fuze.Input{Connections: conns}
		pairNames = append(pairNames, fmt.Sprintf("%s -> %s", pair.Second.Name(), pair.First.Name()))

	}
	log.Infof("Fuz testing workload pairs %v", pairNames)

	var retErr error
	// copy all configuration files the the workloads
	for _, ws := range workloads {
		wsName := ws.Name()
		// copy server configuration

		setUpFile := func(input fuze.Input, filename string) (err error) {

			if jdata, ferr := json.Marshal(input); ferr != nil {
				err = fmt.Errorf("error marshaling json data: %s", err)
			} else if ferr := ioutil.WriteFile(filename, jdata, 0644); ferr != nil {
				err = fmt.Errorf("unable to write to file: %s", err)
			}
			if ferr := sm.Tb.CopyToWorkload(ws.NodeName(), wsName, []string{filename}, "."); ferr != nil {
				err = fmt.Errorf("unable to copy configs to workload: %s", err)
			}

			return err
		}
		if _, ok := serverInput[wsName]; ok {
			if len(serverInput[wsName].Connections) > 1 {
				filename := fmt.Sprintf("/tmp/%s_fuz_server.json", wsName)
				if err := setUpFile(serverInput[wsName], filename); err != nil {
					break
				}
			}
		}

		if _, ok := clientInput[wsName]; ok {
			if len(clientInput[wsName].Connections) > 1 {
				// copy the client configuration
				filename := fmt.Sprintf("/tmp/%s_fuz_client.json", wsName)
				if err := setUpFile(clientInput[wsName], filename); err != nil {
					break
				}
			}
		}
	}
	if retErr != nil {
		log.Errorf("%s", retErr)
		return retErr
	}

	// stop old fuz client/servers if its running
	trig := sm.Tb.NewTrigger()
	for _, ws := range workloads {
		trig.AddCommand("pkill fuz", ws.Name(), ws.NodeName())
		trig.AddCommand("pkill nc", ws.Name(), ws.NodeName())
	}
	trig.Run()

	// run fuz servers on each workload
	trig = sm.Tb.NewTrigger()
	for _, ws := range workloads {

		wsName := ws.Name()
		if _, ok := serverInput[wsName]; ok {
			if len(serverInput[wsName].Connections) > 1 {
				filename := fmt.Sprintf("%s_fuz_server.json", ws.Name())
				trig.AddBackgroundCommand(fmt.Sprintf("./fuz -jsonInput %s -jsonOut", filename),
					ws.Name(), ws.NodeName())
			} else {
				trig.AddBackgroundCommand(fmt.Sprintf("./fuz -proto %v %v", serverInput[wsName].Connections[0].Proto,
					serverInput[wsName].Connections[0].ServerIPPort),
					ws.Name(), ws.NodeName())
			}
		}
	}
	srvResp, err := trig.Run()
	if err != nil {
		retErr = fmt.Errorf("Failed to run fuz server. Err: %v, Trigger Req: %+v, Resp: %+v", err, trig, srvResp)
		log.Errorf("%s", retErr)
		return retErr
	}
	log.Debugf("Got fuz server resp: %+v", srvResp)

	// run fuz client from each server to the others and verify the client results
	var clientErr error

	trig = sm.Tb.NewTrigger()
	for _, ws := range workloads {
		wsName := ws.Name()
		if _, ok := clientInput[wsName]; ok {
			if len(clientInput[wsName].Connections) > 1 {
				filename := fmt.Sprintf("%s_fuz_client.json", ws.Name())
				outfilename := fmt.Sprintf("%s_fuz_client_out.json", ws.Name())
				trig.AddCommand(fmt.Sprintf("./fuz  -attempts %v -duration %v -conns %v -cps %v -talk -jsonInput %s -jsonOut > %s",
					options.ReconnectAttempts,
					options.Duration,
					numConns,
					cps,
					filename, outfilename),
					ws.Name(), ws.NodeName())
			} else {
				trig.AddCommand(fmt.Sprintf("./fuz  -attempts %v -duration %v -conns %v -cps %v -talk -proto %v %v",
					options.ReconnectAttempts,
					options.Duration,
					numConns,
					cps,
					clientInput[wsName].Connections[0].Proto,
					clientInput[wsName].Connections[0].ServerIPPort),
					ws.Name(), ws.NodeName())
			}
		}
	}

	// run trigger
	clientResp, clientErr := trig.RunParallel()
	if clientErr != nil {
		retErr = fmt.Errorf("Failed to run fuz client. Err: %v, Trigger Req: %+v, Resp: %+v", clientErr, trig, clientResp)
		log.Errorf("%s", retErr)
		return retErr
	}
	log.Debugf("Got fuz client resp: %+v", clientResp)

	// verify client commands succeeded
	totalConns := 0
	totalConnsFailed := 0

	for _, cmdResp := range clientResp {
		// clientOutput := make(map[string]fuze.Output)
		if cmdResp.ExitCode != 0 {
			retErr = fmt.Errorf("Fuz client failed on %s, exit code %v", cmdResp.EntityName, cmdResp.ExitCode)
		}
	}
	if retErr != nil {
		log.Errorf("%s", retErr)
		return retErr
	}

	// fetch the output files and parse the results
	for _, ws := range workloads {
		wsName := ws.Name()
		if _, ok := clientInput[wsName]; ok {
			if len(clientInput[wsName].Connections) > 1 {
				filename := fmt.Sprintf("%s_fuz_client_out.json", wsName)
				if err := sm.Tb.CopyFromWorkload(ws.NodeName(), wsName, []string{filename}, "/tmp"); err != nil {
					log.Errorf("unable to copy output from workload: %s", err)
					// return fmt.Errorf("unable to copy output from workload: %s", err)
				}

				fuzOut := fuze.Output{}
				if jdata, err := ioutil.ReadFile(filepath.Join("/tmp", filename)); err != nil {
					retErr = fmt.Errorf("unable to read file %s from for client connection", filename)
				} else if err := json.Unmarshal(jdata, &fuzOut); err != nil {
					retErr = fmt.Errorf("error %s unmarshlig data from file %s", err, filename)
				} else if fuzOut.FailedConnections > 0 {
					for ii := 0; ii < len(fuzOut.Connections); ii++ {
						connOut := fuzOut.Connections[ii]
						if connOut.Failed == 1 {
							log.Errorf("[%d] %s: from %s to %s error %s", ii,
								wsName, connOut.ClientIPPort, connOut.ServerIPPort, connOut.ErrorMsg)
							break
						}
					}
					retErr = fmt.Errorf("Fuz client returned failure for %d/%d connections",
						fuzOut.FailedConnections, fuzOut.SuccessConnections+fuzOut.FailedConnections)
				}
				if retErr != nil {
					log.Errorf("%s", retErr)
				} else {
					log.Infof("Fuz client succeeded on %s for %d connections", wsName, fuzOut.SuccessConnections)
				}
				totalConnsFailed += int(fuzOut.FailedConnections)
				totalConns += int(fuzOut.SuccessConnections) + int(fuzOut.FailedConnections)
			} else {
				for _, cmdResp := range clientResp {
					// clientOutput := make(map[string]fuze.Output)
					if cmdResp.ExitCode != 0 {
						totalConnsFailed++
						retErr = fmt.Errorf("Fuz client returned failure for 1/1 connections")
					} else {
						totalConns++
					}
				}
				totalConns += totalConnsFailed
			}
		}
	}

	log.Infof("Fuz Cliens: workloads %d conns %d failed %d", len(workloads), totalConns, totalConnsFailed)

	// do not clean up upon an error
	if retErr != nil {
		return retErr
	}

	// stop the fuz server
	trig = sm.Tb.NewTrigger()
	stopResp, err := trig.StopCommands(srvResp)
	if err != nil {
		log.Errorf("Error stopping fuz servers. Err: %v. trig: %+v, resp: %+v", err, srvResp, stopResp)
		return nil
	}

	// verify stop was successful
	for _, cmdResp := range stopResp {
		if cmdResp.ExitCode != 0 {
			log.Errorf("Fuz stopping server failed. %+v", cmdResp)
		}
	}

	return nil
}

func (sm *SysModel) NetcatWrapper(wpc *objects.WorkloadPairCollection, serverOpt, clientOpt string, port int, expFail bool, expClientExitCode int32, expOutput string) error {
	return sm.netcatTrigger(wpc, serverOpt, clientOpt, port, expFail, expClientExitCode, expOutput)
}

func (sm *SysModel) DropIcmpFlowTTLSession(wpc *objects.WorkloadPairCollection, cmd string) error {
	if sm.Tb.IsMockMode() {
		return nil
	}
	//Setup session
	sm.TriggerHping(wpc, cmd)
	// Trigger -ttl 0
	sm.TriggerHping(wpc, cmd+" --ttl 0")
	return nil
}

//MoveWorkloads not supported here
func (sm *SysModel) MoveWorkloads(*objects.WorkloadCollection, *objects.HostCollection) error {
	return errors.New("Model does not implement workload moves")
}
