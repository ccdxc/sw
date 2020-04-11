#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.qos.qos_utils as qos
import re

def Setup(tc):
 
    tc.desc = '''
    Test        :   QoS Set classification type
    Opcode      :   Config, Verify
    '''

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    class_type  = getattr(tc.args, 'class_type', None)

    if class_type == None:
        api.Logger.error("Mandatory argument class_type not passed")
        return api.types.status.FAILURE

    if int(class_type) != 1 and int(class_type) != 2:
        api.Logger.error("Invalid argument class_type {} passed. Supported values are 1 (PCP) and 2 (DSCP)".format(class_type))
        return api.types.status.FAILURE

    tc.class_type = class_type

    return api.types.status.SUCCESS


def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    w1 = tc.w[0]
    w2 = tc.w[1]

    cmd = 'sysctl dev.ionic.0.qos.classification_type=' + str(tc.class_type)

    # Trigger Classification type config
    if w1.IsNaples():
        api.Trigger_AddCommand(req, 
                               w1.node_name, 
                               w1.workload_name,
                               cmd)
    if w2.IsNaples():
        api.Trigger_AddCommand(req, 
                               w2.node_name, 
                               w2.workload_name,
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

    result = api.types.status.SUCCESS

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

