#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):

    tc.desc = '''
    Test  :   ib_write_lat
    Opcode:   Write Only
    Num QP:   1
    Pad   :   No
    Inline:   No
    modes :   inherently bidirectional
    '''

    tc.iota_path = api.GetTestsuiteAttr("driver_path")

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.devices = []
    tc.gid = []
    tc.ib_prefix = []
    for i in range(2):
        tc.devices.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_device'))
        tc.gid.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_gid'))
        if tc.w[i].IsNaples():
            tc.ib_prefix.append('cd ' + tc.iota_path + ' && . ./env.sh && ')
        else:
            tc.ib_prefix.append('')

    return api.types.status.SUCCESS

def Trigger(tc):

    i = 0
    j = i + 1
    w1 = tc.w[i]
    w2 = tc.w[j]

    #==============================================================
    # trigger the commands
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting ib_write_lat test from %s" % (tc.cmd_descr))

    # cmd for server
    cmd = "ib_write_lat -d " + tc.devices[i] + " -n 10 -F -x " + tc.gid[i] + " -s 1024 --report_gbits"
    api.Trigger_AddCommand(req, 
                           w1.node_name, 
                           w1.workload_name,
                           tc.ib_prefix[i] + cmd,
                           background = True)

    # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
    # starts listening. So sleep for a few seconds before trying to start the client
    cmd = 'sleep 2'
    api.Trigger_AddCommand(req,
                           w1.node_name,
                           w1.workload_name,
                           cmd)

    # cmd for client
    cmd = "ib_write_lat -d " + tc.devices[j] + " -n 10 -F -x " + tc.gid[j] + " -s 1024 --report_gbits " + w1.ip_address
    api.Trigger_AddCommand(req, 
                           w2.node_name, 
                           w2.workload_name,
                           tc.ib_prefix[j] + cmd)

    # trigger the request
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("ib_write_lat results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
