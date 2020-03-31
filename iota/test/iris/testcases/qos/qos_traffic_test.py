#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.qos.qos_utils as qos
import re

def Setup(tc):
 
    tc.desc = '''
    Test        :   QoS Traffic test
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

    tc.server_idx = 0
    tc.client_idx = 0

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

    qos.TriggerTrafficTest(req, tc, ws, wc)

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

    for cmd in tc.resp.commands:
        api.Logger.info("{}".format(tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)

        if cmd.exit_code != 0:
            result = api.types.status.FAILURE

        if "run_rdma" in tc.cmd_cookies[cookie_idx]:
            get_bw_avg_flag = False

            if tc.w[tc.server_idx].ip_address in tc.cmd_cookies[cookie_idx]:
                # client cmd
                cs_str = 'CLIENT'
            else:
                # server cmd
                cs_str = 'SERVER'

            lines = cmd.stdout.split('\n')
            for line in lines:
                if "BW average" in line:
                    get_bw_avg_flag = True
                    continue

                if get_bw_avg_flag == True:
                    line_attrs = line.split()
                    if len(line_attrs) == 5:
                        api.Logger.info("{} : BW average: {} Gbps".format(cs_str, line_attrs[3]))
                    else:
                        api.Logger.info("Invalid number of line attributes: {}; cannot get BW average".format(len(line_attrs)))
                    break

                '''
                # TODO: get the sqcb and check for retransmissions (if any)
                sqcb=0
                lines = cmd.stdout.split('\n')
                for line in lines:
                    if "local address" in line:
                        line_attrs = line.split()
                        for i in range(0, len(line_attrs)):
                            if(line_attr[i] == "QPN"):
                                sqcb = int(line_attr[i+1], 0)
                                break
                if(sqcb != 0):
                    cmd = 'cd ' + tc.iota_path + ' && ./rdmactl.py --DEVNAME '\
                          + tc.devices[tc.client_idx] + ' --sqcb0 ' + sqcb
                    api.Trigger_AddCommand(req, 
                                           tc.w[tc.client_idx].node_name, 
                                           tc.w[tc.client_idx].workload_name,
                                           cmd)
                    tc.cmd_cookies.append(cmd)
                else:
                    api.Logger.error("Couldnt extract sqcb")

                #TODO: look for p_index1 and c_index1 to be 0
                '''

        # PFC verification cmd
        elif "halctl show port" in tc.cmd_cookies[cookie_idx]:
            # cmd output
            # halctl show port --port 1 statistics
            # FRAMES RX PAUSE          0
            # FRAMES RX PRIPAUSE       194074
            # FRAMES TX PAUSE          0
            # FRAMES TX PRIPAUSE       0
            # FRAMES RX PRI 3          194074
            # FRAMES TX PRI 3          0

            pcp = tc.cmd_cookies[cookie_idx].split()[-1]
            rx_pause_counter = 'FRAMES RX PAUSE'
            tx_pause_counter = 'FRAMES TX PAUSE'
            rx_pfc_counter = 'FRAMES RX PRIPAUSE'
            tx_pfc_counter = 'FRAMES TX PRIPAUSE'

            rx_pfc_class_counter = 'FRAMES RX PRI ' + str(pcp)
            tx_pfc_class_counter = 'FRAMES TX PRI ' + str(pcp)

            lines = cmd.stdout.split('\n')
            for line in lines:
                if rx_pfc_counter in line:
                    line_attrs = line.split()
                    rx_pripause = int(line_attrs[len(line_attrs)-1])
                    if rx_pripause == 0:
                        api.Logger.info("No PFC frames received")
                    else:
                        api.Logger.info("PFC Frames received: {}".format(rx_pripause))
                elif tx_pfc_counter in line:
                    line_attrs = line.split()
                    tx_pripause = int(line_attrs[len(line_attrs)-1])
                    if tx_pripause == 0:
                        api.Logger.info("No PFC frames transmitted")
                    else:
                        api.Logger.info("PFC Frames transmitted: {}".format(tx_pripause))
                elif rx_pfc_class_counter in line:
                    line_attrs = line.split()
                    rx_pri = int(line_attrs[len(line_attrs)-1])
                    if rx_pri == 0:
                        api.Logger.info("No Pri {} PFC frames received".format(pcp))
                    else:
                        api.Logger.info("Pri {} PFC Frames received: {}".format(pcp, rx_pri))
                elif tx_pfc_class_counter in line:
                    line_attrs = line.split()
                    tx_pri = int(line_attrs[len(line_attrs)-1])
                    if tx_pri == 0:
                        api.Logger.info("No Pri {} PFC frames transmitted".format(pcp))
                    else:
                        api.Logger.info("Pri {} PFC Frames transmitted: {}".format(pcp, tx_pri))
                elif rx_pause_counter in line:
                    line_attrs = line.split()
                    rx_pause = int(line_attrs[len(line_attrs)-1])
                    if rx_pause == 0:
                        api.Logger.info("No Link Level PAUSE frames received")
                    else:
                        api.Logger.info("Link Level PAUSE Frames received: {}".format(rx_pause))
                elif tx_pause_counter in line:
                    line_attrs = line.split()
                    tx_pause = int(line_attrs[len(line_attrs)-1])
                    if tx_pause == 0:
                        api.Logger.info("No Link Level PAUSE frames transmitted")
                    else:
                        api.Logger.info("Link Level PAUSE Frames transmitted: {}".format(tx_pause))

        #Check if there are drops
        elif "show drops cmd" in tc.cmd_cookies[cookie_idx]:
            cookie_attrs = tc.cmd_cookies[cookie_idx].split()
            ip_address = cookie_attrs[-1]
            node_name = cookie_attrs[5]
            dev = api.GetTestsuiteAttr(ip_address+"_device")[-1]

            curr_drops = qos.QosGetDropsForDevFromOutput(cmd.stdout, dev)
            prev_drops = qos.QosGetDropsForDevFromTestSuite(dev, node_name)
            
            qos.QosSetDropsForDev(curr_drops, dev, node_name)
            
            if int(curr_drops) > int(prev_drops):
                api.Logger.error("Additional {} drops found on {}".format(curr_drops-prev_drops, node_name))
                return api.types.status.FAILURE
 
        cookie_idx += 1

    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

