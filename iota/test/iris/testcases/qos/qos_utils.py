#! /usr/bin/python3
import iota.harness.api as api


############################################################
# Trigger FlowCtrl Config Test
#   * Adds the config and verification commands
#   * Callers to trigger and check the response output
#   * Test Valid only on FreeBSD
#
# FC TYPE: 
#   0: Default
#   1: Link Level
#   2: PFC
############################################################

def TriggerFcConfigTest(req, tc, w1, w2, fc_type):

    #sanity checks
    if tc.os != 'freebsd':
        api.Logger.info("Not FreeBSD - unsupported configuration")
        return api.types.status.DISABLED

    if((fc_type != 0) and (fc_type != 1) and (fc_type != 2)):
        api.logger.error("invalid fc_type value passed: {}".format(fc_type))
        return api.types.status.FAILURE

    # TODO: for now run the command on ionic0 and ionic1 - until it is fixed in the HAL
    #cmd += 'sysctl dev.ionic.' + str(dev) + '.flow_ctrl=' + str(fc_type)
    cmd = 'sleep 2'
    cmd0 = 'sysctl dev.ionic.0.flow_ctrl=' + str(fc_type)
    cmd1 = 'sysctl dev.ionic.1.flow_ctrl=' + str(fc_type)

    if w1.IsNaples():
        api.Logger.info("Running command {} on node_name {} workload_name {}".format(cmd0, w1.node_name, w1.workload_name))

        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd0)
        tc.cmd_cookies.append(cmd0)

        api.Logger.info("Running command {} on node_name {} workload_name {}".format(cmd1, w1.node_name, w1.workload_name))

        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd1)
        tc.cmd_cookies.append(cmd1)

        #flow_ctrl mode change triggers port flaps - hence sleep for 2 seconds to wait for the ports to be up
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd)
        tc.cmd_cookies.append(cmd)

    if w2.IsNaples():
        api.Logger.info("Running command {} on node_name {} workload_name {}".format(cmd0, w2.node_name, w2.workload_name))

        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               cmd0)
        tc.cmd_cookies.append(cmd0)

        api.Logger.info("Running command {} on node_name {} workload_name {}".format(cmd1, w2.node_name, w2.workload_name))

        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               cmd1)
        tc.cmd_cookies.append(cmd1)

        #flow_ctrl mode change triggers port flaps - hence sleep for 2 seconds to wait for the ports to be up
        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               cmd)
        tc.cmd_cookies.append(cmd)

    # verification command
    cmd = '/nic/bin/halctl show port'
    # run the command on ionic0 and ionic1
    '''
    if dev == 0:
        port = 1
    elif dev == 1:
        port = 5
    else:
        api.Logger.info("invalid dev number {}; defaulting to port 1".format(str(dev)))
        port = 1
    cmd += ' --port ' + str(port)
    '''

    if w1.IsNaples():
        api.Logger.info("Running command {} on node_name {}".format(cmd, w1.node_name))

        api.Trigger_AddNaplesCommand(req, 
                                     w1.node_name, 
                                     cmd)
        tc.cmd_cookies.append(cmd)

    if w2.IsNaples():
        api.Logger.info("Running command {} on node_name {}".format(cmd, w2.node_name))

        api.Trigger_AddNaplesCommand(req, 
                                     w2.node_name, 
                                     cmd)
        tc.cmd_cookies.append(cmd)

############################################################
# Get current PFC Config 
############################################################

