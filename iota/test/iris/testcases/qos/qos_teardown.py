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

    tc.cmd_descr = "FC Config"
    if w1.IsNaples():
        tc.cmd_descr += " on Server: {}({})".format(w1.workload_name, w1.ip_address)
    if w2.IsNaples():
        tc.cmd_descr += " on Client: {}({})".format(w2.workload_name, w2.ip_address)

    if hasattr(tc.iterators, 'tclass'):
        tclass = tc.iterators.tclass
        if (tclass < 1) or (tclass > 6):
            api.logger.error("invalid tclass passed: {}".format(tclass))
            return api.types.status.FAILURE
    else:
        api.Logger.error("mandatory attribute tclass not passed")
        return api.types.status.FAILURE

    if hasattr(tc.iterators, 'fc_config'):
        fc_config = tc.iterators.fc_config
        if (fc_config != 0) and (fc_config != 1):
            api.logger.error("invalid fc_config value passed: {}".format(fc_config))
            return api.types.status.FAILURE

        if(fc_config == 1):
            if hasattr(tc.iterators, 'fc_type'):
                fc_type = tc.iterators.fc_type
                if((fc_type != 0) and (fc_type != 1) and (fc_type != 2)):
                    api.logger.error("invalid fc_type value passed: {}".format(fc_type))
                    return api.types.status.FAILURE

                #trigger FC config
                qos.TriggerFcConfigTest(req, tc, w1, w2, fc_type)
            else:
                api.Logger.error("mandatory attribute fc_type not passed when fc_config is set")
                return api.types.status.FAILURE


    # trigger PFC Config - QOS Class deletion
    if w1.IsNaples():
        qos.TriggerPfcConfigTest(req, tc, w1, tclass)
    if w2.IsNaples():
        qos.TriggerPfcConfigTest(req, tc, w2, tclass)

    #Disable Service Policy on the interfaces
    result = api.DisableQosPorts(tc.nodes, "pmap-iota")
    if result != api.types.status.SUCCESS:
        api.Logger.error("Disable Service policy failed")

    #Enable link level pause on the interfaces
    result = api.EnablePausePorts(tc.nodes)
    if result != api.types.status.SUCCESS:
        api.Logger.error("Enabling link level pause failed")


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

