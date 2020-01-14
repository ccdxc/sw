#! /usr/bin/python3
import sys
import os
import traceback
import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.testcases.security.utils as utils
import iota.harness.infra.utils.periodic_timer as pt

from trex.astf.api import *
from iota.test.iris.utils.trex_wrapper import *

def findWorkloadPeers(tc):
    for w1,w2 in  api.GetRemoteWorkloadPairs():
        if w1 not in tc.workloadPeers and w2 not in tc.workloadPeers:
            peers = tc.workloadPeers.get(w1, [])
            peers.append(w2)
            tc.workloadPeers[w1] = peers

            peers = tc.workloadPeers.get(w2, [])
            peers.append(w1)
            tc.workloadPeers[w2] = peers

def getRole(w):
    role = ['client', 'server']
    naples_node_name_list = api.GetWorkloadNodeHostnames()
    idx = naples_node_name_list.index(w.node_name)
    return role[idx%2]

def getClientServerPair(w, peerList):
    out = []
    if getRole(w) == "server":
        for p in peerList:
            out.append("%s,%s"%(w.ip_address, p.ip_address))
    else:
        for p in peerList:
            out.append("%s,%s"%(p.ip_address, w.ip_address))
    return ":".join(out)

def getTunables(tc, w, peerList):
    tunables = {}
    tunables['client_server_pair'] = getClientServerPair(w, peerList)
    tunables['cps'] = int((tc.iterators.cps) / (len(tc.workloadPeers)/2))
    tunables['max_active_flow'] = int((tc.iterators.max_active_flow) / (len(tc.workloadPeers)/2))
    return tunables

def getProfilePath(tc):
    file_path = os.path.dirname(__file__)
    return os.path.join(file_path, 'trex_profile/http_udp_high_active_flow.py')

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

def printWorkloadInfo(w):
    return "%s: %s(%s)(%s)"%(getRole(w), w.workload_name,
                             w.ip_address, w.mgmt_ip)

def printPeerList(peerList):
    out = ''
    for p in peerList:
        out += "%s,"%(printWorkloadInfo(p))
    return out

def showStats(tc):
    dec = "="*20
    print("%s"%"X"*60)
    for w in tc.workloadPeers.keys():
        try:
            print("%s %s %s"%(dec, printWorkloadInfo(w), dec))
            w.trexHandle._show_traffic_stats(False, is_sum = True)
        except Exception as e:
            #traceback.print_exc()
            api.Logger.error("Failed to show traffic stats for %s : %s"%(w.workload_name, e))

def switchPortFlap(tc):
    flap_count = 1
    num_ports = 1
    interval = 2
    down_time  = 2
    naples_nodes = api.GetNaplesHostnames()

    api.Logger.info("Flapping switch port on %s ..."%naples_nodes)
    ret = api.FlapDataPorts(naples_nodes, num_ports, down_time, flap_count, interval)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to flap the switch port")

def startPeriodicEvents(tc):
    for t in tc.events:
        t.start()

def cancelPeriodicEvents(tc):
    for thread in tc.events:
        thread.cancel()
        thread.join()
    tc.events = []

def securityPolicyChangeEvent(tc):
    agent_api.DeleteSgPolicies()

    for proto in ["tcp", "udp"]:
        policies = utils.GetTargetJsons(proto)
        for policy_json in policies:
            api.Logger.info("======================> pushing policy: %s "%(policy_json))
            newObjects = agent_api.AddOneConfig(policy_json)
            ret = agent_api.PushConfigObjects(newObjects)
            if ret != api.types.status.SUCCESS:
                api.error("Failed to push policies for %s"%policy_json)
            if agent_api.DeleteConfigObjects(newObjects):
                api.Logger.error("Failed to delete config object for %s"%policy_json)
            if agent_api.RemoveConfigObjects(newObjects):
                api.Logger.error("Failed to remove config object for %s"%policy_json)

    newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    agent_api.PushConfigObjects(newObjects)

