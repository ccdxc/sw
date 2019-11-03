#! /usr/bin/python3
# VPP utils
import pdb
import subprocess
import os
import re
from scapy.layers.inet import IP, TCP, UDP, ICMP
from infra.common.logging import logger
import apollo.config.utils as utils
import artemis.test.callbacks.networking.packets as packets

def __get_vppctl_path():
    rel_path = "nic/vpp/tools/vppctl-sim.sh"
    abs_path = os.path.join(os.environ['WS_TOP'], rel_path)
    return abs_path

def ExecuteVPPCommand(command):
    vppctl=__get_vppctl_path()
    try:
        output=str(subprocess.check_output([vppctl, command], stderr=subprocess.STDOUT))
        retval=True
    except subprocess.CalledProcessError as e:
        output="Command execution failed."
        retval=False
    logger.info("VPP: command[%s], output[%s], retval[%d]" \
                 %(command, output, retval))
    return retval, output

def FrameFlowDumpCommand(src, dst, ip_proto, sport, dport, lkp_id):
    command="dump flow entry source-ip " + str(src) +\
            " destination-ip " + str(dst) + " ip-protocol " + ip_proto +\
            " lookup-id " + str(lkp_id)
    if sport!=0 and dport!=0:
        command+=" source-port " + str(sport) +  " destination-port " + str(dport)
    return command

def GetFlowTuppleFromPkt(pkt):
    packet_tuples=packets.__get_packet_tuples(pkt)
    #VPP needs protocol string
    if packet_tuples[2] == 1:
        proto="ICMP"
    elif packet_tuples[2] == 6:
        proto="TCP"
    elif packet_tuples[2] == 17:
        proto="UDP"
    elif packet_tuples[2] == 58:
        proto="ICMPv6"
    else:
        assert 0, "Unexpected ip proto"
    return packet_tuples[0], packet_tuples[1],\
           proto, packet_tuples[3], packet_tuples[4]

def CheckFlowExists(pkt, lkp_id, check_rflow=True):
    ip_src, ip_dst, ip_proto, sport, dport = GetFlowTuppleFromPkt(pkt)
    command = FrameFlowDumpCommand(ip_src, ip_dst, ip_proto,\
                                    sport, dport, lkp_id)
    ret, output = ExecuteVPPCommand(command)
    if utils.IsDryRun() != True and ret == False or "Entry not found" in output:
        return False
    if check_rflow == False:
        return True
    command = FrameFlowDumpCommand(ip_dst, ip_src, ip_proto,\
                                    dport, sport, lkp_id)
    ret, output = ExecuteVPPCommand(command)
    if utils.IsDryRun() != True and ret == False or "Entry not found" in output:
        return False
    return True
