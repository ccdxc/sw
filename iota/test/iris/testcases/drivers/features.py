#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.utils.iperf as iperf
import iota.test.utils.naples_host as host
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.interface as interface
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.testcases.drivers.verify as verify
import iota.test.iris.utils.naples_workloads as workloads


INTF_TEST_TYPE_OOB_1G       = "oob-1g"
INTF_TEST_TYPE_IB_100G      = "inb-100g"
INTF_TEST_TYPE_INT_MGMT     = "int-mgmt"
INTF_TEST_TYPE_HOST         = "host"

taggedWorkload = False

def getIntMgmtWorkloadPairs():
    pairs = []
    host_wls = workloads.GetIntMgmtHostWorkloads()
    naples_wls = workloads.GetIntMgmtNaplestWorkloads()
    for srv in host_wls:
        for cli in naples_wls:
            if srv.node_name == cli.node_name:
                pairs.append((srv,cli))
    return pairs

def getRemoteWorkloadPairs():
    pairs = []
    wpairs = api.GetRemoteWorkloadPairs()
    for srv, cli in wpairs:
        if taggedWorkload:
            if srv.encap_vlan != 0 and srv.encap_vlan == cli.encap_vlan:
                pairs.append((srv,cli))
        else:
            if srv.encap_vlan == 0 and cli.encap_vlan == 0:
                pairs.append((srv,cli))

    return pairs

workloads_pairs_map = {
    INTF_TEST_TYPE_HOST   : getRemoteWorkloadPairs(),
    INTF_TEST_TYPE_OOB_1G : workloads.GetOobMgmtRemoteWorkloadPairs(),
    INTF_TEST_TYPE_IB_100G : workloads.GetInbandMgmtRemoteWorkloadPairs(),
    INTF_TEST_TYPE_INT_MGMT : getIntMgmtWorkloadPairs(),
}

def _get_workloads(tc):
    test_type = getattr(tc.args, "test-type", INTF_TEST_TYPE_HOST)
    global taggedWorkload
    if getattr(tc.iterators, 'vlantag', 'off') == 'on':
        taggedWorkload = True
    else:
        taggedWorkload = False

    pairs = workloads_pairs_map[test_type]
    return pairs[0][0], pairs[0][1]

def Setup(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS
    tc.nodes = api.GetWorkloadNodeHostnames()
    tc.node_intfs = {}
    srv,cli = _get_workloads(tc)
    tc.workloads = [srv, cli]
    #for node in tc.nodes:
    #    tc.node_intfs[node] = interface.GetNodeInterface(node)

    if getattr(tc.args, 'restart', False):
        ret = api.RestartNodes(tc.nodes)
        if ret != api.types.status.SUCCESS:
            api.Logger.error("Node restart failed")
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
    if api.IsDryrun(): return api.types.status.SUCCESS

    srv = tc.workloads[0]
    cli = tc.workloads[1]
    
    # Determine where the commands will be run - host or Naples.
    test_type = getattr(tc.args, "test-type", INTF_TEST_TYPE_HOST)
    is_naples_cmd = True
    if test_type == INTF_TEST_TYPE_HOST:
        is_naples_cmd = False

    srv_req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    cli_req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    proto = getattr(tc.iterators, "proto", 'tcp')
    number_of_iperf_threads = getattr(tc.args, "iperfthreads", 1)
    pktsize = getattr(tc.iterators, "pktsize", None)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')

    if ipproto == 'v4':
        server_ip = srv.ip_address
        client_ip = cli.ip_address
    else:
        server_ip = srv.ipv6_address
        client_ip = cli.ipv6_address
        
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (srv.interface, server_ip, cli.interface, client_ip)

    api.Logger.info("Starting Iperf(%s/%s) test from %s"
                    % (proto, ipproto, tc.cmd_descr))

    duration =  10
    for i in range(number_of_iperf_threads):
        if proto == 'tcp':
            port = api.AllocateTcpPort()
        else:
            port = api.AllocateUdpPort()
 
        iperf_server_cmd = iperf.ServerCmd(port, naples = is_naples_cmd)
        api.Trigger_AddCommand(srv_req, srv.node_name, srv.workload_name, iperf_server_cmd, background = True)

        iperf_client_cmd = iperf.ClientCmd(server_ip, port, time=duration,
                                 proto=proto, jsonOut=True, ipproto=ipproto,
                                 pktsize=pktsize, client_ip=client_ip, naples = is_naples_cmd)
        api.Trigger_AddCommand(cli_req, cli.node_name, cli.workload_name, iperf_client_cmd)

    srv_resp = api.Trigger(srv_req)
    # Wait for iperf server to start.
    time.sleep(10)
    tc.cli_resp = api.Trigger(cli_req)

    srv_resp1 = api.Trigger_TerminateAllCommands(srv_resp)

    return api.types.status.SUCCESS


def Verify(tc):
    if api.IsDryrun(): return api.types.status.SUCCESS
    if getattr(tc.args, 'capture_pcap', False):
        ret = common.stop_pcap_capture(tc)
        if ret != api.types.status.SUCCESS:
            api.Logger.info("pcap caputre failed")
            return ret

    if tc.cli_resp is None:
        return api.types.status.FAILURE

    for cmd in tc.cli_resp.commands:

        if cmd.exit_code != 0:
            api.Logger.error("Iperf client exited with error")
            api.PrintCommandResults(cmd)
            if iperf.ConnectionTimedout(cmd.stdout):
                api.Logger.error("Connection timeout, ignoring for now")
                continue
            if iperf.ControlSocketClosed(cmd.stdout):
                api.Logger.error("Control socket cloned, ignoring for now")
                continue
            if iperf.ServerTerminated(cmd.stdout):
                api.Logger.error("Iperf server terminated")
                return api.types.status.FAILURE
            if not iperf.Success(cmd.stdout):
                api.Logger.error("Iperf failed", iperf.Error(cmd.stdout))
                return api.types.status.FAILURE
        elif not api.GlobalOptions.dryrun:
            api.Logger.info("Iperf Send Rate in Gbps ", iperf.GetSentGbps(cmd.stdout))
            api.Logger.info("Iperf Receive Rate in Gbps ", iperf.GetReceivedGbps(cmd.stdout))

    api.Logger.info("iperf test successfull")

    return verify.driver_feature_verify(tc)

def Teardown(tc):
    #interface.RestoreIntMmgmtInterfaceConfig()
    return api.types.status.SUCCESS
