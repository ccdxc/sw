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
import copy
from scapy.utils import *
from scapy.utils import rdpcap
from scapy.utils import wrpcap
from scapy import packet
from scapy.all import Ether

NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION  = 6
NUMBER_OF_UDP_ERSPAN_PACKETS_PER_SESSION  = 2
NUMBER_OF_ICMP_ERSPAN_PACKETS_PER_SESSION = 2

FLOWMON_COLLECTOR_MAX = 4

def Setup(tc):
    #
    # Set up global variables
    #
    tc.feature = 'lif-flow-erspan'
    tc.udp_count = NUMBER_OF_UDP_ERSPAN_PACKETS_PER_SESSION >> 1
    tc.icmp_count = NUMBER_OF_ICMP_ERSPAN_PACKETS_PER_SESSION >> 1

    tc.tcp_close_val = None
    tc.classic_mode = False
    tc.ignore = False
    tc.error = False

    #
    # Establish Workloads
    #
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
    # Ignore non-applicable test-options in Sanity mode
    # - [peer = local] is not supported until ps_2263 is fixed
    # - [collector = local] is not supported until ps_2790 is fixed
    # - Multi-collector testing is not enabled in freebsd
    #   environment until tcpdump capture on secondary-IP is resolved
    #
    if (tc.args.ps_2263 != 'fixed' and tc.iterators.peer == 'local') or\
       (tc.args.ps_2790 != 'fixed' and tc.iterators.collector == 'local') or\
       (api.GetNodeOs(tc.naples.node_name) == 'freebsd' and\
        tc.iterators.ccount > 1):
        tc.ignore = True
        return api.types.status.SUCCESS

    if api.IsBareMetalWorkloadType(tc.naples.node_name):
        tc.classic_mode = True

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

    if tc.classic_mode == True:
        result = eutils.establishCollectorWorkloadInClassicMode(tc,
                                                                '10.255.0.2')
        if result != api.types.status.SUCCESS:
            tc.error = True
            return result

    #
    # Allocate Secondary-IPs for collectors as needed in order to test up to
    # Eight collectors
    #
    result = eutils.establishCollectorSecondaryIPs(tc)
    if result != api.types.status.SUCCESS:
        tc.error = True
        return result

    #
    # Generate feature specific Collector list
    #
    eutils.generateFeatureCollectorList(tc)

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
    if tc.classic_mode == False:
        tc.tcp_close_val = get_timeout_val('tcp-close')
        update_timeout('tcp-close', "1s")

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.ignore == True:
        return api.types.status.SUCCESS

    if tc.error == True:
        return api.types.status.FAILURE

    protoDir1 = api.GetTopologyDirectory() +\
                "/gen/telemetry/{}/{}".format('mirror', 'lif')
    api.Logger.info("Template Config files location: ", protoDir1)
    protoDir2 = api.GetTopologyDirectory() +\
                "/gen/telemetry/{}/{}".format('mirror', tc.iterators.proto)
    api.Logger.info("Template Config files location: ", protoDir2)

    result = api.types.status.SUCCESS

    count = 0
    MirrorPolicies = utils.GetTargetJsons('mirror', tc.iterators.proto)
    LifPolicies = utils.GetTargetJsons('mirror', 'lif')
    for mirror_json in MirrorPolicies:
        #
        # Get template-Mirror Config
        #
        newMirrorObjects = agent_api.AddOneConfig(mirror_json)
        if len (newMirrorObjects) == 0:
            api.Logger.error("Adding new Mirror objects to store failed")
            tc.error = True
            return api.types.status.FAILURE
        agent_api.RemoveConfigObjects(newMirrorObjects)

        #
        # Ignore Multi-collector template config's, since Expanded-Telemetry
        # testbundle dynamically creates such config's
        #
        if len(newMirrorObjects[0].spec.collectors) > 1:
            continue

        #
        # Modify template-Mirror Config to make sure that
        # Naples-node # act as either source or destination
        #
        # Set up Collector in the remote node
        #
        eutils.generateMirrorConfig(tc, mirror_json, newMirrorObjects)

        ret_count = 0
        for i in range(0, len(tc.mirror_verif)):
            #
            # If Execution-Optimization is enabled, no need to run the test
            # for the same protocol more than once
            #
            if i > 0 and tc.mirror_verif[i]['protocol'] ==\
                         tc.mirror_verif[i-1]['protocol']:
                continue

            #
            # Flow-ERSPAN for TCP-traffic is not tested (yet) in
            # Classic-mode until applicable pkt-trigger tools are identified
            #
            if tc.classic_mode == True and\
               tc.mirror_verif[i]['protocol'] == 'tcp':
                continue

            for policy_json in LifPolicies:
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
                if newObjects[0].kind == 'InterfaceMirrorSession':
                    tc.lif_collector_objects = newObjects
                    agent_api.RemoveConfigObjects(tc.lif_collector_objects)
                elif newObjects[0].kind == 'Interface':
                    tc.interface_objects = newObjects
                    agent_api.RemoveConfigObjects(tc.interface_objects)

            #
            # Push Collector object, if applicable
            #
            colObjects = tc.lif_collector_objects
            ret = eutils.generateLifCollectorConfig(tc, colObjects)
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Unable to identify Collector Workload")
                tc.error = True
                return api.types.status.FAILURE

            ret = agent_api.PushConfigObjects(colObjects, 
                                              [tc.naples.node_name])
            if ret != api.types.status.SUCCESS:
                api.Logger.error("Unable to push collector objects")
                tc.error = True
                return api.types.status.FAILURE

            #
            # Push Mirror Config to Naples
            #
            ret = agent_api.PushConfigObjects(newMirrorObjects,
                                              [tc.naples.node_name])
            if ret != api.types.status.SUCCESS:
                agent_api.DeleteConfigObjects(tc.lif_collector_objects, 
                                              [tc.naples.node_name])
                api.Logger.error("Unable to push mirror objects")
                tc.error = True
                return api.types.status.FAILURE

            #
            # Update Interface objects
            #
            ifObjects = tc.interface_objects
            ret = eutils.generateLifInterfaceConfig(tc, ifObjects, 
                                                   tc.lif_collector_objects)
            if ret != api.types.status.SUCCESS:
                agent_api.DeleteConfigObjects(tc.lif_collector_objects, 
                                              [tc.naples.node_name])
                agent_api.DeleteConfigObjects(newMirrorObjects, 
                                              [tc.naples.node_name])
                api.Logger.error("Unable to identify Uplink/LIF Interfaces")
                tc.error = True
                return api.types.status.FAILURE

            ret = agent_api.UpdateConfigObjects(ifObjects, 
                                               [tc.naples.node_name])
            if ret != api.types.status.SUCCESS:
                agent_api.DeleteConfigObjects(tc.lif_collector_objects, 
                                              [tc.naples.node_name])
                agent_api.DeleteConfigObjects(newMirrorObjects, 
                                              [tc.naples.node_name])
                api.Logger.error("Unable to update interface objects")
                tc.error = True
                return api.types.status.FAILURE

            #
            # Establish Forwarding set up between Naples-peer and Collectors
            #
            eutils.establishForwardingSetup(tc)

            req_tcpdump_lif_erspan = api.Trigger_CreateExecuteCommandsRequest(\
                                     serial = True)
            for c in range(0, len(tc.lif_collector)):
                #
                # Set up TCPDUMP's on the collector
                #
                idx = tc.lif_collector_idx[c]
                if tc.lif_collector[c].IsNaples():
                    cmd = "tcpdump -c 1000 -XX -vv -nni {} ip proto gre\
                          and dst {} --immediate-mode -U -w lif-mirror-{}.pcap"\
                          .format(tc.lif_collector[c].interface, 
                                  tc.collector_ip_address[idx], c)
                else:
                    cmd = "tcpdump -p -c 1000 -XX -vv -nni {} ip proto gre\
                          and dst {} --immediate-mode -U -w lif-mirror-{}.pcap"\
                          .format(tc.lif_collector[c].interface, 
                                  tc.collector_ip_address[idx], c)
                eutils.add_command(req_tcpdump_lif_erspan, 
                                   tc.lif_collector[c], cmd, True)

            resp_tcpdump_lif_erspan = api.Trigger(req_tcpdump_lif_erspan)
            for cmd in resp_tcpdump_lif_erspan.commands:
                api.PrintCommandResults(cmd)

            if tc.collection == 'distinct':
                req_tcpdump_flow_erspan = \
                    api.Trigger_CreateExecuteCommandsRequest(serial = True)
                for c in range(0, len(tc.flow_collector)):
                    #
                    # Set up TCPDUMP's on the collector
                    #
                    idx = tc.flow_collector_idx[c]
                    if tc.flow_collector[c].IsNaples():
                        cmd = "tcpdump -c 600 -XX -vv -nni {} ip proto gre\
                        and dst {} --immediate-mode -U -w flow-mirror-{}.pcap"\
                        .format(tc.flow_collector[c].interface,
                                tc.collector_ip_address[idx], c)
                    else:
                        cmd = "tcpdump -p -c 600 -XX -vv -nni {} ip proto\
                        gre and dst {} --immediate-mode -U\
                        -w flow-mirror-{}.pcap"\
                        .format(tc.flow_collector[c].interface,
                                tc.collector_ip_address[idx], c)
                    eutils.add_command(req_tcpdump_flow_erspan,
                                       tc.flow_collector[c], cmd, True)

                resp_tcpdump_flow_erspan = api.Trigger(req_tcpdump_flow_erspan)
                for cmd in resp_tcpdump_flow_erspan.commands:
                    api.PrintCommandResults(cmd)

            #
            # Classic mode requires a delay to make sure that TCPDUMP 
            # background process is fully up
            #
            if tc.classic_mode == True:
                time.sleep(2)

            #
            # Trigger packets for ERSPAN / FLOWMON to take effect
            #
            tc.protocol = tc.mirror_verif[i]['protocol']
            tc.dest_port = utils.GetDestPort(tc.mirror_verif[i]['port'])

            protocol = tc.protocol
            tc.protocol = 'all'
            if api.GetNodeOs(tc.naples.node_name) == 'linux':
                eutils.triggerTrafficInClassicModeLinux(tc)
            else:
                eutils.triggerTrafficInHostPinModeOrFreeBSD(tc)
            tc.protocol = protocol

            #
            # Dump sessions/flows/P4-tables for debug purposes
            #
            eutils.showSessionAndP4TablesForDebug(tc)

            #
            # Terminate TCPDUMP background process
            #
            term_resp_tcpdump_lif_erspan = api.Trigger_TerminateAllCommands(\
                                           resp_tcpdump_lif_erspan)
            tc.resp_tcpdump_lif_erspan = api.Trigger_AggregateCommandsResponse(\
                          resp_tcpdump_lif_erspan, term_resp_tcpdump_lif_erspan)

            if tc.collection == 'distinct':
                term_resp_tcpdump_flow_erspan = \
                api.Trigger_TerminateAllCommands(resp_tcpdump_flow_erspan)
                tc.resp_tcpdump_flow_erspan = \
                api.Trigger_AggregateCommandsResponse(\
                resp_tcpdump_flow_erspan, term_resp_tcpdump_flow_erspan)

            # Delete the objects
            eutils.deGenerateLifInterfaceConfig(tc, tc.interface_objects, 
                                                tc.lif_collector_objects)
            agent_api.UpdateConfigObjects(tc.interface_objects,
                                         [tc.naples.node_name])
            agent_api.DeleteConfigObjects(tc.lif_collector_objects, 
                                          [tc.naples.node_name])
            agent_api.DeleteConfigObjects(newMirrorObjects, 
                                          [tc.naples.node_name])

            #
            # Make sure that Mirror-config has been removed
            #
            tc.resp_cleanup = eutils.showP4TablesForValidation(tc)

            #
            # Validate ERSPAN packets reception
            #
            tc.tcp_erspan_pkts_expected = \
                                   NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION
            tc.udp_erspan_pkts_expected = (tc.udp_count << 1)
            tc.icmp_erspan_pkts_expected = (tc.udp_count << 1) + \
                                           (tc.icmp_count << 1)

            if tc.iterators.direction != 'both':
                tc.tcp_erspan_pkts_expected >>= 1
                tc.udp_erspan_pkts_expected >>= 1
                tc.icmp_erspan_pkts_expected >>= 1

            if tc.dupcheck == 'disable':
                tc.tcp_erspan_pkts_expected = (tc.tcp_erspan_pkts_expected+1)\
                                               << 1
                tc.udp_erspan_pkts_expected  <<= 1
                tc.icmp_erspan_pkts_expected <<= 1

            #
            # Adjust Expected-pkt-counts taking into account Flow-ERSPAN
            # Config's
            #
            if tc.collection == 'unified':
                if (tc.protocol == 'tcp' or tc.iterators.proto == 'mixed')\
                    and tc.iterators.direction != 'both':
                    tc.tcp_erspan_pkts_expected <<= 1
                if (tc.protocol == 'udp' or tc.iterators.proto == 'mixed')\
                    and tc.iterators.direction != 'both':
                    tc.udp_erspan_pkts_expected <<= 1
                if tc.protocol == 'icmp' or tc.iterators.proto == 'mixed':
                    if tc.iterators.direction != 'both':
                        tc.icmp_erspan_pkts_expected <<= 1
                    #if tc.iterators.direction != 'egress':
                    #    tc.icmp_erspan_pkts_expected += 1

            protocol = tc.protocol
            tc.protocol = 'all'
            tc.feature = 'lif-erspan'
            tc.resp_tcpdump_erspan = tc.resp_tcpdump_lif_erspan
            res_1 = eutils.validateErspanPackets(tc, tc.lif_collector,
                                                 tc.lif_collector_idx)
            tc.protocol = protocol

            if tc.collection == 'distinct':
                tc.tcp_erspan_pkts_expected = 0
                tc.udp_erspan_pkts_expected = 0
                tc.icmp_erspan_pkts_expected = 0
                if tc.protocol == 'tcp' or tc.iterators.proto == 'mixed':
                    tc.tcp_erspan_pkts_expected = \
                               NUMBER_OF_TCP_ERSPAN_PACKETS_PER_SESSION
                if tc.protocol == 'udp' or tc.iterators.proto == 'mixed':
                    tc.udp_erspan_pkts_expected = (tc.udp_count << 1)
                if tc.protocol == 'icmp' or tc.iterators.proto == 'mixed':
                    tc.icmp_erspan_pkts_expected = (tc.udp_count << 1) + \
                                                   (tc.icmp_count << 1)

                tc.protocol = 'all'
                tc.feature = 'flow-erspan'
                tc.resp_tcpdump_erspan = tc.resp_tcpdump_flow_erspan
                res_f = eutils.validateErspanPackets(tc, tc.flow_collector,
                                                     tc.flow_collector_idx)
                tc.protocol = protocol
                if res_f == api.types.status.FAILURE:
                    result = api.types.status.FAILURE

            #
            # Validate Config-cleanup
            #
            res_2 = eutils.validateConfigCleanup(tc)

            if res_1 == api.types.status.FAILURE or\
               res_2 == api.types.status.FAILURE:
                result = api.types.status.FAILURE

            if result == api.types.status.FAILURE:
                break

            ret_count += 1

        if result == api.types.status.FAILURE:
            break

        count += ret_count

    tc.SetTestCount(count)
    return result

def Verify(tc):
    if tc.ignore == True:
        return api.types.status.SUCCESS

    if tc.error == True:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    if tc.ignore == True:
        return api.types.status.SUCCESS

    #
    # Restore current Time-Close configs
    #
    if tc.tcp_close_val is not None:
        update_timeout('tcp-close', tc.tcp_close_val)

    if tc.error == True:
        return api.types.status.FAILURE

    eutils.deEstablishCollectorSecondaryIPs(tc)

    return api.types.status.SUCCESS