def QosGetCurrentConfig():
    dscp_dict = {}

    dscp_0_7_to_tc = api.GetTestsuiteAttr("dscp_0_7_to_tc")
    if dscp_0_7_to_tc == None:
        dscp_0_7_to_tc = "0,0,0,0,0,0,0,0"

    dscp_8_15_to_tc = api.GetTestsuiteAttr("dscp_8_15_to_tc")
    if dscp_8_15_to_tc == None:
        dscp_8_15_to_tc = "0,0,0,0,0,0,0,0"

    dscp_16_23_to_tc = api.GetTestsuiteAttr("dscp_16_23_to_tc")
    if dscp_16_23_to_tc == None:
        dscp_16_23_to_tc = "0,0,0,0,0,0,0,0"

    dscp_24_31_to_tc = api.GetTestsuiteAttr("dscp_24_31_to_tc")
    if dscp_24_31_to_tc == None:
        dscp_24_31_to_tc = "0,0,0,0,0,0,0,0"

    dscp_32_39_to_tc = api.GetTestsuiteAttr("dscp_32_39_to_tc")
    if dscp_32_39_to_tc == None:
        dscp_32_39_to_tc = "0,0,0,0,0,0,0,0"

    dscp_40_47_to_tc = api.GetTestsuiteAttr("dscp_40_47_to_tc")
    if dscp_40_47_to_tc == None:
        dscp_40_47_to_tc = "0,0,0,0,0,0,0,0"

    dscp_48_55_to_tc = api.GetTestsuiteAttr("dscp_48_55_to_tc")
    if dscp_48_55_to_tc == None:
        dscp_48_55_to_tc = "0,0,0,0,0,0,0,0"

    dscp_56_63_to_tc = api.GetTestsuiteAttr("dscp_56_63_to_tc")
    if dscp_56_63_to_tc == None:
        dscp_56_63_to_tc = "0,0,0,0,0,0,0,0"

    dscp_dict['dscp_0_7_to_tc'] = dscp_0_7_to_tc
    dscp_dict['dscp_8_15_to_tc'] = dscp_8_15_to_tc
    dscp_dict['dscp_16_23_to_tc'] = dscp_16_23_to_tc
    dscp_dict['dscp_24_31_to_tc'] = dscp_24_31_to_tc
    dscp_dict['dscp_32_39_to_tc'] = dscp_32_39_to_tc
    dscp_dict['dscp_40_47_to_tc'] = dscp_40_47_to_tc
    dscp_dict['dscp_48_55_to_tc'] = dscp_48_55_to_tc
    dscp_dict['dscp_56_63_to_tc'] = dscp_56_63_to_tc

    pcp_to_tc = api.GetTestsuiteAttr("pcp_to_tc")
    if pcp_to_tc == None:
        pcp_to_tc = "0,0,0,0,0,0,0,0"

    tc_pfc_cos = api.GetTestsuiteAttr("tc_pfc_cos")
    if tc_pfc_cos == None:
        tc_pfc_cos = "0,0,0,0,0,0,0,0"

    tc_no_drop = api.GetTestsuiteAttr("tc_no_drop")
    if tc_no_drop  == None:
        tc_no_drop = "1,0,0,0,0,0,0,0"

    tc_enable = api.GetTestsuiteAttr("tc_enable")
    if tc_enable == None:
        tc_enable = "1,0,0,0,0,0,0,0"

    return dscp_dict,pcp_to_tc,tc_pfc_cos,tc_no_drop,tc_enable

def QosAddPcpConfig(req, w, tc, tclass, pcp_to_tc):
    if tc.pcp_configured == True:
        pcp = tc.pcp
    else:
        pcp = 0

    pcp_index = 2 * pcp
    pcp_to_tc_value = tclass if tc.enable==1 else 0

    pcp_cmd = 'sysctl dev.ionic.0.qos.pcp_to_tc=' + pcp_to_tc[:pcp_index] + str(pcp_to_tc_value) + pcp_to_tc[pcp_index+1:]
    api.Logger.info("Running pcp to tc command {} on node_name {} workload_name {}"\
                    .format(pcp_cmd, w.node_name, w.workload_name))
    api.Trigger_AddCommand(req,
                           w.node_name,
                           w.workload_name,
                           pcp_cmd)
    tc.cmd_cookies.append(pcp_cmd)

