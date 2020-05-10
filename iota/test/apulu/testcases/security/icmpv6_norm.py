#! /usr/bin/python3
import pdb
import re
import time
import os
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.utils.traffic as traffic_utils

from scapy.all import *

dir_path = 'iota/test/utils/' 

icmpv6_pkt_count = 100

# This represents the number of commands sent per workload pair during the Trigger phase.
# This is used in Verify phase to track the drop stats differences b/w each WL pair.
# As of now 3 commands are sent in total; in case this changes adjust this count accordingly.
num_cmds_per_wl_pair = 3


def PrintWLInfo(tc):
    for wl in tc.workloads:
        api.Logger.info("Node:%s, WL:%s, Intf:%s, ipv4:%s, ipv6:%s"%\
                (wl.node_name, wl.workload_name, wl.interface, wl.ip_address, wl.ipv6_address))


def InstallScapyPackge(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []
    for wl in tc.workloads:
        if not wl.IsNaples():
            continue
        # Install python scapy packages
        cmd_cookie = "Installing Scapy packages in WL:%s"% wl.workload_name
        api.Logger.info(cmd_cookie)
        cmd = "apt-get install python-scapy"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        tc.cmd_cookies.append(cmd_cookie)
    tc.resp = api.Trigger(req)

    if tc.resp:
        cookie_idx = 0
        for cmd in tc.resp.commands:
            if cmd.exit_code != 0:
                api.Logger.info(tc.cmd_cookies[cookie_idx])
                api.PrintCommandResults(cmd)
            cookie_idx += 1
    tc.cmd_cookies = []
    return api.types.status.SUCCESS


def SetupPacketScapy(tc):
    pktscapyscript = api.GetTopDir() +  '/' + dir_path + "packet_scapy.py"
    for wl in tc.workloads:
        resp = api.CopyToWorkload(wl.node_name, wl.workload_name, [pktscapyscript], "")
        if resp is None:
            api.Logger.info("Failed to copy packet scapy script to WL:%s"%wl.workload_name)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


def FormICMPv6NSPkt(wl):
    # Prepare packets parameters
    ether_params = {}
    ether_params["src"] = wl.mac_address
    ipv6_params = {}
    ipv6_params["src"] = "2001:bade:cd8::1"
    # Compute the solicited-node multicast address
    # associated with the target address.
    tmp = inet_ntop(socket.AF_INET6,
                    in6_getnsma(inet_pton(socket.AF_INET6, "2001:bade:cd8::2")))
    ipv6_params["dst"] = tmp
    pkt = Ether(**ether_params)
    pkt /= IPv6(**ipv6_params)
    pkt /= ICMPv6ND_NS(tgt="2001:bade:cd8::2")
    pkt /= ICMPv6NDOptSrcLLAddr(lladdr=wl.mac_address)
    return pkt

def FormICMPv6NAPkt(wl):
    # Prepare packets parameters
    ether_params = {}
    ether_params["src"] = wl.mac_address
    ipv6_params = {}
    ipv6_params["src"] = "2001:bade:cd8::1"
    # set dest for unsolicited all node address
    ipv6_params["dst"] = "ff02::1"
    pkt = Ether(**ether_params)
    pkt /= IPv6(**ipv6_params)
    pkt /= ICMPv6ND_NA(tgt="2001:bade:cd8::1", S=0, R=0, O=0)
    pkt /= ICMPv6NDOptSrcLLAddr(lladdr=wl.mac_address)
    return pkt


def FormICMPv6RSPkt(wl):
    pkt = Ether(src=wl.mac_address)
    # set dest for unsolicited all node address
    pkt /= IPv6(src="2001:bade:cd8::1", dst="ff02::1")
    pkt /= ICMPv6ND_RS()
    pkt /= ICMPv6NDOptSrcLLAddr(lladdr=wl.mac_address)
    return pkt


def FormICMPv6RAPkt(wl):
    pkt = Ether(src=wl.mac_address)
    pkt /= IPv6(src="2001:bade:cd8::1", dst="ff02::1")
    pkt /= ICMPv6ND_RA(M=0,O=0)
    pkt /= ICMPv6NDOptSrcLLAddr(lladdr=wl.mac_address)
    pkt /= ICMPv6NDOptPrefixInfo(prefixlen=64, prefix="2001:bad:dead:fe8::",L=1,A=1)
    return pkt


def GetICMPv6PacketPcap(wl, test_type):
    PktFunc = {
          "ICMPv6-NS":FormICMPv6NSPkt,
          "ICMPv6-NA":FormICMPv6NAPkt,
          "ICMPv6-RS":FormICMPv6RSPkt,
          "ICMPv6-RA":FormICMPv6RAPkt
    }
    pcap_file = None
    pcap_file_full_path = None
    # Get the Packet form function
    func = PktFunc.get(test_type, None) 
    # Execute the function
    if func:
        pcap_file = "%s.pcap"%test_type
        pkt = func(wl)
        pcap_file_full_path = (api.GetTopDir() + '/' + dir_path + pcap_file)
        wrpcap(pcap_file_full_path, pkt)
    return (pcap_file_full_path, pcap_file)


def ICMPv6Test(tc, packet_type):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []
    for wl in tc.workloads:
        # Use only untagged WL for now
        if wl.uplink_vlan != 0:
            continue
        # Pick Naples workload as the source for packet tx
        if not wl.IsNaples():
            continue
        # Form ICMPv6 NS Packet PCAP file and copy to WL
        pcap_file_full_path, pcap_file = GetICMPv6PacketPcap(wl, packet_type)
        if pcap_file is None:
            api.Logger.error("Failed to get PCAP file for packet: %s, for WL:%s"%(packet_type,wl.workload_name))
            continue
        resp = api.CopyToWorkload(wl.node_name, wl.workload_name, [pcap_file_full_path], "")
        if resp is None:
            api.Logger.error("Failed to copy PCAP file %s to WL:%s"%(pcap_file,wl.workload_name))
            continue

        # get drop stats before test
        naples_cmd = r"/nic/bin/pdsctl show system statistics drop | grep nacl"
        api.Trigger_AddNaplesCommand(req, wl.node_name, naples_cmd)
        tc.cmd_cookies.append("NACL Drops at start of test")

        # Send ICMPv6 NS
        cmd_cookie = "Send %s packets from %s(%s, %s)" % \
                (packet_type, wl.workload_name, "2001:bade:cd8::1", wl.interface)
        tc.cmd_cookies.append(cmd_cookie)
        cmd = "sh -c 'chmod +x packet_scapy.py && ./packet_scapy.py --pcap-file %s --iface %s --count %s'"%(pcap_file,wl.interface,icmpv6_pkt_count)
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)

        # get drop stats before test
        naples_cmd = r"/nic/bin/pdsctl show system statistics drop | grep nacl"
        api.Trigger_AddNaplesCommand(req, wl.node_name, naples_cmd)
        tc.cmd_cookies.append("NACL Drops at end of test")
    tc.resp = api.Trigger(req)

    if tc.resp is None:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS


