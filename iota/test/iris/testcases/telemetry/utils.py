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
from scapy.all import Dot1Q
import glob
import json
import copy
import random
import ipaddress as ipaddr
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
from datetime import datetime
from iota.test.utils.erspan import ERSPAN_III
from iota.test.utils.erspan import PlatformSpecific

uplink_vlan = 0
# for local work loads, the packet vlan may not be wire encap vlan.
# its hard to predict which lif the local wl's are mapped to.
# ignore vlan check for that case
local_wls_ignore_vlan_check = False

def GetProtocolDirectory(feature, proto):
    return api.GetTopologyDirectory() + "/gen/telemetry/{}/{}".format(feature, proto)

def GetTargetJsons(feature, proto):
    return glob.glob(GetProtocolDirectory(feature, proto) + "/*_policy.json")

def GetTargetVerifJsons(feature, proto):
    return glob.glob(GetProtocolDirectory(feature, proto) + "/*_verif.json")

def ReadJson(filename):
    return api.parser.JsonParse(filename)

def checkIfWorkloadsInSameSubnet(w1, w2):
    w1_sub = ipaddr.IPv4Interface(w1.ip_prefix)
    w2_sub = ipaddr.IPv4Interface(w2.ip_prefix)
    if w1_sub.network == w2_sub.network:
        #api.Logger.info("W1 IP: {} subnet: {}".format(w1_sub.ip, w1_sub.network))
        #api.Logger.info("W2 IP: {} subnet: {}".format(w2_sub.ip, w2_sub.network))
        return True
    return False

def checkCollectorIpInNaplesMgmtSubnet(naples_mgmt_ip_prefix, collector_ip):
    mgmt_sub = ipaddr.IPv4Interface(naples_mgmt_ip_prefix)
    coll_sub = ipaddr.IPv4Interface((collector_ip+"/"+(naples_mgmt_ip_prefix.split('/')[1])))
    if coll_sub.network == mgmt_sub.network:
        #api.Logger.info("Naples mgmt ip: {} subnet: {}".format(mgmt_sub.ip, mgmt_sub.network))
        #api.Logger.info("Collector ip: {} subnet: {}".format(coll_sub.ip, coll_sub.network))
        return True
    return False

def generateMirrorCollectorConfig(mirror_objects, num_collectors):
    api.Logger.info("Extending number of collectors to {}".format(num_collectors))

    for obj in mirror_objects:
        coll_len = len(obj.spec.collectors)
        #api.Logger.info("Existing collector list len {}".format(coll_len))
        for c in range(coll_len, num_collectors):
            tmp = copy.deepcopy(obj.spec.collectors[0])
            obj.spec.collectors.append(tmp)
            obj.spec.collectors[c].export_config.destination = "{}".format(obj.spec.collectors[0].export_config.destination)
            obj.spec.collectors[c].type = "{}".format(obj.spec.collectors[0].type)
            #api.Logger.info("updating collector count: {} dst: {} type: {}".format(c, 
            #            obj.spec.collectors[c].export_config.destination, obj.spec.collectors[c].type ))
    return api.types.status.SUCCESS


def generateMirrorSpecConfig(rule_id, wl1, wl2, proto, port, mirror_objects):
    for obj in mirror_objects:
        #api.Logger.info("proto: {} ".format(obj.spec.match_rules[0].destination.proto_ports))

        obj.meta.name = "test-mirror-{}".format(rule_id)

        obj.spec.match_rules[0].source.addresses = [wl1.ip_address]
        obj.spec.match_rules[0].destination.addresses = [wl2.ip_address]
        obj.spec.match_rules[0].destination.proto_ports[0].protocol = "{}".format(proto)
        obj.spec.match_rules[0].destination.proto_ports[0].port = "{}".format(port)
        obj.spec.match_rules[1].source.addresses = [wl2.ip_address]
        obj.spec.match_rules[1].destination.addresses = [wl1.ip_address]
        obj.spec.match_rules[1].destination.proto_ports[0].protocol = "{}".format(proto)
        obj.spec.match_rules[1].destination.proto_ports[0].port = "{}".format(port)
        #api.Logger.info("before updating match rule rule1 src: {} dst: {} rule2 src: {} dst: {}".format
        #        (obj.spec.match_rules[0].source.addresses,
        #         obj.spec.match_rules[0].destination.addresses,
        #         obj.spec.match_rules[1].source.addresses,
        #         obj.spec.match_rules[1].destination.addresses))

    return api.types.status.SUCCESS

