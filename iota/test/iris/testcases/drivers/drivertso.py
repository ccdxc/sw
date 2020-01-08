#! /usr/bin/python3
import time
import copy
from scapy.all import rdpcap
import iota.harness.api as api
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

VXLAN_SERVER_IP = "100.1.1.1"
VXLAN_CLIENT_IP = "100.1.1.2"
VXLAN_SERVER_IPV6 = "3000::1"
VXLAN_CLIENT_IPV6 = "3000::2"

# In netstat output for FreeBSD, first one is received bad checksum counter.
NETSTAT_INVALID_CHECKSUM = 'sudo netstat -s | grep "discarded for bad checksums" | cut -c1-3'


def addBSDVxLAN(node, local_ip, remote_ip, vxlan_ip):
    if api.GetNodeOs(node) != host.OS_TYPE_BSD:
        return api.types.status.SUCCESS
    api.Logger.info("Creating VxLAN on %s with local: %s remote: %s VxLAN IP: %s"
                    % (node, local_ip, remote_ip, vxlan_ip))
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, 'sudo ifconfig vxlan0 destroy')
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.info("vxlan0 doesn't exit on %s" % (node))

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan create vxlanid 100 vxlanlocal %s vxlanremote %s'
        % (local_ip, remote_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to create vxlan0 on %s" % (node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to create vxlan0, stderr: %s"
                         % (cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan0 inet6 accept_rtadv')
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan0 %s' % (vxlan_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(
            "Failed to  configure IP %s on vxlan0 on %s" % (vxlan_ip, node))
        return api.types.status.FAILURE

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to  configure IP %s on vxlan0 on %s, stderr: %s"
                         % (vxlan_ip, node, cmd.stderr))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

    time.sleep(2)
    return api.types.status.SUCCESS


def addLinuxVxLAN(node, local_ip, remote_ip, vxlan_ip):
    if api.GetNodeOs(node) != host.OS_TYPE_LINUX:
        return api.types.status.SUCCESS
    api.Logger.info("Creating VxLAN on %s with local: %s remote: %s VxLAN IP: %s"
                    % (node, local_ip, remote_ip, vxlan_ip))
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, 'sudo ip link delele vxlan0')
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.info("vxlan0 doesn't exit on %s" % (node))

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ip link add vxlan0 type vxlan id 100 local %s remote %s dstport 4789'
        % (local_ip, remote_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to create vxlan0 on %s" % (node))
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(
        req, node, 'sudo ifconfig vxlan0 %s' % (vxlan_ip))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(
            "Failed to  configure IP %s on vxlan0 on %s" % (vxlan_ip, node))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def addVxLAN(node, local_ip, remote_ip, vxlan_ip):
    if api.GetNodeOs(node) == host.OS_TYPE_BSD:
        return addBSDVxLAN(node, local_ip, remote_ip, vxlan_ip)

    return addLinuxVxLAN(node, local_ip, remote_ip, vxlan_ip)


def setupVxLAN(ipv4, srv, cli):

    if ipv4:
        srv_addr = srv.ip_address
        cli_addr = cli.ip_address
    else:
        srv_addr = srv.ipv6_address
        cli_addr = cli.ipv6_address

    if ipv4:
        vxlan_ip_str = VXLAN_SERVER_IP
    else:
        vxlan_ip_str = "inet6 " + VXLAN_SERVER_IPV6 + "/120"

    status = addVxLAN(srv.node_name, srv_addr,
                      cli_addr, vxlan_ip_str)
    if status != api.types.status.SUCCESS:
        api.Logger.error("Failed to setup server VxLAN")
        return api.types.status.FAILURE

    if ipv4:
        vxlan_ip_str = VXLAN_CLIENT_IP
    else:
        vxlan_ip_str = "inet6 " + VXLAN_CLIENT_IPV6 + "/120"
    status = addVxLAN(cli.node_name, cli_addr,
                      srv_addr, vxlan_ip_str)
    if status != api.types.status.SUCCESS:
        api.Logger.error("Failed to setup client VxLAN")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


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
                "Failed to start tcpdump for %s on %s, stderr: %s"
                % (intf, node, cmd.stderr))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


def getNetstatBadCsum(node):
    if api.GetNodeOs(node) != host.OS_TYPE_BSD:
        return 0

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    api.Trigger_AddHostCommand(req, node, NETSTAT_INVALID_CHECKSUM)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error(
            "Failed to start netstat on %s" % (node))
        return -1

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            if cmd.stdout is not None:
                api.Logger.error("Received bad checksum counter is non-zero(%s)"
                                 % (cmd.stdout))
                api.PrintCommandResults(cmd)
                return -1

    return int(cmd.stdout)


