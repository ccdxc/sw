#! /usr/bin/python3
import iota.harness.api as api
import re
import iota.test.utils.naples_host as host

def Setup(tc):
 
    tc.desc = '''
    Test        :   Dump RDMA qstate
    Opcode      :   N/A
    '''

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.lifs = api.GetTestsuiteAttr("lifs")

    return api.types.status.SUCCESS

def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    w1 = tc.w[1]
    w2 = tc.w[0]

    for w in [w1, w2]:
        for lif in tc.lifs:
            cmd = 'export PATH=$PATH:/nic/lib:/platform/lib && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nic/lib:/platform/lib && /platform/bin/eth_dbgtool rdma_qstate ' + lif
            api.Logger.info("Collecting rdma qstate command {} on node_name {}, lif {}"\
                             .format(cmd, w.node_name, lif))

            api.Trigger_AddNaplesCommand(req,
                                         w.node_name,
                                         cmd)

    #==============================================================
    # trigger the request
    #==============================================================
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    return api.types.status.SUCCESS

def Verify(tc):
    #==============================================================
    # verify the output
    #==============================================================

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

    return api.types.status.SUCCESS

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