def updateMirrorCollectorConfig(tc_workloads, num_collectors, local_wl, collector_wls_ip_dict, mirror_objects):
    #api.Logger.info("Updating Mirror config for {} collectors ".format(num_collectors))

    naples_mgmt_ip = common.PenctlGetNaplesMgtmIp(local_wl.node_name)
    #api.Logger.info("Naples Mgmt IP {}".format(naples_mgmt_ip))

    coll_wl_list = []
    coll_ip_list = []

    for wl in tc_workloads:
        #collector cannot be in local node
        if wl.node_name == local_wl.node_name:
            continue

        #collector IP dict for the workloads should not be empty
        if len(collector_wls_ip_dict[wl.workload_name]) == 0:
            continue

        for ip in collector_wls_ip_dict[wl.workload_name]:
            if checkCollectorIpInNaplesMgmtSubnet(naples_mgmt_ip, ip) == True:
                coll_wl_list.append(wl)
                coll_ip_list.append(ip)
                #api.Logger.info("Adding IP {} to collector ip list".format(ip))

    coll_ip_list_len = len(coll_ip_list)
    #api.Logger.info("Collector WL list-len: {} ip list-len: {}".format(len(coll_wl_list), coll_ip_list_len))

    if coll_ip_list_len == 0:
        return (coll_wl_list, coll_ip_list)

    if coll_ip_list_len < num_collectors:
        api.Logger.info("Number of collector IP's available {} in topology is less than the"
                        "number of collectors requested {} for this test, "
                        "reusing some collectors".format(coll_ip_list_len,num_collectors))

        #appending the list for resuing some of collector IP's to reach num_collectors
        idx = 0
        while coll_ip_list_len < num_collectors:
            coll_ip_list.append(coll_ip_list[idx])
            coll_wl_list.append(coll_wl_list[idx])
            idx += 1
            coll_ip_list_len += 1

    coll_ip_list_len = len(coll_ip_list)
    #api.Logger.info("Updated Collector (after reusing some) WL list-len: {} ip list-len: {}".format(len(coll_wl_list), coll_ip_list_len))

    for obj in mirror_objects:
        for c in range(0, num_collectors):
            obj.spec.collectors[c].export_config.destination = "{}".format(coll_ip_list[c])
            #api.Logger.info("updating collector idx: {} to dst: {} from collector_WL: {}".format(c,
            #            obj.spec.collectors[c].export_config.destination, coll_wl_list[c]))

    return (coll_wl_list, coll_ip_list)

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

def VerifyVlan(pcap_file_name):
    result = api.types.status.SUCCESS
    dir_path = os.path.dirname(os.path.realpath(__file__))
    mirrorscapy = dir_path + '/' + pcap_file_name
    api.Logger.info("File Name: %s" % (mirrorscapy))
    try:
        pkts = rdpcap(mirrorscapy)
    except Exception as e:
        api.Logger.error("VerifyVlan Failed: Exception {} in parsing pcap file."
                "Possibly file size is 0 bytes.".format(str(e)))
        return api.types.status.FAILURE
    spanpktsfound = False
    for pkt in pkts:
        if pkt.haslayer(ERSPAN_III):
            spanpktsfound = True
            if (uplink_vlan == 0 and pkt.haslayer(Dot1Q)):
                result = api.types.status.FAILURE
                api.Logger.error("Vlan verification Failed: uplink_vlan: {} Pkt Vlan: {} ".format(uplink_vlan, pkt[Dot1Q].vlan))
                pkt.show()
            elif ((local_wls_ignore_vlan_check == False) and pkt.haslayer(Dot1Q) and (pkt[Dot1Q].vlan != uplink_vlan)):
                result = api.types.status.FAILURE
                api.Logger.error("Vlan verfication Failed: uplink_vlan: {} Pkt Vlan id: {}".format(uplink_vlan, pkt[Dot1Q].vlan))
                pkt.show()
    if spanpktsfound == False:
       result = api.types.status.FAILURE
       api.Logger.info("VerifyVlan Failed: spanpkts not found ")
    return result

