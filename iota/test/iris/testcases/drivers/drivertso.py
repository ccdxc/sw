#! /usr/bin/python3
import time
import copy
from scapy.all import rdpcap
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.utils.ionic_utils as ionic_utils
import iota.test.utils.ionic_stats as ionic_stats
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.config.netagent.hw_push_config as hw_config

def startTcpDump(node, intf, src_port, filename):

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    # XXX: for testing capture only few packets.
    api.Trigger_AddHostCommand(req, node, 'sudo tcpdump -i %s -c 10 -w %s port %s &'
                               % (intf, filename, src_port))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to start tcpdump for %s on %s" % (intf, node))
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error(
                "Failed to start tcpdump for %s on %s\n"
                % (intf, node))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def VerifyNetStat(tc):
    status = api.types.status.SUCCESS

    proto = getattr(tc.iterators, "proto", 'tcp')
    srv_bad_csum = ionic_stats.getNetstatBadCsum(tc.srv, proto)
    # Compare the bad csum counter before and after the test on receiver.
    if tc.srv_bad_csum != srv_bad_csum:
        api.Logger.error("Server got bad checksum packets, before: %d, after: %d"
                         % (tc.srv_bad_csum, srv_bad_csum))
        status = api.types.status.FAILURE
    cli_bad_csum = ionic_stats.getNetstatBadCsum(tc.cli, proto)
    # Compare the bad csum counter before and after the test on receiver.
    if tc.cli_bad_csum != cli_bad_csum:
        api.Logger.error("Client got bad checksum packets, before: %d, after: %d"
                         % (tc.cli_bad_csum, cli_bad_csum))
        status = api.types.status.FAILURE

    api.Logger.info("Bad checksum counter for client: %d, server: %d"
                    % (cli_bad_csum, srv_bad_csum))
    return status

# Retransmission count is only for Client.
def VerifyRetrans(tc):
    if tc.resp is None:
        api.Logger.error("Failed to read client iperf output")
        return api.types.status.FAILURE, 0, 0

    cmd = tc.resp.commands[0]
    if cmd.exit_code != 0:
        if cmd.stdout is not None:
            api.Logger.error("Client iperf didn't run\n");
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE, 0, 0

    if len(tc.resp.commands) < 2:
        api.Logger.error("Client commands after iperf didn't run")
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE, 0, 0

    # Second client command is to grep for retrans
    cmd = tc.resp.commands[1]
    retran = 0
    proto = getattr(tc.iterators, "proto", 'tcp')
    if cmd.exit_code == 0 and proto == 'tcp' and cmd.stdout:
        retran = int(cmd.stdout)

    cmd = tc.resp.commands[2]
    bw = 0
    if cmd.exit_code == 0 and cmd.stdout:
        bw_str = cmd.stdout
        bw = int(float(cmd.stdout) / (1024 * 1024))

    cmd = tc.resp.commands[3]
    tc.tx_bytes = 0
    if cmd.exit_code == 0 and cmd.stdout:
        tc.tx_bytes = int(cmd.stdout)

    return api.types.status.SUCCESS, retran, bw


def VerifyMSS(tc):
    srv_packets = rdpcap(tc.srv_pcap_file)
 #   cli_packets = scapy.rdpcap(tc.cli_pcap_file)

    for p in srv_packets:
        if p.haslayer(scapy.TCP):
            print(p)

    return api.types.status.SUCCESS


def verifySingle(tc):
# TODO: covered in csum test.
#    status = VerifyNetStat(tc)

    status1, retran, bw = VerifyRetrans(tc)
    if status1 == api.types.status.SUCCESS:
        status = status1
        
 #   status = VerifyMSS(tc)
 #   if status != api.types.status.SUCCESS:
 #       return status
    return status, retran, bw


