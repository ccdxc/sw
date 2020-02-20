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

#
# Test send valid and invalid checksum packets to peer.
# *  If Tx checskum offload is enabled, bad checksum is corrected, otheriwse
#    peer receive bad packet.
# Setup:
#       Transmitter has to be Naples
#       If receiver is Naples, we validate rx checskum offload
def startTcpDump(node, intf, filename, src_ip):

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Logger.info("Starting tcpdump for host %s on %s" %(src_ip, node))
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
    proto = getattr(tc.iterators, "proto", 'icmp')
    srv_bad_csum = ionic_stats.getNetstatBadCsum(tc.srv, proto)
    # Compare the bad csum counter before and after the test on receiver.
    if tc.srv_bad_csum != srv_bad_csum:
        api.Logger.error("Server got bad checksum packets, before: %d, after: %d"
                         % (tc.srv_bad_csum, srv_bad_csum))
        return api.types.status.FAILURE
    cli_bad_csum = ionic_stats.getNetstatBadCsum(tc.cli, proto)
    # Compare the bad csum counter before and after the test on receiver.
    if tc.cli_bad_csum != cli_bad_csum:
        api.Logger.error("Client got bad checksum packets, before: %d, after: %d"
                         % (tc.cli_bad_csum, cli_bad_csum))
        return api.types.status.FAILURE

    api.Logger.info("Bad %s checksum counter for client: %d, server: %d"
                    % (proto, cli_bad_csum, srv_bad_csum))
    return api.types.status.SUCCESS

def VerifyPacket(tc):
    dir_path = os.path.dirname(os.path.realpath(__file__))
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
        
    pcap_file = dir_path  + "/" + tc.srv_pcap_file
    if os.path.isfile(pcap_file):
        #api.Logger.info("Server pcap file: %s\n" %(pcap_file))
        cli_packets = rdpcap(pcap_file)
        proto = getattr(tc.iterators, "proto", 'icmp')
        for pkt in cli_packets:
            if not pkt.haslayer('IP'):
                continue
            
            rx_check_sum = getattr(tc.iterators, "rx_check_sum", 'off')
            # If receiver is Naples and rx-checksum offload is enabled, packet checksum is non-zero. If
            # rx checksum is off, it is zero.
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

def runHping(tc):
    if api.IsDryrun():
        return api.types.status.SUCCESS

    srv = tc.srv
    cli = tc.cli
    
    req1 = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    req2 = api.Trigger_CreateExecuteCommandsRequest(serial=False)

    valid_csum = getattr(tc.iterators, 'csum', 'valid')
    proto = getattr(tc.iterators, "proto", 'icmp')
    ipproto = getattr(tc.iterators, "ipproto", 'v4')
    tc_check_sum = getattr(tc.iterators, "tx_check_sum", 'on')
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

    api.Logger.info("Starting Checksum tests test %s" % (tc.cmd_descr))
    api.Logger.info("proto: %s/%s " % (proto, ipproto))

    tc.srv_bad_csum = ionic_stats.getNetstatBadCsum(srv, proto)
    tc.cli_bad_csum = ionic_stats.getNetstatBadCsum(cli, proto)

    client_cmd = "hping3 " + srv_ip_address + " -c " + str(PKT_COUNT)
    server_cmd = "nc -D -v -l " + str(PORT_NUM)
    if valid_csum != "valid":
        client_cmd += " -b"
        
    if proto == 'icmp':
        client_cmd += " -1 "
    else:
        client_cmd += " -p " + str(PORT_NUM)
        
    if proto == 'udp':
        client_cmd += " -2 "
        server_cmd += " -u"
    
    
    file_name_suffix= "_" + proto + "_" + ipproto + "_csum_" + valid_csum + \
        "_txcsum_" + tc_check_sum + "_rxcsum_" + rx_check_sum

    file_name = 'srv_' + srv.interface + file_name_suffix
    tc.srv_pcap_file=file_name + '.pcap'
    status, srv_resp = startTcpDump(srv.node_name, srv.interface, tc.srv_pcap_file, cli_ip_address)

    file_name = 'cli_' + cli.interface + file_name_suffix
    tc.cli_pcap_file=file_name + ".pcap"
    status, cli_resp = startTcpDump(cli.node_name, cli.interface, tc.cli_pcap_file, srv_ip_address)

    # Server side.
    if proto != "icmp":
        api.Trigger_AddCommand(
            req1, srv.node_name, srv.workload_name, server_cmd, background = True)
        resp1 = api.Trigger(req1)
        if resp1 is None:
            api.Logger.error("Failed to start %s on server: %s" %(server_cmd, srv.node_name))
            return api.types.status.FAILURE

    # Client side
    api.Trigger_AddCommand(
        req2, cli.node_name, cli.workload_name, client_cmd)
    
    tc.resp = api.Trigger(req2)
    if tc.resp is None:
        api.Logger.error("Failed to run %s on client: %s\n" %(client_cmd, cli.node_name))
        return api.types.status.FAILURE

    tc.check_rx = False
    for cmd in tc.resp.commands:
        if cmd.exit_code != 0 and tc_check_sum == "on":
            api.Logger.error("Failed to start client hping3\n");
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE
        elif cmd.exit_code != 0 and tc_check_sum == "off" and valid_csum != "valid":
            api.Logger.info("hping3 fail as expected")
            return api.types.status.SUCCESS
        elif cmd.exit_code != 0:
            api.Logger.error("Failed to start client hping3 Tx csum: %s\n"
                             % (tc_check_sum))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    # Data was transferred, check for Rx checksum counters.
    tc.check_rx = True;
    api.Logger.info("Verifying tcpdump capture for Tx csum: %s\n" %(tc_check_sum))

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
    
    vlan = getattr(tc.iterators, 'vlantag', 'off')
    vxlan = getattr(tc.iterators, 'vxlan', 'off')
    tx_check_sum = getattr(tc.iterators, "tx_check_sum", 'off')
    rx_check_sum = getattr(tc.iterators, "rx_check_sum", 'off')
   
    status = verifySingle(tc) 
    
    api.Logger.info("Result Tx csum: %s Rx csum: %s VLAN: %s VXLAN: %s Proto: %s/%s "
                    % (tx_check_sum, rx_check_sum, vlan, vxlan, proto, ipproto))
    
    return status

