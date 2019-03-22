#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api


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
    serverResp = store.get("server_resp", None)
    if serverResp:
        api.Trigger_TerminateAllCommands(serverResp)
    serverResp = store.get("combined_server_resp", None)
    if serverResp:
        api.Trigger_TerminateAllCommands(serverResp)
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