def VerifyTimeStamp(command, pcap_file_name):
    global g_time
    result = api.types.status.SUCCESS
    dir_path = os.path.dirname(os.path.realpath(__file__))
    mirrorscapy = dir_path + '/' + pcap_file_name
    api.Logger.info("File Name: %s" % (mirrorscapy))
    try:
        pkts = rdpcap(mirrorscapy)
    except Exception as e:
        api.Logger.error("VerifyTimeStamp Failed: Exception {} in parsing pcap file."
                "Possibly file size is 0 bytes.".format(str(e)))
        return api.types.status.FAILURE
    spanpktsfound = False
    g_time = datetime.fromtimestamp(time.clock_gettime(time.CLOCK_REALTIME))
    api.Logger.info("Current Global time {}".format(g_time))
    for pkt in pkts:
        if pkt.haslayer(ERSPAN_III):
            spanpktsfound = True
            l_ts = pkt[ERSPAN_III].timestamp
            u_ts = pkt[PlatformSpecific].timestamp
            pkttime = u_ts << 32 | l_ts
            api.Logger.info("Timestamp from the packet: %s" % (pkttime)) 
            pkttime /= 1000000000
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

def VerifyCmd(cmd, action, feature, pcap_file_name, export_cfg_port=2055):
    api.PrintCommandResults(cmd)
    result = api.types.status.SUCCESS
    if 'tcpdump' in cmd.command:
        if feature == 'mirror':
            if 'pcap' in cmd.command:
                if VerifyVlan(pcap_file_name) == api.types.status.FAILURE:
                    result = api.types.status.FAILURE
                    api.Logger.info("VerifyVlan failed")
                if VerifyTimeStamp(cmd, pcap_file_name) == api.types.status.FAILURE:
                    result = api.types.status.FAILURE
        elif feature == 'flowmon':
            search_str = r'(.*)%s: UDP, length(.*)'%export_cfg_port
            matchObj = re.search(search_str, cmd.stdout, 0)
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

def RunCmd(src_wl, protocol, dest_wl, destination_ip, destination_port, collector_count, collector_w, collector_ip, action, feature, is_wl_type_bm=False):
    backgroun_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    result = api.types.status.SUCCESS
    
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    # Add the ping commands from collector to source and dest workload
    # to avoid flooding on the vswitch
    api.Trigger_AddCommand(req, collector_w[0].node_name, collector_w[0].workload_name,
                           "ping -c 1 %s " % (src_wl.ip_address), timeout=2)
    api.Trigger_AddCommand(req, collector_w[0].node_name, collector_w[0].workload_name,
                           "ping -c 1 %s " % (destination_ip), timeout=2)
    trig_resp = api.Trigger(req)

    coll_idx = 0
    while coll_idx < collector_count:
        coll_wl = collector_w[coll_idx]
        coll_ip = collector_ip[coll_idx]

        if feature == 'mirror':
            api.Trigger_AddCommand(backgroun_req, coll_wl.node_name, coll_wl.workload_name,
                                   "tcpdump -c 10 -nnSXi %s  ip proto gre and dst %s -U -w mirror-%d.pcap" %
                                   (coll_wl.interface, coll_ip, coll_idx), background=True, timeout=20)
            #api.Trigger_AddCommand(backgroun_req, coll_wl.node_name, coll_wl.workload_name,
            #                       "tcpdump -c 10 -nni %s ip proto gre and dst %s" %
            #                       (coll_wl.interface, coll_ip), background=True, timeout=20)
        elif feature == 'flowmon':
            api.Trigger_AddCommand(backgroun_req, coll_wl.node_name, coll_wl.workload_name,
                                   "tcpdump -c 10 -nni %s udp and dst port %s and dst host %s"%
                                   (coll_wl.interface, coll_ip.proto_port.port, coll_ip.destination),
                                   background=True, timeout=20)
        coll_idx += 1

    background_trig_resp = api.Trigger(backgroun_req)

    #delay for background cmds to start before issuing ping
    if feature == 'flowmon':
        time.sleep(2)
    if feature == 'mirror':
        time.sleep(2)

    # Workaround for hping issue on BM workload over VLAN tagged sub-if,
    # when there are more than 16 interfaces in the system.
    # hping sends local host addr as source IP for hping on tagged sub-if;
    # hence using ping instead of hping on BM.
    if is_wl_type_bm:
        cmd = GetNpingCmd(protocol, destination_ip, destination_port)
    else:
        cmd = GetHping3Cmd(protocol, src_wl, destination_ip, destination_port)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddCommand(req, src_wl.node_name, src_wl.workload_name, cmd, timeout=3)
    api.Logger.info("Running from src_wl_ip {} COMMAND {}".format(src_wl.ip_address, cmd))

    trig_resp = api.Trigger(req)

    api.Logger.info("Trigger resp commands")
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

    if feature == 'flowmon':
        time.sleep(2)
    if feature == 'mirror':
        time.sleep(2)

    term_resp = api.Trigger_TerminateAllCommands(background_trig_resp)
    background_resp = api.Trigger_AggregateCommandsResponse(background_trig_resp, term_resp)

    #api.Logger.info("background resp commands")
    #for cmd in background_resp.commands:
    #    api.PrintCommandResults(cmd)

    dir_path = os.path.dirname(os.path.realpath(__file__))
    coll_idx = 0
    while coll_idx < collector_count:
        coll_wl = collector_w[coll_idx]
        coll_ip = collector_ip[coll_idx]
        pcap_file_name = None
        proto_port = None
        cmd_resp_idx = coll_idx
        if feature == 'mirror':
            pcap_file_name = ('mirror-%d.pcap'%coll_idx)
            api.CopyFromWorkload(coll_wl.node_name, coll_wl.workload_name, [pcap_file_name], dir_path)
            # For mirror feature, 2 commands are sent to WL.
            # Only the first command has tcpdump pcap file.
            cmd_resp_idx = coll_idx*1
        elif feature == 'flowmon':
            proto_port = coll_ip.proto_port.port

        cmd = background_resp.commands[cmd_resp_idx]
        result = VerifyCmd(cmd, action, feature, pcap_file_name, proto_port)
        if (result == api.types.status.FAILURE):
            api.Logger.info("Testcase FAILED!! cmd: {}".format(cmd))
            break
        coll_idx += 1

    return result

