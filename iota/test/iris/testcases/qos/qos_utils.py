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

    #Enable/Disable flow control on the switch
    if tc.enable == 0:
        api.DisablePausePorts(tc.nodes)
    else:
        api.EnablePausePorts(tc.nodes)

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

def PfcGetConfig():
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

    return pcp_to_tc,tc_pfc_cos,tc_no_drop,tc_enable

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
    pcp_to_tc,tc_pfc_cos,tc_no_drop,tc_enable = PfcGetConfig()
    api.Logger.info("Current PFC config:\n \ttc_enable '{}' \n\ttc_no_drop '{}' \n\ttc_pfc_cos '{}' \n\tpcp_to_tc '{}'"\
                    .format(tc_enable, tc_no_drop, tc_pfc_cos, pcp_to_tc))

    #Enable/Disable PFC on the switch
    """if tc.enable == 0:
        api.DisablePfcPorts(tc.nodes)
    else:
        api.EnablePfcPorts(tc.nodes)
    """
    cmd = 'sysctl'
    qos_cmd = ' dev.ionic.' + str(dev) + '.qos'
    verify_qos_cmd = cmd + qos_cmd

    tc_index = 2*tclass
    enable_qos_cmd = cmd + qos_cmd + ".tc_enable=" + tc_enable[:tc_index] + str(tc.enable) + tc_enable[tc_index+1:]

    if tc.pcp_configured == True:
        pcp = tc.pcp
    else:
        pcp = 0
    
    pcp_index = 2*pcp
    pcp_to_tc_value = tclass if tc.enable==1 else 0

    if tc.pfc_cos_configured == True:
        pfc_cos = tc.pfc_cos
    else:
        pfc_cos = 0

    no_drop_cmd = cmd + qos_cmd + ".tc_no_drop=" + tc_no_drop[:tc_index] + str(tc.no_drop) + tc_no_drop[tc_index+1:]
    pfc_cos_cmd = cmd + qos_cmd + ".tc_pfc_cos=" + tc_pfc_cos[:tc_index] + str(pfc_cos) + tc_pfc_cos[tc_index+1:]
    pcp_cmd = cmd + qos_cmd + ".pcp_to_tc=" + pcp_to_tc[:pcp_index] + str(pcp_to_tc_value) + pcp_to_tc[pcp_index+1:]

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

        api.Logger.info("Running pcp to tc command {} on node_name {} workload_name {}"\
                        .format(pcp_cmd, w.node_name, w.workload_name))
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               pcp_cmd)
        tc.cmd_cookies.append(pcp_cmd)

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

def PostTrafficTestCommands(req, tc, w, pcp):
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
        tc.cmd_cookies.append(cmd + " for pcp " + str(pcp))

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


