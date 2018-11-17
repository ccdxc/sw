#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify

DRIVER_TEST_TYPE_OOB_1G_INTF       = "oob-1g"
DRIVER_TEST_TYPE_IB_100G_INTF      = "ib-100g"
DRIVER_TEST_TYPE_INT_MGMT_INTF     = "int-mgmt"
DRIVER_TEST_TYPE_HOST_INTF         = "host"

ip_prefix = 24

ip_map =  {
    DRIVER_TEST_TYPE_HOST_INTF     : ("1.2.2.2", "1.2.2.3"),
    DRIVER_TEST_TYPE_OOB_1G_INTF   : ("2.2.2.2", "2.2.2.3"),
    DRIVER_TEST_TYPE_INT_MGMT_INTF : ("2.2.2.2", "2.2.2.3"),
    DRIVER_TEST_TYPE_IB_100G_INTF  : ("2.2.2.2", "2.2.2.3"),
}

def __configure_interfaces(tc, tc_type):
    ip1 = ip_map[tc_type][0]
    ip2 = ip_map[tc_type][1]
    ret = tc.intf1.ConfigureInterface(ip1, ip_prefix)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Configure interface failed")
        return api.types.status.FAILURE
    ret = tc.intf2.ConfigureInterface(ip2, ip_prefix)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Configure interface failed")
        return api.types.status.FAILURE
    tc.intf1.SetIP(ip1)
    tc.intf2.SetIP(ip2)

    return api.types.status.SUCCESS

def __setup_host_interface_test(tc):
    host_intfs = tc.node_intfs[tc.nodes[0]].HostIntfs()
    host_intfs1 = tc.node_intfs[tc.nodes[1]].HostIntfs()
    tc.intf1 = host_intfs[0]
    tc.intf2 = host_intfs1[0]
    return __configure_interfaces(tc, DRIVER_TEST_TYPE_HOST_INTF)

def __setup_int_mgmt_interface_test(tc):
    host_intfs = tc.node_intfs[tc.nodes[0]].HostIntIntfs()
    naples_intfs = tc.node_intfs[tc.nodes[0]].NaplesIntMgmtIntfs()
    tc.intf1 = host_intfs[0]
    tc.intf2 = naples_intfs[1]
    return __configure_interfaces(tc, DRIVER_TEST_TYPE_INT_MGMT_INTF)

def __setup_oob_1g_interface_test(tc):
    intfs = tc.node_intfs[tc.nodes[0]].Oob1GIntfs()
    intfs1 = tc.node_intfs[tc.nodes[1]].Oob1GIntfs()
    tc.intf1 = intfs[0]
    tc.intf2 = intfs1[0]
    return __configure_interfaces(tc, DRIVER_TEST_TYPE_OOB_1G_INTF)

def __setup_inb_100g_inteface_test(tc):
    intfs = tc.node_intfs[tc.nodes[0]].Inb100GIntfs()
    intfs1 = tc.node_intfs[tc.nodes[1]].Inb100GIntfs()
    tc.intf1 = intfs[0]
    tc.intf2 = intfs1[1]
    return __configure_interfaces(tc, DRIVER_TEST_TYPE_OOB_1G_INTF)

def __set_interfaces(tc, test_type):
    if test_type == DRIVER_TEST_TYPE_HOST_INTF:
        ret = __setup_host_interface_test(tc)
    elif test_type == DRIVER_TEST_TYPE_INT_MGMT_INTF:
        ret = __setup_int_mgmt_interface_test(tc)
    elif test_type == DRIVER_TEST_TYPE_OOB_1G_INTF:
        ret = __setup_oob_1g_interface_test(tc)
    elif test_type == DRIVER_TEST_TYPE_IB_100G_INTF:
        ret = __setup_inb_100g_inteface_test(tc)
    else:
        api.Logger.error("Invalid test type : ", test_type)
        return api.types.status.FAILURE

    tc.test_intfs = [tc.intf1, tc.intf2]

    return ret

def Setup(tc):
    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.node_intfs = {}
    for node in tc.nodes:
        tc.node_intfs[node] = common.GetNodeInterface(node)

    test_type = getattr(tc.args, "test-type", DRIVER_TEST_TYPE_HOST_INTF)
    ret = __set_interfaces(tc, test_type)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Set interface failed")
        return api.types.status.FAILURE

    api.Logger.info("Setting driver features")
    if common.setup_features(tc) != api.types.status.SUCCESS:
        api.Logger.info("Setting driver features :Failed")
        return api.types.status.FAILURE

    api.Logger.info("Setting driver features : Success")

    if getattr(tc.args, 'capture_pcap', False):
        if common.start_pcap_capture(tc) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Trigger(tc):

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (tc.intf1.Name(), tc.intf1.GetIP(), tc.intf2.Name(), tc.intf2.GetIP())
    api.Logger.info("Starting Iperf test from %s" % (tc.cmd_descr))

    iperf_server_cmd = cmd_builder.iperf_server_cmd()
    tc.intf1.AddCommand(req, iperf_server_cmd, background = True)
    proto = getattr(tc.iterators, "proto", 'tcp')
    pktsize = getattr(tc.iterators, "pktsize", 512)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    iperf_client_cmd = cmd_builder.iperf_client_cmd(server_ip = tc.intf1.GetIP(),
                            proto=proto, pktsize=pktsize, ipproto=ipproto)
    tc.intf2.AddCommand(req, iperf_client_cmd)


    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS


def Verify(tc):
    if getattr(tc.args, 'capture_pcap', False):
        ret = common.stop_pcap_capture(tc)
        if ret != api.types.status.SUCCESS:
            api.Logger.info("pcap caputre failed")
            return ret

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    return verify.driver_feature_verify(tc)

def Teardown(tc):
    return api.types.status.SUCCESS