def runCsumTest(tc):
    srv = tc.srv
    cli = tc.cli
    
    tx_check_sum = getattr(tc.iterators, "tx_check_sum", 'off')
    rx_check_sum = getattr(tc.iterators, "rx_check_sum", 'off')
    
    cli_tx_before_no_csum = ionic_stats.getTxNoCsumStats(cli, cli.interface)
    cli_tx_before_csum = ionic_stats.getTxCsumStats(cli, cli.interface)
    
    if tc.is_srv_naples:
        srv_rx_before_csum_good = ionic_stats.getRxL4CsumGoodStats(srv, srv.interface)
        srv_rx_before_csum_bad = ionic_stats.getRxL4CsumBadStats(srv, srv.interface)
    
    status = runHping(tc);
    if status != api.types.status.SUCCESS:
        return status
    
    cli_tx_after_no_csum = ionic_stats.getTxNoCsumStats(cli, cli.interface)
    cli_tx_after_csum =  ionic_stats.getTxCsumStats(cli, cli.interface)

    api.Logger.info("TX-NO-CSUM before: %s after: %s"
                     % (str(cli_tx_before_no_csum), str(cli_tx_after_no_csum)))
    api.Logger.info("TX-CSUM before: %s after: %s"
                     %(str(cli_tx_before_csum), str(cli_tx_after_csum)))
    
    if cli_tx_before_no_csum == cli_tx_after_no_csum and tx_check_sum == "off":
        api.Logger.error("For Tx checskum off, TX_NO_CSUM match, before: %s after: %s"
                         %(str(cli_tx_before_no_csum), str(cli_tx_after_no_csum)))
        return api.types.status.FAILURE
        
    if cli_tx_before_csum == cli_tx_after_csum and tx_check_sum == "on":
        api.Logger.error("For Tx checskum on, TX-CSUM match, before: %s after: %s"
                         %(str(cli_tx_before_csum), str(cli_tx_after_csum)))
        return api.types.status.FAILURE
    
    csum_valid = getattr(tc.iterators, 'csum', 'valid')
    
    if tc.is_srv_naples and tc.check_rx:
        srv_rx_after_csum_good = ionic_stats.getRxL4CsumGoodStats(srv, srv.interface)
        srv_rx_after_csum_bad = ionic_stats.getRxL4CsumBadStats(srv, srv.interface)
         
        api.Logger.info("Rx bad checksum counters before: %s after: %s"
                        %(str(srv_rx_before_csum_bad), str(srv_rx_after_csum_bad)))
        api.Logger.info("Rx good checksum counters before: %s after: %s"
                        %(str(srv_rx_before_csum_good), str(srv_rx_after_csum_good)))
               
        if srv_rx_after_csum_good == srv_rx_before_csum_good and csum_valid == "valid":
            api.Logger.error("For valid packets, server checksum rx counters are same: %s after: %s"
                             %(str(srv_rx_before_csum_bad), str(srv_rx_after_csum_bad)))
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
