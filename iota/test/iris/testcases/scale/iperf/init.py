#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.utils.iperf as iperf


class IperfClient:

    def __init__(self, server_ip, server_port, node_name, workload_name):
        self.server_ip = server_ip
        self.server_port = server_port
        self.node_name = node_name
        self.workload_name = workload_name

def Setup(tc):

    tc.skip = False

    max_workload_pairs = int(getattr(tc.args, "num_ep_pairs", 1))
    tc.workload_pairs = tc.selected[:max_workload_pairs]
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    tc.serverCmds = []
    tc.clientCmds = []
    tc.cmd_descr = []


    iperfClients = []
    serverReq = api.Trigger_CreateAllParallelCommandsRequest()

    for idx, pairs in enumerate(tc.workload_pairs):
        client = pairs[0]
        server = pairs[1]

        cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        tc.cmd_descr.append(cmd_descr)
        num_sessions = int(getattr(tc.args, "num_sessions", 1))
        api.Logger.info("Starting Iperf test from %s num-sessions %d" % (cmd_descr, num_sessions))

        serverCmd = None

        #if tc.iterators.proto == 'udp':
        port = api.AllocateTcpPort()
        serverCmd = iperf.ServerCmd(port)
        tc.serverCmds.append(serverCmd)
        iperfClients.append(IperfClient(server.ip_address, port,
                    client.node_name, client.workload_name))

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background = True,
                               stdout_on_err = True, stderr_on_err = True)


    store = tc.GetBundleStore()
    store["server_req"] = serverReq
    store["client_ctxts"] = iperfClients


    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
