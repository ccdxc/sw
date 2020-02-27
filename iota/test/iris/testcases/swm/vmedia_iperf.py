#! /usr/bin/python3
import paramiko
import logging
import sys
import time
import traceback

import iota.harness.api as api
from iota.harness.infra.redfish import redfish_client
from .utils.ping import ping
from .utils.ncsi_ops import check_set_ncsi
import iota.test.iris.utils.iperf as iperf
from .utils.nic_ops import get_nic_mode
from .utils.ncsi_ops import set_ncsi_mode
from .utils.dedicated_mode_ops import set_dedicated_mode
from .utils.common import get_redfish_obj
from .utils.host import tuneLinux
from .utils.vmedia_ops import mount_vmedia
from .utils.vmedia_ops import eject_vmedia

VMEDIA_PATH = "http://pxe/iso/swm_test/test_vmedia_iperf.iso"

def Setup(tc):
    tc.mounted = False
    tc.RF = None
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found, exiting...")
        return api.types.status.ERROR
    tc.test_node = naples_nodes[0]
    tc.node_name = tc.test_node.Name()

    cimc_info = tc.test_node.GetCimcInfo()
    if not cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
        
    tc.ilo_ip = cimc_info.GetIp()
    tc.ilo_ncsi_ip = cimc_info.GetNcsiIp()
    try:
        check_set_ncsi(cimc_info)
        # Create a Redfish client object
        tc.RF = get_redfish_obj(cimc_info, mode="ncsi")
        # Mount vmedia
        if mount_vmedia(tc.RF, VMEDIA_PATH) != api.types.status.SUCCESS:
            api.Logger.error("Mounting vmedia unsuccessful")
            return api.types.status.ERROR
        api.Logger.info("Vmedia mount success")
        time.sleep(10)
        tc.mounted = True
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR
    
    # Tune linux settings
    tuneLinux()

    workload_pairs = api.GetRemoteWorkloadPairs()
    
    if not workload_pairs:
        api.Logger.error('No workload pairs found')
        return api.types.status.ERROR
    
    tc.wl_pair = workload_pairs[0]

    return api.types.status.SUCCESS

def __execute_cmd(node_name, cmd, timeout=None):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    if timeout:
        api.Trigger_AddHostCommand(req, node_name, cmd, timeout=timeout)
    else:
        api.Trigger_AddHostCommand(req, node_name, cmd)

    resp = api.Trigger(req)
    if resp is None:
        raise RuntimeError("Failed to trigger on host %s cmd %s" % (node_name, cmd))
    return resp

def _run_vmedia_traffic(node_name):
    retries = 10
    for _i in range(retries):
        cddev = "/dev/sr0"
        cmd = "ls %s" % cddev
        resp = __execute_cmd(node_name, cmd)
        cmd = resp.commands.pop()
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            cddev = "/dev/sr1"
            cmd = "ls %s" % cddev
            resp = __execute_cmd(node_name, cmd)
            cmd = resp.commands.pop()
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                api.Logger.info("Device not available %s" % cddev)
                if _i < (retries - 1):
                    api.Logger.info("Retrying after 30s...")
                    time.sleep(30)
                continue
        api.Logger.info("Vmedia is mapped to device %s" % (cddev))
        cmd = "dd if=%s of=/dev/null bs=1M" % cddev
        resp = __execute_cmd(node_name, cmd, timeout=3600)
        cmd = resp.commands.pop()
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            raise RuntimeError("Vmedia traffic test is not successfull")
        return
    raise RuntimeError("Vmedia device was not detected on the host after %d retries"
                       % (retries))

def Trigger(tc):
    serverCmd = None
    clientCmd = None
    IPERF_TIMEOUT = 86400
    try:
        serverReq = api.Trigger_CreateExecuteCommandsRequest()
        clientReq = api.Trigger_CreateExecuteCommandsRequest()
        
        client = tc.wl_pair[0]
        server = tc.wl_pair[1]

        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address,
                        client.workload_name, client.ip_address)
        num_streams = int(getattr(tc.args, "num_streams", 2))
        api.Logger.info("Starting Iperf test from %s num-sessions %d"
                        % (tc.cmd_descr, num_streams))

        if tc.iterators.proto == 'tcp':
            port = api.AllocateTcpPort()
            serverCmd = iperf.ServerCmd(port, jsonOut=True, run_core=3)
            clientCmd = iperf.ClientCmd(server.ip_address, port, time=IPERF_TIMEOUT,
                                        jsonOut=True, num_of_streams=num_streams,
                                        run_core=3)
        else:
            port = api.AllocateUdpPort()
            serverCmd = iperf.ServerCmd(port, jsonOut=True, run_core=3)
            clientCmd = iperf.ClientCmd(server.ip_address, port, proto='udp',
                                        jsonOut=True, num_of_streams=num_streams,
                                        run_core=3)

        tc.serverCmd = serverCmd
        tc.clientCmd = clientCmd

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background=True, timeout=IPERF_TIMEOUT)

        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd, background=True, timeout=IPERF_TIMEOUT)
        
        tc.server_resp = api.Trigger(serverReq)
        time.sleep(5)
        tc.iperf_client_resp = api.Trigger(clientReq)

        _run_vmedia_traffic(tc.node_name)
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def verify_iperf(tc):
    conn_timedout = 0
    control_socker_err = 0
    resp = api.Trigger_TerminateAllCommands(tc.iperf_client_resp)
    cmd = resp.commands.pop()
    api.Logger.info("Iperf Results for %s" % (tc.cmd_descr))
    api.Logger.info("Iperf Server cmd  %s" % (tc.serverCmd))
    api.Logger.info("Iperf Client cmd %s" % (tc.clientCmd))
    #api.PrintCommandResults(cmd)
    if cmd.exit_code != 0:
        api.Logger.error("Iperf client exited with error")
        if iperf.ConnectionTimedout(cmd.stdout):
            api.Logger.error("Connection timeout, ignoring for now")
            conn_timedout = conn_timedout + 1
        if iperf.ControlSocketClosed(cmd.stdout):
            api.Logger.error("Control socket cloned, ignoring for now")
            control_socker_err = control_socker_err + 1
        if iperf.ServerTerminated(cmd.stdout):
            api.Logger.error("Iperf server terminated")
            return api.types.status.FAILURE
        if not iperf.Success(cmd.stdout):
            api.Logger.error("Iperf failed", iperf.Error(cmd.stdout))
            return api.types.status.FAILURE
    api.Logger.info("Iperf Send Rate in Gbps ", iperf.GetSentGbps(cmd.stdout))
    api.Logger.info("Iperf Receive Rate in Gbps ", iperf.GetReceivedGbps(cmd.stdout))
    api.Trigger_TerminateAllCommands(tc.server_resp)
    
    api.Logger.info("Iperf test successfull")
    api.Logger.info("Number of connection timeouts : {}".format(conn_timedout))
    api.Logger.info("Number of control socket errors : {}".format(control_socker_err))
    return api.types.status.SUCCESS

def Verify(tc):
    if verify_iperf(tc) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    try:
        if tc.mounted is True:
            if eject_vmedia(tc.RF) != api.types.status.SUCCESS:
                api.Logger.error("Ejecting vmedia unsuccessful")
        if tc.RF:
            tc.RF.logout()
    except:
        api.Logger.error(traceback.format_exc())
        return api.types.status.ERROR

    return api.types.status.SUCCESS