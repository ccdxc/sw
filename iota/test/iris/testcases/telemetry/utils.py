#! /usr/bin/python3
import json
#import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import time
import re
import pdb
import os
from scapy.utils import *
from scapy.utils import rdpcap
from scapy.utils import wrpcap
from scapy import packet
from scapy.all import Ether
import glob
import iota.harness.api as api
import ipaddress as ipaddr
from datetime import datetime

uplink_vlan = 0

def GetProtocolDirectory(feature, proto):
    return api.GetTopologyDirectory() + "/gen/telemetry/{}/{}".format(feature, proto)

def GetTargetJsons(feature, proto):
    return glob.glob(GetProtocolDirectory(feature, proto) + "/*_policy.json")

def GetTargetVerifJsons(feature, proto):
    return glob.glob(GetProtocolDirectory(feature, proto) + "/*_verif.json")

def ReadJson(filename):
    return api.parser.JsonParse(filename)

def GetTcpDumpCmd(intf, protocol = None, port = 0):
    cmd = "tcpdump -nni %s " % intf

    if protocol != None:
        cmd = cmd + " %s" % protocol

    if port != 0:
        cmd = cmd + " port %d" % port

    return cmd

def GetHping3Cmd(protocol, src_wl, destination_ip, destination_port):
    if protocol == 'tcp':
        cmd = "hping3 -S -p {} -c 1 {} -I {}".format(int(destination_port), destination_ip, src_wl.interface)
    elif protocol == 'udp':
        cmd = "hping3 --{} -p {} -c 1 {} -I {}".format(protocol.lower(), int(destination_port), destination_ip, src_wl.interface)
    else:
        # Workaround for hping issue on BM workload over VLAN tagged sub-if;
        # hping sends local host addr as source IP for hping on tagged sub-if;
        # hence using ping instead of hping on BM.
        if api.IsBareMetalWorkloadType(src_wl.node_name):
            cmd = "ping -c 1 {} ".format(destination_ip)
        else:
            cmd = "hping3 --{} -c 1 {} -I {}".format(protocol.lower(), destination_ip, src_wl.interface)
        
    return cmd

def GetNpingCmd(protocol, destination_ip, destination_port, source_ip = "", count = 1):
    if protocol == 'tcp':
        if source_ip != "":
            cmd = "nping --tcp --source-ip {} -p {} -c {} {}".format(source_ip, int(destination_port), int(count), destination_ip)
        else:
            cmd = "nping --tcp -p {} -c {} {}".format(int(destination_port), int(count), destination_ip)
    elif protocol == 'udp':
        if source_ip != "":
            cmd = "nping --udp --source-ip {} -p {} -c {} {}".format(source_ip, int(destination_port), int(count), destination_ip)
        else:
            cmd = "nping --udp -p {} -c {} {}".format(int(destination_port), int(count), destination_ip)
    else:
        if source_ip != "":
            cmd = "nping --{} --source-ip {} -c {} {}".format(protocol.lower(), source_ip, int(count), destination_ip)
        else:
            cmd = "nping --{} -c {} {}".format(protocol.lower(), int(count), destination_ip)

    return cmd

def GetVerifJsonFromPolicyJson(policy_json):
    return policy_json.replace("_policy", "_verif")

def VerifyVlan():
    result = api.types.status.SUCCESS
    dir_path = os.path.dirname(os.path.realpath(__file__))
    mirrorscapy = dir_path + '/' + "mirror.pcap"
    api.Logger.info("File Name: %s" % (mirrorscapy))
    pkts = rdpcap(mirrorscapy)
    spanpktsfound = False
    for pkt in pkts:
        if pkt.haslayer('GRE'):
            spanpktsfound = True
            inner=Ether(pkt['Raw'].load[20:])
            #api.Logger.info("Pkt: {}".format(pkt))
            #api.Logger.info("Inner Pkt: {}".format(inner))
            if (uplink_vlan == 0 and inner.haslayer('Dot1Q')):
                result = api.types.status.FAILURE
                api.Logger.info("VerifyVlan Failed: uplink_vlan: {} Inner Vlan: {} ".format(uplink_vlan, inner['Dot1Q'].vlan))
                api.Logger.info("Inner Pkt: {}".format(inner))
            elif (inner.haslayer('Dot1Q') and inner['Dot1Q'].vlan != uplink_vlan):
                result = api.types.status.FAILURE
                api.Logger.info("VerifyVlan Failed: uplink_vlan: {} Inner Vlan id: {}".format(uplink_vlan, inner['Dot1Q'].vlan))
                api.Logger.info("Inner Pkt: {}".format(inner))
    if spanpktsfound == False:
       result = api.types.status.FAILURE
       api.Logger.info("VerifyVlan Failed: spanpkts not found ")
    return result

