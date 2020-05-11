#! /usr/bin/python3

import time
import iota.harness.api as api
import iota.test.iris.testcases.security.utils as utils
import iota.test.apulu.utils.vppctl as vppctl
import iota.test.apulu.utils.pdsctl as pdsctl
import iota.test.apulu.utils.flow as flowutils
import random
import sys
import os
import traceback
import sys
from iota.test.iris.utils.trex_wrapper import *

def __clearVPPEntity(entity):
    res = api.types.status.SUCCESS
    for node in api.GetNaplesHostnames():
        ret = vppctl.ParseClearCommand(node, entity)
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run clear errors at node {node}")
            res = ret

    return res

def __showVPPEntity(entity):
    res = api.types.status.SUCCESS
    for node in api.GetNaplesHostnames():
        ret, resp = vppctl.ParseShowCommand(node, entity)
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run clear errors at node {node}")
            res = ret

    return res

def chooseWorkload(tc):
    tc.skip = False
    if tc.selected:
        tc.workload_pairs = tc.selected
    else:
        if tc.args.type == 'local_only':
            tc.workload_pairs = api.GetLocalWorkloadPairs()
        else:
            tc.workload_pairs = api.GetRemoteWorkloadPairs()

    naples_node_name_list = api.GetNaplesHostnames()
    random.seed(tc.seed)

    while 1:
        w1, w2 = random.choice(tc.workload_pairs)
        tc.workload_pair = [w1, w2]

        if w1.node_name in naples_node_name_list or \
           w2.node_name in naples_node_name_list:
            tc.workload_pair = (w1, w2)
            return

def getTunables(tc):
    tunables = {}
    tunables['server_ip'] = tc.clientHandle.workload.ip_address
    tunables['client_ip'] = tc.serverHandle.workload.ip_address
    tunables['cps'] = int(tc.iterators.cps)

    if tc.iterators.proto == 'tcp':
        tunables['pcap_file'] = "http_gzip.cap"
    elif tc.iterators.proto == 'udp':
        tunables['pcap_file'] = "chargen-udp.pcap" #"sip_0.pcap"
    else:
        raise Exception("Pcap file not defined for proto: %s"%
                        tc.iterators.proto)
    return tunables

def getProfilePath(tc):
    file_path = os.path.dirname(__file__)
    if tc.iterators.proto in ['tcp', 'udp']:
        return os.path.join(file_path, 'trex_profile/trex_generic_profile_apulu.py')
    else:
        raise ValueError('Profile not defined for protocol %s'%tc.iterators.proto)