def VerifyNetStat(tc):
    srv_bad_csum = getNetstatBadCsum(tc.nodes[0])
    # Compare the bad csum counter before and after the test on receiver.
    if tc.srv_bad_csum != srv_bad_csum:
        api.Logger.error("Server got bad checksum packets, before: %d, after: %d"
                         % (tc.srv_bad_csum, srv_bad_csum))
        return api.types.status.FAILURE
    cli_bad_csum = getNetstatBadCsum(tc.nodes[1])
    # Compare the bad csum counter before and after the test on receiver.
    if tc.cli_bad_csum != cli_bad_csum:
        api.Logger.error("Client got bad checksum packets, before: %d, after: %d"
                         % (tc.cli_bad_csum, cli_bad_csum))
        return api.types.status.FAILURE

    api.Logger.info("Bad checksum counter for client: %d, server: %d"
                    % (cli_bad_csum, srv_bad_csum))
    return api.types.status.SUCCESS

# Retransmission count is only for Client.


def VerifyRetrans(tc):
    if tc.resp is None:
        api.Logger.error("Failed to read client iperf output")
        return api.types.status.FAILURE

    cmd = tc.resp.commands[0]
    if cmd.exit_code != 0:
        if cmd.stdout is not None:
            api.Logger.error("Client iperf didn't run, stdout: %s stderr: %s"
                             % (cmd.stdout, cmd.stderr))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    if len(tc.resp.commands) < 2:
        api.Logger.error("Client commands after iperf didn't run")
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE

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
        bw = int(float(tc.resp.commands[2].stdout) / (1024 * 1024))

    api.Logger.info("Client iperf retransmission counter: %d BW: %d Mbps"
                    % (retran, bw))

    return api.types.status.SUCCESS


def VerifyMSS(tc):
    srv_packets = rdpcap(tc.srv_pcap_file)
 #   cli_packets = scapy.rdpcap(tc.cli_pcap_file)

    for p in srv_packets:
        if p.haslayer(scapy.TCP):
            print(p)

    return api.types.status.SUCCESS


def verifySingle(tc):
    status = VerifyNetStat(tc)
    if status != api.types.status.SUCCESS:
        return status

    status = VerifyRetrans(tc)
    if status != api.types.status.SUCCESS:
        return status

 #   status = VerifyMSS(tc)
 #   if status != api.types.status.SUCCESS:
 #       return status
    return api.types.status.SUCCESS


def triggerSingle(tc, srv, cli):
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
    api.Logger.info("proto: %s/%s Pkt size: %d Threads: %d"
                    % (proto, ipproto, pktsize, number_of_iperf_threads))

    tc.srv_bad_csum = getNetstatBadCsum(srv.node_name)
    tc.cli_bad_csum = getNetstatBadCsum(cli.node_name)

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
            api.Logger.error("Failed to start client iperf, stderr: %s"
                             % (cmd.stderr))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return verifySingle(tc)


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

    api.Logger.info("Setting driver features : Success")
    return api.types.status.SUCCESS


def Trigger(tc):
    if api.IsDryrun():
        return api.types.status.SUCCESS

    for pair in tc.workload_pairs:
        srv = pair[0]
        cli = pair[1]
        vxlan = False
        if getattr(tc.iterators, 'vxlan', 'off') == 'on':
            vxlan = True

        if not vxlan:
            status = triggerSingle(tc, srv, cli)
            if status != api.types.status.SUCCESS:
                return status

        else:
            ipproto = getattr(tc.iterators, "ipproto", 'v4')
            ipv4 = True
            if ipproto == 'v6':
                ipv4 = False
            srv_vxlan = copy.deepcopy(srv)
            cli_vxlan = copy.deepcopy(cli)

            status = setupVxLAN(ipv4, srv, cli)
            if status != api.types.status.SUCCESS:
                return status

            # ipproto selects which IP version is used.
            srv_vxlan.interface = "vxlan0@" + srv.interface
            srv_vxlan.ip_address = VXLAN_SERVER_IP
            srv_vxlan.ipv6_address = VXLAN_SERVER_IPV6
            cli_vxlan.interface = "vxlan0@" + cli.interface
            cli_vxlan.ip_address = VXLAN_CLIENT_IP
            cli_vxlan.ipv6_address = VXLAN_CLIENT_IPV6
            status = triggerSingle(tc, srv_vxlan, cli_vxlan)
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