def QosAddDscpConfig(req, w, tc, tclass, dscp_dict):
    if tc.dscps_configured == True:
        dscps = tc.dscps
    else:
        dscps = [0]

    dscp_to_tc_value = tclass if tc.enable==1 else 0

    for dscp in dscps:
        dscp_low_index = 8 * (dscp // 8)
        dscp_high_index = dscp_low_index + 7
        dscp_conf_str = 'dscp_' + str(dscp_low_index) + '_' + str(dscp_high_index) + '_to_tc'
        dscp_to_tc = dscp_dict[dscp_conf_str]

        dscp_index = 2 * (dscp % 8)


        dscp_cmd = "sysctl dev.ionic.0.qos." + str(dscp_conf_str) + "=" + dscp_to_tc[:dscp_index] + str(dscp_to_tc_value) + dscp_to_tc[dscp_index+1:] 
        api.Logger.info("Running dscp to tc command {} on node_name {} workload_name {}"\
                        .format(dscp_cmd, w.node_name, w.workload_name))
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               dscp_cmd)
        tc.cmd_cookies.append(dscp_cmd)

def QosAddTcEthernetConfig(req, tc, w, ethernet_tc):
    ethernet_cmd = "sysctl dev.ionic.0.qos.tc_ethernet=" + str(ethernet_tc)
    api.Logger.info("Running tc_ethernet command {} on node_name {} workload_name {}"\
                    .format(ethernet_cmd, w.node_name, w.workload_name))
    api.Trigger_AddCommand(req,
                           w.node_name,
                           w.workload_name,
                           ethernet_cmd)
    tc.cmd_cookies.append(ethernet_cmd)

############################################################
# Trigger PFC Config Test
#   * Adds the config and verification commands
#   * Callers to trigger and check the response output
#   * Test Valid only on FreeBSD
############################################################

def TriggerPfcConfigTest(req, tc, w, tclass):

    #sanity checks
    if tc.os != 'freebsd':
        api.Logger.info("Not FreeBSD - unsupported configuration")
        return api.types.status.DISABLED

    if (tclass < 1) or (tclass > 6):
        api.logger.error("invalid tclass passed: {}".format(tclass))
        return api.types.status.FAILURE

    dev = api.GetTestsuiteAttr(w.ip_address+'_device')[-1]
    dscp_dict,pcp_to_tc,tc_pfc_cos,tc_no_drop,tc_enable = QosGetCurrentConfig()
    api.Logger.info("Current PFC config:\n \ttc_enable '{}' \n\ttc_no_drop '{}' \n\ttc_pfc_cos '{}' \n\tpcp_to_tc '{}'"\
                    "\n\tdscp_dict '{}'"\
                    .format(tc_enable, tc_no_drop, tc_pfc_cos, pcp_to_tc, dscp_dict))

    cmd = 'sysctl'
    #qos_cmd = ' dev.ionic.' + str(dev) + '.qos'
    qos_cmd = ' dev.ionic.0.qos'
    verify_qos_cmd = cmd + qos_cmd

    tc_index = 2*tclass
    enable_qos_cmd = cmd + qos_cmd + ".tc_enable=" + tc_enable[:tc_index] + str(tc.enable) + tc_enable[tc_index+1:]
    
    if tc.pfc_cos_configured == True:
        pfc_cos = tc.pfc_cos
    else:
        pfc_cos = 0

    no_drop_cmd = cmd + qos_cmd + ".tc_no_drop=" + tc_no_drop[:tc_index] + str(tc.no_drop) + tc_no_drop[tc_index+1:]
    pfc_cos_cmd = cmd + qos_cmd + ".tc_pfc_cos=" + tc_pfc_cos[:tc_index] + str(pfc_cos) + tc_pfc_cos[tc_index+1:]

    api.Logger.info("Running enable qos command {} on node_name {} workload_name {}"\
                    .format(enable_qos_cmd, w.node_name, w.workload_name))
    api.Trigger_AddCommand(req,
                           w.node_name,
                           w.workload_name,
                           enable_qos_cmd)
    tc.cmd_cookies.append(enable_qos_cmd)

    if tc.enable: #Apply other config only during enable.
        api.Logger.info("Running no drop command {} on node_name {} workload_name {}"\
                        .format(no_drop_cmd, w.node_name, w.workload_name))
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               no_drop_cmd)
        tc.cmd_cookies.append(no_drop_cmd)

        api.Logger.info("Running pfc cos command {} on node_name {} workload_name {}"\
                        .format(pfc_cos_cmd, w.node_name, w.workload_name))
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               pfc_cos_cmd)
        tc.cmd_cookies.append(pfc_cos_cmd)

        if tc.class_type == 1:
            QosAddPcpConfig(req, w, tc, tclass, pcp_to_tc)
        else:
            QosAddDscpConfig(req, w, tc, tclass, dscp_dict)

    api.Logger.info("Running qos show command {} on node_name {} workload_name {}"\
                    .format(verify_qos_cmd, w.node_name, w.workload_name))
    api.Trigger_AddCommand(req,
                           w.node_name,
                           w.workload_name,
                           verify_qos_cmd)
    tc.cmd_cookies.append("qos show cmd")

    # verification command
    cmd = '/nic/bin/halctl show qos-class'
    if tc.enable != 0:
        # delete QoS class
        cmd += ' --qosgroup user-defined-' + str(tclass)

    api.Logger.info("Running command {} on node_name {}"\
                    .format(cmd, w.node_name))

    api.Trigger_AddNaplesCommand(req, 
                                 w.node_name, 
                                 cmd)
    tc.cmd_cookies.append(cmd)


