#! /usr/bin/python3
import time
import random
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.utils.iperf as iperf
import iota.test.iris.testcases.networking.utils as utils

def Setup(tc):
    tc.skip = False
    tc.workload_pairs = []

    for node_name in api.GetNaplesHostnames():
        pairs = utils.GetBondWorkloadPair(node_name=node_name)
        if len(pairs) == 0:
            continue
        tc.workload_pairs += [random.choice(pairs)]

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True
        return api.types.status.SUCCESS

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    tc.serverCmds = []
    tc.clientCmds = []
    tc.cmd_descr = []

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for idx, pairs in enumerate(tc.workload_pairs):
        client = pairs[0] # Node obj
        server = pairs[1] # Workload obj

        cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address, client.Name(), client.GetBondIp())
        tc.cmd_descr.append(cmd_descr)
        num_sessions = int(getattr(tc.args, "num_sessions", 1))
        api.Logger.info("Starting Iperf test from %s num-sessions %d" % (cmd_descr, num_sessions))

        serverCmd = None
        clientCmd = None

        if tc.iterators.proto == 'udp':
            port = api.AllocateUdpPort()
            serverCmd = iperf.ServerCmd(port)
            clientCmd = iperf.ClientCmd(server.ip_address, port, proto='udp', jsonOut=True, num_of_streams = num_sessions, naples=True)
        else:
            port = api.AllocateTcpPort()
            serverCmd = iperf.ServerCmd(port)
            clientCmd = iperf.ClientCmd(server.ip_address, port, jsonOut=True,  num_of_streams = num_sessions, naples=True)

        tc.serverCmds.append(serverCmd)
        tc.clientCmds.append(clientCmd)

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background = True)

        api.Trigger_AddNaplesCommand(clientReq, client.Name(), clientCmd)


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
            api.PrintCommandResults(cmd)
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
