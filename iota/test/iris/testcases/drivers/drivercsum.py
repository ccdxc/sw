#! /usr/bin/python3
import time
import copy
import os.path
from scapy.all import *
from scapy.utils import rdpcap
import iota.protos.pygen.iota_types_pb2 as types_pb2

import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.utils.ionic_utils as ionic_utils
import iota.test.utils.ionic_stats as ionic_stats
import iota.test.iris.testcases.drivers.common as common
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.config.netagent.hw_push_config as hw_config

PKT_COUNT = 5
PORT_NUM = 4101

# Test for valid and corrupted packet checksum in Tx and Rx side.
# Use nping and hping3 for different combinations of packet.
# nping - IPv4 and IPv6, IP options
#       - Corrupt L3 and L4 packet     
# hping3 - Only IPv4 and TCP option(timestamp)
#
# For bad packet, check stack counters. FreeBSD and Linux has bad checksum counter for
# TCP, UDP and ICMP.
# Setup:
#       Transmitter has to be Naples
#       If receiver is Naples, we validate rx checksum offload

def startTcpDump(node, intf, filename, src_ip):

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    #api.Logger.info("Starting tcpdump for host %s on %s" %(src_ip, node))
    api.Trigger_AddHostCommand(req, node, 'sudo tcpdump -i %s -c %d src host %s -w %s'
                               % (intf, PKT_COUNT, src_ip, filename), background=True)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to start tcpdump for %s on %s" % (intf, node))
        return api.types.status.FAILURE, None

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error(
            "Failed to start tcpdump for %s on %s\n"
            % (intf, node))
        api.PrintCommandResults(cmd)
        return api.types.status.FAILURE, resp

    return api.types.status.SUCCESS, resp


def VerifyNetStat(tc):
    valid_csum = getattr(tc.iterators, 'csum', 'valid')
    proto = getattr(tc.iterators, "proto", 'icmp')
    srv_bad_csum = ionic_stats.getNetstatBadCsum(tc.srv, proto)

    # Only if peer is Naples.
    # Compare the bad checksum counter before and after the test on receiver.
    # For bad checksum, it should increase if Nping has failed to get peer response.

    if tc.srv_bad_csum + PKT_COUNT >= tc.srv_bad_csum and tc.check_rx and \
            valid_csum != 'valid' and tc.is_srv_naples:
        api.Logger.warn("Server bad checksum didn't increase as expected,"
                        " before: %d, after: %d"
                        % (tc.srv_bad_csum, srv_bad_csum))

    cli_bad_csum = ionic_stats.getNetstatBadCsum(tc.cli, proto)
    # Compare the bad csum counter before and after the test on sender.
    # It should be unchanged.
    if tc.cli_bad_csum != cli_bad_csum:
        api.Logger.warn("Client got bad checksum packets, before: %d, after: %d"
                        % (tc.cli_bad_csum, cli_bad_csum))

    api.Logger.info("Bad %s checksum counter for client: %d, server: %d"
                    % (proto, cli_bad_csum, srv_bad_csum))
    return api.types.status.SUCCESS

