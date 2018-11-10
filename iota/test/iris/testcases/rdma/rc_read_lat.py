#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):
    tc.iota_path = api.GetTestsuiteAttr("driver_path")
    tc.ib_prefix = 'cd ' + tc.iota_path + ' && . ./env.sh && '

    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    w1 = pairs[0][0]
    w2 = pairs[0][1]

    #==============================================================
    # get the device and GID
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    api.Logger.info("Extracting device and GID using show_gid")

    cmd = "show_gid | grep %s | grep v2" % w1.ip_address
    api.Trigger_AddCommand(req,
                           w1.node_name,
                           w1.workload_name,
                           tc.iota_path + cmd)

    cmd = "show_gid | grep %s | grep v2" % w2.ip_address
    api.Trigger_AddCommand(req,
                           w2.node_name,
                           w2.workload_name,
                           tc.iota_path + cmd)
    trig_resp = api.Trigger(req)

    w1_device = rdma.GetWorkloadDevice(trig_resp.commands[0].stdout)
    w1_gid = rdma.GetWorkloadGID(trig_resp.commands[0].stdout)

    w2_device = rdma.GetWorkloadDevice(trig_resp.commands[1].stdout)
    w2_gid = rdma.GetWorkloadGID(trig_resp.commands[1].stdout)

    #==============================================================
    # trigger the commands
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting ib_read_lat test from %s" % (tc.cmd_descr))

    # cmd for server
    cmd = "ib_read_lat -d " + w1_device + " -n 10 -F -x " + w1_gid + " -s 1024 --report_gbits"
    api.Trigger_AddCommand(req, 
                           w1.node_name, 
                           w1.workload_name,
                           tc.ib_prefix + cmd,
                           background = True)

    # cmd for client
    cmd = "ib_read_lat -d " + w2_device + " -n 10 -F -x " + w2_gid + " -s 1024 --report_gbits " + w1.ip_address
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

    api.Logger.info("ib_read_lat results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
