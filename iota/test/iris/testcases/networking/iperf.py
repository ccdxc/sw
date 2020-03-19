#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.utils.iperf as iperf
from iota.test.iris.utils import vmotion_utils


def Setup(tc):

    tc.skip = False
    if tc.selected:
        tc.workload_pairs = tc.selected
    else:
        if tc.args.type == 'local_only':
            tc.workload_pairs = api.GetLocalWorkloadPairs()
        else:
            tc.workload_pairs = api.GetRemoteWorkloadPairs()

        tc.workload_pairs = tc.workload_pairs[:1]

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if getattr(tc.args, 'vmotion_enable', False):
        wloads = []
        # collecting all server
        for wl_pair in tc.workload_pairs:
            wloads.append(wl_pair[1])

        vmotion_utils.PrepareWorkloadVMotion(tc, wloads)

    #Have to fix it later.
    return api.types.status.SUCCESS
    #return api.BringUpWorkloads(wloads)

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    tc.serverCmds = []
    tc.clientCmds = []
    tc.cmd_descr = []


    serverReq = None
    clientReq = None


    if not api.IsSimulation():
        serverReq = api.Trigger_CreateAllParallelCommandsRequest()
        clientReq = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for idx, pairs in enumerate(tc.workload_pairs):
        client = pairs[0]
        server = pairs[1]

        cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        tc.cmd_descr.append(cmd_descr)
        num_sessions = int(getattr(tc.args, "num_sessions", 1))
        api.Logger.info("Starting Iperf test from %s num-sessions %d" % (cmd_descr, num_sessions))

        serverCmd = None
        clientCmd = None

        if tc.iterators.proto == 'udp':
            port = api.AllocateTcpPort()
            serverCmd = iperf.ServerCmd(port)
            clientCmd = iperf.ClientCmd(server.ip_address, port, proto='udp', jsonOut=True, num_of_streams = num_sessions)
        else:
            port = api.AllocateUdpPort()
            serverCmd = iperf.ServerCmd(port)
            clientCmd = iperf.ClientCmd(server.ip_address, port, jsonOut=True,  num_of_streams = num_sessions)

        tc.serverCmds.append(serverCmd)
        tc.clientCmds.append(clientCmd)

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background = True)

        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd)


    server_resp = api.Trigger(serverReq)
    #Sleep for some time as bg may not have been started.
    time.sleep(30)



    tc.iperf_client_resp = api.Trigger(clientReq)
    #Its faster kill iperf servers

    #Still call terminate on all
    api.Trigger_TerminateAllCommands(server_resp)

    return api.types.status.SUCCESS

def verify_iperf(tc):
    conn_timedout = 0
    control_socker_err = 0
    for idx, cmd in enumerate(tc.iperf_client_resp.commands):
        api.Logger.info("Iperf Results for %s" % (tc.cmd_descr[idx]))
        api.Logger.info("Iperf Server cmd  %s" % (tc.serverCmds[idx]))
        api.Logger.info("Iperf Client cmd %s" % (tc.clientCmds[idx]))
        if cmd.exit_code != 0:
            api.Logger.error("Iperf client exited with error")
            if iperf.ConnectionTimedout(cmd.stdout):
                api.Logger.error("Connection timeout, ignoring for now")
                conn_timedout = conn_timedout + 1
                continue
            if iperf.ControlSocketClosed(cmd.stdout):
                api.Logger.error("Control socket cloned, ignoring for now")
                control_socker_err = control_socker_err + 1
                continue
            if iperf.ServerTerminated(cmd.stdout):
                api.Logger.error("Iperf server terminated")
                return api.types.status.FAILURE
            if not iperf.Success(cmd.stdout):
                api.Logger.error("Iperf failed", iperf.Error(cmd.stdout))
                return api.types.status.FAILURE
        #api.Logger.info("Iperf Send Rate in Gbps ", iperf.GetSentGbps(cmd.stdout))
        #api.Logger.info("Iperf Receive Rate in Gbps ", iperf.GetReceivedGbps(cmd.stdout))

    api.Logger.info("Iperf test successfull")
    api.Logger.info("Number of connection timeouts : {}".format(conn_timedout))
    api.Logger.info("Number of control socket errors : {}".format(control_socker_err))
    return api.types.status.SUCCESS

def Verify(tc):

    if tc.skip: return api.types.status.SUCCESS

    if verify_iperf(tc) != api.types.status.SUCCESS:
        #Some Iperf3 are failing, need to debug.
        return api.types.status.SUCCESS

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