############################################################
# Commands to run after Traffic Test
#   * Adds the commands to run after traffic test
#
############################################################

def PostTrafficTestCommands(req, tc, w, pcp_or_dscp):
    #verify PFC/PAUSE frames post traffic
    if w.IsNaples():
        dev = api.GetTestsuiteAttr(w.ip_address+'_device')[-1]
        if dev == '0':
            port = 1
        elif dev == '1':
            port = 5
        else:
            api.Logger.info("invalid dev number {}; defaulting to port 1".format(str(dev)))
            port = 1
        cmd = '/nic/bin/halctl show port --port ' + str(port) + ' statistics'

        api.Logger.info("Running command {} on node_name {} workload_name {}"\
                        .format(cmd, w.node_name, w.workload_name))
        api.Trigger_AddNaplesCommand(req, 
                               w.node_name,
                               cmd)
        mode = "pcp" if tc.class_type==1 else "dscp"
        tc.cmd_cookies.append(cmd + " for " + mode + " " + str(pcp_or_dscp))

        # show drops command
        cmd = '/nic/bin/halctl show system statistics drop | grep -i "occupancy"'
        api.Logger.info("Running show drops command {} on node_name {}"\
                        .format(cmd, w.node_name))

        api.Trigger_AddNaplesCommand(req, 
                                     w.node_name, 
                                     cmd)
        tc.cmd_cookies.append("show drops cmd for node {} ip_address {}".format(w.node_name, w.ip_address))
 
    else:
        api.Logger.info("node {} is not Naples; cannot check for PFC frames".format(w.node_name));


############################################################
# Trigger Traffic Test
#   * Adds the traffic test type commands
#   * Callers to trigger and check the response output
#
# Traffic Types: 
#   0: no traffic test
#   1: RDMA traffic test
#   2: iPerf traffic test
############################################################