def TriggerRDMATraffic(req, tc, ws, wc):
    if hasattr(tc.args, 'rdma_pcps'):
        rdma_pcps = getattr(tc.args, 'rdma_pcps')
    else:
        rdma_pcps = [0]

    if hasattr(tc.iterators, 'num_qp'):
        num_qp = tc.iterators.num_qp
        qp_opt = ' -q {numqp} '.format(numqp = str(tc.iterators.num_qp))
    else:
        num_qp = 10

    if hasattr(tc.iterators, 'threads') and \
       tc.iterators.threads > 1:
        num_threads = int(getattr(tc.iterators, 'threads'))
        tc.client_bkg = True
    else:
        num_threads = 1

    s_port = 12340

    #RDMA traffic test
    for pcp in rdma_pcps[:-1]:
        e_port = s_port + num_threads
        for port in range(s_port, e_port):
            # cmd for server
            api.Logger.info("Starting server for ib_send_bw test")
            #port = 12340 + int(pcp)
            cmd = "ib_send_bw -d " + tc.devices[tc.server_idx] + " -F -x " + tc.gid[tc.server_idx] + " -m 4096 -s 4096 -q " + str(num_qp) + " --report_gbits -p " + str(port) + " -n 10 "
            if pcp:
                cmd += "-S " + str(pcp) + " "
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

        for port in range(s_port, e_port):
            # cmd for client
            api.Logger.info("Running the client for ib_send_bw test")

            cmd = "ib_send_bw -d " + tc.devices[tc.client_idx] + " -F -x " + tc.gid[tc.client_idx] + " -m 4096 -s 4096 -q " + str(num_qp) + " --report_gbits -p " + str(port) +" -n 10 "
            if pcp:
                cmd += "-S " + str(pcp) + " " 
            cmd += ws.ip_address

            api.Trigger_AddCommand(req, 
                                   wc.node_name, 
                                   wc.workload_name,
                                   tc.ib_prefix[tc.client_idx] + cmd,
                                   background = True)
            tc.cmd_cookies.append(tc.ib_prefix[tc.client_idx] + cmd)

            cmd = 'sleep 5'
            api.Trigger_AddCommand(req,
                                   ws.node_name,
                                   ws.workload_name,
                                   cmd)
            tc.cmd_cookies.append(cmd)

        s_port += num_threads

    e_port = s_port + num_threads

    pcp = rdma_pcps[-1]
    for port in range(s_port, e_port):
        # cmd for server
        api.Logger.info("Starting server for ib_send_bw test")
        #port = 12340 + int(pcp)
        cmd = "ib_send_bw -d " + tc.devices[tc.server_idx] + " -F -x " + tc.gid[tc.server_idx] + " -m 4096 -s 4096 -q " + str(num_qp) + " --report_gbits -p " + str(port) + " -n 10 "
        if pcp:
            cmd += "-S " + str(pcp) + " "
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

    for port in range(s_port, e_port-1):
        # cmd for client
        api.Logger.info("Running the client for ib_send_bw test")

        cmd = "ib_send_bw -d " + tc.devices[tc.client_idx] + " -F -x " + tc.gid[tc.client_idx] + " -m 4096 -s 4096 -q " + str(num_qp) + " --report_gbits -p " + str(port) +" -n 10 "
        if pcp:
            cmd += "-S " + str(pcp) + " " 
        cmd += ws.ip_address

        api.Trigger_AddCommand(req, 
                               wc.node_name, 
                               wc.workload_name,
                               tc.ib_prefix[tc.client_idx] + cmd,
                               background = True)
        tc.cmd_cookies.append(tc.ib_prefix[tc.client_idx] + cmd)

    port = e_port-1
    api.Logger.info("Running the client for ib_send_bw test")

    cmd = "ib_send_bw -d " + tc.devices[tc.client_idx] + " -F -x " + tc.gid[tc.client_idx] + " -m 4096 -s 4096 -q " + str(num_qp) + " --report_gbits -p " + str(port) +" -n 10 "
    if pcp:
        cmd += "-S " + str(pcp) + " " 
    cmd += ws.ip_address

    api.Trigger_AddCommand(req, 
                           wc.node_name, 
                           wc.workload_name,
                           tc.ib_prefix[tc.client_idx] + cmd,
                           timeout = 600)
    tc.cmd_cookies.append(tc.ib_prefix[tc.client_idx] + cmd)

    cmd = 'sleep 5'
    api.Trigger_AddCommand(req,
                           ws.node_name,
                           ws.workload_name,
                           cmd)
    tc.cmd_cookies.append(cmd)

    #Add the post traffic test show commands
    PostTrafficTestCommands(req, tc, ws, pcp)
    PostTrafficTestCommands(req, tc, wc, pcp)
    
    return api.types.status.SUCCESS

def TriggerIPerfTraffic(req, tc, ws, wc):
    #TODO: Add iPerf testcases
    return api.types.status.SUCCESS

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

def TriggerTrafficTest(req, tc, ws, wc):

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

    TriggerRDMATraffic(req, tc, ws, wc)
    TriggerIPerfTraffic(req, tc, ws, wc)

    return api.types.status.SUCCESS

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

    return int(drops)

def QosGetDropsForDevFromTestSuite(dev, node_name):
    attr = node_name + "_dev" + dev + "_drops"
    drops = api.GetTestsuiteAttr(attr)

    api.Logger.info("{} prev drops are: {}".format(node_name, drops))
    return int(drops)

def QosSetDropsForDev(drops, dev, node_name):
    attr = node_name + "_dev" + dev + "_drops"
    api.SetTestsuiteAttr(attr, drops)
    return api.types.status.SUCCESS
