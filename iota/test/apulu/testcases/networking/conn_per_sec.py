#! /usr/bin/python3

import time
import iota.harness.api as api
import iota.test.iris.testcases.security.utils as utils
import random
import sys
import os
import traceback
import sys
from iota.test.iris.utils.trex_wrapper import *

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
        tunables['pcap_file'] = "http_get.pcap"
    elif tc.iterators.proto == 'udp':
        tunables['pcap_file'] = "sip_0.pcap"
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

def StoreCurrentHalLogLevel(tc):
    tc.halLogLevelByNode = {}
    try:
        for n in api.GetNaplesHostnames():
            tc.halLogLevelByNode[n] = utils.GetHalLogsLevel(n)
    except Exception as e:
        api.Logger.error("%s"%e)

def SetHalLogsLevelToError(tc):
    try:
        for n in api.GetNaplesHostnames():
            utils.SetHalLogsLevel(n, "error")
    except Exception as e:
        api.Logger.error("%s"%e)

def RestoreHalLogLevel(tc):
    try:
        for n,l in tc.halLogLevelByNode.items():
            utils.SetHalLogsLevel(n, l)
    except Exception as e:
        api.Logger.error("%s"%e)

def Setup(tc):
    tc.seed = random.randrange(sys.maxsize)
    api.Logger.info("Using seed : %s"%(tc.seed))
    tc.serverHandle = None
    tc.clientHandle = None

    #StoreCurrentHalLogLevel(tc)
    #SetHalLogsLevelToError(tc)

    chooseWorkload(tc)
    server, client = tc.workload_pair[0], tc.workload_pair[1]
    api.Logger.info("Server: %s(%s)(%s) <--> Client: %s(%s)(%s)" %\
                    (server.workload_name, server.ip_address,
                     server.mgmt_ip, client.workload_name,
                     client.ip_address, client.mgmt_ip))
    try:
        tc.serverHandle = TRexIotaWrapper(server, role="server", gw=client.ip_address)
        tc.clientHandle = TRexIotaWrapper(client, role="client", gw=server.ip_address)

        tc.serverHandle.connect()
        tc.clientHandle.connect()

        tc.serverHandle.reset()
        tc.clientHandle.reset()

        profile_path = getProfilePath(tc)
        tc.serverHandle.load_profile(getProfilePath(tc), getTunables(tc))
        tc.clientHandle.load_profile(getProfilePath(tc), getTunables(tc))

    except Exception as e:
        traceback.print_exc()
        api.Logger.info("Failed to setup TRex topology: %s"%e)
        cleanup(tc)
        return api.types.status.FAILURE

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
        #traceback.print_exc()
        
        api.Logger.info("Exception hit for traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))


        api.Logger.error("Failed to start traffic between server(%s) and client(%s) "
                         ": %s"%(tc.serverHandle.workload.workload_name,
                                 tc.clientHandle.workload.workload_name, e))
        cleanup(tc)
        return api.types.status.FAILURE

def Verify(tc):
    try:
        if tc.skip: return api.types.status.SUCCESS

        cstat = tc.clientHandle.get_stats()
        sstat = tc.serverHandle.get_stats()
        ct = cstat['traffic']['client']
        st = sstat['traffic']['server']

        #cleanup(tc)

        api.Logger.info("Verifying traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))


        if tc.iterators.proto == 'tcp':
            return  TRexIotaWrapper.validateTcpStats(ct, st, int(tc.iterators.cps),
                                          int(tc.iterators.duration))
        elif tc.iterators.proto == 'udp':
            return  TRexIotaWrapper.validateUdpStats(ct, st, int(tc.iterators.cps),
                                          int(tc.iterators.duration))
        else:
            raise Exception("Unknown protocol %s"%tc.iterators.proto)

    except Exception as e:
        #traceback.print_exc()

        api.Logger.info("Verify failed for traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))


        api.Logger.error("Failed to verify the CPS : %s"%(e))
        cleanup(tc)
        return api.types.status.FAILURE

def cleanup(tc):
    try:
        #TODO:bmital
        #utils.clearNaplesSessions()
        #RestoreHalLogLevel(tc)

        if tc.clientHandle:
            api.Logger.info("disconnect client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle.disconnect()
            api.Logger.info("cleanup client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle.cleanup()
            api.Logger.info("all cleanup done on client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle = None

        if tc.serverHandle:
            api.Logger.info("disconnect server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle.disconnect()
            api.Logger.info("cleanup server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle.cleanup()
            api.Logger.info("all cleanup done on server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle = None
    except:
        pass

def Teardown(tc):
    #cleanup(tc)
    return api.types.status.SUCCESS
