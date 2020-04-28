#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.api as cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.config.netagent.hw_sec_ip_config as sec_ip_api
import iota.test.iris.testcases.telemetry.utils as utils
import iota.test.iris.config.infra.main as cfg_main
from iota.test.iris.testcases.aging.aging_utils import *
import pdb
import json
import ipaddress
import copy
from scapy.utils import *
from scapy.utils import rdpcap
from scapy.utils import wrpcap
from scapy import packet
from scapy.all import Ether
from scapy.all import Dot1Q
from scapy.all import IP
from scapy.all import TCP
from scapy.all import UDP
from scapy.all import ICMP
from scapy.all import GRE
from scapy.contrib.erspan import ERSPAN_II, ERSPAN_III, ERSPAN_PlatformSpecific
from iota.test.utils.flowmon import *

IP_HEADER_LENGTH = 20
IP_PROTO_TCP     = 6
IP_PROTO_UDP     = 17
IP_PROTO_ICMP    = 1

DOT1Q_ETYPE      = 0x8100
GRE_ENCAP_LENGTH = 38

ICMP_ECHO         = 0x0800
ICMP_ECHO_REPLY   = 0x0000
ICMP_PORT_UNREACH = 0x0303

NUMBER_OF_IPFIX_PACKETS_PER_SESSION = 10

TCP_EXPORT_ENABLE_EXPECTED            = 2
UDP_EXPORT_ENABLE_EXPECTED            = 20
ICMP_EXPORT_ENABLE_EXPECTED           = 2

IPFIX_TEMPLATE_SET_ID = 0x0002
IPFIX_HEADER_LEN = 16

ERSPAN_COLLECTOR_MAX_IN_ESX = 2
FLOWMON_COLLECTOR_MAX       = 4

MIRROR_DIR_BOTH    = 0
MIRROR_DIR_INGRESS = 1
MIRROR_DIR_EGRESS  = 2

#
# Add a Command in the context of a Workload
#
def add_command(req, host, cmd, bg):
    api.Trigger_AddCommand(req, host.node_name, host.workload_name, cmd,
                           background = bg)

#
# Add a Command in the context of Naples
#
def add_naples_command(req, naples, cmd):
    api.Trigger_AddNaplesCommand(req, naples.node_name, cmd)

#
# Get a list of all the workloads in both the nodes
#
def establishWorkloads(tc):
    tc.workloads = api.GetWorkloads()
    if len(tc.workloads) == 0:
        api.Logger.error("ERROR: No workloads")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

#
# Identify Naples Workload which would be WUT (workload under test)
#
def establishNaplesWorkload(tc):
    tc.naples = None
    for wl in tc.workloads:
        if wl.IsNaples():
            if tc.iterators.vlan == 'tag' and wl.uplink_vlan != 0:
                tc.naples = wl
                break
            elif tc.iterators.vlan == 'native' and wl.uplink_vlan == 0:
                tc.naples = wl
                break

    if tc.naples is None:
        api.Logger.error("ERROR: No Naples workload")
        return api.types.status.FAILURE

    ip_mask = (0 - (1 << (32 - int(tc.naples.ip_prefix.split('/')[1]))))
    tc.naples_ip_prefix = int(ipaddress.ip_address(tc.naples.ip_address)) &\
                          ip_mask

    return api.types.status.SUCCESS