def runIperfTest(tc, srv, cli):
    if api.IsDryrun():
        return api.types.status.SUCCESS

    req1 = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    req2 = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    proto = getattr(tc.iterators, "proto", 'tcp')

    number_of_iperf_threads = getattr(tc.args, "iperfthreads", 1)

    pktsize = getattr(tc.iterators, "pktsize", 512)
    ipproto = getattr(tc.iterators, "ipproto", 'v4')

    if ipproto == 'v6':
        srv_ip_address = srv.ipv6_address
        tc.cmd_descr = " Server: %s(%s) <--> Client: %s(%s)" %\
            (srv.interface, srv.ipv6_address, cli.interface, cli.ipv6_address)
    else:
        srv_ip_address = srv.ip_address
        tc.cmd_descr = " Server: %s(%s) <--> Client: %s(%s)" %\
            (srv.interface, srv.ip_address, cli.interface, cli.ip_address)

    api.Logger.info("Starting TSO test %s" % (tc.cmd_descr))

    tc.srv_bad_csum = ionic_stats.getNetstatBadCsum(srv, proto)
    tc.cli_bad_csum = ionic_stats.getNetstatBadCsum(cli, proto)

    for i in range(number_of_iperf_threads):
        if proto == 'tcp':
            port = api.AllocateTcpPort()
        else:
            port = api.AllocateUdpPort()

        file_name_suffix = "_instance" + \
            str(i) + proto + "_" + ipproto + "_" + str(pktsize)

        file_name = '/tmp/' + 'srv_' + srv.interface + file_name_suffix
        tc.srv_pcap_file = file_name + '.pcap'
        startTcpDump(srv.node_name, srv.interface, port, tc.srv_pcap_file)
        iperf_server_cmd = cmd_builder.iperf_server_cmd(port=port)
        api.Trigger_AddCommand(
            req1, srv.node_name, srv.workload_name,
            iperf_server_cmd, background=True)

        file_name = '/tmp/' + 'cli_' + cli.interface + file_name_suffix
        tc.cli_pcap_file = file_name + ".pcap"
        iperf_file_name = file_name + ".log"
        startTcpDump(cli.node_name, cli.interface, port, tc.cli_pcap_file)
        iperf_client_cmd = cmd_builder.iperf_client_cmd(server_ip=srv_ip_address,
                                                        port=port,
                                                        proto=proto, pktsize=pktsize,
                                                        ipproto=ipproto)

        # Once iperf JSON support  is available, we don't need this hacks.
        api.Trigger_AddCommand(
            req2, cli.node_name, cli.workload_name, iperf_client_cmd +
            " -J | tee " + iperf_file_name)
        # Read the retransmission counter from the log
        api.Trigger_AddCommand(
            req2, cli.node_name, cli.workload_name,
            'grep retrans ' + iperf_file_name +
            '| tail -1| cut -d ":" -f 2 | cut -d "," -f 1')
        # Read the bandwidth numbers.
        api.Trigger_AddCommand(
            req2, cli.node_name, cli.workload_name,
            'cat ' + iperf_file_name +
            ' | grep bits_per_second | tail -1 |  cut -d ":" -f 2 | cut -d "," -f 1')
        # Read the bytes transferred numbers.
        api.Trigger_AddCommand(
            req2, cli.node_name, cli.workload_name,
            'cat ' + iperf_file_name +
            ' | grep bytes | tail -1 |  cut -d ":" -f 2 | cut -d "," -f 1')

    trig_resp1 = api.Trigger(req1)
    if trig_resp1 is None:
        api.Logger.error("Failed to run iperf server")
        return api.types.status.FAILURE

    tc.resp = api.Trigger(req2)
    if tc.resp is None:
        api.Logger.error("Failed to run iperf client")
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Failed to start client iperf\n");
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    status, retran, bw = verifySingle(tc)
    vlan = getattr(tc.iterators, 'vlantag', 'off')
    vxlan = getattr(tc.iterators, 'vxlan', 'off')
    tso = getattr(tc.iterators, 'tso_offload', 'off')
    
    api.Logger.info("Result TSO: %s VLAN: %s VXLAN: %s Proto: %s/%s Pkt size: %d Threads: %d"
                    " Bandwidth: %d Mbps"
                    % (tso, vlan, vxlan, proto, ipproto, pktsize,
                       number_of_iperf_threads, bw))
    return status

def runTest(tc, srv, cli):
    vlan = getattr(tc.iterators, 'vlantag', 'off')
    vxlan = getattr(tc.iterators, 'vxlan', 'off')
    
    if vlan == 'on' or vxlan == 'on':
        cli_intf = cli.parent_interface
        srv_intf = srv.parent_interface
    else:
        cli_intf = cli.interface
        srv_intf = srv.interface

    lro = getattr(tc.iterators, 'lro_offload', 'off')
    mtu = getattr(tc.iterators, 'mtu', '1500')

    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    if ipproto == 'v4':
        cli_before_tso_stats = ionic_stats.getTSOIPv4Stats(cli, cli_intf)
    else:
        cli_before_tso_stats = ionic_stats.getTSOIPv6Stats(cli, cli_intf)

    srv_before_lro_stats = ionic_stats.getLROStats(cli, cli_intf)

    status = runIperfTest(tc, srv, cli)
    if status != api.types.status.SUCCESS:
        return status
    
    srv_after_lro_stats = ionic_stats.getLROStats(cli, cli_intf)
    pkt_count = tc.tx_bytes / int(mtu)

    if ipproto == 'v4':
        cli_after_tso_stats = ionic_stats.getTSOIPv4Stats(cli, cli_intf)
    else:
        cli_after_tso_stats = ionic_stats.getTSOIPv6Stats(cli, cli_intf)
        
    api.Logger.info("TSO Client stats ip: %s pkts: %d before: %s after: %s"
                     % (ipproto, pkt_count, str(cli_before_tso_stats), str(cli_after_tso_stats)))

    tso = getattr(tc.iterators, 'tso_offload', 'off')
