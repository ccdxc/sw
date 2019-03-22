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
    serverReq = store["server_req"]

    tc.server_resp  = api.Trigger(serverReq)
    store["server_resp"] = tc.server_resp
    #Sleep for some time as bg may not have been started.
    time.sleep(30)

    return api.types.status.SUCCESS

def Verify(tc):

    for idx, cmd in enumerate(tc.server_resp.commands):
        if cmd.exit_code != 0:
            api.Logger.error("Error starting iperf")
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
