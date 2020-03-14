#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.api as cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.config.netagent.hw_sec_ip_config as sec_ip_api
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

DOT1Q_ETYPE      = 0x8100
GRE_ENCAP_LENGTH = 38

ICMP_ECHO         = 0x0800
ICMP_ECHO_REPLY   = 0x0000
ICMP_PORT_UNREACH = 0x0303

NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION  = 6
NUMBER_OF_UDP_ERSPAN_PACKETS_PER_SESSION  = 2
NUMBER_OF_ICMP_ERSPAN_PACKETS_PER_SESSION = 4

NUMBER_OF_IPFIX_PACKETS_PER_SESSION = 10

TCP_EXPORT_ENABLE_EXPECTED            = 2
UDP_EXPORT_ENABLE_EXPECTED            = 20
ICMP_EXPORT_ENABLE_EXPECTED           = 2

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
                    tc.classic_mode == False and\
                    len(tc.collector) < tc.iterators.ccount:
                    tc.collector.append(wl)
    if len(tc.remote_workloads) == 0:
        api.Logger.info("ERROR: No Remote workload")
        return api.types.status.FAILURE
    if tc.iterators.peer == 'remote' and tc.naples_peer is None:
        api.Logger.info("ERROR: No Remote Naples-peer workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'remote' or tc.iterators.collector == 'all')\
        and tc.classic_mode == False and len(tc.collector) == 0:
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
                    tc.classic_mode == False and\
                    len(tc.collector) < tc.iterators.ccount:
                    tc.collector.append(wl)
    if len(tc.local_workloads) == 0:
        api.Logger.info("ERROR: No Local workload")
        return api.types.status.FAILURE
    if tc.iterators.peer == 'local' and tc.naples_peer is None:
        api.Logger.info("ERROR: No Local Naples-peer workload")
        return api.types.status.FAILURE
    if (tc.iterators.collector == 'local' or tc.iterators.collector == 'all')\
        and tc.classic_mode == False and len(tc.collector) == 0:
        api.Logger.info("ERROR: No Local Collector workload")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

#
# Allocate necessary Secondary-IPs as needed for Multi-collector testing
#
def establishCollectorSecondaryIPs(tc):
    tc.collector_ip_address = []
    tc.collector_seq_num = []
    tc.collector_ipfix_pkts = []
    tc.collector_tcp_pkts = []
    tc.collector_udp_pkts = []
    tc.collector_icmp_pkts = []
    tc.collector_other_pkts = []
    tc.collector_validation_done = []
    tc.result = []

    for c in range(0, len(tc.collector)):
        tc.collector_ip_address.append(tc.collector[c].ip_address)
        tc.collector_seq_num.append(0)
        tc.collector_ipfix_pkts.append(0)
        tc.collector_tcp_pkts.append(0)
        tc.collector_udp_pkts.append(0)
        tc.collector_icmp_pkts.append(0)
        tc.collector_other_pkts.append(0)
        tc.collector_validation_done.append(False)
        tc.result.append(api.types.status.SUCCESS)

    tc.sec_ip_count = 0
    if tc.iterators.ccount <= len(tc.collector):
        return api.types.status.SUCCESS

    sec_ip_count = tc.iterators.ccount - len(tc.collector)
    if tc.classic_mode == True:
        sec_ip_list = sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[c], 
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
            sec_ip_list = sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[c],
                                                               True, 1)
            if len(sec_ip_list) == 0:
                return api.types.status.FAILURE
            tc.sec_ip_count += 1

            tc.collector.append(tc.collector[c])
            tc.collector_ip_address.append(sec_ip_list[0])
            i += 1
            c += 1
            if c == len(tc.collector):
                c = 0

    c = 0
    while c < tc.sec_ip_count:
        tc.collector_seq_num.append(0)
        tc.collector_ipfix_pkts.append(0)
        tc.collector_tcp_pkts.append(0)
        tc.collector_udp_pkts.append(0)
        tc.collector_icmp_pkts.append(0)
        tc.collector_other_pkts.append(0)
        tc.collector_validation_done.append(False)
        tc.result.append(api.types.status.SUCCESS)
        c += 1

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
    else:
        i = 0
        c = 0
        while c < tc.sec_ip_count:
            sec_ip_api.ConfigWorkloadSecondaryIp(tc.collector[c], False, 1)
            i += 1
            c += 1
            if c == len(tc.collector):
                c = 0

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
    for c in range(0, len(tc.collector_ip_address)):
        if c != 0:
            tmp = copy.deepcopy(colObjects[0])
            colObjects.append(tmp)
        colObjects[c].meta.name = "lif-collector-{}".format(c)
        colObjects[c].spec.packet_size = tc.iterators.pktsize
        colObjects[c].spec.destination = tc.collector_ip_address[c]

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
        for c in range(0, len(tc.collector_ip_address)):
            if c != 0:
                tmp = copy.deepcopy(obj.spec.collectors[0])
                obj.spec.collectors.append(tmp)
            obj.spec.collectors[c].export_config.destination = \
                                   tc.collector_ip_address[c]

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
        for c in range(0, len(tc.collector_ip_address)):
            if c != 0:
                tmp = copy.deepcopy(obj.spec.exports[0])
                obj.spec.exports.append(tmp)
            obj.spec.exports[c].destination = tc.collector_ip_address[c]

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
    # Start with a clean slate by clearing all sessions/flows
    #
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "/nic/bin/halctl clear session"
    add_naples_command(req, tc.naples, cmd)

    #
    # Make sure that Naples<=>Naples-peer Forwarding is set up
    #
    if api.GetNodeOs(tc.naples_peer.node_name) == 'linux':
        cmd = "ping -I %s -c1 %s" %\
              (tc.naples_peer.interface, tc.naples_peer.ip_address)
    else:
        cmd = "ping -c1 %s" % (tc.naples_peer.ip_address)
    add_command(req, tc.naples, cmd, False)

    if api.GetNodeOs(tc.naples_peer.node_name) == 'linux':
        cmd = "ping -I %s -c1 %s" %\
              (tc.naples_peer.interface, tc.naples.ip_address)
    else:
        cmd = "ping -c1 %s" % (tc.naples.ip_address)
    add_command(req, tc.naples_peer, cmd, False)

    if tc.classic_mode == True:
        for c in range(0, len(tc.collector)):
            #
            # Make sure that Collector<=>Collector-Peer Forwarding is set up
            #
            cmd = "ping -c1 %s" % (tc.collector_ip_address[c])
            add_command(req, tc.bond0_collector_peer, cmd, False)

            cmd = "ping -c1 %s" % (tc.bond0_collector_peer.ip_address)
            add_command(req, tc.collector[c], cmd, False)
    else:
        for c in range(0, len(tc.collector)):
            #
            # Make sure that Collector<=>Naples Forwarding is set up
            #
            cmd = "ping -c1 %s" % (tc.collector_ip_address[c])
            add_command(req, tc.naples, cmd, False)

            #
            # Make sure that Collector<=>Naples-peer Forwarding is set up
            #
            cmd = "ping -c1 %s" % (tc.collector_ip_address[c])
            add_command(req, tc.naples_peer, cmd, False)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

