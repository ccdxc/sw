#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.config.add_routes as add_routes

workloads = {}

def __getOperations(tc_operation):
    opers = list()
    if tc_operation is None:
        return opers
    else:
        opers = list(map(lambda x:x.capitalize(), tc_operation))
    return opers

def verify_dhcp_ips():
    api.Logger.info("Verifying the IP addresses acquired")
    if not api.IsSimulation():
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for workload in workloads.keys():
        cmd = 'ifconfig ' + workload.interface
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
    resp = api.Trigger(req)

    for workload, cmd in zip(workloads.keys(), resp.commands):
        if workload.ip_address not in cmd.stdout:
            api.Logger.error( "DHCP didn't fetch expected address, expected: %s", workload.ipaddress)
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS
 
def acquire_dhcp_ips(workload_pairs):
    global workloads

    workloads.clear()
    for pair in workload_pairs:
        workloads[ pair[0] ] = True
        workloads[ pair[1] ] = True

    if not api.IsSimulation():
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for workload in workloads.keys():
        api.Logger.info("DHCP: %s %s %s" % (workload.node_name, workload.workload_name,
                                            workload.interface))
        cmd = 'dhclient -r ' + workload.interface
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
        cmd = 'ifconfig ' + workload.interface + ' 0.0.0.0'
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
        cmd = 'ifconfig ' + workload.interface
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
        cmd = 'dhclient ' + workload.interface
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)

    resp = api.Trigger(req)

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.info("Couldn't reacquire IP addresses over DHCP")
            api.PrintCommandResults(cmd)
            return

        if 'ifconfig' in cmd.command and '0.0.0.0' not in cmd.command and 'inet' in cmd.stdout:
            api.Logger.info("Couldn't clear static IP address")
            api.PrintCommandResults(cmd)
            return

    add_routes.AddRoutes()
    return

def Setup(tc):
    tc.opers = []
    if hasattr(tc.iterators, 'oper'):
        tc.opers = __getOperations(tc.iterators.oper)
        tc.selected_objs = config_api.SetupConfigObjects(tc.iterators.objtype)

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

    return api.types.status.SUCCESS
 
def Trigger(tc):
    tc.is_config_deleted = False
    tc.is_config_updated = False

    for op in tc.opers:
        tc.is_config_deleted = True if op == 'Delete' else False
        tc.is_config_updated = True if op == 'Update' else False
        res = config_api.ProcessObjectsByOperation(op, tc.selected_objs)
        if res != api.types.status.SUCCESS:
            break;

    tc.ipconfig = getattr(tc.args, "ipconfig", None)
    if tc.iterators.ipconfig == 'dhcp':
        acquire_dhcp_ips(tc.workload_pairs)

    for pair in tc.workload_pairs:
        api.Logger.info("%s between %s and %s" % (tc.iterators.proto, pair[0].ip_address, pair[1].ip_address))

    tc.cmd_cookies, tc.resp = conn_utils.TriggerConnectivityTest(tc.workload_pairs, tc.iterators.proto,
                                                                 tc.iterators.ipaf, tc.iterators.pktsize,
                                                                 tc.sec_ip_test_type)
    return api.types.status.SUCCESS

def Verify(tc):
    if verify_dhcp_ips() != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    if conn_utils.VerifyConnectivityTest(tc.iterators.proto, tc.cmd_cookies, tc.resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    if tc.iterators.workload_type == "igw":
        return flow_utils.verifyFlows(tc.iterators.ipaf, tc.workload_pairs)

def Teardown(tc):
    if tc.is_config_updated:
        rs = config_api.RestoreObjects('Update', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"Teardown failed to restore objs from Update operation: {rs}")
    if tc.is_config_deleted:
        rs = config_api.RestoreObjects('Delete', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"Teardown failed to restore objs from Delete operation: {rs}")
    return flow_utils.clearFlowTable(tc.workload_pairs)
