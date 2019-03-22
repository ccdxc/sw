#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.utils.iperf as iperf


def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):

    store = tc.GetBundleStore()
    clientReq = api.Trigger_CreateAllParallelCommandsRequest()
    num_sessions = int(getattr(tc.args, "num_sessions", 1))
    for client in store["client_ctxts"]:
        clientCmd = iperf.ClientCmd(client.server_ip, client.server_port,
                proto=tc.iterators.proto, jsonOut=True,
                num_of_streams = num_sessions, connect_timeout = 15000)
        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd, timeout = 0,
                               stdout_on_err = True, stderr_on_err = True)


    tc.iperf_client_resp = api.Trigger(clientReq)

    return api.types.status.SUCCESS

def verify_iperf(tc):
    conn_timedout = 0
    control_socker_err = 0
    measured_send_bw = 0
    measured_received_bw = 0
    for idx, cmd in enumerate(tc.iperf_client_resp.commands):
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
            sent_bw = iperf.GetSentGbps(cmd.stdout)
            api.Logger.info("Iperf Send Rate in Gbps ", sent_bw)
            measured_send_bw += float(sent_bw)
            received_bw = iperf.GetReceivedGbps(cmd.stdout)
            api.Logger.info("Iperf Receive Rate in Gbps ", received_bw)
            measured_received_bw += float(received_bw)

    api.Logger.info("Iperf test successfull")
    api.Logger.info("Number of connection timeouts : {}".format(conn_timedout))
    api.Logger.info("Number of control socket errors : {}".format(control_socker_err))
    api.Logger.info("Total Send BW : {}".format(measured_send_bw))
    api.Logger.info("Total Received BW : {}".format(measured_received_bw))
    return api.types.status.SUCCESS

def Verify(tc):

    if verify_iperf(tc) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