#
# Use Applicable tools to trigger packets in Classic-mode
#
def triggerTrafficInClassicModeLinux(tc, protocol, udp_count, icmp_count):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # TCP
    #if protocol == 'tcp' or protocol == 'all':
    #    cmd = "nc -l {}".format(int(tc.dest_port))
    #    add_command(req, tc.naples_peer, cmd, False)
    #    cmd = "nc {} {} -p {} "\
    #          .format(tc.naples_peer.ip_address, int(tc.dest_port), 
    #                  int(tc.dest_port))
    #    add_command(req, tc.naples, cmd, False)

    # UDP
    if protocol == 'udp' or protocol == 'all':
        cmd = "nping --udp --dest-port {} --source-port {} --count {}\
               --interface {} --data-length {} {}"\
              .format(int(tc.dest_port), int(tc.dest_port), udp_count,
                      tc.naples.interface, tc.iterators.pktsize*2, 
                      tc.naples_peer.ip_address)
        add_command(req, tc.naples, cmd, False)

        cmd = "nping --udp --dest-port {} --source-port {} --count {}\
               --interface {} --data-length {} {}"\
              .format(int(tc.dest_port), int(tc.dest_port), udp_count,
                      tc.naples_peer.interface, tc.iterators.pktsize*2, 
                      tc.naples.ip_address)
        add_command(req, tc.naples_peer, cmd, False)

    # ICMP
    if protocol == 'icmp' or protocol == 'all':
        cmd = "nping --icmp --icmp-type 8 --count {} --interface {}\
               --data-length {} {}"\
              .format(icmp_count, tc.naples.interface, tc.iterators.pktsize*2,
                      tc.naples_peer.ip_address)
        add_command(req, tc.naples, cmd, False)

    trig_resp = api.Trigger(req)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)
    #api.Trigger_TerminateAllCommands(trig_resp)

