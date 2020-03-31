#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.qos.qos_utils as qos
import re

def Setup(tc):
 
    tc.desc = '''
    Test        :   QoS PFC Config test
    Opcode      :   Config, Verify
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
    tc.wt = 0
    tc.pcp = 0
    tc.pfc_cos = 0

    tc.wt_configured = False
    tc.pcp_configured = False
    tc.pfc_cos_configured = False

    tc.server_idx = 0
    tc.client_idx = 0

    # setup for traffic test
    tc.traffic_type = 0     # 0: no traffic test by default
                            # 1: RDMA; 2: iPerf

    tc.iota_path = api.GetTestsuiteAttr("driver_path")
    tc.vlan_idx = api.GetTestsuiteAttr("vlan_idx")

    tc.w.append(pairs[tc.vlan_idx][0])
    tc.w.append(pairs[tc.vlan_idx][1])

    tc.devices = []
    tc.gid = []
    tc.ib_prefix = []

    for i in range(4):
        tc.devices.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_device'))
        tc.gid.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_gid'))
        if tc.w[i].IsNaples():
            tc.ib_prefix.append('cd ' + tc.iota_path + ' && ./run_rdma.sh  ')
        else:
            tc.ib_prefix.append('')

    return api.types.status.SUCCESS


def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if tc.os != 'freebsd':
        api.Logger.info("Not FreeBSD - unsupported configuration")
        return api.types.status.DISABLED

    if tc.w[0].IsNaples():
        if tc.w[1].IsNaples():
            tc.server_idx = 2
            tc.client_idx = 3
        else:
            tc.server_idx = 3
            tc.client_idx = 2

        w1 = tc.w[0]
        w2 = tc.w[1]
    else:
        if tc.w[1].IsNaples():
            tc.server_idx = 2
            tc.client_idx = 3
        else:
            api.Logger.info("No naples - unsupported configuration")
            return api.types.status.DISABLED

        w1 = tc.w[1]
        w2 = tc.w[0]

    ws = tc.w[tc.server_idx]
    wc = tc.w[tc.client_idx]

    tc.cmd_descr = "QoS TC Config with PFC"
    if w1.IsNaples():
        tc.cmd_descr += " on Server: {}({})".format(w1.workload_name, w1.ip_address)
    if w2.IsNaples():
        tc.cmd_descr += " on Client: {}({})".format(w2.workload_name, w2.ip_address)

    if tc.os == 'freebsd':
        cmd = 'sysctl'

    if hasattr(tc.iterators, 'tclass'):
        tclass = tc.iterators.tclass
        if (tclass < 1) or (tclass > 6):
            api.logger.error("invalid tclass passed: {}".format(tclass))
            return api.types.status.FAILURE
    else:
        api.Logger.error("mandatory attribute tclass not passed")
        return api.types.status.FAILURE

    if hasattr(tc.iterators, 'enable'):
        tc.enable = tc.iterators.enable
        if (tc.enable != 0) and (tc.enable != 1):
            api.logger.error("invalid enable value passed: {}".format(tc.enable))
            return api.types.status.FAILURE
    else:
        api.Logger.error("mandatory attribute enable not passed")
        return api.types.status.FAILURE

    if hasattr(tc.iterators, 'no_drop'):
        tc.no_drop = tc.iterators.no_drop
        if (tc.no_drop != 0) and (tc.no_drop != 1):
            api.logger.error("invalid no_drop value passed: {}".format(tc.no_drop))
            return api.types.status.FAILURE
    else:
        api.Logger.error("mandatory attribute no_drop not passed")
        return api.types.status.FAILURE

    if hasattr(tc.iterators, 'wt'):
        tc.wt = tc.iterators.wt
        if tc.wt < 0 or tc.wt > 100:
            api.logger.error("invalid weight passed: {}".format(tc.wt))
            return api.types.status.FAILURE
        tc.wt_configured = True

    if hasattr(tc.iterators, 'pcp'):
        tc.pcp = tc.iterators.pcp
        if tc.pcp < 0 or tc.pcp > 7:
            api.logger.error("invalid pcp passed: {}".format(tc.pcp))
            return api.types.status.FAILURE
        tc.pcp_configured = True

    if hasattr(tc.iterators, 'pfc_cos'):
        tc.pfc_cos = tc.iterators.pfc_cos
        if tc.pfc_cos < 0 or tc.pfc_cos > 7:
            api.logger.error("invalid pfc_cos passed: {}".format(tc.pfc_cos))
            return api.types.status.FAILURE
        tc.pfc_cos_configured = True

    if hasattr(tc.iterators, 'traffic_test_type'):
        if ((tc.iterators.traffic_type != 0) and\
            (tc.iterators.traffic_type != 1) and (tc.iterators.traffic_type != 2)):
            api.logger.error("invalid traffic_type value passed: {}"\
                             .format(tc.iterators.traffic_type))
            return api.types.status.FAILURE
        tc.traffic_type = tc.iterators.traffic_type

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

    if tc.pcp_configured == True and tc.pfc_cos_configured == True:
        if tc.pcp != tc.pfc_cos: #Ignore the testcase if pcp and pfc_cos are different
            return api.types.status.IGNORED 
        
    # Trigger PFC Config - QOS Class creation
    if w1.IsNaples():
        qos.TriggerPfcConfigTest(req, tc, w1, tclass)
    if w2.IsNaples():
        qos.TriggerPfcConfigTest(req, tc, w2, tclass)


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

        if "halctl show qos-class" in tc.cmd_cookies[cookie_idx]:
            user_class = "user-defined-" + str(tc.iterators.tclass)

            lines = cmd.stdout.split('\n')
            for line in lines:
                if user_class in line:
                    qos_class_attr = line.split()

                    if tc.pfc_cos_configured == True:
                        if int(qos_class_attr[3]) != tc.pfc_cos:
                            api.Logger.error("PFC COS values dont match {} {}"\
                                     .format(tc.pfc_cos, qos_class_attr[3]))
                            result = api.types.status.FAILURE

                    if tc.wt_configured == True:
                        if str(tc.iterators.wt) not in qos_class_attr[4]:
                            api.Logger.error("Weights dont match {} {}"\
                                     .format(tc.iterators.wt, qos_class_attr[4]))
                            result = api.types.status.FAILURE

                    if tc.pcp_configured == True:
                        if int(qos_class_attr[5]) != tc.iterators.pcp:
                            result = api.types.status.FAILURE
                            api.Logger.error("PCPs dont match {} {}"\
                                     .format(tc.iterators.pcp, qos_class_attr[5]))

                    break

        # QOS verification cmd
        elif "qos show cmd" in tc.cmd_cookies[cookie_idx]:
            lines = cmd.stdout.split('\n')
            attrs = ["pcp_to_tc", "tc_pfc_cos", "tc_no_drop", "tc_enable"]

            for line in lines:
                for attr in attrs:
                    if attr in line:
                        line_attrs = line.split(": ")
                        if attr == "pcp_to_tc": #pcp_to_tc output has 8 tcs while others have only 7
                            api.SetTestsuiteAttr(attr, line_attrs[1].replace(" ", ","))
                        else:
                            api.SetTestsuiteAttr(attr, line_attrs[1].replace(" ", ",")+",0")

        cookie_idx += 1

    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

