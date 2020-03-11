#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.api as cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
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

IP_PROTO_TCP  = 6
IP_PROTO_UDP  = 17
IP_PROTO_ICMP = 1

ICMP_ECHO         = 0x0800
ICMP_ECHO_REPLY   = 0x0000
ICMP_PORT_UNREACH = 0x0303

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
        api.Logger.info("ERROR: No workloads")
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
        api.Logger.info("ERROR: No Naples workload")
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
    tc.collector_mode = []
    tc.collector_seq_num = []
    tc.collector_ipfix_pkts = []
    tc.collector_tcp_pkts = []
    tc.collector_udp_pkts = []
    tc.collector_icmp_pkts = []
    tc.collector_other_pkts = []
    tc.collector_validation_done = []
    tc.result = []

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
                if (tc.iterators.collector == 'remote' or\
                    tc.iterators.collector == 'all') and\
                    len(tc.collector) < tc.iterators.ccount:
                    tc.collector.append(wl)
                    tc.collector_mode.append('remote')
                    tc.collector_seq_num.append(0)
                    tc.collector_ipfix_pkts.append(0)
                    tc.collector_tcp_pkts.append(0)
                    tc.collector_udp_pkts.append(0)
                    tc.collector_icmp_pkts.append(0)
                    tc.collector_other_pkts.append(0)
                    tc.collector_validation_done.append(False)
                    tc.result.append(api.types.status.SUCCESS)
    if len(tc.remote_workloads) == 0:
        api.Logger.info("ERROR: No Remote workload")
        return api.types.status.FAILURE
    if tc.iterators.peer == 'remote' and tc.naples_peer is None:
        api.Logger.info("ERROR: No Remote Naples-peer workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'remote' or tc.iterators.collector == 'all')\
        and len(tc.collector) == 0:
        api.Logger.info("ERROR: No Remote Collector workload")
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
                if (tc.iterators.collector == 'local' or\
                    tc.iterators.collector == 'all') and\
                    len(tc.collector) < tc.iterators.ccount:
                    tc.collector.append(wl)
                    tc.collector_mode.append('local')
                    tc.collector_seq_num.append(0)
                    tc.collector_ipfix_pkts.append(0)
                    tc.collector_tcp_pkts.append(0)
                    tc.collector_udp_pkts.append(0)
                    tc.collector_icmp_pkts.append(0)
                    tc.collector_other_pkts.append(0)
                    tc.collector_validation_done.append(False)
                    tc.result.append(api.types.status.SUCCESS)
    if len(tc.local_workloads) == 0:
        api.Logger.info("ERROR: No Local workload")
        return api.types.status.FAILURE
    if tc.iterators.peer == 'local' and tc.naples_peer is None:
        api.Logger.info("ERROR: No Local Naples-peer workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'local' or tc.iterators.collector == 'all')\
        and len(tc.collector) == 0:
        api.Logger.info("ERROR: No Local Collector workload")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

#
# Identify Collector Workload in Classic mode that is tied to Bond0 subnet
#
def establishCollectorWorkloadInClassicMode(tc, template_collector_ip):
    tc.bond0_collector = None
    tc.bond0_collector_peer = None
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
        api.Logger.info("ERROR: No Bond0 Collector workload")
        return api.types.status.FAILURE

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
        api.Logger.info("ERROR: No Bond0 Collector Local workload")
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

    api.Logger.info("NEW COLLECTOR LOCAL ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
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
    if tc.classic_mode == True:
        result = establishCollectorWorkloadInClassicMode(tc, 
                 colObjects[0].spec.destination)
        if result != api.types.status.SUCCESS:
            return result

        for c in range(0, len(tc.collector)):
            tc.collector[c] = tc.bond0_collector

    for c in range(0, len(tc.collector)):
        if c != 0:
            tmp = copy.deepcopy(colObjects[0])
            colObjects.append(tmp)
        colObjects[c].meta.name = "lif-collector-{}".format(c)
        colObjects[c].spec.packet_size = tc.iterators.pktsize
        colObjects[c].spec.destination = tc.collector[c].ip_address

    return api.types.status.SUCCESS

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
        if tc.iterators.direction == 'ingress':
            obj.spec.TxCollectors.pop()
        elif tc.iterators.direction == 'egress':
            obj.spec.RxCollectors.pop()

        for c in range(0, len(colObjects)):
            if tc.iterators.direction == 'ingress' or\
               tc.iterators.direction == 'both':
                if c != 0:
                    tmp = copy.deepcopy(obj.spec.RxCollectors[0])
                    obj.spec.RxCollectors.append(tmp)
                obj.spec.RxCollectors[c] = colObjects[c].meta.name

            if tc.iterators.direction == 'egress' or\
               tc.iterators.direction == 'both':
                if c != 0:
                    tmp = copy.deepcopy(obj.spec.TxCollectors[0])
                    obj.spec.TxCollectors.append(tmp)
                obj.spec.TxCollectors[c] = colObjects[c].meta.name
        i += 1

    return api.types.status.SUCCESS

#
# Superimpose template-if-config with Mirror-references removal
#
def deGenerateLifInterfaceConfig(tc, ifObjects, colObjects):
    for obj in ifObjects:
        if tc.iterators.direction == 'ingress' or\
           tc.iterators.direction == 'both':
            for c in range(0, len(colObjects)):
                obj.spec.RxCollectors.pop()
        if tc.iterators.direction == 'egress' or\
           tc.iterators.direction == 'both':
            for c in range(0, len(colObjects)):
                obj.spec.TxCollectors.pop()

#
# Use Applicable tools to trigger packets in Classic-mode
#
def triggerTrafficInClassicModeLinux(tc):
    # TCP
    #cmd = "nc -l {}".format(int(tc.dest_port))
    #add_command(req, tc.naples_peer, cmd, False)
    #cmd = "nc {} {} -p {} "\
    #      .format(tc.naples_peer.ip_address, int(tc.dest_port), 
    #              int(tc.dest_port))
    #add_command(req, tc.naples, cmd, False)

    # UDP
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "nping --udp --dest-port {} --source-port {} --count 1\
           --interface {} --data-length {} {}"\
          .format(int(tc.dest_port), int(tc.dest_port), tc.naples.interface,
                  tc.iterators.pktsize*2, tc.naples_peer.ip_address)
    add_command(req, tc.naples, cmd, False)

    cmd = "nping --udp --dest-port {} --source-port {} --count 1\
           --interface {} --data-length {} {}"\
          .format(int(tc.dest_port), int(tc.dest_port),
                  tc.naples_peer.interface, tc.iterators.pktsize*2, 
                  tc.naples.ip_address)
    add_command(req, tc.naples_peer, cmd, False)

    # ICMP
    cmd = "nping --icmp --icmp-type 8 --count 1 --interface {}\
           --data-length {} {}"\
          .format(tc.naples.interface, tc.iterators.pktsize*2,
                  tc.naples_peer.ip_address)
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

    # TCP
    if tc.classic_mode == False:
    #   cmd = "nc -l {}".format(int(tc.dest_port))
    #   add_command(req, tc.naples_peer, cmd, False)
    #   cmd = "nc {} {} -p {} "\
    #         .format(tc.naples_peer.ip_address, int(tc.dest_port), 
    #                 int(tc.dest_port))
    #   add_command(req, tc.naples, cmd, False)
    #else:
        cmd = "nc --listen {}".format(int(tc.dest_port))
        add_command(req, tc.naples_peer, cmd, False)
        cmd = "nc {} {} --source-port {} "\
              .format(tc.naples_peer.ip_address, int(tc.dest_port), 
                      int(tc.dest_port))
        add_command(req, tc.naples, cmd, False)

    # UDP
    cmd = "hping3 --udp --destport {} --baseport {} --count 1 {}\
           -I {} --data {}"\
          .format(int(tc.dest_port), int(tc.dest_port),
                  tc.naples_peer.ip_address, tc.naples.interface,
                  tc.iterators.pktsize*2)
    add_command(req, tc.naples, cmd, False)

    cmd = "hping3 --udp --destport {} --baseport {} --count 1 {}\
           -I {} --data {}"\
          .format(int(tc.dest_port), int(tc.dest_port), 
                  tc.naples.ip_address, tc.naples_peer.interface, 
                  tc.iterators.pktsize*2)
    add_command(req, tc.naples_peer, cmd, False)

    # ICMP
    cmd = "hping3 --icmp --count 1 {} -I {} --data {}"\
          .format(tc.naples_peer.ip_address, tc.naples.interface,
                  tc.iterators.pktsize*2)
    add_command(req, tc.naples, cmd, False)

    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    #api.Trigger_TerminateAllCommands(trig_resp)

#
# Validate function for sanity-checking IP-protocol embedded in the packet
# being validated
#
def validate_ip_proto(tc, protocol, ip_proto):
    if protocol == 'tcp' and ip_proto == IP_PROTO_TCP:
        return api.types.status.SUCCESS
    elif protocol == 'udp':
        if (ip_proto == IP_PROTO_UDP) or\
           (tc.iterators.proto == 'mixed' and ip_proto == IP_PROTO_ICMP):
            return api.types.status.SUCCESS
    elif protocol == 'icmp' and ip_proto == IP_PROTO_ICMP:
        return api.types.status.SUCCESS

    api.Logger.info("ERROR: IP-Prococol {} {}".format(protocol, ip_proto))
    return api.types.status.FAILURE

#
# Validate function for sanity-checking SIP/DIP/SPORT/DPORT embedded in the 
# packet being validated
#
def validate_sip_dip(tc, sip_msb, sip_lsb, dip_msb, dip_lsb, sport, dport,
                     ip_proto, c):

    sip = (sip_msb << 16) | sip_lsb
    dip = (dip_msb << 16) | dip_lsb

    result = api.types.status.SUCCESS
    if sip == int(ipaddress.ip_address(tc.naples.ip_address)) and\
       dip == int(ipaddress.ip_address(tc.naples_peer.ip_address)):
        if ip_proto != IP_PROTO_ICMP and dport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif ip_proto == IP_PROTO_ICMP and sport != ICMP_ECHO and\
             sport != ICMP_ECHO_REPLY and sport != ICMP_PORT_UNREACH:
            result = api.types.status.FAILURE
    elif sip == int(ipaddress.ip_address(tc.naples_peer.ip_address)) and\
         dip == int(ipaddress.ip_address(tc.naples.ip_address)):
        if ip_proto == IP_PROTO_TCP and sport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif ip_proto == IP_PROTO_UDP and dport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif ip_proto == IP_PROTO_ICMP and sport != ICMP_ECHO and\
             sport != ICMP_ECHO_REPLY and sport != ICMP_PORT_UNREACH:
            result = api.types.status.FAILURE
    elif tc.classic_mode == False:
        result = api.types.status.FAILURE
    elif ip_proto == IP_PROTO_UDP:
        tc.collector_udp_pkts[c] -= 1
        tc.collector_other_pkts[c] += 1
    elif ip_proto == IP_PROTO_ICMP:
        tc.collector_icmp_pkts[c] -= 1
        tc.collector_other_pkts[c] += 1

    if result == api.types.status.FAILURE:
        api.Logger.info("ERROR: SIP/DIP {} {} {} {} {} {} {} {}"\
        .format(int(ipaddress.ip_address(tc.naples.ip_address)),
                int(ipaddress.ip_address(tc.naples_peer.ip_address)), sip, dip,
                ip_proto, sport, dport, tc.dest_port))

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
    for wl in tc.collector:
        api.Logger.info("COLLECTOR ATTRIBUTES {} {} {} {} {} {} {} {} {} {} {}"\
        .format(wl.workload_name,
                wl.workload_type,
                wl.workload_image,
                wl.node_name,
                wl.encap_vlan,
                wl.ip_prefix,
                wl.ip_address,
                wl.mac_address,
                wl.interface,
                wl.parent_interface,
                wl.interface_type,
                wl.pinned_port,
                wl.uplink_vlan))

#
# Trace function to provide Workload-list 
#
def debugWorkLoadTraces(tc):
    api.Logger.info("NAPLES WORKLOAD/VLAN {} {}, {} {}"\
                 .format(tc.naples.uplink_vlan, tc.naples.ip_address,
                         tc.naples_peer.uplink_vlan, tc.naples_peer.ip_address))
    for wl in tc.collector:
        api.Logger.info("COLLECTOR WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    for wl in tc.local_workloads:
        api.Logger.info("LOCAL WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    for wl in tc.remote_workloads:
        api.Logger.info("REMOTE WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    debugWorkLoadAttributeTraces(tc)