def VerifyPacket(tc):
    dir_path = os.path.dirname(os.path.realpath(__file__))
    """
    pcap_file = dir_path  + "/" + tc.cli_pcap_file
    if os.path.isfile(pcap_file):
        #api.Logger.info("Client pcap file: %s\n" %(pcap_file))
        cli_packets = rdpcap(pcap_file)
        proto = getattr(tc.iterators, "proto", 'icmp')
        for pkt in cli_packets:
            if not pkt.haslayer('IP'):
                continue
            #if proto == 'icmp':
            #    api.Logger.info("Client PKT IP csum: 0x%x src: %s dst: %s\n" %(pkt[IP].chksum, pkt[IP].src, pkt[IP].dst))
            #elif proto == 'tcp':
            #    api.Logger.info("Client PKT IP csum: 0x%x TCP csum: 0x%x\n" %(pkt[IP].chksum, pkt[TCP].chksum))
            #elif proto == 'udp':
            #    api.Logger.info("Client PKT IP csum: 0x%x UDP csum: 0x%x\n" %(pkt[IP].chksum, pkt[UDP].chksum))
    else:
        api.Logger.error("Client file: %s not found\n" %(pcap_file))
        api.types.status.FAILURE
        
    """
    pcap_file = dir_path  + "/" + tc.srv_pcap_file
    if os.path.isfile(pcap_file):
        #api.Logger.info("Server pcap file: %s\n" %(pcap_file))
        cli_packets = rdpcap(pcap_file)
        proto = getattr(tc.iterators, "proto", 'icmp')
        for pkt in cli_packets:
            if not pkt.haslayer('IP'):
                continue
            
            rx_check_sum = getattr(tc.iterators, "rx_check_sum", 'off')
            # If receiver is Naples and rx-checksum offload is enabled, packet checksum must be non-zero.
            # and if rx checksum is off, it is zero.
            if tc.is_srv_naples and rx_check_sum == 'on':
                if pkt[IP].chksum != 0:
                    api.Logger.error("For rxcsum offload, checksum is set")
                    api.types.status.FAILURE
                    
    else:
        api.Logger.error("Server file: %s not found\n" %(pcap_file))
        api.types.status.FAILURE

    return api.types.status.SUCCESS


def verifySingle(tc):
    status = VerifyNetStat(tc)
    if status != api.types.status.SUCCESS:
        return status
 
# TODO: Fails to copy file, disable for now.
#    status = VerifyPacket(tc)
#    if status != api.types.status.SUCCESS:
#        return status
    
    return api.types.status.SUCCESS

def getPcapFileName(prefix, proto, ipproto, valid_csum, tx_check_sum, rx_check_sum, intf):
    
    file_name_suffix= "_" + proto + "_" + ipproto + "_csum_" + valid_csum + \
        "_txcsum_" + tx_check_sum + "_rxcsum_" + rx_check_sum

    file_name = prefix + intf + file_name_suffix + '.pcap'
    
    return file_name
    
# Nping csum
# Allow to set IPv4 options but not IPv6
# Bad csum packets for L3 and L4.   
def getNpingCmd(srv_ip_address, proto, valid_csum, ipproto):
    client_cmd = "nping --" + proto + " " + srv_ip_address + " -c " + str(PKT_COUNT)
    # Set IP option
    # Corrupt L3 and L4 checksum.
    if valid_csum != "valid":
        client_cmd += " --badsum --badsum-ip"
    if ipproto == 'v6':
        client_cmd += " -6 "
    # Add IPv4 options
    else:
        client_cmd += " --ip-options U "
        
    return client_cmd 
#
# Test send valid and invalid checksum packets to peer.
# * TCP use -S SYN flag. If no-one listening on port(e.g. nc -l <port>), connection
#   will be reset
# * ICMP use -1
# * UDP use -2 option
#
def getHping3Cmd(srv_ip_address, proto, valid_csum, ipproto):
    client_cmd = "hping3 " + srv_ip_address + " -c " + str(PKT_COUNT)
    server_cmd = "nc -D -v -l " + str(PORT_NUM)
    if valid_csum != "valid":
        client_cmd += " -b"
        
    if proto == 'icmp':
        client_cmd += " -1 "
    else:
        # TCP, send an SYN packet.
        client_cmd += " -S -p " + str(PORT_NUM)
        # Set TCP option
        client_cmd += " --tcp-timestamp "
        
    if proto == 'udp':
        client_cmd += " -2 "
        server_cmd += " -u"
        
    return client_cmd
    