#
# Identify Remote Workloads relative to Naples-Workload, based on
# testbundle options, choose generate Remote-collector-list as needed
#
def establishRemoteWorkloads(tc):
    tc.remote_workloads = []
    tc.naples_peer = None
    tc.collector = []
    tc.bond0_collector_peer = None

    for wl in tc.workloads:
        if wl.node_name != tc.naples.node_name:
            tc.remote_workloads.append(wl)
            ip_mask = (0 - (1 << (32 - int(wl.ip_prefix.split('/')[1]))))
            wl_ip_prefix = int(ipaddress.ip_address(wl.ip_address)) & ip_mask
            if wl.uplink_vlan == tc.naples.uplink_vlan and\
               wl_ip_prefix == tc.naples_ip_prefix:
                if tc.iterators.peer == 'remote':
                    if tc.naples_peer is None:
                        tc.naples_peer = wl
            if wl.uplink_vlan == 0:
                if (tc.iterators.collector == 'remote' or\
                    tc.iterators.collector == 'all') and\
                    tc.classic_mode == False and\
                    len(tc.collector) < tc.iterators.ccount:
                    tc.collector.append(wl)
        else:
            if wl.uplink_vlan == 0:
                if (tc.iterators.collector == 'remote' or\
                    tc.iterators.collector == 'all') and\
                    tc.classic_mode == False:
                    tc.bond0_collector_peer = wl

    if len(tc.remote_workloads) == 0:
        api.Logger.error("ERROR: No Remote workload")
        return api.types.status.FAILURE
    if tc.iterators.peer == 'remote' and tc.naples_peer is None:
        api.Logger.error("ERROR: No Remote Naples-peer workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'remote' or tc.iterators.collector == 'all')\
        and tc.classic_mode == False and len(tc.collector) == 0:
        api.Logger.error("ERROR: No Remote Collector workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'remote' or tc.iterators.collector == 'all')\
        and tc.classic_mode == False and tc.bond0_collector_peer is None:
        api.Logger.error("ERROR: No Bond0 Collector Peer workload")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

#
# Identify Local Workloads relative to Naples-Workload, based on
# testbundle options, choose generate Local-collector-list as needed
#
def establishLocalWorkloads(tc):
    tc.local_workloads = []
    for wl in tc.workloads:
        if wl.node_name == tc.naples.node_name:
            tc.local_workloads.append(wl)
            ip_mask = (0 - (1 << (32 - int(wl.ip_prefix.split('/')[1]))))
            wl_ip_prefix = int(ipaddress.ip_address(wl.ip_address)) & ip_mask
            if wl.uplink_vlan == tc.naples.uplink_vlan and\
               wl_ip_prefix == tc.naples_ip_prefix:
                if tc.iterators.peer == 'local':
                    if tc.naples_peer is None and wl != tc.naples:
                        tc.naples_peer = wl
            if wl.uplink_vlan == 0:
                if (tc.iterators.collector == 'local' or\
                    tc.iterators.collector == 'all') and\
                    tc.classic_mode == False and\
                    len(tc.collector) < tc.iterators.ccount:
                    tc.collector.append(wl)
        else:
            if wl.uplink_vlan == 0:
                if (tc.iterators.collector == 'local' or\
                    tc.iterators.collector == 'all') and\
                    tc.classic_mode == False:
                    tc.bond0_collector_peer = wl

    if len(tc.local_workloads) == 0:
        api.Logger.error("ERROR: No Local workload")
        return api.types.status.FAILURE
    if tc.iterators.peer == 'local' and tc.naples_peer is None:
        api.Logger.error("ERROR: No Local Naples-peer workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'local' or tc.iterators.collector == 'all')\
        and tc.classic_mode == False and len(tc.collector) == 0:
        api.Logger.error("ERROR: No Local Collector workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'local' or tc.iterators.collector == 'all')\
        and tc.classic_mode == False and tc.bond0_collector_peer is None:
        api.Logger.error("ERROR: No Bond0 Collector Peer workload")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def updateErspanTypeOption(tc, c):
    if tc.iterators.erspan == 'type_2' or tc.iterators.erspan == 'type_3':
        tc.collector_erspan_type[c] = tc.iterators.erspan
    elif (c % 2) == 0:
        tc.collector_erspan_type[c] = 'type_2'
    else:
        tc.collector_erspan_type[c] = 'type_3'

def updateVlanStripOption(tc, c):
    if tc.iterators.vlan_strip == False or tc.iterators.vlan_strip == True:
        tc.collector_vlan_strip[c] = tc.iterators.vlan_strip
    elif (c % 2) == 0:
        tc.collector_vlan_strip[c] = False
    else:
        tc.collector_vlan_strip[c] = True

#
# Allocate necessary Secondary-IPs as needed for Multi-collector testing
#
def establishCollectorSecondaryIPs(tc):
    tc.collector_ip_address = []
    tc.collector_seq_num = []
    tc.collector_ipfix_records = []
    tc.collector_ipfix_pkts = []
    tc.collector_ipfix_template_pkts = []
    tc.collector_tcp_pkts = []
    tc.collector_udp_pkts = []
    tc.collector_icmp_pkts = []
    tc.collector_other_pkts = []
    tc.collector_erspan_type = []
    tc.collector_vlan_strip = []
    tc.result = []

    for c in range(0, len(tc.collector)):
        tc.collector_ip_address.append(tc.collector[c].ip_address)
        tc.collector_seq_num.append(0)
        tc.collector_ipfix_records.append(0)
        tc.collector_ipfix_pkts.append(0)
        tc.collector_ipfix_template_pkts.append(0)
        tc.collector_tcp_pkts.append(0)
        tc.collector_udp_pkts.append(0)
        tc.collector_icmp_pkts.append(0)
        tc.collector_other_pkts.append(0)
        tc.collector_erspan_type.append('type_3')
        updateErspanTypeOption(tc, c)
        tc.collector_vlan_strip.append(False)
        updateVlanStripOption(tc, c)
        tc.result.append(api.types.status.SUCCESS)

    tc.sec_ip_count = 0
    if tc.iterators.ccount <= len(tc.collector):
        return api.types.status.SUCCESS

    collector_count = len(tc.collector)
    sec_ip_count = tc.iterators.ccount - collector_count
    if tc.classic_mode == True:
        sec_ip_list = sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[0], 
                      True, sec_ip_count)
        if len(sec_ip_list) == 0:
            return api.types.status.FAILURE
        tc.sec_ip_count = sec_ip_count

        c = 0
        while c < sec_ip_count:
            tc.collector.append(tc.collector[0])
            tc.collector_ip_address.append(sec_ip_list[c])
            c += 1
    else:
        i = 0
        c = 0
        while i < sec_ip_count:
            #sec_ip_list = sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[c],
            #                                                   True, 1)
            #if len(sec_ip_list) == 0:
            #    return api.types.status.FAILURE
            tc.sec_ip_count += 1

            tc.collector.append(tc.collector[c])
            #tc.collector_ip_address.append(sec_ip_list[0])
            tc.collector_ip_address.append(tc.collector[c].ip_address)
            i += 1
            c += 1
            if c == collector_count:
                c = 0

    c = 0
    while c < tc.sec_ip_count:
        tc.collector_seq_num.append(0)
        tc.collector_ipfix_records.append(0)
        tc.collector_ipfix_pkts.append(0)
        tc.collector_ipfix_template_pkts.append(0)
        tc.collector_tcp_pkts.append(0)
        tc.collector_udp_pkts.append(0)
        tc.collector_icmp_pkts.append(0)
        tc.collector_other_pkts.append(0)
        tc.collector_erspan_type.append('type_3')
        updateErspanTypeOption(tc, c)
        tc.collector_vlan_strip.append(False)
        updateVlanStripOption(tc, c)
        tc.result.append(api.types.status.SUCCESS)
        c += 1

    for c in range(0, len(tc.collector)):
        updateErspanTypeOption(tc, c)
        updateVlanStripOption(tc, c)

    return api.types.status.SUCCESS

#
# De-Allocate applicable Secondary-IPs
#
def deEstablishCollectorSecondaryIPs(tc):
    if tc.sec_ip_count == 0:
        return api.types.status.SUCCESS

    if tc.classic_mode == True:
        sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[0], False,
                                             tc.sec_ip_count)
    #else:
    #    i = 0
    #    c = 0
    #    while c < tc.sec_ip_count:
    #        sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[c], False, 1)
    #        i += 1
    #        c += 1
    #        if c == len(tc.collector):
    #            c = 0

#
# Generate feature specific Collector list
#
def generateFeatureCollectorList(tc):
    tc.lif_collector = []
    tc.flow_collector = []
    tc.flowmon_collector = []

    tc.lif_collector_idx = []
    tc.flow_collector_idx = []
    tc.flowmon_collector_idx = []

    tc.collection = 'unified'
    collector_count = len(tc.collector)
    if tc.feature == 'lif-erspan':
        for c in range(0, len(tc.collector)):
            if tc.classic_mode == True or\
               len(tc.lif_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                tc.lif_collector.append(tc.collector[c])
                tc.lif_collector_idx.append(c)
    elif tc.feature == 'flow-erspan':
        for c in range(0, len(tc.collector)):
            if tc.classic_mode == True or\
               len(tc.flow_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                tc.flow_collector.append(tc.collector[c])
                tc.flow_collector_idx.append(c)
    elif tc.feature == 'flowmon':
        for c in range(0, len(tc.collector)):
            if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                tc.flowmon_collector.append(tc.collector[c])
                tc.flowmon_collector_idx.append(c)
    elif tc.feature == 'flow-erspan-flowmon':
        for c in range(0, len(tc.collector)):
            if tc.iterators.collection == 'unified' or collector_count < 2:
                if tc.classic_mode == True or\
                   len(tc.flow_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                    tc.flow_collector.append(tc.collector[c])
                    tc.flow_collector_idx.append(c)
                if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                    tc.flowmon_collector.append(tc.collector[c])
                    tc.flowmon_collector_idx.append(c)
            elif ((c % 2) == 0):
                tc.collection = 'distinct'
                if tc.classic_mode == True or c == 0:
                    tc.flow_collector.append(tc.collector[c])
                    tc.flow_collector_idx.append(c)
            else:
                tc.collection = 'distinct'
                if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                    tc.flowmon_collector.append(tc.collector[c])
                    tc.flowmon_collector_idx.append(c)
    elif tc.feature == 'lif-erspan-flowmon':
        for c in range(0, len(tc.collector)):
            if tc.iterators.collection == 'unified' or collector_count < 2:
                if tc.classic_mode == True or\
                   len(tc.lif_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                    tc.lif_collector.append(tc.collector[c])
                    tc.lif_collector_idx.append(c)
                if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                    tc.flowmon_collector.append(tc.collector[c])
                    tc.flowmon_collector_idx.append(c)
            elif ((c % 2) == 0):
                tc.collection = 'distinct'
                if tc.classic_mode == True or c == 0:
                    tc.lif_collector.append(tc.collector[c])
                    tc.lif_collector_idx.append(c)
            else:
                tc.collection = 'distinct'
                if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                    tc.flowmon_collector.append(tc.collector[c])
                    tc.flowmon_collector_idx.append(c)
    elif tc.feature == 'lif-flow-erspan':
        for c in range(0, len(tc.collector)):
            if tc.iterators.collection == 'unified' or collector_count < 2:
                if tc.classic_mode == True or\
                   len(tc.lif_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                    tc.lif_collector.append(tc.collector[c])
                    tc.lif_collector_idx.append(c)
                if tc.classic_mode == True or\
                   len(tc.flow_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                    tc.flow_collector.append(tc.collector[c])
                    tc.flow_collector_idx.append(c)
            elif ((c % 2) == 0):
                tc.collection = 'distinct'
                if tc.classic_mode == True or c == 0:
                    tc.lif_collector.append(tc.collector[c])
                    tc.lif_collector_idx.append(c)
            else:
                tc.collection = 'distinct'
                if tc.classic_mode == True or c == 1:
                    tc.flow_collector.append(tc.collector[c])
                    tc.flow_collector_idx.append(c)
    elif tc.feature == 'lif-flow-erspan-flowmon':
        for c in range(0, len(tc.collector)):
            if tc.iterators.collection == 'unified' or collector_count < 3:
                if tc.classic_mode == True or\
                   len(tc.lif_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                    tc.lif_collector.append(tc.collector[c])
                    tc.lif_collector_idx.append(c)
                if tc.classic_mode == True or\
                   len(tc.flow_collector) < ERSPAN_COLLECTOR_MAX_IN_ESX:
                    tc.flow_collector.append(tc.collector[c])
                    tc.flow_collector_idx.append(c)
                if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                    tc.flowmon_collector.append(tc.collector[c])
                    tc.flowmon_collector_idx.append(c)
            elif ((c % 3) == 0):
                tc.collection = 'distinct'
                if tc.classic_mode == True or c == 0:
                    tc.lif_collector.append(tc.collector[c])
                    tc.lif_collector_idx.append(c)
            elif ((c % 3) == 1):
                tc.collection = 'distinct'
                if tc.classic_mode == True or c == 1:
                    tc.flow_collector.append(tc.collector[c])
                    tc.flow_collector_idx.append(c)
            else:
                tc.collection = 'distinct'
                if len(tc.flowmon_collector) < FLOWMON_COLLECTOR_MAX:
                    tc.flowmon_collector.append(tc.collector[c])
                    tc.flowmon_collector_idx.append(c)

#
# Identify Collector Workload in Classic mode that is tied to Bond0 subnet
#
def establishCollectorWorkloadInClassicMode(tc, template_collector_ip):
    tc.bond0_collector = None
    if tc.iterators.collector == 'local':
        wl_pool = tc.local_workloads
        wl_peer_pool = tc.remote_workloads
    else:
        wl_pool = tc.remote_workloads
        wl_peer_pool = tc.local_workloads

    for wl in wl_pool:
        wl_ip_mask = (0 - (1 << (32 - int(wl.ip_prefix.split('/')[1]))))
        wl_ip_prefix = int(ipaddress.ip_address(wl.ip_address)) & wl_ip_mask
        col_ip_prefix = int(ipaddress.ip_address(template_collector_ip)) &\
                        wl_ip_mask
        if wl_ip_prefix == col_ip_prefix:
        #if wl.ip_address == template_collector_ip:
            tc.bond0_collector = wl
            break
    if tc.bond0_collector is None:
        api.Logger.error("ERROR: No Bond0 Collector workload")
        return api.types.status.FAILURE
    tc.collector.append(tc.bond0_collector)

    for wl in wl_peer_pool:
        wl_ip_mask = (0 - (1 << (32 - int(wl.ip_prefix.split('/')[1]))))
        wl_ip_prefix = int(ipaddress.ip_address(wl.ip_address)) & wl_ip_mask
        col_ip_prefix = int(ipaddress.ip_address(template_collector_ip)) &\
                        wl_ip_mask
        if wl_ip_prefix == col_ip_prefix:
        #if wl.ip_address == template_collector_ip:
            tc.bond0_collector_peer = wl
            break

    if tc.bond0_collector_peer is None:
        api.Logger.error("ERROR: No Bond0 Collector Peer workload")
        return api.types.status.FAILURE

    api.Logger.info("NEW COLLECTOR ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
    .format(tc.bond0_collector.workload_name,
            tc.bond0_collector.workload_type,
            tc.bond0_collector.workload_image,
            tc.bond0_collector.node_name,
            tc.bond0_collector.encap_vlan,
            tc.bond0_collector.ip_prefix,
            tc.bond0_collector.ip_address,
            tc.bond0_collector.mac_address,
            tc.bond0_collector.interface,
            tc.bond0_collector.parent_interface,
            tc.bond0_collector.interface_type,
            tc.bond0_collector.pinned_port,
            tc.bond0_collector.uplink_vlan))

    api.Logger.info(\
    "NEW COLLECTOR LOCAL ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
    .format(tc.bond0_collector_peer.workload_name,
            tc.bond0_collector_peer.workload_type,
            tc.bond0_collector_peer.workload_image,
            tc.bond0_collector_peer.node_name,
            tc.bond0_collector_peer.encap_vlan,
            tc.bond0_collector_peer.ip_prefix,
            tc.bond0_collector_peer.ip_address,
            tc.bond0_collector_peer.mac_address,
            tc.bond0_collector_peer.interface,
            tc.bond0_collector_peer.parent_interface,
            tc.bond0_collector_peer.interface_type,
            tc.bond0_collector_peer.pinned_port,
            tc.bond0_collector_peer.uplink_vlan))

    return api.types.status.SUCCESS

#
# Superimpose template-collector-config with Workload-IP-attributes
#
def generateLifCollectorConfig(tc, colObjects):
    i = 0
    for obj in colObjects:
        obj.meta.name = "lif-collector-{}".format(i)
        obj.spec.packet_size = tc.iterators.pktsize

        if tc.iterators.direction == 'ingress':
            obj.spec.mirror_direction = MIRROR_DIR_INGRESS
        elif tc.iterators.direction == 'egress':
            obj.spec.mirror_direction = MIRROR_DIR_EGRESS
        elif tc.iterators.direction == 'both':
            obj.spec.mirror_direction = MIRROR_DIR_BOTH

        for c in range(0, len(tc.lif_collector)):
            idx = tc.lif_collector_idx[c]
            if c != 0:
                tmp = copy.deepcopy(obj.spec.collectors[0])
                obj.spec.collectors.append(tmp)
            obj.spec.collectors[c].export_config.destination = \
                                   tc.collector_ip_address[idx]
            if tc.collector_erspan_type[idx] == 'type_2':
                obj.spec.collectors[c].type = 'erspan_type_2'
            elif tc.collector_erspan_type[idx] == 'type_3':
                obj.spec.collectors[c].type = 'erspan_type_3'
            obj.spec.collectors[c].strip_vlan_hdr = tc.collector_vlan_strip[idx]

        i += 1

    return api.types.status.SUCCESS

#
# Superimpose template-collector-config with Workload-IP-attributes
# Multiple InterfaceMirrorSessions case
#
def generateLifCollectorConfigForMultiMirrorSession(tc, colObjects):
    for c in range(0, len(tc.lif_collector)):
        idx = tc.lif_collector_idx[c]
        if c != 0:
            tmp = copy.deepcopy(colObjects[0])
            colObjects.append(tmp)
        colObjects[c].meta.name = "lif-collector-{}".format(c)
        colObjects[c].spec.packet_size = tc.iterators.pktsize
        if tc.iterators.direction == 'ingress':
            colObjects[c].spec.mirror_direction = MIRROR_DIR_INGRESS
        elif tc.iterators.direction == 'egress':
            colObjects[c].spec.mirror_direction = MIRROR_DIR_EGRESS
        elif tc.iterators.direction == 'both':
            colObjects[c].spec.mirror_direction = MIRROR_DIR_BOTH

        colObjects[c].spec.collectors[0].export_config.destination = \
                                         tc.collector_ip_address[idx]
        if tc.collector_erspan_type[idx] == 'type_2':
            colObjects[c].spec.collectors[0].type = 'erspan_type_2'
        elif tc.collector_erspan_type[idx] == 'type_3':
            colObjects[c].spec.collectors[0].type = 'erspan_type_3'
        colObjects[c].spec.collectors[0].strip_vlan_hdr = \
                                         tc.collector_vlan_strip[idx]

    return api.types.status.SUCCESS

def generateUplinkIntfCfgObj(node_name):
    cfgObjs = []
    uplinks = getUplinkInterfaceObj(node_name)
    for uplink in uplinks:
        del uplink['status']
        uplink["spec"]['TxCollectors'] = []
        uplink["spec"]['RxCollectors'] = []
        cfgObj = cfg_main.ConfigObject(uplink, "meta.tenant/meta.namespace/meta.name", "api/interfaces/")
        cfgObjs.append(cfgObj)
    return cfgObjs

def getUplinkInterfaceObj(node_name):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/usr/bin/curl localhost:9007/api/interfaces/"
    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    uplink = []
    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
        intfs = json.loads(cmd.stdout)
        for intf in intfs:
            if intf['spec']['type'] == 'UPLINK_ETH':
                uplink.append(intf)
    return uplink

#
# Retrieve applicable IfNames from Netagent IF objects
#
def getIfNames(tc, ifObjects):
    tc.if_name = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/usr/bin/curl localhost:9007/api/interfaces/ | jq . |\
           grep name | grep uplink"
    add_naples_command(req, tc.naples, cmd)

    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        for line in cmd.stdout.split('\n'):
            if 'uplink' in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        if_name_tmp = s.replace(',', '')
                        if_name = if_name_tmp.replace('"', '')
                        tc.if_name.append(if_name)
                        break
                    w += 1

    if len(tc.if_name) == 0 or len(tc.if_name) != len(ifObjects):
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

#
# Superimpose template-if-config with Netagent-objects' IfNames
#
def generateLifInterfaceConfig(tc, ifObjects, colObjects):
    result = getIfNames(tc, ifObjects)
    if result != api.types.status.SUCCESS:
        return result

    i = 0
    for obj in ifObjects:
        obj.meta.name = tc.if_name[i]

        for c in range(0, len(colObjects)):
            if c != 0:
                tmp = copy.deepcopy(obj.spec.mirror_sessions[0])
                obj.spec.mirror_sessions.append(tmp)
            obj.spec.mirror_sessions[c] = "default/default/{}"\
                     .format(colObjects[c].meta.name)

        i += 1

    return api.types.status.SUCCESS

#
# Superimpose template-if-config with Mirror-references removal
#
def deGenerateLifInterfaceConfig(tc, ifObjects, colObjects):
    for obj in ifObjects:
        for c in range(0, len(colObjects)):
            obj.spec.mirror_sessions.pop()

#
# Superimpose template-mirror-config with Workload-IP-attributes
#
def generateMirrorConfig(tc, policy_json, newObjects):
    for obj in newObjects:
        for i in range(0, len(obj.spec.match_rules)):
            if (i % 2) == 0:
                obj.spec.match_rules[i].source.addresses[0] = \
                                        tc.naples.ip_address
                if '/24' in obj.spec.match_rules[i].destination.addresses[0]:
                    obj.spec.match_rules[i].destination.addresses[0] = \
                    '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
                elif '/0' not in obj.spec.match_rules[i].destination.\
                                                         addresses[0]:
                    obj.spec.match_rules[i].destination.addresses[0] = \
                                            tc.naples_peer.ip_address
            else:
                obj.spec.match_rules[i].destination.addresses[0] = \
                                        tc.naples.ip_address
                if '/24' in obj.spec.match_rules[i].source.addresses[0]:
                    obj.spec.match_rules[i].source.addresses[0] = \
                    '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
                elif '/0' not in obj.spec.match_rules[i].source.addresses[0]:
                    obj.spec.match_rules[i].source.addresses[0] = \
                                            tc.naples_peer.ip_address

        obj.spec.packet_size = tc.iterators.pktsize
        for c in range(0, len(tc.flow_collector)):
            idx = tc.flow_collector_idx[c]
            if c != 0:
                tmp = copy.deepcopy(obj.spec.collectors[0])
                obj.spec.collectors.append(tmp)
            obj.spec.collectors[c].export_config.destination = \
                                   tc.collector_ip_address[idx]
            if tc.collector_erspan_type[idx] == 'type_2':
                obj.spec.collectors[c].type = 'erspan_type_2'
            elif tc.collector_erspan_type[idx] == 'type_3':
                obj.spec.collectors[c].type = 'erspan_type_3'
            obj.spec.collectors[c].strip_vlan_hdr = tc.collector_vlan_strip[idx]

    verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
    api.Logger.info("VERIFY JSON FILE {}".format(verif_json))
    tc.mirror_verif = []
    with open(verif_json, 'r') as fp:
        tc.mirror_verif = json.load(fp)

    for i in range(0, len(tc.mirror_verif)):
        if (i % 2) == 0:
            tc.mirror_verif[i]['src_ip'] = tc.naples.ip_address
            if '/24' in tc.mirror_verif[i]['dst_ip']:
                tc.mirror_verif[i]['dst_ip'] = \
                '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
            elif 'any' not in tc.mirror_verif[i]['dst_ip']:
                tc.mirror_verif[i]['dst_ip'] = tc.naples_peer.ip_address
        else:
            tc.mirror_verif[i]['dst_ip'] = tc.naples.ip_address
            if '/24' in tc.mirror_verif[i]['src_ip']:
                tc.mirror_verif[i]['src_ip'] = \
                '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
            elif 'any' not in tc.mirror_verif[i]['src_ip']:
                tc.mirror_verif[i]['src_ip'] = tc.naples_peer.ip_address
    api.Logger.info("VERIF = {}".format(tc.mirror_verif))

#
# Superimpose template-Flowmon-config with Workload-IP-attributes
#
def generateFlowMonConfig(tc, policy_json, newObjects):
    tc.export_port = []

    for obj in newObjects:
        for i in range(0, len(obj.spec.match_rules)):
            if (i % 2) == 0:
                obj.spec.match_rules[i].source.addresses[0] = \
                                        tc.naples.ip_address
                if '/24' in obj.spec.match_rules[i].destination.addresses[0]:
                    obj.spec.match_rules[i].destination.addresses[0] = \
                    '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
                elif '/0' not in obj.spec.match_rules[i].destination.\
                                                         addresses[0]:
                    obj.spec.match_rules[i].destination.addresses[0] = \
                                            tc.naples_peer.ip_address
            else:
                obj.spec.match_rules[i].destination.addresses[0] = \
                                        tc.naples.ip_address
                if '/24' in obj.spec.match_rules[i].source.addresses[0]:
                    obj.spec.match_rules[i].source.addresses[0] = \
                    '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
                elif '/0' not in obj.spec.match_rules[i].source.addresses[0]:
                    obj.spec.match_rules[i].source.addresses[0] = \
                                            tc.naples_peer.ip_address

        obj.spec.interval = tc.iterators.interval
        for c in range(0, len(tc.flowmon_collector)):
            idx = tc.flowmon_collector_idx[c]
            tc.export_port.append("400{}".format(c))
            if c != 0:
                tmp = copy.deepcopy(obj.spec.exports[0])
                obj.spec.exports.append(tmp)
            obj.spec.exports[c].destination = tc.collector_ip_address[idx]
            obj.spec.exports[c].proto_port.port = tc.export_port[c]

    verif_json = utils.GetVerifJsonFromPolicyJson(policy_json)
    api.Logger.info("VERIFY JSON FILE {}".format(verif_json))
    tc.flowmon_verif = []
    with open(verif_json, 'r') as fp:
        tc.flowmon_verif = json.load(fp)

    for i in range(0, len(tc.flowmon_verif)):
        if (i % 2) == 0:
            tc.flowmon_verif[i]['src_ip'] = tc.naples.ip_address
            if '/24' in tc.flowmon_verif[i]['dst_ip']:
                tc.flowmon_verif[i]['dst_ip'] = \
                '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
            elif 'any' not in tc.flowmon_verif[i]['dst_ip']:
                tc.flowmon_verif[i]['dst_ip'] = tc.naples_peer.ip_address
        else:
            tc.flowmon_verif[i]['dst_ip'] = tc.naples.ip_address
            if '/24' in tc.flowmon_verif[i]['src_ip']:
                tc.flowmon_verif[i]['src_ip'] = \
                '.'.join(tc.naples.ip_address.split('.')[:3]) + '.0/24'
            elif 'any' not in tc.flowmon_verif[i]['src_ip']:
                tc.flowmon_verif[i]['src_ip'] = tc.naples_peer.ip_address
    api.Logger.info("VERIF = {}".format(tc.flowmon_verif))

#
# Establish Forwarding set up between Naples-peer and Collectors
#
def establishForwardingSetup(tc):
    #
    # Make sure that Naples<=>Naples-peer Forwarding is set up
    #
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(tc.naples_peer.node_name) == 'linux':
        cmd = "ping -I %s -c1 %s" %\
              (tc.naples.interface, tc.naples_peer.ip_address)
    else:
        cmd = "ping -c1 %s" % (tc.naples_peer.ip_address)
    add_command(req, tc.naples, cmd, False)

    if api.GetNodeOs(tc.naples_peer.node_name) == 'linux':
        cmd = "ping -I %s -c1 %s" %\
              (tc.naples_peer.interface, tc.naples.ip_address)
    else:
        cmd = "ping -c1 %s" % (tc.naples.ip_address)
    add_command(req, tc.naples_peer, cmd, False)

    for c in range(0, len(tc.collector)):
        #
        # Make sure that Collector<=>Collector-Peer Forwarding is set up
        #
        cmd = "ping -c1 %s" % (tc.collector_ip_address[c])
        add_command(req, tc.bond0_collector_peer, cmd, False)

        cmd = "ping -c1 %s" % (tc.bond0_collector_peer.ip_address)
        add_command(req, tc.collector[c], cmd, False)

    #
    # Start with a clean slate by clearing all sessions/flows
    #
    cmd = "/nic/bin/halctl clear session"
    add_naples_command(req, tc.naples, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

#
# Use Applicable tools to trigger packets in Classic-mode
#
def triggerTrafficInClassicModeLinux(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # TCP
    #if tc.protocol == 'tcp' or tc.protocol == 'all':
    #    cmd = "nc -l {}".format(int(tc.dest_port))
    #    add_command(req, tc.naples_peer, cmd, False)
    #    cmd = "nc {} {} -p {} "\
    #          .format(tc.naples_peer.ip_address, int(tc.dest_port), 
    #                  int(tc.dest_port))
    #    add_command(req, tc.naples, cmd, False)

    # UDP
    if tc.protocol == 'udp' or tc.protocol == 'all':
        cmd = "nping --udp --dest-port {} --source-port {} --count {}\
               --interface {} --data-length {} {}"\
              .format(int(tc.dest_port), int(tc.dest_port), tc.udp_count,
                      tc.naples.interface, tc.iterators.pktsize*2, 
                      tc.naples_peer.ip_address)
        add_command(req, tc.naples, cmd, False)

        cmd = "nping --udp --dest-port {} --source-port {} --count {}\
               --interface {} --data-length {} {}"\
              .format(int(tc.dest_port), int(tc.dest_port), tc.udp_count,
                      tc.naples_peer.interface, tc.iterators.pktsize*2, 
                      tc.naples.ip_address)
        add_command(req, tc.naples_peer, cmd, False)

    # ICMP
    if tc.protocol == 'icmp' or tc.protocol == 'all':
        cmd = "nping --icmp --icmp-type 8 --count {} --interface {}\
               --data-length {} {}"\
              .format(tc.icmp_count, tc.naples.interface, 
                      tc.iterators.pktsize*2, tc.naples_peer.ip_address)
        add_command(req, tc.naples, cmd, False)

    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    #api.Trigger_TerminateAllCommands(trig_resp)

#
# Use Applicable tools to trigger packets in Hostpin-mode
#
def triggerTrafficInHostPinModeOrFreeBSD(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #
    # TCP-traffic is used only in useg_enf mode and during
    # sanity and regression runs to save time
    #
    # TCP
    if tc.protocol == 'tcp' or tc.protocol == 'all':
        if tc.classic_mode == False and tc.args.type != 'precheckin':
    #       cmd = "nc -l {}".format(int(tc.dest_port))
    #       add_command(req, tc.naples_peer, cmd, False)
    #       cmd = "nc {} {} -p {} "\
    #             .format(tc.naples_peer.ip_address, int(tc.dest_port), 
    #                     int(tc.dest_port))
    #       add_command(req, tc.naples, cmd, False)
    #    else:
            cmd = "nc --listen {}".format(int(tc.dest_port))
            add_command(req, tc.naples_peer, cmd, False)
            cmd = "nc {} {} --source-port {} "\
                  .format(tc.naples_peer.ip_address, int(tc.dest_port), 
                          int(tc.dest_port))
            add_command(req, tc.naples, cmd, False)

    # UDP
    if tc.protocol == 'udp' or tc.protocol == 'all':
        cmd = "hping3 --udp --destport {} --baseport {} --count {} {}\
               -I {} --data {}"\
              .format(int(tc.dest_port), int(tc.dest_port), tc.udp_count,
                      tc.naples_peer.ip_address, tc.naples.interface,
                      tc.iterators.pktsize*2)
        add_command(req, tc.naples, cmd, False)

        cmd = "hping3 --udp --destport {} --baseport {} --count {} {}\
               -I {} --data {}"\
              .format(int(tc.dest_port), int(tc.dest_port), tc.udp_count,
                      tc.naples.ip_address, tc.naples_peer.interface, 
                      tc.iterators.pktsize*2)
        add_command(req, tc.naples_peer, cmd, False)

    # ICMP
    if tc.protocol == 'icmp' or tc.protocol == 'all':
        cmd = "hping3 --icmp --count {} {} -I {} --data {}"\
              .format(tc.icmp_count, tc.naples_peer.ip_address, 
                      tc.naples.interface, tc.iterators.pktsize*2)
        add_command(req, tc.naples, cmd, False)

    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    #api.Trigger_TerminateAllCommands(trig_resp)

#
# Dump sessions/flows/P4-tables for debug purposes
#
def showSessionAndP4TablesForDebug(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/nic/bin/halctl show session"
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show session --yaml | grep mirrorsession: |\
           grep -v 0"
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show session --yaml | grep flowexportenablebitmap |\
           grep -v 0"
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0"\
           .format(tc.lif_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0"\
           .format(tc.omap_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0"\
           .format(tc.flow_info_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = '/nic/bin/halctl show table dump --table-id {} | \
           grep export_en | grep -v _en=0'.format(tc.flow_hash_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {}"\
           .format(tc.mirror_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {}"\
           .format(tc.tunnel_rewrite_table_id)
    add_naples_command(req, tc.naples, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

#
# Make sure that Mirror-config has been removed
#
def showP4TablesForValidation(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/nic/bin/halctl show session --yaml | grep mirrorsession: |\
           grep -v 0"
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show session --yaml | grep flowexportenablebitmap |\
           grep -v 0"
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0".format(tc.lif_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep irror_session_id | grep -v 0x0".format(tc.omap_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep rror_session_id | grep -v 0x0"\
           .format(tc.flow_info_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = '/nic/bin/halctl show table dump --table-id {} | \
           grep export_en | grep -v _en=0'.format(tc.flow_hash_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep MIRROR_ERSPAN_MIRROR_ID".format(tc.mirror_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep ip_da".format(tc.tunnel_rewrite_table_id)
    add_naples_command(req, tc.naples, cmd)

    resp_cleanup = api.Trigger(req)

    return resp_cleanup

#
# Validate function for sanity-checking IP-protocol embedded in the packet
# being validated
#
def validate_ip_proto(tc, ip_proto):
    if ip_proto == IP_PROTO_TCP:
        if tc.protocol == 'tcp' or tc.protocol == 'all':
            return api.types.status.SUCCESS
    elif ip_proto == IP_PROTO_UDP:
        if tc.protocol == 'udp' or tc.protocol == 'all':
            return api.types.status.SUCCESS
    elif ip_proto == IP_PROTO_ICMP:
        if tc.protocol == 'icmp' or tc.protocol == 'all' or\
           tc.iterators.proto == 'mixed':
            return api.types.status.SUCCESS

    api.Logger.error("ERROR: IP-Prococol {} {}".format(tc.protocol, ip_proto))
    return api.types.status.FAILURE

#
# Validate VLAN-tag
#
def validate_vlan_tag(tc, tag_etype, vlan_tag, idx):
    result = api.types.status.SUCCESS

    if tc.collector_vlan_strip[idx] == True:
        if tag_etype == DOT1Q_ETYPE:
            result = api.types.status.FAILURE
    else:
        if tag_etype == DOT1Q_ETYPE:
            if tc.naples.uplink_vlan == 0 or\
               vlan_tag != int(tc.naples.uplink_vlan):
                result = api.types.status.FAILURE
        elif tc.naples.uplink_vlan != 0:
            result = api.types.status.FAILURE

    if result == api.types.status.FAILURE:
        api.Logger.error("ERROR: VLAN-Tag {} {} {}".format(tag_etype, vlan_tag,
                        int(tc.naples.uplink_vlan)))

    return result

#
# Validate function for sanity-checking SIP/DIP/SPORT/DPORT embedded in the 
# packet being validated
#
def validate_ip_tuple(tc, sip, dip, sport, dport, tag_etype, vlan_tag, 
                      ip_proto, c, idx):

    result = api.types.status.SUCCESS
    if sip == int(ipaddress.ip_address(tc.naples.ip_address)) and\
       dip == int(ipaddress.ip_address(tc.naples_peer.ip_address)):
        if ip_proto != IP_PROTO_ICMP and dport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif tc.feature != 'flowmon':
            if ip_proto == IP_PROTO_ICMP and sport != ICMP_ECHO and\
               sport != ICMP_ECHO_REPLY and sport != ICMP_PORT_UNREACH:
                result = api.types.status.FAILURE
            else:
                result = validate_vlan_tag(tc, tag_etype, vlan_tag, idx)
    elif sip == int(ipaddress.ip_address(tc.naples_peer.ip_address)) and\
         dip == int(ipaddress.ip_address(tc.naples.ip_address)):
        if ip_proto == IP_PROTO_TCP and sport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif ip_proto == IP_PROTO_UDP and dport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif tc.feature != 'flowmon':
            if ip_proto == IP_PROTO_ICMP and sport != ICMP_ECHO and\
               sport != ICMP_ECHO_REPLY and sport != ICMP_PORT_UNREACH:
                result = api.types.status.FAILURE
            else:
                result = validate_vlan_tag(tc, tag_etype, vlan_tag, idx)
    elif tc.feature == 'lif-erspan':
        if ip_proto == IP_PROTO_UDP:
            tc.collector_udp_pkts[c] -= 1
            tc.collector_other_pkts[c] += 1
        elif ip_proto == IP_PROTO_ICMP:
            tc.collector_icmp_pkts[c] -= 1
            tc.collector_other_pkts[c] += 1
    else:
        result = api.types.status.FAILURE

    if result == api.types.status.FAILURE:
        api.Logger.error("ERROR: SIP/DIP {} {} {} {} {} {} {} {} {} {}"\
        .format(int(ipaddress.ip_address(tc.naples.ip_address)),
                int(ipaddress.ip_address(tc.naples_peer.ip_address)), sip, dip,
                ip_proto, sport, dport, tc.dest_port, tag_etype, vlan_tag))

    return result

#
# Validate ERSPAN packets reception
#
def validateErspanPackets(tc, lif_flow_collector, lif_flow_collector_idx):

    result = api.types.status.SUCCESS
    for c in range(0, len(lif_flow_collector)):
        idx = lif_flow_collector_idx[c]
        tc.collector_tcp_pkts[c] = 0
        tc.collector_udp_pkts[c] = 0
        tc.collector_icmp_pkts[c] = 0
        tc.collector_other_pkts[c] = 0
        tc.result[c] = api.types.status.SUCCESS

        # print command
        cmd = tc.resp_tcpdump_erspan.commands[c]
        api.PrintCommandResults(cmd)
        pkts_rcvd = 0
        for line in cmd.stderr.split('\n'):
            if 'packets received by filter' in line:
                for s in line.split():
                    pkts_rcvd = int(s, 10)
                    break
                break

        # Read pcap file
        if tc.feature == 'lif-erspan':
            pcap_file_name = ('lif-mirror-%d.pcap'%c)
        elif tc.feature == 'flow-erspan':
            pcap_file_name = ('flow-mirror-%d.pcap'%c)
        dir_path = os.path.dirname(os.path.realpath(__file__))
        api.CopyFromWorkload(lif_flow_collector[c].node_name, 
            lif_flow_collector[c].workload_name, [pcap_file_name], dir_path)

        mirrorscapy = dir_path + '/' + pcap_file_name
        api.Logger.info("File Name: %s" % (mirrorscapy))
        try:
            pkts = rdpcap(mirrorscapy)
        except Exception as e:
            api.Logger.error("ERROR: Exception {} in parsing pcap file."\
                             .format(str(e)))
            return api.types.status.FAILURE

        # Parse pkts in pcap file
        pkt_count = 0
        seq_num_error = False
        for pkt in pkts:
            #
            # Occasionally, pkts are appended to pcap file, the following
            # check prevents processing of stale pkts
            #
            if pkts_rcvd != 0 and pkt_count == pkts_rcvd:
                break
            pkt_count += 1

            if pkt.haslayer(IP):
                # Collector-IP validation
                collector = pkt[IP].dst
                if collector != tc.collector_ip_address[idx]:
                    api.Logger.error("ERROR: Collector-ip {} {}"\
                               .format(collector, tc.collector_ip_address[idx]))
                    tc.result[c] = api.types.status.FAILURE
                    break

                # ERSPAN-Pkt-Size validation
                iplen = pkt[IP].len
                pkt_size = iplen - IP_HEADER_LENGTH - GRE_ENCAP_LENGTH
                if (pkt_size - GRE_ENCAP_LENGTH) > tc.iterators.pktsize:
                    api.Logger.error("ERROR: Packet-Size {} {}"\
                                     .format(pkt_size, tc.iterators.pktsize))
                    tc.result[c] = api.types.status.FAILURE

            # GRE-Sequence-number validation (errors are ignored in
            # in classic-mode until code-fix is in)
            if pkt.haslayer(GRE):
                if pkt[GRE].seqnum_present == 1:
                    curr_seq_num = pkt[GRE].seqence_number
                    if pkt_count > 1 and\
                       (curr_seq_num - tc.collector_seq_num[c]) != 1:
                        api.Logger.error(\
                        "ERROR: [IGNORE] GRE Seq-num seen: {} expected: {}"\
                        .format(curr_seq_num, tc.collector_seq_num[c]+1))
                        seq_num_error = True
                        if tc.classic_mode == False and\
                           tc.args.type == 'regression':
                            tc.result[c] = api.types.status.FAILURE
                    tc.collector_seq_num[c] = curr_seq_num

            # ERSPAN-pkt validation
            ip_proto = 0
            tag_etype = 0
            vlan_tag = 0
            sport = 0
            dport = 0
            if tc.collector_erspan_type[idx] == 'type_3' and\
               pkt.haslayer(ERSPAN_III):
                # Extract Vlan-tag, if present
                if pkt[ERSPAN_III].haslayer(Dot1Q):
                    if tc.collector_vlan_strip[idx] == True and\
                       ((pkt[ERSPAN_III].haslayer(ERSPAN_III) == False and\
                         pkt[ERSPAN_III].haslayer(ERSPAN_II) == False) or\
                        (pkt[ERSPAN_III].haslayer(ERSPAN_III) == True and\
                         pkt[ERSPAN_III][ERSPAN_III].haslayer(Dot1Q) ==\
                         False) or\
                        (pkt[ERSPAN_III].haslayer(ERSPAN_II) == True and\
                         pkt[ERSPAN_III][ERSPAN_II].haslayer(Dot1Q) == False)):
                        api.Logger.error(\
                        "ERROR: Tagged ERSPAN-Type-3 Packet in Vlan-Strip mode")
                        tc.result[c] = api.types.status.FAILURE
                    tag_etype = DOT1Q_ETYPE
                    vlan_tag = pkt[ERSPAN_III][Dot1Q].vlan

                if pkt[ERSPAN_III].haslayer(IP):
                    # Extract IP-Protocol from inner-IP-header
                    ip_proto = int(pkt[ERSPAN_III][IP].proto)

                    # Extract IP-addresses from inner-IP-header
                    sip = int(ipaddress.ip_address(pkt[ERSPAN_III][IP].src))
                    dip = int(ipaddress.ip_address(pkt[ERSPAN_III][IP].dst))

                    # Extract L4-ports from inner-L4-header
                    if ip_proto == IP_PROTO_TCP and\
                       pkt[ERSPAN_III].haslayer(TCP):
                        sport = int(pkt[ERSPAN_III][TCP].sport)
                        dport = int(pkt[ERSPAN_III][TCP].dport)
                    elif ip_proto == IP_PROTO_UDP and\
                         pkt[ERSPAN_III].haslayer(UDP):
                        sport = int(pkt[ERSPAN_III][UDP].sport)
                        dport = int(pkt[ERSPAN_III][UDP].dport)
                    elif ip_proto == IP_PROTO_ICMP and\
                         pkt[ERSPAN_III].haslayer(ICMP):
                        sport = (int(pkt[ERSPAN_III][ICMP].type) << 8) |\
                                     int(pkt[ERSPAN_III][ICMP].code)
                elif tc.feature == 'flow-erspan':
                    api.Logger.error("ERROR: IP-Header Not Present")
                    tc.result[c] = api.types.status.FAILURE
            elif tc.collector_erspan_type[idx] == 'type_2' and\
                 pkt.haslayer(ERSPAN_II):
                # Extract Vlan-tag, if present
                if pkt[ERSPAN_II].haslayer(Dot1Q):
                    if tc.collector_vlan_strip[idx] == True and\
                       ((pkt[ERSPAN_II].haslayer(ERSPAN_II) == False and\
                         pkt[ERSPAN_II].haslayer(ERSPAN_III) == False) or\
                        (pkt[ERSPAN_II].haslayer(ERSPAN_II) == True and\
                         pkt[ERSPAN_II][ERSPAN_II].haslayer(Dot1Q) == False) or\
                        (pkt[ERSPAN_II].haslayer(ERSPAN_III) == True and\
                         pkt[ERSPAN_II][ERSPAN_III].haslayer(Dot1Q) == False)):
                        api.Logger.error(\
                        "ERROR: Tagged ERSPAN-Type-2 Packet in Vlan-Strip mode")
                        tc.result[c] = api.types.status.FAILURE
                    tag_etype = DOT1Q_ETYPE
                    vlan_tag = pkt[ERSPAN_II][Dot1Q].vlan

                if pkt[ERSPAN_II].haslayer(IP):
                    # Extract IP-Protocol from inner-IP-header
                    ip_proto = int(pkt[ERSPAN_II][IP].proto)

                    # Extract IP-addresses from inner-IP-header
                    sip = int(ipaddress.ip_address(pkt[ERSPAN_II][IP].src))
                    dip = int(ipaddress.ip_address(pkt[ERSPAN_II][IP].dst))

                    # Extract L4-ports from inner-L4-header
                    if ip_proto == IP_PROTO_TCP and\
                       pkt[ERSPAN_II].haslayer(TCP):
                        sport = int(pkt[ERSPAN_II][TCP].sport)
                        dport = int(pkt[ERSPAN_II][TCP].dport)
                    elif ip_proto == IP_PROTO_UDP and\
                         pkt[ERSPAN_II].haslayer(UDP):
                        sport = int(pkt[ERSPAN_II][UDP].sport)
                        dport = int(pkt[ERSPAN_II][UDP].dport)
                    elif ip_proto == IP_PROTO_ICMP and\
                         pkt[ERSPAN_II].haslayer(ICMP):
                        sport = (int(pkt[ERSPAN_II][ICMP].type) << 8) |\
                                 int(pkt[ERSPAN_II][ICMP].code)
                elif tc.feature == 'flow-erspan':
                    api.Logger.error("ERROR: IP-Header Not Present")
                    tc.result[c] = api.types.status.FAILURE
            #else:
            #    api.Logger.error("ERROR: Expected ERSPAN-Header Not Present")
            #    tc.result[c] = api.types.status.FAILURE

            if ip_proto == IP_PROTO_TCP:
                tc.collector_tcp_pkts[c] += 1
            elif ip_proto == IP_PROTO_UDP:
                tc.collector_udp_pkts[c] += 1
            elif ip_proto == IP_PROTO_ICMP:
                tc.collector_icmp_pkts[c] += 1
            else:
                ip_proto = 0
                tc.collector_other_pkts[c] += 1
                if tc.feature == 'flow-erspan':
                    tc.result[c] = api.types.status.FAILURE

            if ip_proto != 0:
                # Validate IP-tuple from inner-IP-header
                res = validate_ip_tuple(tc, sip, dip, sport, dport, 
                      tag_etype, vlan_tag, ip_proto, c, idx)
                if res != api.types.status.SUCCESS:
                    tc.result[c] = api.types.status.FAILURE

        #
        # Validate Number-of-ERSPAN-pkts received by the Collector
        #
        # Perform TCP-pkt checks (only for hostpin mode, for now)
        #
        pkt_count_error = False
        if tc.classic_mode == False and tc.args.type != 'precheckin':
            if tc.protocol == 'tcp' or tc.protocol == 'all':
                if tc.collector_tcp_pkts[c] < tc.tcp_erspan_pkts_expected or\
                   tc.collector_tcp_pkts[c] > (tc.tcp_erspan_pkts_expected+1):
                    api.Logger.error(\
                    "ERROR TCP: [IGNORE] {} {} ERSPAN packets to {}"\
                    .format(tc.collector_tcp_pkts[c],
                            tc.tcp_erspan_pkts_expected,
                            tc.collector_ip_address[idx]))
                    pkt_count_error = True
                    if tc.args.pkt_count == 'check':
                        tc.result[c] = api.types.status.FAILURE

        #
        # Perform UDP-pkt checks
        #
        if tc.protocol == 'udp' or tc.protocol == 'udp-mixed' or\
           tc.protocol == 'all':
            if tc.collector_udp_pkts[c] != tc.udp_erspan_pkts_expected:
                api.Logger.error(\
                "ERROR UDP: [IGNORE] {} {} ERSPAN packets to {}"\
                .format(tc.collector_udp_pkts[c],
                        tc.udp_erspan_pkts_expected,
                        tc.collector_ip_address[idx]))
                pkt_count_error = True
                if tc.args.pkt_count == 'check' or\
                   api.GetNodeOs(tc.naples_peer.node_name) != 'esx':
                    tc.result[c] = api.types.status.FAILURE

        #
        # Perform ICMP-pkt checks
        #
        if tc.protocol == 'icmp' or tc.protocol == 'udp-mixed' or\
           tc.protocol == 'all':
            if tc.collector_icmp_pkts[c] != tc.icmp_erspan_pkts_expected:
                api.Logger.error(\
                "ERROR ICMP: [IGNORE] {} {} ERSPAN packets to {}"\
                .format(tc.collector_icmp_pkts[c],
                        tc.icmp_erspan_pkts_expected,
                        tc.collector_ip_address[idx]))
                pkt_count_error = True
                if tc.args.pkt_count == 'check' or\
                   api.GetNodeOs(tc.naples_peer.node_name) != 'esx':
                    tc.result[c] = api.types.status.FAILURE

        if tc.collector_tcp_pkts[c] == 0 and tc.collector_udp_pkts[c] == 0 and\
           tc.collector_icmp_pkts[c] == 0:
            api.Logger.error("ERROR: No ERSPAN packets to {}"\
                             .format(tc.collector_ip_address[idx]))
            result = api.types.status.FAILURE

        # For failed cases, print pkts for debug
        if tc.result[c] == api.types.status.FAILURE or\
           seq_num_error == True or pkt_count_error == True:
            if tc.result[c] == api.types.status.FAILURE:
                result = api.types.status.FAILURE
            for pkt in pkts:
                pkt.show()

    for c in range(0, len(lif_flow_collector)):
        idx = lif_flow_collector_idx[c]
        if tc.result[c] == api.types.status.FAILURE:
            api.Logger.error("ERROR: {} {} {} {} {} {} {} ERSPAN_{}\
                              packets to {} (Vlan-Strip {})"\
            .format(tc.collector_tcp_pkts[c], tc.collector_udp_pkts[c],
                    tc.collector_icmp_pkts[c], tc.collector_other_pkts[c],
                    tc.tcp_erspan_pkts_expected, tc.udp_erspan_pkts_expected,
                    tc.icmp_erspan_pkts_expected, tc.collector_erspan_type[idx],
                    tc.collector_ip_address[idx], tc.collector_vlan_strip[idx]))
            result = api.types.status.FAILURE
        else:
            api.Logger.info("Number of ERSPAN_{} packets {} {} {} {} to {}\
                             (Vlan-Strip {})"\
            .format(tc.collector_erspan_type[idx], tc.collector_tcp_pkts[c],
                    tc.collector_udp_pkts[c], tc.collector_icmp_pkts[c],
                    tc.collector_other_pkts[c], tc.collector_ip_address[idx],
                    tc.collector_vlan_strip[idx]))

    return result

#
# Validate IPFIX packets reception
#
def validateIpFixPackets(tc):
    result = api.types.status.SUCCESS
    for c in range(0, len(tc.flowmon_collector)):
        idx = tc.flowmon_collector_idx[c]
        tc.collector_ipfix_records[c] = 0
        tc.collector_ipfix_pkts[c] = 0
        tc.collector_ipfix_template_pkts[c] = 0
        tc.result[c] = api.types.status.SUCCESS

        # print command
        cmd = tc.resp_tcpdump_flowmon.commands[c]
        api.PrintCommandResults(cmd)
        pkts_rcvd = 0
        for line in cmd.stderr.split('\n'):
            if 'packets received by filter' in line:
                for s in line.split():
                    pkts_rcvd = int(s, 10)
                    break
                break

        # Read pcap file
        pcap_file_name = ('flowmon-%d.pcap'%c)
        dir_path = os.path.dirname(os.path.realpath(__file__))
        api.CopyFromWorkload(tc.flowmon_collector[c].node_name,
            tc.flowmon_collector[c].workload_name, [pcap_file_name], dir_path)

        flowmonscapy = dir_path + '/' + pcap_file_name
        api.Logger.info("File Name: %s" % (flowmonscapy))
        try:
            pkts = rdpcap(flowmonscapy)
        except Exception as e:
            api.Logger.error("ERROR: Exception {} in parsing pcap file."\
                             .format(str(e)))
            return api.types.status.FAILURE

        # Parse pkts in pcap file
        pkt_count = 0
        seq_num_error = False
        for pkt in pkts:
            #
            # Occasionally, pkts are appended to pcap file, the following
            # check prevents processing of stale pkts
            #
            if pkts_rcvd != 0 and pkt_count == pkts_rcvd:
                break
            pkt_count += 1

            if pkt.haslayer(IP):
                # Collector-IP validation
                collector = pkt[IP].dst
                if collector != tc.collector_ip_address[idx]:
                    api.Logger.error("ERROR: Collector-ip {} {}"\
                               .format(collector, tc.collector_ip_address[idx]))
                    tc.result[c] = api.types.status.FAILURE
                    break

            # Parse and validate IPFIX packet
            if pkt.haslayer(Ipfix):
                set_id = pkt[Ipfix].records[0].set_id
                if set_id == IPFIX_TEMPLATE_SET_ID:
                    tc.collector_ipfix_template_pkts[c] += 1
                    continue

                # IPFIX-Sequence-number validation (errors are ignored in
                # in classic-mode until code-fix is in)
                tc.collector_ipfix_pkts[c] += 1

                curr_seq_num = pkt[Ipfix].seq_num
                if tc.collector_ipfix_pkts[c] > 1 and\
                   (curr_seq_num - tc.collector_seq_num[c]) != 1:
                    api.Logger.error(\
                    "ERROR: [IGNORE] IPFIX Seq-num seen: {} expected: {}"\
                    .format(curr_seq_num, tc.collector_seq_num[c]+1))
                    seq_num_error = True
                    if tc.classic_mode == False and\
                       tc.args.type == 'regression':
                        tc.result[c] = api.types.status.FAILURE
                tc.collector_seq_num[c] = curr_seq_num

                # Validate IPFIX-records
                total_len = pkt[Ipfix].len - IPFIX_HEADER_LEN
                record_len = pkt[Ipfix].records[0].len

                i = 0
                while (total_len > 0):
                    tc.collector_ipfix_records[c] += 1

                    # Extract IP-tuple from IPFIX-record
                    ip_proto = int(pkt[Ipfix].records[0][i].proto)
                    sip = int(ipaddress.ip_address(pkt[Ipfix].records[0][i].
                                                   ip_sa))
                    dip = int(ipaddress.ip_address(pkt[Ipfix].records[0][i].
                                                   ip_da))
                    sport = int(pkt[Ipfix].records[0][i].sport)
                    dport = int(pkt[Ipfix].records[0][i].dport)

                    # Validate IP-tuple from inner-IP-header
                    res = validate_ip_proto(tc, ip_proto)
                    if res != api.types.status.SUCCESS:
                        tc.result[c] = api.types.status.FAILURE

                    res = validate_ip_tuple(tc, sip, dip, sport, dport, 0, 0, 
                                            ip_proto, c, idx)
                    if res != api.types.status.SUCCESS:
                        tc.result[c] = api.types.status.FAILURE

                    total_len -= record_len
                    i += 2

        # For failed cases, print pkts for debug
        if tc.result[c] == api.types.status.FAILURE or seq_num_error == True\
           or tc.collector_ipfix_pkts[c] == 0:
            if tc.result[c] == api.types.status.FAILURE:
                result = api.types.status.FAILURE
            for pkt in pkts:
                pkt.show()

    #
    # Validate Number-of-IPFIX-pkts received by the 
    # Collector(s)
    #
    for c in range(0, len(tc.flowmon_collector)):
        idx = tc.flowmon_collector_idx[c]
        if tc.collector_ipfix_pkts[c] == 0:
            result = api.types.status.FAILURE

            api.Logger.error("ERROR: Number of IPFIX Template packets {} to {}"\
            .format(tc.collector_ipfix_template_pkts[c],
                    tc.collector_ip_address[idx]))
            api.Logger.error("ERROR: Number of IPFIX packets {} to {}"\
            .format(tc.collector_ipfix_pkts[c], tc.collector_ip_address[idx]))
            api.Logger.error("ERROR: Number of IPFIX records {} to {}"\
            .format(tc.collector_ipfix_records[c],tc.collector_ip_address[idx]))
        else:
            api.Logger.info("Number of IPFIX Template packets {} to {}"\
            .format(tc.collector_ipfix_template_pkts[c],
                    tc.collector_ip_address[idx]))
            api.Logger.info("Number of IPFIX packets {} to {}"\
            .format(tc.collector_ipfix_pkts[c], tc.collector_ip_address[idx]))
            api.Logger.info("Number of IPFIX records {} to {}"\
            .format(tc.collector_ipfix_records[c],tc.collector_ip_address[idx]))

    return result

def dumpP4TableForDebug(tc, table_id):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/nic/bin/halctl show table dump --table-id {}".format(table_id)
    add_naples_command(req, tc.naples, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

def dumpSessionTableForDebug(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/nic/bin/halctl show session --yaml"
    add_naples_command(req, tc.naples, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

#
# Validate Config-cleanup
#
def validateConfigCleanup(tc):
    result = api.types.status.SUCCESS
    for cmd in tc.resp_cleanup.commands:
        api.PrintCommandResults(cmd)
        if cmd.stdout != '':
            #
            # Ignore halctl cores in non-regression runs
            # For now, ignore halctl cores unconditionally
            # until it is fixed
            #
            if tc.args.type != 'regression' or tc.args.type == 'regression':
                halctl_core = False
                for line in cmd.stdout.split('\n'):
                    if 'Segmentation' in line or 'segmentation' in line:
                        halctl_core = True
                        break
                if halctl_core == True:
                    continue

            if 'table-id {}'.format(tc.lif_table_id) in cmd.command and\
               'grep mirror_session_id' in cmd.command:
                api.Logger.error("ERROR: PD-lif-config Not Removed")
                dumpP4TableForDebug(tc, tc.lif_table_id)
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.omap_table_id) in cmd.command and\
                 'grep irror_session_id' in cmd.command:
                api.Logger.error("ERROR: PD-omap-config Not Removed")
                dumpP4TableForDebug(tc, tc.omap_table_id)
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.flow_info_table_id) in cmd.command and\
                 'grep rror_session_id' in cmd.command:
                api.Logger.error("ERROR: PD-Flow-Info-config Not Removed")
                dumpP4TableForDebug(tc, tc.flow_info_table_id)
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.flow_hash_table_id) in cmd.command and\
                 'export_en' in cmd.command:
                api.Logger.error("ERROR: PD-Flow-Hash-config Not Removed")
                dumpP4TableForDebug(tc, tc.flow_hash_table_id)
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.mirror_table_id) in cmd.command and\
                 'MIRROR_ERSPAN_MIRROR_ID' in cmd.command:
                api.Logger.error("ERROR: PD-Mirror-config Not Removed")
                dumpP4TableForDebug(tc, tc.mirror_table_id)
                result = api.types.status.FAILURE
            elif 'table-id {}'\
                 .format(tc.tunnel_rewrite_table_id) in cmd.command and\
                 'ip_da' in cmd.command:
                api.Logger.error("ERROR: PD-Tunnel-config Not Removed")
                dumpP4TableForDebug(tc, tc.tunnel_rewrite_table_id)
                result = api.types.status.FAILURE
            elif 'mirrorsession:' in cmd.command:
                api.Logger.error("ERROR: PI-Flow-Erspan-config Not Removed")
                dumpSessionTableForDebug(tc)
                result = api.types.status.FAILURE
            elif 'flowexportenablebitmap' in cmd.command:
                api.Logger.error("ERROR: PI-Flow-Export-config Not Removed")
                dumpSessionTableForDebug(tc)
                result = api.types.status.FAILURE

    return result

#
# Retrieve Table-id's to be used duirng feature validation 
#
def retrieveTableIds(tc):
    reqt = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = '/nic/bin/halctl show table info'
    add_naples_command(reqt, tc.naples, cmd)

    trig_respt = api.Trigger(reqt)
    for cmd in trig_respt.commands:
        for line in cmd.stdout.split('\n'):
            if 'lif' in line and 'tx_stage0_lif' not in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        tc.lif_table_id = int(s, 10)
                        break
                    w += 1
            elif 'output_mapping' in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        tc.omap_table_id = int(s, 10)
                        break
                    w += 1
            elif 'flow_info' in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        tc.flow_info_table_id = int(s, 10)
                        break
                    w += 1
            elif 'mirror' in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        tc.mirror_table_id = int(s, 10)
                        break
                    w += 1
            elif 'tunnel_rewrite' in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        tc.tunnel_rewrite_table_id = int(s, 10)
                        break
                    w += 1
            elif 'flow_hash' in line:
                w = 0
                for s in line.split():
                    if w == 1:
                        tc.flow_hash_table_id = int(s, 10)
                        break
                    w += 1

#
# Trace function to provide Workload Attributes 
#
def debugWorkLoadAttributeTraces(tc):
    api.Logger.info("NAPLES ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
    .format(tc.naples.workload_name,
            tc.naples.workload_type,
            tc.naples.workload_image,
            tc.naples.node_name,
            tc.naples.encap_vlan,
            tc.naples.ip_prefix,
            tc.naples.ip_address,
            tc.naples.mac_address,
            tc.naples.interface,
            tc.naples.parent_interface,
            tc.naples.interface_type,
            tc.naples.pinned_port,
            tc.naples.uplink_vlan))
    api.Logger.info("NAPLES-PEER ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
    .format(tc.naples_peer.workload_name,
            tc.naples_peer.workload_type,
            tc.naples_peer.workload_image,
            tc.naples_peer.node_name,
            tc.naples_peer.encap_vlan,
            tc.naples_peer.ip_prefix,
            tc.naples_peer.ip_address,
            tc.naples_peer.mac_address,
            tc.naples_peer.interface,
            tc.naples_peer.parent_interface,
            tc.naples_peer.interface_type,
            tc.naples_peer.pinned_port,
            tc.naples_peer.uplink_vlan))

    c = 0
    for wl in tc.collector:
        api.Logger.info("COLLECTOR ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
        .format(wl.workload_name,
                wl.workload_type,
                wl.workload_image,
                wl.node_name,
                wl.encap_vlan,
                wl.ip_prefix,
                tc.collector_ip_address[c],
                wl.mac_address,
                wl.interface,
                wl.parent_interface,
                wl.interface_type,
                wl.pinned_port,
                wl.uplink_vlan))
        c += 1

#
# Trace function to provide Workload-list 
#
def debugWorkLoadTraces(tc):
    api.Logger.info("NAPLES WORKLOAD/VLAN {} {}, {} {}"\
                 .format(tc.naples.uplink_vlan, tc.naples.ip_address,
                         tc.naples_peer.uplink_vlan, tc.naples_peer.ip_address))
    c = 0
    for wl in tc.collector:
        api.Logger.info("COLLECTOR WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, tc.collector_ip_address[c]))
        c += 1

    c = 0
    for wl in tc.lif_collector:
        api.Logger.info("LIF COLLECTOR WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, tc.collector_ip_address[c]))
        c += 1

    c = 0
    for wl in tc.flow_collector:
        api.Logger.info("FLOW COLLECTOR WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, tc.collector_ip_address[c]))
        c += 1

    c = 0
    for wl in tc.flowmon_collector:
        api.Logger.info("FLOWMON COLLECTOR WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, tc.collector_ip_address[c]))
        c += 1

    for wl in tc.local_workloads:
        api.Logger.info("LOCAL WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    for wl in tc.remote_workloads:
        api.Logger.info("REMOTE WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    debugWorkLoadAttributeTraces(tc)

