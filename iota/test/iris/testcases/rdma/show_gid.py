#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.iris.testcases.qos.qos_utils as qos
import time

def Setup(tc):
    tc.iota_path = api.GetTestsuiteAttr("driver_path")
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()

    tc.vlan_idx = -1
    for i in range(len(pairs)):
        if tc.vlan_idx > -1:
            break
        for j in range(len(pairs[0])):
            if pairs[i][j].encap_vlan != 0:
                tc.vlan_idx = i
                break

    if tc.vlan_idx < 1:
        return api.types.status.FAILURE

    tc.w1 = pairs[0][0]
    tc.w2 = pairs[0][1]
    tc.vlan_w1 = pairs[tc.vlan_idx][0]
    tc.vlan_w2 = pairs[tc.vlan_idx][1]
    tc.cmd_cookies = []

    api.SetTestsuiteAttr("vlan_idx", tc.vlan_idx)

    #==============================================================
    # get the device and GID
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Extracting device and GID using show_gid")
    api.Logger.info("Interfaces are {0} {1}".format(tc.w1.interface, tc.w2.interface))

    # sleep for 10 secs to ensure that show_gid is returning gids on naples
    cmd = 'sleep 10'
    api.Trigger_AddCommand(req,
                           tc.w1.node_name,
                           tc.w1.workload_name,
                           cmd)
    tc.cmd_cookies.append(cmd)

    cmd = "show_gid | grep %s | grep v2" % tc.w1.ip_address
    api.Trigger_AddCommand(req,
                           tc.w1.node_name,
                           tc.w1.workload_name,
                           tc.iota_path + cmd,
                           timeout=60)
    tc.cmd_cookies.append(cmd)

    cmd = "show_gid | grep %s | grep v2" % tc.w2.ip_address
    api.Trigger_AddCommand(req,
                           tc.w2.node_name,
                           tc.w2.workload_name,
                           tc.iota_path + cmd,
                           timeout=60)
    tc.cmd_cookies.append(cmd)

    cmd = "show_gid | grep %s | grep v2" % tc.vlan_w1.ip_address
    api.Trigger_AddCommand(req,
                           tc.vlan_w1.node_name,
                           tc.vlan_w1.workload_name,
                           tc.iota_path + cmd,
                           timeout=60)
    tc.cmd_cookies.append(cmd)

    cmd = "show_gid | grep %s | grep v2" % tc.vlan_w2.ip_address
    api.Trigger_AddCommand(req,
                           tc.vlan_w2.node_name,
                           tc.vlan_w2.workload_name,
                           tc.iota_path + cmd,
                           timeout=60)
    tc.cmd_cookies.append(cmd)

    #Show drops command for QOS testing
    cmd = '/nic/bin/halctl show system statistics drop | grep -i "occupancy"'
    if tc.w1.IsNaples():
        api.Logger.info("Running show drops command {} on node_name {}"\
                        .format(cmd, tc.w1.node_name))

        api.Trigger_AddNaplesCommand(req, 
                                    tc.w1.node_name, 
                                    cmd)
        tc.cmd_cookies.append("show drops cmd for node {} ip_address {}".format(tc.w1.node_name, tc.w1.ip_address))

    if tc.w2.IsNaples():
        api.Logger.info("Running show drops command {} on node_name {}"\
                        .format(cmd, tc.w2.node_name))

        api.Trigger_AddNaplesCommand(req, 
                                    tc.w2.node_name, 
                                    cmd)
        tc.cmd_cookies.append("show drops cmd for node {} ip_address {}".format(tc.w2.node_name, tc.w2.ip_address))

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    api.Logger.info("show_gid results")

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            return api.types.status.FAILURE

    #set the path for testcases in this testsuite to use
    w = [tc.w1, tc.w2, tc.vlan_w1, tc.vlan_w2]
    for i in range(len(w)):
        if api.IsDryrun():
            api.SetTestsuiteAttr(w[i].ip_address+"_device", '0')
        else:
            api.SetTestsuiteAttr(w[i].ip_address+"_device", rdma.GetWorkloadDevice(tc.resp.commands[i+1].stdout))
        if api.IsDryrun():
            api.SetTestsuiteAttr(w[i].ip_address+"_gid", '0')
        else:
            api.SetTestsuiteAttr(w[i].ip_address+"_gid", rdma.GetWorkloadGID(tc.resp.commands[i+1].stdout))

    cookie_idx = 0
    for cmd in tc.resp.commands:
        if "show drops cmd" in tc.cmd_cookies[cookie_idx]:
            cookie_attrs = tc.cmd_cookies[cookie_idx].split()
            ip_address = cookie_attrs[-1]
            node_name = cookie_attrs[5]
            dev = api.GetTestsuiteAttr(ip_address+"_device")[-1]
            curr_drops = qos.QosGetDropsForDevFromOutput(cmd.stdout, dev)
            qos.QosSetDropsForDev(curr_drops, dev, node_name)

        cookie_idx += 1

    return api.types.status.SUCCESS

def Teardown(tc):

    return api.types.status.SUCCESS