#
# Use Applicable tools to trigger packets in Hostpin-mode
#
def triggerTrafficInHostPinModeOrFreeBSD(tc, protocol, udp_count, icmp_count):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # TCP
    if protocol == 'tcp' or protocol == 'all':
        if tc.classic_mode == False:
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
    if protocol == 'udp' or protocol == 'all':
        cmd = "hping3 --udp --destport {} --baseport {} --count {} {}\
               -I {} --data {}"\
              .format(int(tc.dest_port), int(tc.dest_port), udp_count,
                      tc.naples_peer.ip_address, tc.naples.interface,
                      tc.iterators.pktsize*2)
        add_command(req, tc.naples, cmd, False)

        cmd = "hping3 --udp --destport {} --baseport {} --count {} {}\
               -I {} --data {}"\
              .format(int(tc.dest_port), int(tc.dest_port), udp_count,
                      tc.naples.ip_address, tc.naples_peer.interface, 
                      tc.iterators.pktsize*2)
        add_command(req, tc.naples_peer, cmd, False)

    # ICMP
    if protocol == 'icmp' or protocol == 'all':
        cmd = "hping3 --icmp --count {} {} -I {} --data {}"\
              .format(icmp_count, tc.naples_peer.ip_address, 
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
           grep "export_en=1"'.format(tc.flow_hash_table_id)
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
    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0".format(tc.lif_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0".format(tc.omap_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep mirror_session_id | grep -v 0x0"\
           .format(tc.flow_info_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = '/nic/bin/halctl show table dump --table-id {} | \
           grep "export_en=1"'.format(tc.flow_hash_table_id)
    add_naples_command(req, tc.naples, cmd)

    cmd = "/nic/bin/halctl show table dump --table-id {} | \
           grep MIRROR_ERSPAN_MIRROR_ID".format(tc.mirror_table_id)
    add_naples_command(req, tc.naples, cmd)

    resp_cleanup = api.Trigger(req)

    return resp_cleanup

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
# Validate VLAN-tag
#
def validate_vlan_tag(tc, tag_etype, vlan_tag):
    result = api.types.status.SUCCESS
    if tag_etype == DOT1Q_ETYPE:
        if tc.naples.uplink_vlan == 0 or vlan_tag != int(tc.naples.uplink_vlan):
            result = api.types.status.FAILURE
    elif tc.naples.uplink_vlan != 0:
        result = api.types.status.FAILURE

    if result == api.types.status.FAILURE:
        api.Logger.info("ERROR: VLAN-Tag {} {} {}".format(tag_etype, vlan_tag,
                        int(tc.naples.uplink_vlan)))

    return result

#
# Validate function for sanity-checking SIP/DIP/SPORT/DPORT embedded in the 
# packet being validated
#
def validate_ip_tuple(tc, sip_msb, sip_lsb, dip_msb, dip_lsb, sport, dport,
                      tag_etype, vlan_tag, ip_proto, c, feature):

    sip = (sip_msb << 16) | sip_lsb
    dip = (dip_msb << 16) | dip_lsb

    result = api.types.status.SUCCESS
    if sip == int(ipaddress.ip_address(tc.naples.ip_address)) and\
       dip == int(ipaddress.ip_address(tc.naples_peer.ip_address)):
        if ip_proto != IP_PROTO_ICMP and dport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif feature != 'flowmon':
            if ip_proto == IP_PROTO_ICMP and sport != ICMP_ECHO and\
               sport != ICMP_ECHO_REPLY and sport != ICMP_PORT_UNREACH:
                result = api.types.status.FAILURE
            else:
                result = validate_vlan_tag(tc, tag_etype, vlan_tag)
    elif sip == int(ipaddress.ip_address(tc.naples_peer.ip_address)) and\
         dip == int(ipaddress.ip_address(tc.naples.ip_address)):
        if ip_proto == IP_PROTO_TCP and sport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif ip_proto == IP_PROTO_UDP and dport != int(tc.dest_port):
            result = api.types.status.FAILURE
        elif feature != 'flowmon':
            if ip_proto == IP_PROTO_ICMP and sport != ICMP_ECHO and\
               sport != ICMP_ECHO_REPLY and sport != ICMP_PORT_UNREACH:
                result = api.types.status.FAILURE
            else:
                result = validate_vlan_tag(tc, tag_etype, vlan_tag)
    elif tc.classic_mode == False:
        result = api.types.status.FAILURE
    elif feature == 'lif-erspan':
        if ip_proto == IP_PROTO_UDP:
            tc.collector_udp_pkts[c] -= 1
            tc.collector_other_pkts[c] += 1
        elif ip_proto == IP_PROTO_ICMP:
            tc.collector_icmp_pkts[c] -= 1
            tc.collector_other_pkts[c] += 1

    if result == api.types.status.FAILURE:
        api.Logger.info("ERROR: SIP/DIP {} {} {} {} {} {} {} {} {} {}"\
        .format(int(ipaddress.ip_address(tc.naples.ip_address)),
                int(ipaddress.ip_address(tc.naples_peer.ip_address)), sip, dip,
                ip_proto, sport, dport, tc.dest_port, tag_etype, vlan_tag))

    return result

#
# Validate ERSPAN packets reception
#
def validateErspanPackets(tc, protocol, feature, direction):
    tcp_erspan_pkts_expected = NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION
    udp_erspan_pkts_expected = NUMBER_OF_UDP_ERSPAN_PACKETS_PER_SESSION
    icmp_erspan_pkts_expected = NUMBER_OF_ICMP_ERSPAN_PACKETS_PER_SESSION

    if protocol == 'udp-mixed' or protocol == 'icmp':
        icmp_erspan_pkts_expected = icmp_erspan_pkts_expected/2

    if direction != 'both':
        tcp_erspan_pkts_expected = tcp_erspan_pkts_expected/2
        udp_erspan_pkts_expected = udp_erspan_pkts_expected/2
        icmp_erspan_pkts_expected = icmp_erspan_pkts_expected/2

    if tc.dupcheck == 'disable':
        tcp_erspan_pkts_expected  =  (tcp_erspan_pkts_expected+1)*2
        udp_erspan_pkts_expected  *= 2
        icmp_erspan_pkts_expected *= 2

    for c in range(0, len(tc.collector_ip_address)):
        tc.collector_tcp_pkts[c] = 0
        tc.collector_udp_pkts[c] = 0
        tc.collector_icmp_pkts[c] = 0
        tc.collector_other_pkts[c] = 0
        tc.collector_validation_done[c] = False

    result = api.types.status.SUCCESS
    for cmd in tc.resp_tcpdump.commands:
        if 'tcpdump' in cmd.command:
            api.PrintCommandResults(cmd)
            for line in cmd.stdout.split('\n'):
                if 'GREv0' in line:
                    #
                    # Validate Collector IP-Address
                    #
                    w = 0
                    for s in line.split():
                        if w == 2:
                            collector = s.replace(':', '')
                            for c in range(0, len(tc.collector_ip_address)):
                                if collector == tc.collector_ip_address[c]:
                                    break
                        elif w == 7:
                            pkt_size = int(s, 10)
                            if (pkt_size - GRE_ENCAP_LENGTH) >\
                                tc.iterators.pktsize:
                                api.Logger.info("ERROR: Packet-Size {} {}"\
                                    .format(pkt_size, tc.iterators.pktsize))
                                result = api.types.status.FAILURE
                                break
                        w += 1
                elif '0x0040' in line:
                    #
                    # Validate VLAN for ERSPAN packets that embed 
                    # tagged packets (only for uplink peers)
                    #
                    w = 0
                    for s in line.split():
                        if w == 4:
                            tag_etype = int(s, 16)
                        elif w == 5:
                            vlan_tag = int(s, 16)
                        w += 1
                elif '0x0050:' in line:
                    if tag_etype == DOT1Q_ETYPE:
                        #
                        # Validate IP-Protocol
                        #
                        # Extract Source/Destination IP-Addresses
                        # for ERSPAN packets that embed tagged packets
                        #
                        w = 0
                        for s in line.split():
                            if w == 3:
                                ip_proto = int(s, 16) & 0xFF
                                if ip_proto == IP_PROTO_TCP:
                                    tc.collector_tcp_pkts[c] += 1
                                elif ip_proto == IP_PROTO_UDP:
                                    tc.collector_udp_pkts[c] += 1
                                elif ip_proto == IP_PROTO_ICMP:
                                    tc.collector_icmp_pkts[c] += 1
                                else:
                                    tag_etype = 0
                                    tc.collector_other_pkts[c] += 1
                                    break
                            elif w == 5:
                                sip_msb = int(s, 16)
                            elif w == 6:
                                sip_lsb = int(s, 16)
                            elif w == 7:
                                dip_msb = int(s, 16)
                            elif w == 8:
                                dip_lsb = int(s, 16)
                                break
                            w += 1
                    else:
                        #
                        # Validate IP-Protocol, Source/Destination 
                        # IP-Addresses, L4-ports for
                        # ERSPAN packets that embed untagged packets
                        #
                        w = 0
                        for s in line.split():
                            if w == 1:
                                ip_proto = int(s, 16) & 0xFF
                                if ip_proto == IP_PROTO_TCP:
                                    tc.collector_tcp_pkts[c] += 1
                                elif ip_proto == IP_PROTO_UDP:
                                    tc.collector_udp_pkts[c] += 1
                                elif ip_proto == IP_PROTO_ICMP:
                                    tc.collector_icmp_pkts[c] += 1
                                else:
                                    tc.collector_other_pkts[c] += 1
                                    break
                            elif w == 3:
                                sip_msb = int(s, 16)
                            elif w == 4:
                                sip_lsb = int(s, 16)
                            elif w == 5:
                                dip_msb = int(s, 16)
                            elif w == 6:
                                dip_lsb = int(s, 16)
                            elif w == 7:
                                sport = int(s, 16)
                            elif w == 8:
                                dport = int(s, 16)
                                res = validate_ip_tuple(tc, sip_msb, sip_lsb, 
                                      dip_msb, dip_lsb, sport, dport, 
                                      tag_etype, vlan_tag, ip_proto, c, feature)
                                if res != api.types.status.SUCCESS:
                                    result = api.types.status.FAILURE
                                break
                            w += 1
                elif '0x0060:' in line:
                    if tag_etype == DOT1Q_ETYPE:
                        #
                        # Extract Source/Destination L4-ports
                        # for ERSPAN packets that embed tagged packets
                        # And validate IP-tuple
                        #
                        w = 0
                        for s in line.split():
                            if w == 1:
                                sport = int(s, 16)
                            elif w == 2:
                                dport = int(s, 16)
                                res = validate_ip_tuple(tc, sip_msb, sip_lsb, 
                                      dip_msb, dip_lsb, sport, dport, 
                                      tag_etype, vlan_tag, ip_proto, c, feature)
                                if res != api.types.status.SUCCESS:
                                    result = api.types.status.FAILURE
                                break
                            w += 1

    #
    # Validate Number-of-ERSPAN-pkts received by the Collector
    #
    for c in range(0, len(tc.collector_ip_address)):
        tc.result[c] = api.types.status.SUCCESS

        #
        # Perform TCP-pkt checks (only for hostpin mode, for now)
        #
        if tc.classic_mode == False:
            if protocol == 'tcp' or protocol == 'all':
                if tc.collector_tcp_pkts[c] < tcp_erspan_pkts_expected or\
                   tc.collector_tcp_pkts[c] > (tcp_erspan_pkts_expected+1):
                    tc.result[c] = api.types.status.FAILURE

        #
        # Perform UDP-pkt checks
        #
        if protocol == 'udp' or protocol == 'udp-mixed' or protocol == 'all':
            if tc.collector_udp_pkts[c] != udp_erspan_pkts_expected:
                tc.result[c] = api.types.status.FAILURE

        #
        # Perform ICMP-pkt checks
        #
        if protocol == 'icmp' or protocol == 'udp-mixed' or protocol == 'all':
            if tc.collector_icmp_pkts[c] != icmp_erspan_pkts_expected:
                tc.result[c] = api.types.status.FAILURE

    for c in range(0, len(tc.collector_ip_address)):
        if tc.result[c] == api.types.status.FAILURE:
            api.Logger.info("ERROR: {} {} {} {} {} {} {} ERSPAN packets to {}"\
            .format(tc.collector_tcp_pkts[c], tc.collector_udp_pkts[c],
                    tc.collector_icmp_pkts[c], tc.collector_other_pkts[c],
                    tcp_erspan_pkts_expected, udp_erspan_pkts_expected,
                    icmp_erspan_pkts_expected, tc.collector_ip_address[c]))
            result = api.types.status.FAILURE
        else:
            api.Logger.info("Number of ERSPAN packets {} {} {} {} to {}"\
            .format(tc.collector_tcp_pkts[c], tc.collector_udp_pkts[c],
                    tc.collector_icmp_pkts[c],
                    tc.collector_other_pkts[c], tc.collector_ip_address[c]))

    return result

#
# Validate IPFIX packets reception
#
def validateIpFixPackets(tc, protocol, feature):
    result = api.types.status.SUCCESS
    for cmd in tc.resp_tcpdump.commands:
        if 'tcpdump' in cmd.command:
            api.PrintCommandResults(cmd)
            ipfix_pkts = 0
            for c in range(0, len(tc.collector)):
                tc.collector_ipfix_pkts[c] = 0

            for line in cmd.stdout.split('\n'):
                if '.2055:' in line:
                    #
                    # Count IPFIX packets
                    #
                    ipfix_pkts += 1

                    #
                    # Validate Collector IP-Address
                    #
                    w = 0
                    for s in line.split():
                        if w == 2:
                            collector = s.replace('.2055:', '')
                            match = False
                            for idx in range(0, len(tc.collector_ip_address)):
                                if collector == tc.collector_ip_address[idx]:
                                    tc.collector_ipfix_pkts[idx] += 1
                                    match_idx = idx
                                    match = True
                                    break
                            if match == False:
                                result = api.types.status.FAILURE
                            break
                        w += 1
                elif '0x0030:' in line and match == True:
                    #
                    # Validate IPFIX Seq-number
                    #
                    w = 0
                    for s in line.split():
                        if w == 2:
                            seq_msb = int(s, 16)
                        elif w == 3:
                            seq_lsb = int(s, 16)
                            curr_seq_num = (seq_msb << 16) | seq_lsb
                            if tc.collector_ipfix_pkts[match_idx] == 1:
                                tc.collector_seq_num[match_idx] =\
                                curr_seq_num
                            else:
                                if (curr_seq_num -\
                                    tc.collector_seq_num[match_idx]) !=\
                                    1:
                                    api.Logger.info(\
                                    "ERROR: IPFIX Seq-num {} {}"\
                                    .format(curr_seq_num,
                                    tc.collector_seq_num[match_idx]+1))
                                    result = api.types.status.FAILURE
                                tc.collector_seq_num[match_idx] =\
                                curr_seq_num
                            break
                        w += 1
                elif '0x0040:' in line:
                    #
                    # Validate Source/Destination IP-Addresses / Port
                    # for ERSPAN packets that embed tagged packets
                    #
                    w = 0
                    for s in line.split():
                        if w == 2:
                            sip_msb = int(s, 16)
                        elif w == 3:
                            sip_lsb = int(s, 16)
                        elif w == 4:
                            dip_msb = int(s, 16)
                        elif w == 5:
                            dip_lsb = int(s, 16)
                        elif w == 6:
                            ip_proto = int(s, 16) >> 8
                            sport_msb = int(s, 16) & 0xFF
                        elif w == 7:
                            sport_lsb = int(s, 16) >> 8
                            dport_msb = int(s, 16) & 0xFF
                        elif w == 8:
                            dport_lsb = int(s, 16) >> 8

                            res = validate_ip_proto(tc, protocol, ip_proto)
                            if res != api.types.status.SUCCESS:
                                result = api.types.status.FAILURE

                            sport = (sport_msb << 8) | sport_lsb
                            dport = (dport_msb << 8) | dport_lsb
                            res = validate_ip_tuple(tc, sip_msb, sip_lsb, 
                                  dip_msb, dip_lsb, sport, dport, 
                                  0, 0, ip_proto, c, feature)
                            if res != api.types.status.SUCCESS:
                                result = api.types.status.FAILURE
                            break
                        w += 1

    #
    # Validate Number-of-IPFIX-pkts received by the 
    # Collector
    #
    #if ipfix_pkts != NUMBER_OF_IPFIX_PACKETS_PER_SESSION:
    if ipfix_pkts == 0:
        api.Logger.info("ERROR: IPFIX packets {}".format(ipfix_pkts))
        result = api.types.status.FAILURE

    for idx in range(0, len(tc.collector_ip_address)):
        api.Logger.info("Number of IPFIX packets {} to {}"\
        .format(tc.collector_ipfix_pkts[idx], tc.collector_ip_address[c]))

    return result

#
# Validate Config-cleanup
#
def validateConfigCleanup(tc):
    result = api.types.status.SUCCESS
    for cmd in tc.resp_cleanup.commands:
        api.PrintCommandResults(cmd)
        if cmd.stdout != '':
            if 'table-id {}'.format(tc.lif_table_id) in cmd.command:
                api.Logger.info("ERROR: lif-config Not Removed")
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.omap_table_id) in cmd.command:
                api.Logger.info("ERROR: omap-config Not Removed")
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.flow_info_table_id) in cmd.command:
                api.Logger.info("ERROR: Flow-Info-config Not Removed")
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.mirror_table_id) in cmd.command:
                api.Logger.info("ERROR: Mirror-config Not Removed")
                result = api.types.status.FAILURE
            elif 'table-id {}'.format(tc.flow_hash_table_id) in cmd.command:
                api.Logger.info("ERROR: Flow-Hash-config Not Removed")
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

    for wl in tc.local_workloads:
        api.Logger.info("LOCAL WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    for wl in tc.remote_workloads:
        api.Logger.info("REMOTE WORKLOADS/VLAN {} {}"\
                        .format(wl.uplink_vlan, wl.ip_address))

    debugWorkLoadAttributeTraces(tc)

