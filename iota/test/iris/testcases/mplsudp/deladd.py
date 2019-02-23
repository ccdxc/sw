#! /usr/bin/python3
import pdb
import time
import iota.harness.api as api
import iota.test.iris.config.mplsudp.tunnel as tunnel

def Setup(tc):
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True
    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    wload_bringup_req = api.BringUpWorkloadsRequest()
    wload_teardown_req = api.TeardownWorkloadsRequest()
    result = tunnel.GetTunnelManager().DeleteTunnels()
    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to delete tunnels")
        return result

    trig_resp = api.Trigger(req)

    for wl in api.GetWorkloads():
        api.AddWorkloadBringUp(wload_bringup_req, wl)
        api.AddWorkloadTeardown(wload_teardown_req, wl)

    api.Logger.info("Teardown all workloads")
    #Teardown workloads
    ret = api.Trigger_TeardownWorkloadsRequest(wload_teardown_req)
    if ret != api.types.status.SUCCESS:
        api.Trigger_TerminateAllCommands(trig_resp)
        return api.types.status.FAILURE

    #this will fail as workload is already delete and all commands are stopped
    tc.resp = api.Trigger_TerminateAllCommands(trig_resp)

    # Wait for deletes to settle down
    time.sleep(60)

    api.Logger.info("Bringup all workloads")
    #Bring up the same workoad loads
    ret = api.Trigger_BringUpWorkloadsRequest(wload_bringup_req)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    result = tunnel.GetTunnelManager().CreateTunnels()
    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to create tunnels")
        return result
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
