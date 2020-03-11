#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.api as cfg_api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.telemetry.utils as utils
import iota.test.iris.testcases.expanded_telemetry.utils as eutils
from iota.test.iris.testcases.aging.aging_utils import *
import pdb
import json
import ipaddress
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

NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION  = 6
NUMBER_OF_UDP_ERSPAN_PACKETS_PER_SESSION  = 2
NUMBER_OF_ICMP_ERSPAN_PACKETS_PER_SESSION = 4

def Setup(tc):
    #
    # Ignore non-applicable test-options in Sanity mode
    #
    tc.ignore = False
    if tc.args.type == 'sanity':
        if (tc.iterators.ccount == 1 and tc.iterators.collector == 'all') or\
           (tc.iterators.ccount >  1 and tc.iterators.collector != 'all') or\
           (tc.iterators.vlan == 'native' and\
            tc.iterators.collector != 'local'):
            tc.ignore = True
            return api.types.status.SUCCESS

    #
    # Establish Workloads
    #
    tc.classic_mode = None
    tc.error = False
    result = eutils.establishWorkloads(tc)
    if result != api.types.status.SUCCESS:
        tc.error = True
        return result

    #
    # Check to see if Naples-hosted node is present
    #
    result = eutils.establishNaplesWorkload(tc)
    if result != api.types.status.SUCCESS:
        tc.error = True
        return result

    #
    # Figure out (Collector) Workloads that are remote to Naples node and
    # identify remote Naples-peer, if applicable
    #
    result = eutils.establishRemoteWorkloads(tc)
    if result != api.types.status.SUCCESS:
        tc.error = True
        return result

    #
    # Figure out Workloads that are local to Naples node and identify
    # local Naples-peer, if applicable
    #
    result = eutils.establishLocalWorkloads(tc)
    if result != api.types.status.SUCCESS:
        tc.error = True
        return result

    eutils.debugWorkLoadTraces(tc)

    #
    # Retrieve relevant Table-Id's
    #
    eutils.retrieveTableIds(tc)

    #
    # Set up runtime validation knobs
    # - dupcheck:   To circumvent duplicate-pkts checks in case of LIF-ERSPAN
    #               where multiple workloads map to the same LIF
    #
    tc.dupcheck = 'enable'
    if tc.iterators.peer == 'local' or tc.iterators.iftype != 'uplink':
        tc.dupcheck = 'disable'

    #
    # Preserve current TCP-Close configs and shrink it to 1-second
    #
    tc.classic_mode = False
    if api.IsBareMetalWorkloadType(tc.naples.node_name):
        tc.classic_mode = True

    if tc.classic_mode == False:
        tc.tcp_close_val = get_timeout_val('tcp-close')
        update_timeout('tcp-close', "1s")

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.ignore == True:
        return api.types.status.SUCCESS

    if tc.error == True:
        return api.types.status.FAILURE

    tc.resp_tcpdump = None
    tc.resp_cleanup = None

    policies = utils.GetTargetJsons('mirror', 'lif')
    for policy_json in policies:
        #
        # Get template-Mirror Config
        #
        newObjects = agent_api.AddOneConfig(policy_json)
        if len (newObjects) == 0:
            api.Logger.error("Adding new objects to store failed")
            tc.error = True
            return api.types.status.FAILURE

        #
        # Modify template-Mirror Config to make sure that Naples-node
        # act as either source or destination
        #
        # Set up Collector in the remote node
        #
        if newObjects[0].kind == 'Collector':
            tc.collector_objects = newObjects
            agent_api.RemoveConfigObjects(tc.collector_objects)
        elif newObjects[0].kind == 'Interface':
            tc.interface_objects = newObjects
            agent_api.RemoveConfigObjects(tc.interface_objects)

    for i in range(0, len(policies)):
        #
        # Push Collector object
        #
        if i == 0:
            colObjects = tc.collector_objects
            ret = eutils.generateLifCollectorConfig(tc, colObjects)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Unable to identify Collector Workload")
                tc.error = True
                return api.types.status.FAILURE

            ret = agent_api.PushConfigObjects(colObjects, [tc.naples.node_name])
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Unable to push collector objects")
                tc.error = True
                return api.types.status.FAILURE
            continue

        #
        # Start with a clean slate by clearing all sessions/flows
        #
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        cmd = "/nic/bin/halctl clear session"
        eutils.add_naples_command(req, tc.naples, cmd)

        #
        # Make sure that Naples<=>Naples-peer Forwarding is set up
        #
        cmd = "ping -c1 %s" % (tc.naples_peer.ip_address)
        eutils.add_command(req, tc.naples, cmd, False)

        cmd = "ping -c1 %s" % (tc.naples.ip_address)
        eutils.add_command(req, tc.naples_peer, cmd, False)

        if tc.classic_mode == True:
            for c in range(0, len(tc.collector)):
                #
                # Make sure that Collector<=>Collector-Peer Forwarding is set up
                #
                cmd = "ping -c1 %s" % (tc.collector[c].ip_address) 
                eutils.add_command(req, tc.bond0_collector_peer, cmd, False)

                cmd = "ping -c1 %s" % (tc.bond0_collector_peer.ip_address) 
                eutils.add_command(req, tc.collector[c], cmd, False)
        else:
            for c in range(0, len(tc.collector)):
                #
                # Make sure that Collector<=>Naples Forwarding is set up
                #
                cmd = "ping -c1 %s" % (tc.naples.ip_address)
                eutils.add_command(req, tc.collector[c], cmd, False)

                #
                # Make sure that Collector<=>Naples-peer Forwarding is set up
                #
                cmd = "ping -c1 %s" % (tc.naples_peer.ip_address)
                eutils.add_command(req, tc.collector[c], cmd, False)

        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)

        #
        # Update Interface objects
        #
        ifObjects = tc.interface_objects
        ret = eutils.generateLifInterfaceConfig(tc, ifObjects, colObjects)
        if ret != api.types.status.SUCCESS:
            agent_api.DeleteConfigObjects(tc.collector_objects,
                                         [tc.naples.node_name])
            api.Logger.error("Unable to identify Uplink/LIF Interfaces")
            tc.error = True
            return api.types.status.FAILURE

        ret = agent_api.UpdateConfigObjects(ifObjects, [tc.naples.node_name])
        if ret != api.types.status.SUCCESS:
            agent_api.DeleteConfigObjects(tc.collector_objects,
                                         [tc.naples.node_name])
            api.Logger.error("Unable to update interface objects")
            tc.error = True
            return api.types.status.FAILURE

        req_tcpdump = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        for c in range(0, len(tc.collector)):
            #
            # Set up TCPDUMP's on the collector
            #
            if tc.collector[c].IsNaples():
                cmd = "tcpdump -c 200 -XX -vv -nni %s ip proto gre" %\
                      (tc.collector[c].interface)
            else:
                cmd = "tcpdump -p -c 200 -XX -vv -nni %s ip proto gre" %\
                      (tc.collector[c].interface)
            eutils.add_command(req_tcpdump, tc.collector[c], cmd, True)

        resp_tcpdump = api.Trigger(req_tcpdump)
        for cmd in resp_tcpdump.commands:
            api.PrintCommandResults(cmd)

        #
        # Classic mode requires a delay to make sure that TCPDUMP background
        # process is fully up
        #
        if tc.classic_mode == True:
            time.sleep(2)

        #
        # Trigger packets for ERSPAN to take effect
        #
        tc.dest_port = '120'
        if api.GetNodeOs(tc.naples.node_name) == 'linux':
            eutils.triggerTrafficInClassicModeLinux(tc)
        else:
            eutils.triggerTrafficInHostPinModeOrFreeBSD(tc)

        #
        # Introduce a delay to make sure that TCPDUMP background
        # process is terminated only after all applicable packets are captured
        #
        time.sleep(2)

        #
        # Terminate TCPDUMP background process
        #
        term_resp_tcpdump = api.Trigger_TerminateAllCommands(resp_tcpdump)
        tc.resp_tcpdump = api.Trigger_AggregateCommandsResponse(resp_tcpdump,
                                                              term_resp_tcpdump)

        #
        # Dump sessions/flows/P4-tables for debug purposes
        #
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        cmd = "/nic/bin/halctl show session"
        eutils.add_naples_command(req, tc.naples, cmd)

        cmd = "/nic/bin/halctl show table dump --table-id {} | \
               grep mirror_session_id | grep -v 0x0"\
               .format(tc.lif_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        cmd = "/nic/bin/halctl show table dump --table-id {} | \
               grep mirror_session_id | grep -v 0x0"\
               .format(tc.omap_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        cmd = "/nic/bin/halctl show table dump --table-id {}"\
               .format(tc.mirror_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        cmd = "/nic/bin/halctl show table dump --table-id {}"\
               .format(tc.tunnel_rewrite_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)

        # Delete the objects
        eutils.deGenerateLifInterfaceConfig(tc, tc.interface_objects, 
                                            tc.collector_objects)
        agent_api.UpdateConfigObjects(tc.interface_objects, 
                                     [tc.naples.node_name])

        agent_api.DeleteConfigObjects(tc.collector_objects, 
                                     [tc.naples.node_name])

        #
        # Make sure that Mirror-config has been removed
        #
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        cmd = "/nic/bin/halctl show table dump --table-id {} | \
               grep mirror_session_id | grep -v 0x0"\
               .format(tc.lif_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        cmd = "/nic/bin/halctl show table dump --table-id {} | \
               grep mirror_session_id | grep -v 0x0"\
               .format(tc.omap_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        cmd = "/nic/bin/halctl show table dump --table-id {} | \
               grep MIRROR_ERSPAN_MIRROR_ID"\
               .format(tc.mirror_table_id)
        eutils.add_naples_command(req, tc.naples, cmd)

        tc.resp_cleanup = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.ignore == True:
        return api.types.status.SUCCESS

    if tc.error == True:
        return api.types.status.FAILURE

    if tc.resp_tcpdump is None or tc.resp_cleanup is None:
        return api.types.status.FAILURE

    #
    # Validate ERSPAN packets reception
    #
    tcp_erspan_pkts_expected = NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION
    udp_erspan_pkts_expected = NUMBER_OF_UDP_ERSPAN_PACKETS_PER_SESSION
    icmp_erspan_pkts_expected = NUMBER_OF_ICMP_ERSPAN_PACKETS_PER_SESSION
    if tc.iterators.direction != 'both':
        tcp_erspan_pkts_expected = tcp_erspan_pkts_expected/2
        udp_erspan_pkts_expected = udp_erspan_pkts_expected/2
        icmp_erspan_pkts_expected = icmp_erspan_pkts_expected/2

    if tc.dupcheck == 'disable':
        tcp_erspan_pkts_expected  =  (tcp_erspan_pkts_expected+1)*2
        udp_erspan_pkts_expected  *= 2
        icmp_erspan_pkts_expected *= 2

    for c in range(0, len(tc.collector)):
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
                            for c in range(0, len(tc.collector)):
                                if collector == tc.collector[c].ip_address:
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
                                res = eutils.validate_sip_dip(tc, 
                                      sip_msb, sip_lsb, dip_msb, dip_lsb,
                                      sport, dport, ip_proto, c)
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
                                res = eutils.validate_sip_dip(tc,
                                      sip_msb, sip_lsb, dip_msb, dip_lsb, 
                                      sport, dport, ip_proto, c)
                                if res != api.types.status.SUCCESS:
                                    result = api.types.status.FAILURE
                                break
                            w += 1

    #
    # Validate Number-of-ERSPAN-pkts received by the Collector
    #
    for c in range(0, len(tc.collector)):
        tc.result[c] = api.types.status.SUCCESS

        #
        # Perform TCP-pkt checks (only for hostpin mode, for now)
        #
        if tc.classic_mode == False:
            if tc.collector_tcp_pkts[c] < tcp_erspan_pkts_expected or\
               tc.collector_tcp_pkts[c] > (tcp_erspan_pkts_expected+1):
                tc.result[c] = api.types.status.FAILURE

        #
        # Perform UDP-pkt checks
        #
        if tc.collector_udp_pkts[c] != udp_erspan_pkts_expected:
            tc.result[c] = api.types.status.FAILURE

        #
        # Perform ICMP-pkt checks
        #
        if tc.collector_icmp_pkts[c] != icmp_erspan_pkts_expected:
            tc.result[c] = api.types.status.FAILURE

    for c in range(0, len(tc.collector)):
        if tc.result[c] == api.types.status.FAILURE:
            api.Logger.info("ERROR: {} {} {} {} {} {} {} ERSPAN packets to {}"\
            .format(tc.collector_tcp_pkts[c], tc.collector_udp_pkts[c],
                    tc.collector_icmp_pkts[c], tc.collector_other_pkts[c], 
                    tcp_erspan_pkts_expected, udp_erspan_pkts_expected,
                    icmp_erspan_pkts_expected, tc.collector[c].ip_address))
            result = api.types.status.FAILURE
        else:
            api.Logger.info("Number of ERSPAN packets {} {} {} {} to {}"\
            .format(tc.collector_tcp_pkts[c], tc.collector_udp_pkts[c],
                    tc.collector_icmp_pkts[c],
                    tc.collector_other_pkts[c], tc.collector[c].ip_address))

    for cmd in tc.resp_cleanup.commands:
        api.PrintCommandResults(cmd)
        if 'table-id' in cmd.command:
            if cmd.stdout != '':
                if 'table-id {}'.format(tc.lif_table_id) in cmd.command:
                    api.Logger.info("ERROR: lif-config Not Removed")
                    result = api.types.status.FAILURE
                if 'table-id {}'.format(tc.omap_table_id) in cmd.command:
                    api.Logger.info("ERROR: omap-config Not Removed")
                    result = api.types.status.FAILURE
                if 'table-id {}'.format(tc.mirror_table_id) in cmd.command:
                    api.Logger.info("ERROR: Mirror-config Not Removed")
                    result = api.types.status.FAILURE

    return result

def Teardown(tc):
    if tc.ignore == True:
        return api.types.status.SUCCESS

    #
    # Restore current Time-Close configs
    #
    if tc.classic_mode is not None and tc.classic_mode == False:
        update_timeout('tcp-close', tc.tcp_close_val)

    if tc.error == True:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