def VerifyPacketStats(tc, test_type):
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS
    cookie_idx = 0
    cmd_resp_len = len(tc.resp.commands)

    while cookie_idx < cmd_resp_len:
        nacl_drop_start = 0
        nacl_drop_end = 0
        if "NACL Drops at start of test" in tc.cmd_cookies[cookie_idx]:
            cmd = tc.resp.commands[cookie_idx]
            for line in cmd.stdout.split('\n'):
                if "nacl" in line:
                    for s in line.split():
                        if s.isdigit():
                            nacl_drop_start = int(s)
        if "NACL Drops at end of test" in tc.cmd_cookies[cookie_idx+2]:
            cmd = tc.resp.commands[cookie_idx+2]
            for line in cmd.stdout.split('\n'):
                if "nacl" in line:
                    for s in line.split():
                        if s.isdigit():
                            nacl_drop_end = int(s)

        api.PrintCommandResults(tc.resp.commands[cookie_idx])
        api.Logger.info(tc.cmd_cookies[cookie_idx+1])
        api.PrintCommandResults(tc.resp.commands[cookie_idx+2])

        if (nacl_drop_end == nacl_drop_start):
            api.PrintCommandResults(tc.resp.commands[cookie_idx+1])
            api.Logger.info("Change in NACL drops after sending %s packets"%test_type)
        elif (nacl_drop_end < (nacl_drop_start + icmpv6_pkt_count)):
            result = api.types.status.FAILURE
            api.PrintCommandResults(tc.resp.commands[cookie_idx+1])
            api.Logger.info("Test Failed, NACL drop stats before and after sending %s packets "
                            "doesn't match with test expectation, before: %s, after: %s" % \
                            (test_type, nacl_drop_start, nacl_drop_end))
        cookie_idx += num_cmds_per_wl_pair

    if result == api.types.status.FAILURE:
        api.Logger.error("Test Failed for %s"%test_type)
    else:
        api.Logger.info("Test Passed for %s"%test_type)
    return result


def Setup(tc):
    tc.skip = False
    tc.workloads = api.GetWorkloads()
    tc.packet_types = getattr(tc.args, "pkt_type", None)
    if not tc.args.pkt_type:
        tc.packet_types = ["ICMPv6-NS", "ICMPv6-NA", "ICMPv6-RS", "ICMPv6-RA"]

    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True
        return api.types.status.SUCCESS

    PrintWLInfo(tc)

    InstallScapyPackge(tc)

    if SetupPacketScapy(tc) != api.types.status.SUCCESS:
        api.Logger.info("Failed to copy packet scapy script to WL(s)")
        return api.types.status.SUCCESS
    return api.types.status.SUCCESS


def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS
    api.Logger.info("Running %s DROP tests..."%tc.packet_types)

    for pkt_type in tc.packet_types:
        if ICMPv6Test(tc, pkt_type) != api.types.status.SUCCESS:
            api.Logger.error("Failed in %s test packet trigger"%pkt_type)
            return api.types.status.FAILURE

        # verify NACL drop stats
        if VerifyPacketStats(tc, pkt_type) != api.types.status.SUCCESS:
            api.Logger.error("Failed in %s test packet statistics verification"%pkt_type)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


def Verify(tc):
    api.Logger.info("Validating ...")
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
