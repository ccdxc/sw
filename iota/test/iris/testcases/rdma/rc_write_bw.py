#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):
    tc.iota_path = api.GetTestsuiteAttr("driver_path")

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w1 = pairs[0][0]
    tc.w2 = pairs[0][1]

    tc.w1_device = api.GetTestsuiteAttr(tc.w1.ip_address+'_device')
    tc.w1_gid = api.GetTestsuiteAttr(tc.w1.ip_address+'_gid')
    tc.w2_device = api.GetTestsuiteAttr(tc.w2.ip_address+'_device')
    tc.w2_gid = api.GetTestsuiteAttr(tc.w2.ip_address+'_gid')

    tc.ib_prefix = 'cd ' + tc.iota_path + ' && . ./env.sh && '

    return api.types.status.SUCCESS

def Trigger(tc):

    w1 = tc.w1
    w2 = tc.w2

    #==============================================================
    # trigger the commands
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting ib_write_bw test from %s" % (tc.cmd_descr))

    # cmd for server
    cmd = "ib_write_bw -d " + tc.w1_device + " -n 10 -F -x " + tc.w1_gid + " -s 1024 -b --report_gbits"
    api.Trigger_AddCommand(req, 
                           w1.node_name, 
                           w1.workload_name,
                           tc.ib_prefix + cmd,
                           background = True)

    # cmd for client
    cmd = "ib_write_bw -d " + tc.w2_device + " -n 10 -F -x " + tc.w2_gid + " -s 1024 -b --report_gbits " + w1.ip_address
    api.Trigger_AddCommand(req, 
                           w2.node_name, 
                           w2.workload_name,
                           tc.ib_prefix + cmd)

    # trigger the request
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("ib_write_bw results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