def VerifyTimeStamp(command):
    global g_time
    result = api.types.status.SUCCESS
    dir_path = os.path.dirname(os.path.realpath(__file__))
    mirrorscapy = dir_path + '/' + "mirror.pcap"
    api.Logger.info("File Name: %s" % (mirrorscapy))
    pkts = rdpcap(mirrorscapy)
    spanpktsfound = False
    g_time = datetime.fromtimestamp(time.clock_gettime(time.CLOCK_REALTIME))
    api.Logger.info("Current Global time {}".format(g_time))
    for pkt in pkts:
        if pkt.haslayer('GRE'):
            spanpktsfound = True
            # Read raw bytes to interpret time
            p1 = pkt['Raw'].load[4:8]
            p2 = pkt['Raw'].load[16:20]
            p3 = (int.from_bytes(p2, byteorder='big', signed=False) << 32) | (int.from_bytes(p1, byteorder='big', signed=False))
            api.Logger.info("Timestamp from the packet: %s" % (p3)) 
            pkttime=p3/1000000000
            pkttimestamp = datetime.fromtimestamp(pkttime)
            if g_time > pkttimestamp:
               tdelta = g_time-pkttimestamp
            else:
               tdelta = pkttimestamp-g_time
            #Its pretty hard to comapare the time the actual packet
            #was sent to when the comparator is obtained. May be execute
            #date in naples might help
            if (tdelta.days != 0):
                result = api.types.status.FAILURE
            api.Logger.info("Timestamp delta: %s" %(tdelta))
    if spanpktsfound == False:
       result = api.types.status.FAILURE
    return result

def VerifyCmd(cmd, action, feature):
    api.PrintCommandResults(cmd)
    result = api.types.status.SUCCESS
    if 'tcpdump' in cmd.command:
        if feature == 'mirror':
            if 'pcap' in cmd.command:
                if VerifyVlan() == api.types.status.FAILURE:
                    result = api.types.status.FAILURE
                    api.Logger.info("VerifyVlan failed")
                if VerifyTimeStamp(cmd) == api.types.status.FAILURE:
                    result = api.types.status.FAILURE
        elif feature == 'flowmon':
            matchObj = re.search( r'(.*)2055: UDP, length(.*)', cmd.stdout, 0)
            if matchObj is None:
                result = api.types.status.FAILURE
    return result

def GetDestPort(port):
    if ',' in port:
        return "100"
    elif '-' in port:
        return "120"
    elif 'any' in port:
        return '3000'
    return port 

def RunCmd(src_wl, protocol, dest_wl, destination_ip, destination_port, collector_w, action, feature):
    backgroun_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    result = api.types.status.SUCCESS
    
    # Add the ping commands from collector to source and dest workload
    # to avoid flooding on the vswitch
    if (collector_w.node_name != src_wl.node_name):
        api.Trigger_AddCommand(req, collector_w.node_name, collector_w.workload_name,
                                   "ping -c 1 %s " % (src_wl.ip_address))
    if (collector_w.node_name != dest_wl.node_name):
        api.Trigger_AddCommand(req, collector_w.node_name, collector_w.workload_name,
                                   "ping -c 1 %s " % (destination_ip))
    if feature == 'mirror':
        api.Trigger_AddCommand(backgroun_req, collector_w.node_name, collector_w.workload_name,
                               "tcpdump -c 10 -nnSXi %s ip proto gre -w mirror.pcap" % (collector_w.interface), background=True)
        api.Trigger_AddCommand(backgroun_req, collector_w.node_name, collector_w.workload_name,
                               "tcpdump -c 10 -nni %s ip proto gre" % (collector_w.interface), background=True)
    elif feature == 'flowmon':
        api.Trigger_AddCommand(backgroun_req, collector_w.node_name, collector_w.workload_name,
                               "tcpdump -c 10 -nni %s udp and dst port 2055" % (collector_w.interface), background=True)

    background_trig_resp = api.Trigger(backgroun_req)

    #delay for background cmds to start before issuing ping
    if feature == 'flowmon':
        time.sleep(2)
    if feature == 'mirror':
        time.sleep(2)

    cmd = GetHping3Cmd(protocol, src_wl, destination_ip, destination_port)
    api.Trigger_AddCommand(req, src_wl.node_name, src_wl.workload_name, cmd)
    api.Logger.info("Running from src_wl_ip {} COMMAND {}".format(src_wl.ip_address, cmd))

    trig_resp = api.Trigger(req)

    #api.Logger.info("Trigger resp commands")
    #for cmd in trig_resp.commands:
    #    api.PrintCommandResults(cmd)

    if feature == 'flowmon':
        time.sleep(2)
    if feature == 'mirror':
        time.sleep(2)

    term_resp = api.Trigger_TerminateAllCommands(background_trig_resp)
    background_resp = api.Trigger_AggregateCommandsResponse(background_trig_resp, term_resp)

    #api.Logger.info("background resp commands")
    #for cmd in background_resp.commands:
    #    api.PrintCommandResults(cmd)

    #resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)

    if feature == 'mirror':
        dir_path = os.path.dirname(os.path.realpath(__file__))
        api.CopyFromWorkload(collector_w.node_name, collector_w.workload_name, ['mirror.pcap'], dir_path)

    for cmd in background_resp.commands:
        result = VerifyCmd(cmd, action, feature)
        if (result == api.types.status.FAILURE):
            api.Logger.info("Testcase FAILED!! cmd: {}".format(cmd))
            break

    return result

