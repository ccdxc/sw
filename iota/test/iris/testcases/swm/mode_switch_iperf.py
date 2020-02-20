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

def Setup(tc):
    naples_nodes = api.GetNaplesNodes()
    if len(naples_nodes) == 0:
        api.Logger.error("No naples node found")
        return api.types.status.ERROR
    tc.test_node = naples_nodes[0]
    tc.node_name = tc.test_node.Name()

    cimc_info = tc.test_node.GetCimcInfo()
    if not cimc_info:
        api.Logger.error("CimcInfo is None, exiting")
        return api.types.status.ERROR
        
    tc.ilo_ip = cimc_info.GetIp()
    tc.ilo_ncsi_ip = cimc_info.GetNcsiIp()
    tc.cimc_info = cimc_info

    if ping(tc.ilo_ip, 2) == api.types.status.SUCCESS:
        tc.initial_mode = "dedicated"
    elif ping(tc.ilo_ncsi_ip, 2) == api.types.status.SUCCESS:
        tc.initial_mode = "ncsi"
    else:
        api.Logger.error('ILO unreachable')
        return api.types.status.FAILURE
    
    workload_pairs = api.GetRemoteWorkloadPairs()
    
    if not workload_pairs:
        api.Logger.error('No workload pairs found')
        return api.types.status.ERROR
    
    tc.wl_pair = workload_pairs[0]

    return api.types.status.SUCCESS

def Trigger(tc):
    max_pings = int(getattr(tc.args, "max_pings", 60))
    num_runs = int(getattr(tc.args, "num_runs", 50))
    serverCmd = None
    clientCmd = None
    mode = tc.initial_mode
    IPERF_TIMEOUT = 86400
    try:
        serverReq = api.Trigger_CreateExecuteCommandsRequest()
        clientReq = api.Trigger_CreateExecuteCommandsRequest()
        
        client = tc.wl_pair[0]
        server = tc.wl_pair[1]

        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address,
                        client.workload_name, client.ip_address)
        num_streams = int(getattr(tc.args, "num_streams", 1))
        api.Logger.info("Starting Iperf test from %s num-sessions %d"
                        % (tc.cmd_descr, num_streams))

        if tc.iterators.proto == 'tcp':
            port = api.AllocateTcpPort()
            serverCmd = iperf.ServerCmd(port, jsonOut=True, run_core=2)
            clientCmd = iperf.ClientCmd(server.ip_address, port, time=IPERF_TIMEOUT,
                                        jsonOut=True, num_of_streams=num_streams,
                                        run_core=2)
        else:
            port = api.AllocateUdpPort()
            serverCmd = iperf.ServerCmd(port, jsonOut=True, run_core=2)
            clientCmd = iperf.ClientCmd(server.ip_address, port, proto='udp',
                                        jsonOut=True, num_of_streams=num_streams,
                                        run_core=2)

        tc.serverCmd = serverCmd
        tc.clientCmd = clientCmd

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background=True, timeout=IPERF_TIMEOUT)

        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd, background=True, timeout=IPERF_TIMEOUT)
        
        tc.server_resp = api.Trigger(serverReq)
        time.sleep(5)
        tc.iperf_client_resp = api.Trigger(clientReq)

        for _i in range(num_runs):
            RF = get_redfish_obj(tc.cimc_info, mode=mode)
            obs_mode = get_nic_mode(RF)
            api.Logger.info("Iteration %d: curr_mode %s" % (_i, obs_mode))
            if mode != obs_mode:
                raise RuntimeError("Expected NIC mode %s, observed %s" % (mode, obs_mode))

            next_mode = "dedicated" if mode == "ncsi" else "ncsi"
            if next_mode == "ncsi":
                ret = set_ncsi_mode(RF, mode="dhcp")
            else:
                ret = set_dedicated_mode(RF, mode="dhcp")
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Mode switch from %s -> %s failed" %(mode, next_mode))
                return api.types.status.FAILURE

            api.Logger.info("Switched mode to %s" % (next_mode))
            time.sleep(5)
            if ret == api.types.status.SUCCESS:
                curr_ilo_ip = tc.ilo_ip if next_mode == "dedicated" else tc.ilo_ncsi_ip
                ret = ping(curr_ilo_ip, max_pings)
                if ret != api.types.status.SUCCESS:
                    RF.logout()
                    raise RuntimeError('Unable to ping ILO, Port Switch fail from'
                                      ' %s -> %s' % (mode, next_mode))
                api.Logger.info("Mode switch from %s -> %s successful" % (mode, next_mode))
            else:
                raise RuntimeError('Mode switch config failed')
            mode = next_mode
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
    return api.types.status.SUCCESS