def Setup(tc):
    tc.seed = random.randrange(sys.maxsize)
    api.Logger.info("Using seed : %s"%(tc.seed))
    tc.serverHandle = None
    tc.clientHandle = None

    chooseWorkload(tc)
    server, client = tc.workload_pair[0], tc.workload_pair[1]
    api.Logger.info("Server: %s(%s)(%s) <--> Client: %s(%s)(%s)" %\
                    (server.workload_name, server.ip_address,
                     server.mgmt_ip, client.workload_name,
                     client.ip_address, client.mgmt_ip))
    try:
        tc.serverHandle = TRexIotaWrapper(server, role="server", gw=client.ip_address)
        tc.clientHandle = TRexIotaWrapper(client, role="client", gw=server.ip_address)

        api.Logger.info("connect trex...")
        tc.serverHandle.connect()
        tc.clientHandle.connect()

        api.Logger.info("reset connection...")
        tc.serverHandle.reset()
        tc.clientHandle.reset()

        api.Logger.info("setting profile...")
        profile_path = getProfilePath(tc)
        tc.serverHandle.load_profile(getProfilePath(tc), getTunables(tc))
        tc.clientHandle.load_profile(getProfilePath(tc), getTunables(tc))

    except Exception as e:
        #traceback.print_exc()
        api.Logger.info("Failed to setup TRex topology: %s"%e)
        #cleanup(tc)
        return api.types.status.FAILURE

    api.Logger.info("Clear hardware state before trex trigger...")
    flowutils.clearFlowTable(tc.workload_pairs)
    #__clearVPPEntity("errors")
    __clearVPPEntity("flow statistics")
    __clearVPPEntity("flow entries")

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        if tc.skip: return api.types.status.SUCCESS

        tc.serverHandle.clear_stats()
        tc.clientHandle.clear_stats()

        # To avoid any packet drop because of server and client start/stop mismatch
        # running server little longer than client.
        tc.serverHandle.start(duration = int(tc.iterators.duration)+15)
        tc.clientHandle.start(duration = int(tc.iterators.duration))

        api.Logger.info("Started traffic for duration %s sec @cps %s between server(%s) and client(%s) "
                        %( tc.iterators.duration, tc.iterators.cps,
                          tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        tc.serverHandle.wait_on_traffic()
        tc.clientHandle.wait_on_traffic()

        api.Logger.info("Stopped traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        return api.types.status.SUCCESS
    except Exception as e:
        
        api.Logger.info("Exception hit for traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))


        api.Logger.error("Failed to start traffic between server(%s) and client(%s) "
                         ": %s"%(tc.serverHandle.workload.workload_name,
                                 tc.clientHandle.workload.workload_name, e))
        #cleanup(tc)
        return api.types.status.FAILURE

def Verify(tc):
    udp_tolerance = 0.1
    server, client = tc.workload_pair[0], tc.workload_pair[1]
    try:
        if tc.skip: return api.types.status.SUCCESS

        cstat = tc.clientHandle.get_stats()
        sstat = tc.serverHandle.get_stats()
        ct = cstat['traffic']['client']
        st = sstat['traffic']['server']

        api.Logger.info("Verifying traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        #__showVPPEntity("flow statistics")
        ret, client_inserts = vppctl.GetVppV4FlowStatistics(client.node_name, "Insert")
        ret, client_removals = vppctl.GetVppV4FlowStatistics(client.node_name, "Remove")
        ret, server_inserts = vppctl.GetVppV4FlowStatistics(server.node_name, "Insert")
        ret, server_removals = vppctl.GetVppV4FlowStatistics(server.node_name, "Remove")

        api.Logger.info("Flow inserts between server(%d) and client(%d) "
                        %(int(server_inserts), int(client_inserts)))

        api.Logger.info("Flow removed/aged on server(%d) and client(%d) "
                        %(int(server_removals), int(client_removals)))

        client_hw_rem = int(client_removals)
        client_hw_ins = int(client_inserts)
        server_hw_rem = int(server_removals)
        server_hw_ins = int(server_inserts)

        #Following useful when we wait for 120 seconds and allow UDP ageing
        if client_hw_rem > client_hw_ins:
            api.Logger.error("Flow inserts on client-DSC (%d) is less than flows removed:%d"%
                             (client_hw_ins, client_hw_rem))
            return api.types.status.FAILURE

        #Following useful when we wait for 120 seconds and allow UDP ageing
        if server_hw_rem > server_hw_ins:
            api.Logger.error("Flow inserts on server-DSC (%d) is less than flows removed:%d"%
                             (server_hw_ins, server_hw_rem))
            return api.types.status.FAILURE


        if tc.iterators.proto == 'tcp':
            return  TRexIotaWrapper.validateCloudTcpStats(ct, st, int(tc.iterators.cps),
                                          int(tc.iterators.duration),
                                          int(client_inserts),
                                          int(server_inserts))
        elif tc.iterators.proto == 'udp':
            return  TRexIotaWrapper.validateCloudUdpStats(ct, st, int(tc.iterators.cps),
                                          int(tc.iterators.duration),
                                          int(client_inserts),
                                          int(server_inserts),
                                          udp_tolerance)
        else:
            raise Exception("Unknown protocol %s"%tc.iterators.proto)

    except Exception as e:

        api.Logger.info("Verify failed for traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))


        api.Logger.error("Failed to verify the CPS : %s"%(e))
        #cleanup(tc)
        return api.types.status.FAILURE

def cleanup(tc):
    try:

        if tc.clientHandle:
            #api.Logger.info("disconnect client(%s) "
            #            %( tc.clientHandle.workload.workload_name))

            #tc.clientHandle.disconnect()
            api.Logger.info("cleanup client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle.cleanup()
            api.Logger.info("all cleanup done on client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle = None

        if tc.serverHandle:
            #api.Logger.info("disconnect server(%s) "
            #            %( tc.serverHandle.workload.workload_name))

            #tc.serverHandle.disconnect()
            api.Logger.info("cleanup server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle.cleanup()
            api.Logger.info("all cleanup done on server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle = None
    except:
        pass

def Teardown(tc):
    flowutils.clearFlowTable(tc.workload_pairs)
    __clearVPPEntity("flow statistics")
    __clearVPPEntity("flow entries")
    cleanup(tc)
    api.Logger.info("Teardown done")
    return api.types.status.SUCCESS