def getPeriodicEvents(tc):
    # TRex Stats
    statsInterval = 10 #sec
    tc.events.append(pt.PeriodicTimer(statsInterval, showStats, args=[tc]))

    # Switch Port Flap
    flapInterval = getattr(tc.args, "switch_port_flap_interval", 10) #sec
    tc.events.append(pt.PeriodicTimer(flapInterval, switchPortFlap, args=[tc]))

    # Security policy
    securityPolicyChangeInterval = getattr(tc.args, "sec_policy_interval", 30)
    tc.events.append(pt.PeriodicTimer(securityPolicyChangeInterval,
                                      securityPolicyChangeEvent, args=[tc]))

def connectTrex(tc):
    for w, peerList in tc.workloadPeers.items():
        api.Logger.info("%s <--> %s"%(printWorkloadInfo(w), printPeerList(peerList)))
        try:
            api.Logger.info(getTunables(tc, w, peerList))
            w.trexHandle = TRexIotaWrapper(w, role=getRole(w), gw=peerList[0].ip_address)
            w.trexHandle.connect()
            w.trexHandle.reset()
            w.trexHandle.load_profile(getProfilePath(tc), getTunables(tc, w, peerList))
            w.trexHandle.clear_stats()
        except Exception as e:
            traceback.print_exc()
            api.Logger.info("Failed to setup TRex topology: %s"%e)
            cleanup(tc)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def startTrex(tc):
    latency_pps = getattr(tc.args, "latency_pps", 10)
    api.Logger.info("Starting traffic for duration %s sec @cps %s"
                    %(tc.iterators.duration, tc.iterators.cps))
    try:
        for w in tc.workloadPeers.keys():
            w.trexHandle.start(duration = int(tc.iterators.duration), latency_pps=latency_pps)
    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to start traffic on %s : %s"%(w.workload_name, e))
        cleanup(tc)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def barrier(tc):
    for w in tc.workloadPeers.keys():
        w.trexHandle.wait_on_traffic()
        api.Logger.info("Stopped traffic on %s"%(w.workload_name))

def Setup(tc):
    tc.workloadPeers = {}
    tc.events = []

    try:
        StoreCurrentHalLogLevel(tc)
        SetHalLogsLevelToError(tc)
        findWorkloadPeers(tc)

        ret = connectTrex(tc)
        if ret != api.types.status.SUCCESS:
            return ret

        ret = startTrex(tc)
        if ret != api.types.status.SUCCESS:
            return ret
    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to trigger the session scale : %s"%(e))
        cleanup(tc)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        # Start the events and let the party begin.
        getPeriodicEvents(tc)
        startPeriodicEvents(tc)

        try:
            barrier(tc)
        except:
            api.Logger.error("Barrier failed..")
            cancelPeriodicEvents(tc)
            raise

        cancelPeriodicEvents(tc)
        return api.types.status.SUCCESS
    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to trigger the session scale : %s"%(e))
        cleanup(tc)
        return api.types.status.FAILURE

def verifySessions(tc):
    try:
        utils.clearNaplesSessions()
        for n in api.GetNaplesHostnames():
            metrics = utils.GetDelphiSessionSummaryMetrics(n)
            api.Logger.info("Session summary metrics for %s => %s"%(n, metrics))
            if metrics['num_tcp_sessions'] != 0 or \
               metrics['num_tcp_sessions'] != 0:
                api.Logger.error("Found active udp or tcp session!")
                return api.types.status.FAILURE
    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to verify sessions : %s"%(e))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    try:
        ret = verifySessions(tc)
        if ret != api.types.status.SUCCESS:
            return ret
        api.Logger.info("Verified session stats successfully.")
        
        ret = utils.isHalAlive()
        if ret != api.types.status.SUCCESS:
            api.Logger.error("HAL is DEAD ..")
            return ret
        api.Logger.info("Verified the HAL process.")

    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to verify : %s"%(e))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def cleanup(tc):
    try:
        utils.clearNaplesSessions()
        RestoreHalLogLevel(tc)
    except:
        pass

    for w in tc.workloadPeers.keys():
        try:
            w.trexHandle.stop()
            w.trexHandle.disconnect()
            w.trexHandle.cleanup()
            w.trexHandle = None
        except:
            pass

def Teardown(tc):
    cleanup(tc)
    newObjects = agent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    agent_api.PushConfigObjects(newObjects)
    return api.types.status.SUCCESS