def TriggerTrafficTest(req, tc, ws, wc, traffic_type, pcp_or_dscp, is_background):

    if traffic_type == 0:
        return api.types.status.SUCCESS

    #clear port counters before running traffic test
    cmd = '/nic/bin/halctl clear port'

    if wc.IsNaples():
        api.Logger.info("Running command {} on node_name {} workload_name {}"\
                        .format(cmd, wc.node_name, wc.workload_name))
        api.Trigger_AddNaplesCommand(req, 
                               wc.node_name,
                               cmd)
        tc.cmd_cookies.append(cmd)
    else:
        api.Logger.info("client node is not Naples; cannot clear port stats");

    if ws.IsNaples():
        api.Logger.info("Running command {} on node_name {} workload_name {}"\
                        .format(cmd, ws.node_name, ws.workload_name))
        api.Trigger_AddNaplesCommand(req, 
                               ws.node_name,
                               cmd)
        tc.cmd_cookies.append(cmd)
    else:
        api.Logger.info("client node is not Naples; cannot clear port stats");

    if (traffic_type == 1):
        #RDMA traffic test

        # cmd for server
        api.Logger.info("Starting server for ib_send_bw test")
        port = 12340 + int(pcp_or_dscp)
        cmd = "ib_send_bw -d " + tc.devices[tc.server_idx] + " -F -x " + tc.gid[tc.server_idx] + " -m 4096 -s 65536 -q 125 --report_gbits -p " + str(port) + " -n 1000 -b -W 16 "
        if tc.class_type == 1:
            cmd += "-S " + str(pcp_or_dscp)
        else:
            tos = 4 * int(pcp_or_dscp) #left shift dscp by 2 bits
            cmd += "-R -T " + str(tos) + " "
        api.Trigger_AddCommand(req, 
                               ws.node_name, 
                               ws.workload_name,
                               tc.ib_prefix[tc.server_idx] + cmd,
                               background = True)
        tc.cmd_cookies.append(tc.ib_prefix[tc.server_idx] + cmd)

        # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
        # starts listening. So sleep for a few seconds before trying to start the client
        cmd = 'sleep 2'
        api.Trigger_AddCommand(req,
                               ws.node_name,
                               ws.workload_name,
                               cmd)
        tc.cmd_cookies.append(cmd)

        # cmd for client
        api.Logger.info("Running the client for ib_send_bw test")

        cmd = "ib_send_bw -d " + tc.devices[tc.client_idx] + " -F -x " + tc.gid[tc.client_idx] + " -m 4096 -s 65536 -q 125 --report_gbits -p " + str(port) +" -n 1000 -b -W 16 "
        if tc.class_type == 1:
            cmd += "-S " + str(pcp_or_dscp) + " "
        else:
            tos = 4 * int(pcp_or_dscp) #left shift dscp by 2 bits
            cmd += "-R -T " + str(tos) + " "
        cmd += ws.ip_address

        api.Trigger_AddCommand(req, 
                               wc.node_name, 
                               wc.workload_name,
                               tc.ib_prefix[tc.client_idx] + cmd,
                               timeout=600,
                               background = is_background)
        tc.cmd_cookies.append(tc.ib_prefix[tc.client_idx] + cmd)

    elif (traffic_type == 2):
        #iPerf traffic test
        iperf_port = 8001 + pcp_or_dscp
        iperf_opts = " -p " + str(iperf_port)

        iperf_server_cmd = "iperf3 -s " + iperf_opts

        iperf_client_cmd = "iperf3 -c " + ws.ip_address + iperf_opts + " -t 300 "
        if tc.class_type == 2:
            tos = 4 * int(pcp_or_dscp) #left shift dscp by 2 bits
            iperf_client_cmd += " -S " + str(tos)

        api.Trigger_AddCommand(req, 
                               ws.node_name, 
                               ws.workload_name,
                               iperf_server_cmd,
                               background = True)
        tc.cmd_cookies.append(iperf_server_cmd)

        api.Trigger_AddCommand(req, 
                               wc.node_name, 
                               wc.workload_name,
                               iperf_client_cmd,
                               background = True)
        tc.cmd_cookies.append(iperf_client_cmd)


    else:
        api.logger.error("Invalid traffic_type passed: {}. Valid values are 1 - RDMA and 2 - iPerf".format(traffic_type))
        return api.types.status.FAILURE

    #Add the post traffic test show commands
    PostTrafficTestCommands(req, tc, ws, pcp_or_dscp)
    PostTrafficTestCommands(req, tc, wc, pcp_or_dscp)

    return api.types.status.SUCCESS

def TriggerQoSTeardown(req, tc, w):
    # Disable tc_ethernet before disabling the TCs
    QosAddTcEthernetConfig(req, tc, w, 0)

    # Disable all the TCs
    disable_cmd = "sysctl dev.ionic.0.qos.tc_enable=" + '"1 0 0 0 0 0 0"'
    api.Logger.info("Running TC disable command {} on node_name {} workload_name {}"\
                    .format(disable_cmd, w.node_name, w.workload_name))
    api.Trigger_AddCommand(req,
                           w.node_name,
                           w.workload_name,
                           disable_cmd)
    tc.cmd_cookies.append(disable_cmd)

    # Set the Flow Control type to LLFC
    flow_ctrl_cmd = 'sysctl dev.ionic.0.flow_ctrl=1'
    api.Logger.info("Running flow control command {} on node_name {} workload_name {}"\
                    .format(flow_ctrl_cmd, w.node_name, w.workload_name))
    api.Trigger_AddCommand(req,
                           w.node_name,
                           w.workload_name,
                           flow_ctrl_cmd)
    tc.cmd_cookies.append(flow_ctrl_cmd)

    # Set the Classification type to PCP
    classification_cmd = 'sysctl dev.ionic.0.qos.classification_type=1'

    api.Trigger_AddCommand(req, 
                           w.node_name, 
                           w.workload_name,
                           classification_cmd)
    tc.cmd_cookies.append(classification_cmd)

