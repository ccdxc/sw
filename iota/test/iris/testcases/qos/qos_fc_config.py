#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.qos.qos_utils as qos
import re

def Setup(tc):
 
    tc.desc = '''
    Test        :   QoS FC Config test
    Opcode      :   Config, Verify
    FC TYPE     :   2 (1=Link Level; 2=PFC)
    '''

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.cmd_cookies = []

    return api.types.status.SUCCESS

def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    server_idx = 0
    client_idx = 1

    w1 = tc.w[server_idx]
    w2 = tc.w[client_idx]

    if not w1.IsNaples() and not w2.IsNaples():
        api.Logger.info("No naples - unsupported configuration")
        return api.types.status.DISABLED

    if tc.os != 'freebsd':
        api.Logger.info("Not FreeBSD - unsupported configuration")
        return api.types.status.DISABLED

    tc.cmd_descr = "FC Config"
    if w1.IsNaples():
        tc.cmd_descr += " on Server: {}({})".format(w1.workload_name, w1.ip_address)
    if w2.IsNaples():
        tc.cmd_descr += " on Client: {}({})".format(w2.workload_name, w2.ip_address)

    if tc.os == 'freebsd':
        cmd = 'sysctl'

    if hasattr(tc.iterators, 'fc_type'):
        fc_type = tc.iterators.fc_type
        if((fc_type != 0) and (fc_type != 1) and (fc_type != 2)):
            api.logger.error("invalid fc_type value passed: {}".format(fc_type))
            return api.types.status.FAILURE
    else:
        api.Logger.error("mandatory attribute fc_type not passed")
        return api.types.status.FAILURE

    #==============================================================
    # trigger FC config test
    #==============================================================
    qos.TriggerFcConfigTest(req, tc, w1, w2, fc_type)

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

    result = api.types.status.SUCCESS

    cookie_idx = 0
    api.Logger.info("Results for {}".format(tc.cmd_descr))

    for cmd in tc.resp.commands:
        api.Logger.info("{}".format(tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)

        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

        cookie_idx += 1

        if "sysctl" in tc.cmd_cookies[cookie_idx-1]:
            continue

        lines = cmd.stdout.split('\n')
        for line in lines:
            if "eth" in line:
                if "pfc" in line:
                    mode = 2
                elif "link" in line:
                    mode = 1
                else:
                    mode = 0

                if mode == tc.iterators.fc_type:
                    result = api.types.status.SUCCESS
                else:
                    result = api.types.status.FAILURE

    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