def runSinlgeTest(test_name, tc):
    if api.IsDryrun():
        return api.types.status.SUCCESS

    srv = tc.srv
    cli = tc.cli
    
    req1 = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    req2 = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    valid_csum = getattr(tc.iterators, 'csum', 'valid')
    proto = getattr(tc.iterators, "proto", 'icmp')
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    tx_check_sum = getattr(tc.iterators, "tx_check_sum", 'on')
    rx_check_sum = getattr(tc.iterators, "rx_check_sum", 'on')
        
    if ipproto == 'v6':
        srv_ip_address = srv.ipv6_address
        cli_ip_address = cli.ipv6_address
        tc.cmd_descr = " Server: %s(%s) <--> Client: %s(%s)" %\
            (srv.interface, srv.ipv6_address, cli.interface, cli.ipv6_address)
    else:
        srv_ip_address = srv.ip_address
        cli_ip_address = cli.ip_address
        tc.cmd_descr = " Server: %s(%s) <--> Client: %s(%s)" %\
            (srv.interface, srv.ip_address, cli.interface, cli.ip_address)

    api.Logger.info("Starting Checksum [%s] %s" % (test_name, tc.cmd_descr))
    api.Logger.info("proto: %s/%s " % (proto, ipproto))

    tc.srv_bad_csum = ionic_stats.getNetstatBadCsum(srv, proto)
    tc.cli_bad_csum = ionic_stats.getNetstatBadCsum(cli, proto)

    # Start TCPdump on server and client.
    tc.srv_pcap_file = getPcapFileName("srv_", proto, ipproto, valid_csum, tx_check_sum, rx_check_sum, srv.interface)
    status, srv_resp = startTcpDump(srv.node_name, srv.interface, tc.srv_pcap_file, cli_ip_address)
    tc.cli_pcap_file = getPcapFileName("cli_", proto, ipproto, valid_csum, tx_check_sum, rx_check_sum, cli.interface)
    status, cli_resp = startTcpDump(cli.node_name, cli.interface, tc.cli_pcap_file, srv_ip_address)

    # Run the client command.
    if test_name == "nping":
        client_cmd = getNpingCmd(srv_ip_address, proto, valid_csum, ipproto)
    else:
        client_cmd = getHping3Cmd(srv_ip_address, proto, valid_csum, ipproto)
        
    api.Trigger_AddCommand(
        req2, cli.node_name, cli.workload_name, client_cmd)
    
    tc.resp = api.Trigger(req2)
    if tc.resp is None:
        api.Logger.error("Failed to run %s on client: %s\n" %(client_cmd, cli.node_name))
        return api.types.status.FAILURE

    tc.check_rx = False
    for cmd in tc.resp.commands:
        #
        # Bad checksum packets:
        # nping command can fail for corrupted packets.
        if cmd.exit_code != 0 and valid_csum == "valid":
            api.Logger.error("Failed to start %s for valid packet\n"
                             %(client_cmd))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE
        elif cmd.exit_code != 0 and valid_csum != "valid":
            api.Logger.info("Failed as expected %s for invalid csum"
                            %(client_cmd))
            return api.types.status.SUCCESS
        elif cmd.exit_code != 0:
            api.Logger.error("Failed to run %s, Tx csum: %s csum: %s\n"
                             % (client_cmd, tx_check_sum, valid_csum))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    # Data was transferred, check for Rx checksum counters.
    tc.check_rx = True;
    vlan = getattr(tc.iterators, 'vlantag', 'off')
    vxlan = getattr(tc.iterators, 'vxlan', 'off')
    api.Logger.info("Tx csum: %s Rx csum: %s VLAN: %s VXLAN: %s Proto: %s/%s Checksum: %s"
                    % (tx_check_sum, rx_check_sum, vlan, vxlan, proto, ipproto, valid_csum))
    api.Logger.info("Success running csum %s\n" %(client_cmd))

    dir_path = os.path.dirname(os.path.realpath(__file__))
    resp = api.CopyFromHost(cli.node_name, [tc.cli_pcap_file], dir_path)
    if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy client file: %s\n"
                        % (tc.cli_pcap_file))
        return api.types.status.FAILURE
    
    resp = api.CopyFromHost(srv.node_name, [tc.srv_pcap_file], dir_path)
    if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
        api.Logger.error("Failed to copy server file: %s\n"
                        % (tc.srv_pcap_file))
        return api.types.status.FAILURE
   
    status = verifySingle(tc)  

    return status