def GetSourceWorkload(verif, tc):
    global uplink_vlan
    workloads = api.GetWorkloads()
    sip = verif['src_ip']
    src_wl = None
    for wl in workloads:
        api.Logger.info("Uplink vlan: {}".format(wl.uplink_vlan))
        if (wl.uplink_vlan != 0):
            uplink_vlan = wl.uplink_vlan
        if '/24' in sip or 'any' in sip:
            if verif['dst_ip'] != wl.ip_address:
                src_wl = wl
                break
        else:
            if sip == wl.ip_address:
                src_wl = wl
                break
    if src_wl is None:
        api.Logger.info("Did not get a matching workload. Dump all workloads")
        api.Logger.info("sip: {}".format(sip))
        api.Logger.info("verif_dst_ip: {}".format(verif['dst_ip']))
        for wl in workloads:
            api.Logger.info("wl.ip_address: {}".format(wl.ip_address))
    return src_wl

def GetDestWorkload(verif, tc, src_w):
    workloads = api.GetWorkloads()
    dip = verif['dst_ip']
    dst_wl = None
    # For dest workload selection,
    # check and select the workload in same subnet as source
    for wl in workloads:
        if src_w.ip_address == wl.ip_address:
            continue
        src_ip_pref = ipaddr.IPv4Interface(src_w.ip_prefix)
        src_ip_subnet = src_ip_pref.network

        wl_ip_pref = ipaddr.IPv4Interface(wl.ip_prefix)
        wl_ip_subnet = wl_ip_pref.network

        #source & dest IP not in same subnet
        if src_ip_subnet != wl_ip_subnet:
            continue

        if 'any' in dip or 'any' in verif['src_ip']:
            if wl.node_name == src_w.node_name:
                continue
        if '/24' in dip or 'any' in dip:
            if src_w.ip_address != wl.ip_address:
                dst_wl = wl
                break
        else:
            if dip == wl.ip_address:
                dst_wl = wl
                break
    return dst_wl

def RunAll(collector_w, verif_json, tc, feature):
    res = api.types.status.SUCCESS
    api.Logger.info("VERIFY JSON FILE {}".format(verif_json))

    verif = []
    ret = {}
    with open(verif_json, 'r') as fp:
        verif = json.load(fp)

    api.Logger.info("VERIF = {}".format(verif))
    count = 0
    if feature == 'flowmon':
        time.sleep(4)
    for i in range(0, len(verif)):
        protocol = verif[i]['protocol'] 
        src_w = GetSourceWorkload(verif[i], tc)
        dest_w = GetDestWorkload(verif[i], tc, src_w)
        if src_w == None:
            continue
        if dest_w == None:
            continue
        # If both workload nodes are not naples, continue
        if not src_w.IsNaples():
            if not dest_w.IsNaples():
                continue
        if api.IsBareMetalWorkloadType(collector_w.node_name):
            if (dest_w.node_name == src_w.node_name):
                api.Logger.info("Source and Dest workloads are same {}".format(dest_w.node_name))
                continue
        dest_port = GetDestPort(verif[i]['port'])
        action = verif[i]['result']
        res = RunCmd(src_w, protocol, dest_w, dest_w.ip_address, dest_port, collector_w, action, feature)
        if (res == api.types.status.FAILURE):
            api.Logger.info("Testcase FAILED!!")
            break;
        count = count + 1
    ret['res'] = res
    ret['count'] = count
    return ret
