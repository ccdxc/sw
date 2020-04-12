#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.config.add_routes as add_routes

def __get_latest_workload_address(wl):
    if not api.IsSimulation():
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, "ifconfig " + wl.interface)
    resp = api.Trigger(req)
    ifconfig_regexp = "inet (\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})"
    x = re.findall(ifconfig_regexp, resp.commands[0].stdout)
    if len(x) > 0:
        api.Logger.info("Read management IP %s %s %s %s" % (wl.node_name, wl.workload_name, wl.interface, x[0]))
        return x[0]
    return "0.0.0.0"

def acquire_dhcp_ips(workload_pairs):
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        api.Logger.error("acquire_dhcp_ips")
        # Remove the IP addresses if already acquired
        if not api.IsSimulation():
            req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        else:
            req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        dhcp1_cmd = 'dhclient -r ' + w1.interface
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, dhcp1_cmd)
        dhcp2_cmd = 'dhclient -r ' + w2.interface
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, dhcp2_cmd)

        dhcp1_cmd = 'ifconfig ' + w1.interface + ' 0.0.0.0'
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, dhcp1_cmd)
        dhcp2_cmd = 'ifconfig ' + w2.interface + ' 0.0.0.0'
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, dhcp2_cmd)

        # Get the IP addresses
        dhcp1_cmd = 'dhclient ' + w1.interface
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, dhcp1_cmd)
        dhcp2_cmd = 'dhclient ' + w2.interface
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, dhcp2_cmd)
        resp = api.Trigger(req)

    add_routes.AddRoutes()
    return

def Setup(tc):
    tc.sec_ip_test_type = getattr(tc.args, "use-sec-ip", 'none')
    if tc.sec_ip_test_type not in ['all', 'random', 'none']:
        api.Logger.error("Invalid value for use-sec-ip %s" %(tc.sec_ip_test_type))
        return api.types.status.FAILURE

    tc.workload_pairs = config_api.GetWorkloadPairs(conn_utils.GetWorkloadType(tc.iterators),
            conn_utils.GetWorkloadScope(tc.iterators))
    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        if tc.iterators.workload_type == 'local' and tc.iterators.workload_scope == 'intra-subnet':
            # Currently we dont support local-to-local intra-subnet connectivity
            return api.types.status.SUCCESS
        return api.types.status.FAILURE

    tc.ipconfig = getattr(tc.args, "ipconfig", None)
    if tc.iterators.ipconfig == 'dhcp':
        acquire_dhcp_ips(tc.workload_pairs)

    return api.types.status.SUCCESS

def Trigger(tc):
    for pair in tc.workload_pairs:
        api.Logger.info("%s between %s and %s" % (tc.iterators.proto, pair[0].ip_address, pair[1].ip_address))

    tc.cmd_cookies, tc.resp = conn_utils.TriggerConnectivityTest(tc.workload_pairs, tc.iterators.proto, tc.iterators.ipaf, tc.iterators.pktsize, tc.sec_ip_test_type)
    return api.types.status.SUCCESS

def Verify(tc):
    if conn_utils.VerifyConnectivityTest(tc.iterators.proto, tc.cmd_cookies, tc.resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    if tc.iterators.workload_type == "igw":
        return flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)

    return api.types.status.SUCCESS

def Teardown(tc):
    return flow_utils.clearFlowTable(tc.workload_pairs)