def QosGetDropsForDevFromOutput(output, dev):
    if not output:
        drops = '0'
    else:
        lines = output.split('\n')
        for line in lines:
            line_attrs = line.split()
            if line_attrs[0] == dev:
                drops = line_attrs[4]
                break
    api.Logger.info("curr drops are: {}".format(drops))

    return drops

def QosGetDropsForDevFromTestSuite(dev, node_name):
    attr = node_name + "_dev" + dev + "_drops"
    drops = api.GetTestsuiteAttr(attr)

    api.Logger.info("{} prev drops are: {}".format(node_name, drops))
    return drops

def QosSetDropsForDev(output, dev, node_name):
    if not output:
        drops = '0'
    else:
        lines = output.split('\n')
        for line in lines:
            line_attrs = line.split()
            if line_attrs[0] == dev:
                drops = line_attrs[4]
                break
    attr = node_name + "_dev" + dev + "_drops"
    api.SetTestsuiteAttr(attr, drops)

#Sample Output:
#    dev.ionic.0.qos.dscp_56_63_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_48_55_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_40_47_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_32_39_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_24_31_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_16_23_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_8_15_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.dscp_0_7_to_tc: 0 0 0 0 0 0 0 0
#    dev.ionic.0.qos.pcp_to_tc: 0 0 0 0 2 0 0 0
#    dev.ionic.0.qos.tc_sched_type: 1 0 1 1 0 0 0
#    dev.ionic.0.qos.tc_pfc_cos: 0 0 0 0 0 0 0
#    dev.ionic.0.qos.tc_no_drop: 1 0 0 0 0 0 0
#    dev.ionic.0.qos.tc_ethernet: 0
#    dev.ionic.0.qos.tc_enable: 1 0 1 0 0 0 0
#    dev.ionic.0.qos.classification_type: 0
#    dev.ionic.0.qos.max_tcs: 7
def QosSetTestsuiteAttrs(output):
    lines = output.split('\n')
    for line in lines:
        if line == None or line == "":
            continue
        attrs = line.split('.')
        attr = attrs[4]
        [key, value] = attr.split(": ")
        if (key != "pcp_to_tc") and ('dscp' not in key):#pcp_to_tc and dscp_to_tc outputs have 8 tcs while others have only 7
            value += " 0"
        api.SetTestsuiteAttr(key, value.replace(" ", ","))

# Get the TC for which the given pcp is configured
def QosGetTcForPcp(pcp):
    tclass = 0
    pcp_to_tc = api.GetTestsuiteAttr('pcp_to_tc')
    pcp_index = 2 * int(pcp)
    tclass = pcp_to_tc[pcp_index]
    return tclass

# Get the TC for which the given dscp is configured
def QosGetTcForDscp(dscp):
    tclass = 0
    dscp_low_index = 8 * (int(dscp) // 8)
    dscp_high_index = dscp_low_index + 7
    dscp_to_tc_str = 'dscp_' + str(dscp_low_index) + '_' + str(dscp_high_index) + '_to_tc'
    dscp_to_tc = api.GetTestsuiteAttr(dscp_to_tc_str)
    dscp_index = 2 * (int(dscp) % 8)

    tclass = dscp_to_tc[dscp_index]
    return tclass

# Get the Cos for a given TC
def QosGetCosForTc(tclass):
    cos = 0
    tc_pfc_cos = api.GetTestsuiteAttr('tc_pfc_cos')
    cos_index = 2 * int(tclass)
    cos = tc_pfc_cos[cos_index]
    return cos