#    if cli_before_tso_stats == cli_after_tso_stats and tso == 'on':
#        api.Logger.error("For TSO ON, counters are same, before: %s after: %s"
#                         %(str(cli_before_tso_stats), str(cli_after_tso_stats)))
#        return api.types.status.FAILURE
    
    if cli_before_tso_stats != cli_after_tso_stats and tso == 'off':
        api.Logger.error("For TSO OFF, counters are not matching, before: %s after: %s"
                         %(str(cli_before_tso_stats), str(cli_after_tso_stats)))
        return api.types.status.FAILURE

    #
    # Validate LRO if receiver is Naples card.
    #
    if tc.is_srv_naples and lro == 'on' and api.GetNodeOs(tc.srv.node_name) == host.OS_TYPE_BSD:
        api.Logger.info("For LRO, counters before: %s after: %s"
                         %(str(srv_before_lro_stats), str(srv_after_lro_stats)))
        if srv_before_lro_stats == srv_after_lro_stats:
            api.Logger.error("For LRO, counters are same, before: %s after: %s"
                         %(str(srv_before_lro_stats), str(srv_after_lro_stats)))
            return api.types.status.FAILURE
        
        #
        # Packet count reported by iperf is bytes/MTU should match
        # the LRO packets queued.
        status = api.types.status.FAILURE
        for pkt in srv_after_lro_stats:
            if pkt_count >= pkt:
                status = api.types.status.SUCCESS
                break
 
        if status != api.types.status.SUCCESS:
            api.Logger.error("For LRO, pkt count: %d doesn't match any of queue stats: %s"
                            %(pkt_count, str(srv_after_lro_stats)))
            return api.types.status.FAILURE

    return status


def getRemoteWorkloadPairs(tc):
    pairs = []
    wpairs = api.GetRemoteWorkloadPairs()
    vlan = False
    if getattr(tc.iterators, 'vlantag', 'off') == 'on':
        vlan = True

    for w1, w2 in wpairs:
        if vlan:
            if w1.encap_vlan != 0 and w1.encap_vlan == w2.encap_vlan:
                pairs.append((w1, w2))
        else:
            # Need only one host interface
            if w1.encap_vlan == 0 and w2.encap_vlan == 0:
                pairs.append((w1, w2))

        # We need only one combinations.
        if (len(pairs)) == 1:
            break

    return pairs


def Setup(tc):
    if api.IsDryrun():
        return api.types.status.SUCCESS

    tc.nodes = api.GetWorkloadNodeHostnames()

    tc.workload_pairs = getRemoteWorkloadPairs(tc)

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        tc.skip = True
        return api.types.status.FAILURE

    for pair in tc.workload_pairs:
        srv = pair[0]
        cli = pair[1]
        tc.workloads = [srv, cli]
        api.Logger.info("Setting up interfaces %s(%s) --> %s(%s)" %
                        (srv.workload_name, srv.ip_address, cli.workload_name, cli.ip_address))
        if common.setup_features(tc) != api.types.status.SUCCESS:
            api.Logger.info("Setting driver features :Failed")
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Trigger(tc):
    if api.IsDryrun():
        return api.types.status.SUCCESS

    for pair in tc.workload_pairs:
        srv = pair[0]
        cli = pair[1]
        if not cli.IsNaples():
            cli = pair[0]
            srv = pair[1]
            
        tc.srv = srv
        tc.cli = cli

        tc.is_srv_naples = False
        if srv.IsNaples():
            tc.is_srv_naples = True

        vxlan = False
        if getattr(tc.iterators, 'vxlan', 'off') == 'on':
            vxlan = True

        if not vxlan:
            status = runTest(tc, srv, cli)
            if status != api.types.status.SUCCESS:
                return status

        else:
            ipproto = getattr(tc.iterators, "ipproto", 'v4')
            ipv4 = True
            if ipproto == 'v6':
                ipv4 = False
            srv_vxlan = copy.deepcopy(srv)
            cli_vxlan = copy.deepcopy(cli)

            status = ionic_utils.setupVxLAN(ipv4, srv, cli)
            if status != api.types.status.SUCCESS:
                return status

            # ipproto selects which IP version is used.
            srv_vxlan.interface = "vxlan0@" + srv.interface
            srv_vxlan.ip_address = ionic_utils.VXLAN_SERVER_IP
            srv_vxlan.ipv6_address = ionic_utils.VXLAN_SERVER_IPV6
            cli_vxlan.interface = "vxlan0@" + cli.interface
            cli_vxlan.ip_address = ionic_utils.VXLAN_CLIENT_IP
            cli_vxlan.ipv6_address = ionic_utils.VXLAN_CLIENT_IPV6
            tc.srv = srv_vxlan
            tc.cli = cli_vxlan
            status = runTest(tc, srv_vxlan, cli_vxlan)
            if status != api.types.status.SUCCESS:
                return status

    return api.types.status.SUCCESS


def Verify(tc):
    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