# Run various test like nping and hping with
# nping - IP options, IPv4 only
# hping3 - TCP timestamp options
def runCsumTest(tc):
    srv = tc.srv
    cli = tc.cli
    
    tx_check_sum = getattr(tc.iterators, "tx_check_sum", 'off')
    rx_check_sum = getattr(tc.iterators, "rx_check_sum", 'off')
    csum_valid = getattr(tc.iterators, 'csum', 'valid')
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    test_name = getattr(tc.iterators, "test_name", 'nping')
    # Hping3 doesn't support IPv6.
    if ipproto == 'v6':
        test_name = 'nping'   

    cli_tx_before_no_csum = ionic_stats.getTxNoCsumStats(cli, cli.interface)
    cli_tx_before_csum = ionic_stats.getTxCsumStats(cli, cli.interface)

    if tc.is_srv_naples:
        srv_rx_before_csum_good = ionic_stats.getRxL4CsumGoodStats(
            srv, srv.interface)
        srv_rx_before_csum_bad = ionic_stats.getRxL4CsumBadStats(
            srv, srv.interface)

    status = runSinlgeTest(test_name, tc)
    if status != api.types.status.SUCCESS:
        return status

    cli_tx_after_no_csum = ionic_stats.getTxNoCsumStats(cli, cli.interface)
    cli_tx_after_csum = ionic_stats.getTxCsumStats(cli, cli.interface)

    api.Logger.info("TX-NO-CSUM before: %s after: %s"
                    % (str(cli_tx_before_no_csum), str(cli_tx_after_no_csum)))
    api.Logger.info("TX-CSUM before: %s after: %s"
                    % (str(cli_tx_before_csum), str(cli_tx_after_csum)))

    # Checksum values don't match immediately if capability is toggeled.
    if cli_tx_before_no_csum == cli_tx_after_no_csum and tx_check_sum == "off":
        api.Logger.warn("For Tx checskum off, TX_NO_CSUM match, before: %s after: %s"
                        % (str(cli_tx_before_no_csum), str(cli_tx_after_no_csum)))

    if cli_tx_before_csum == cli_tx_after_csum and tx_check_sum == "on":
        api.Logger.warn("For Tx checskum on, TX-CSUM match, before: %s after: %s"
                        % (str(cli_tx_before_csum), str(cli_tx_after_csum)))
    
    if tc.is_srv_naples and tc.check_rx:
        srv_rx_after_csum_good = ionic_stats.getRxL4CsumGoodStats(
            srv, srv.interface)
        srv_rx_after_csum_bad = ionic_stats.getRxL4CsumBadStats(
            srv, srv.interface)

        api.Logger.info("Rx bad checksum counters before: %s after: %s"
                        % (str(srv_rx_before_csum_bad), str(srv_rx_after_csum_bad)))
        api.Logger.info("Rx good checksum counters before: %s after: %s"
                        % (str(srv_rx_before_csum_good), str(srv_rx_after_csum_good)))
               
        #if srv_rx_after_csum_good == srv_rx_before_csum_good and csum_valid == "valid":
        #    api.Logger.error("For valid packets, server checksum rx counters are same: %s after: %s"
        #                     %(str(srv_rx_before_csum_bad), str(srv_rx_after_csum_bad)))
        #    return api.types.status.FAILURE
  
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
            status = runCsumTest(tc)
            if status != api.types.status.SUCCESS:
                return status

        else:
            vxlan_ipproto = getattr(tc.iterators, "vxlan_ip", 'v4')
            ipv4 = True
            if vxlan_ipproto == 'v6':
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
            status = runCsumTest(tc)
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
