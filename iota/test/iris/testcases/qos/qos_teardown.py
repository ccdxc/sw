#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.qos.qos_utils as qos
import re

def Setup(tc):
 
    tc.desc = '''
    Test        :   QoS Config teardown test
    Opcode      :   Config, Verify
    FC TYPE     :   1 (1=Link Level; 2=PFC)
    '''

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.cmd_cookies = []

    tc.enable = 0
    tc.no_drop = 0
    # hardcode mtu to 1500 while deleting the user class as 0 is not allowed
    tc.mtu = 1500
    tc.wt = 0
    tc.pcp = 0
    tc.pfc_cos = 0

    tc.wt_configured = False
    tc.pcp_configured = False
    tc.pfc_cos_configured = False

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

    qos.TriggerQoSTeardown(req, tc, w1)
    qos.TriggerQoSTeardown(req, tc, w2)

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
    api.Logger.info("Results for Teardown")

    for cmd in tc.resp.commands:
        api.Logger.info("{}".format(tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)

        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

        if "sysctl" in tc.cmd_cookies[cookie_idx]:
            cookie_idx += 1
            continue

        if "halctl show port" in tc.cmd_cookies[cookie_idx]:
            lines = cmd.stdout.split('\n')
            for line in lines:
                if "eth" in line:
                    if ((tc.iterators.fc_type == 1) and ("pfc" in line)):
                        result = api.types.status.FAILURE
                        break

                    if ((tc.iterators.fc_type == 2) and ("link" in line)):
                        result = api.types.status.FAILURE
                        break

        elif "halctl show qos-class" in tc.cmd_cookies[cookie_idx]:
            user_class = "user-defined-" + str(tc.iterators.tclass)
            lines = cmd.stdout.split('\n')
            for line in lines:
                if user_class in line:
                    result = api.types.status.FAILURE
                    break

        cookie_idx += 1

    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