def setSourceWorkloadsUpLinkVlan(vlan):
    global uplink_vlan
    global local_wls_ignore_vlan_check
    uplink_vlan = vlan
    local_wls_ignore_vlan_check = False
    api.Logger.info("user config Uplink vlan: {} local_wls_ignore_vlan_check: {}".format(uplink_vlan,local_wls_ignore_vlan_check))
    return

def GetSourceWorkload(verif, tc):
    global uplink_vlan
    global local_wls_ignore_vlan_check
    workloads = api.GetWorkloads()
    sip = verif['src_ip']
    src_wl = None
    uplink_vlan = 0
    local_wls_ignore_vlan_check = False
    for wl in workloads:
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
    global local_wls_ignore_vlan_check
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

    # ignore wire encap vlan check in packet validation if src & dst are chosen from same node
    if dst_wl is not None and dst_wl.node_name == src_w.node_name:
        local_wls_ignore_vlan_check = True

    api.Logger.info("Uplink vlan: {} local_wls_ignore_vlan_check: {}".format(uplink_vlan,local_wls_ignore_vlan_check))
    return dst_wl

def RunAll(collector_w, verif_json, tc, feature, collector_ip, is_wl_type_bm=False):
    res = api.types.status.SUCCESS
    api.Logger.info("VERIFY JSON FILE {}".format(verif_json))

    verif = []
    ret = {}
    collector_count = len(collector_ip)

    with open(verif_json, 'r') as fp:
        verif = json.load(fp)

    api.Logger.info("VERIF = {}".format(verif))
    count = 0
    if feature == 'flowmon':
        time.sleep(4)
    for i in range(0, len(verif)):
        protocol = verif[i]['protocol'] 
        src_w = GetSourceWorkload(verif[i], tc)
        if src_w == None:
            continue
        dest_w = GetDestWorkload(verif[i], tc, src_w)
        if dest_w == None:
            continue
        # If both workload nodes are not naples, continue
        if not src_w.IsNaples():
            if not dest_w.IsNaples():
                continue
        if is_wl_type_bm:
            if (dest_w.node_name == src_w.node_name):
                api.Logger.info("Source and Dest workloads are same {}".format(dest_w.node_name))
                continue
        dest_port = GetDestPort(verif[i]['port'])
        action = verif[i]['result']
        res = RunCmd(src_w, protocol, dest_w, dest_w.ip_address, dest_port, collector_count, collector_w, collector_ip, action, feature, is_wl_type_bm)
        if (res == api.types.status.FAILURE):
            api.Logger.info("Testcase FAILED!!")
            break;
        count = count + 1
    ret['res'] = res
    ret['count'] = count
    return ret
