#! /usr/bin/python3
import pdb
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2

def Setup(tc):

    tc.workload_pairs = api.GetRemoteWorkloadPairs()

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    #Run all commands in parallel.
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    wload_bringup_req = api.BringUpWorkloadsRequest()
    wload_teardown_req = api.TeardownWorkloadsRequest()
    #Start traffic commands in background
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))

        basecmd = 'iperf -p %d ' % api.AllocateTcpPort()
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               "%s -s -t 300" % basecmd, background = True)
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                               "%s -c %s" % (basecmd, w1.ip_address),  background = True)

        api.AddWorkloadBringUp(wload_bringup_req, w1)
        api.AddWorkloadBringUp(wload_bringup_req, w2)
        api.AddWorkloadTeardown(wload_teardown_req, w1)
        api.AddWorkloadTeardown(wload_teardown_req, w2)

        #Just try with 1 workload pair for now
        break


    #Now Send all the commands
    trig_resp = api.Trigger(req)

    #Sleep for some time for traffic to stabalize
    time.sleep(10)
    #Teardown workloads
    ret = api.Trigger_TeardownWorkloadsRequest(wload_teardown_req)
    if ret != api.types.status.SUCCESS:
        api.Trigger_TerminateAllCommands(trig_resp)
        return api.types.status.FAILURE

    #this will fail as workload is already delete and all commands are stopped
    tc.resp = api.Trigger_TerminateAllCommands(trig_resp)

    #Bring up the same workoad loads
    ret = api.Trigger_BringUpWorkloadsRequest(wload_bringup_req)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE


    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
