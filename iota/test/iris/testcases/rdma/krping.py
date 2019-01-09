#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):

    tc.desc = '''
    Test  :   krping
    Opcode:   REG_MR, LOCAL_INV, Send, Read, Write
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

        # skip, until rdma_krping is installed on Mellanox sanity/regression servers
        if not tc.w[i].IsNaples():
            return api.types.status.IGNORED

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

    api.Logger.info("Starting krping_rdma test from %s" % (tc.cmd_descr))

    # cmd for server
    if api.GetNodeOs(w1.node_name) == 'linux':
        krpfile = " /proc/krping "
    else:
        krpfile = " /dev/krping "
        
    cmd = "sudo echo -n 'server,port=9999,addr=" + w1.ip_address + ",verbose,validate,count=100 ' > " + krpfile
    api.Trigger_AddCommand(req, 
                           w1.node_name, 
                           w1.workload_name,
                           cmd,
                           background = True)

    # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
    # starts listening. So sleep for a few seconds before trying to start the client
    cmd = 'sleep 2'
    api.Trigger_AddCommand(req,
                           w1.node_name,
                           w1.workload_name,
                           cmd)

    # cmd for client
    cmd = "sudo echo -n 'client,port=9999,addr=" + w1.ip_address + ",verbose,validate,count=100 ' > " + krpfile
    api.Trigger_AddCommand(req, 
                           w2.node_name, 
                           w2.workload_name,
                           cmd)

    # trigger the request
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("krping results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
