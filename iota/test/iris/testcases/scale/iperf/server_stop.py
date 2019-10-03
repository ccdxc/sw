#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.utils.iperf as iperf


def Setup(tc):

    tc.skip = False
    tc.workload_pairs = tc.selected
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip: return api.types.status.SUCCESS

    store = tc.GetBundleStore()
    serverResp = store["server_resp"]


    kill_cmd = "pkill -9 iperf3"
    server_nodes = set()
    for cmd in serverResp.commands:
        server_nodes.add(cmd.node_name)

    serverReq = api.Trigger_CreateAllParallelCommandsRequest()
    for server in server_nodes:
        api.Trigger_AddHostCommand(serverReq, server, kill_cmd)
    api.Trigger(serverReq)

    #Still call terminate on all
    api.Trigger_TerminateAllCommands(serverResp)
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
