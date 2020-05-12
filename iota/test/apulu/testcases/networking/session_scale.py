#! /usr/bin/python3
import sys
import os
import time
import random
import traceback
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.pdsutils as pds_utils
import iota.test.apulu.utils.flow as flowutils
import iota.harness.infra.utils.periodic_timer as pt
import zmq

from iota.test.iris.utils.trex_wrapper import *

skip_trex_traffic = False

def installRequiredPackages(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_cookies = []
    tc.resp = None
    for wl in tc.workloads:
        if not wl.IsNaples():
            continue
        # Install python scapy packages
        cmd_cookie = "Installing yum packages in WL:%s"% wl.workload_name
        tc.cmd_cookies.append(cmd_cookie)
        api.Logger.info(cmd_cookie)
        cmd = "yum -y -q update"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        #cmd = "apt remove -y python3-pip && apt install -y python3-pip"
        #api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)

        cmd_cookie = "Installing pyzmq packages in WL:%s"% wl.workload_name
        tc.cmd_cookies.append(cmd_cookie)
        api.Logger.info(cmd_cookie)
        cmd = "/usr/bin/yes | pip3 uninstall pyzmq ; pip3 install pyzmq"
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)

    tc.resp = api.Trigger(req)

    if tc.resp:
        cookie_idx = 0
        for cmd in tc.resp.commands:
            api.Logger.info(tc.cmd_cookies[cookie_idx])
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                api.Logger.info("Failed for %s"%tc.cmd_cookies[cookie_idx])
            cookie_idx += 1
    tc.cmd_cookies = []
    return api.types.status.SUCCESS


def findWorkloadPeers(tc):
    for w1,w2 in  api.GetRemoteWorkloadPairs():
        if w1 not in tc.workloadPeers and w2 not in tc.workloadPeers:
            peers = tc.workloadPeers.get(w1, [])
            peers.append(w2)
            tc.workloadPeers[w1] = peers

            peers = tc.workloadPeers.get(w2, [])
            peers.append(w1)
            tc.workloadPeers[w2] = peers


def __getOperations(tc_operation):
    opers = list()
    if tc_operation is None:
        return opers
    else:
        opers = list(map(lambda x:x.capitalize(), tc_operation))
    return opers


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
    return os.path.join(file_path, 'trex_profile/http_udp_high_active_flow_apulu.py')

def StoreCurrentPdsLogLevel(tc):
    tc.pdsLogLevelByNode = {}
    try:
        for n in api.GetNaplesHostnames():
            tc.pdsLogLevelByNode[n] = pds_utils.GetPdsDefaultLogLevel(n)
    except Exception as e:
        api.Logger.error("%s"%e)
    return api.types.status.SUCCESS

def SetPdsLogsLevelToError(tc):
    try:
        for n in api.GetNaplesHostnames():
            pds_utils.SetPdsLogsLevel(n, "error")
    except Exception as e:
        api.Logger.error("%s"%e)
    return api.types.status.SUCCESS

def RestorePdsLogLevel(tc):
    try:
        for n,l in tc.pdsLogLevelByNode.items():
            pds_utils.SetPdsLogsLevel(n, l)
    except Exception as e:
        api.Logger.error("%s"%e)
    return api.types.status.SUCCESS

def printWorkloadInfo(w):
    return "%s: %s(%s)(%s)"%(getRole(w), w.workload_name,
                             w.ip_address, w.mgmt_ip)

def printPeerList(peerList):
    out = ''
    for p in peerList:
        out += "%s,"%(printWorkloadInfo(p))
    return out

def showStats(tc):
    if tc.cancel:
        api.Logger.info("Canceling showStats...")
        sys.exit(0)

    api.Logger.info("Running showStats...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    if skip_trex_traffic:
        return api.types.status.SUCCESS
    dec = "="*20
    print("%s"%"X"*60)
    for w in tc.workloadPeers.keys():
        try:
            print("%s %s %s"%(dec, printWorkloadInfo(w), dec))
            w.trexHandle._show_traffic_stats(False, is_sum = True)
        except Exception as e:
            #traceback.print_exc()
            api.Logger.error("Failed to show traffic stats for %s : %s"%(w.workload_name, e))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


def switchPortFlap(tc):
    if tc.cancel:
        api.Logger.info("Canceling switchPortFlap...")
        sys.exit(0)

    api.Logger.info("Running switchPortFlap...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    flap_count = 1
    num_ports = 1
    interval = 2
    down_time  = 2
    naples_nodes = api.GetNaplesHostnames()

    api.Logger.info("Flapping switch port on %s ..."%naples_nodes)
    ret = api.FlapDataPorts(naples_nodes, num_ports, down_time, flap_count, interval)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to flap the switch port")
        return ret
    return api.types.status.SUCCESS


def clearFlows(tc):
    if tc.cancel:
        api.Logger.info("Canceling clearFlows...")
        sys.exit(0)

    api.Logger.info("Running clearFlows...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    flowutils.clearFlowTable(api.GetRemoteWorkloadPairs())
    return api.types.status.SUCCESS


def configDeleteTrigger(tc):
    res = api.types.status.SUCCESS
    tc.is_config_deleted = False
    api.Logger.info("configDeleteTrigger for %s"%tc.obj_sel)
    for op in tc.opers:
        res = config_api.ProcessObjectsByOperation(op, tc.selected_objs)
        if op == 'Delete':
            tc.is_config_deleted = True
        if res != api.types.status.SUCCESS:
            api.Logger.error(f"config delete operation failed : {res}")
            break;
    return res

def configRestoreTrigger(tc):
    res = api.types.status.SUCCESS
    if tc.is_config_deleted:
        api.Logger.info("configRestoreTrigger for %s"%tc.obj_sel)
        rs = config_api.RestoreObjects('Delete', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"config restore operation failed : {rs}")
            res = api.types.status.FAILURE
    return res

def configurationChangeEvent(tc):
    if tc.cancel:
        api.Logger.info("Canceling configurationChangeEvent...")
        sys.exit(0)
    api.Logger.info("Running configurationChangeEvent...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    tc.opers = __getOperations('delete')
    # select random object for each config change event
    tc.obj_sel = random.choice(tc.args.objtype)
    tc.selected_objs = config_api.SetupConfigObjects(tc.obj_sel)

    if configDeleteTrigger(tc) != api.types.status.SUCCESS:
        api.Logger.error("Failed in configDeleteTrigger...")

    if configRestoreTrigger(tc) != api.types.status.SUCCESS:
        api.Logger.error("Failed in configRestoreTrigger...")

    return api.types.status.SUCCESS


def connectTrex(tc):
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for w, peerList in tc.workloadPeers.items():
        api.Logger.info("%s <--> %s"%(printWorkloadInfo(w), printPeerList(peerList)))
        try:
            api.Logger.info(getTunables(tc, w, peerList))
            w.trexHandle = TRexIotaWrapper(w, role=getRole(w), gw=peerList[0].ip_address)
            w.trexHandle.connect()
            w.trexHandle.reset()
            w.trexHandle.load_profile(getProfilePath(tc), getTunables(tc, w, peerList))
            api.Logger.info("%s <--> %s connect and load done"%(printWorkloadInfo(w), printPeerList(peerList)))
            w.trexHandle.clear_stats()
        except Exception as e:
            traceback.print_exc()
            api.Logger.info("Failed to setup TRex topology: %s"%e)
            cleanup(tc)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def startTrex(tc):
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    latency_pps = getattr(tc.args, "latency_pps", 10)
    api.Logger.info("Starting traffic for duration %s sec @cps %s"
                    %(tc.iterators.duration, tc.iterators.cps))
    try:
        for w in tc.workloadPeers.keys():
            w.trexHandle.start(duration = int(tc.iterators.duration), latency_pps=latency_pps, nc=True)
    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to start traffic on %s : %s"%(w.workload_name, e))
        cleanup(tc)
        return api.types.status.FAILURE
    return api.types.status.SUCCESS


def Setup(tc):
    tc.cancel = False
    tc.workloadPeers = {}
    tc.workloads = api.GetWorkloads()
    tc.events = []

    # temporary change to install required packages for trex,
    # until they are made part of the WL container.
    installRequiredPackages(tc)

    try:
        StoreCurrentPdsLogLevel(tc)
        SetPdsLogsLevelToError(tc)
        findWorkloadPeers(tc)

        if not skip_trex_traffic:
            ret = connectTrex(tc)
            if ret != api.types.status.SUCCESS:
                return ret

            ret = startTrex(tc)
            if ret != api.types.status.SUCCESS:
                return ret
        else:
            for w, peerList in tc.workloadPeers.items():
                w.trexHandle = None

    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to trigger the session scale : %s"%(e))
        cleanup(tc)
        return api.types.status.FAILURE
    return api.types.status.SUCCESS


def Trigger(tc):
    time.sleep(int(tc.iterators.duration))
    tc.cancel = True
    return api.types.status.SUCCESS


def verifyFlows(tc):
    try:
        pds_utils.pdsClearFlows()
    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to verify sessions : %s"%(e))
        return api.types.status.FAILURE
    return api.types.status.SUCCESS


def Verify(tc):
    try:
        # Disconnect and stop Trex, so that we dont get any more packets from Trex.
        # Without this session verification will fail.
        cleanup(tc)

        ret = pds_utils.isPdsAlive()
        if ret != api.types.status.SUCCESS:
            api.Logger.error("PDS Agent is DEAD ..")
            return ret
        api.Logger.info("Verified the PDS Agent process.")

    except Exception as e:
        traceback.print_exc()
        api.Logger.error("Failed to verify : %s"%(e))
        return api.types.status.FAILURE
    return api.types.status.SUCCESS


def cleanup(tc):
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    if not skip_trex_traffic:
        for w in tc.workloadPeers.keys():
            if not w.trexHandle:
                continue
            try:
                w.trexHandle.stop(block=False)
                w.trexHandle.disconnect()
                w.trexHandle.cleanup()
                w.trexHandle = None
            except:
                pass
    try:
        pds_utils.pdsClearFlows()
        RestorePdsLogLevel(tc)
    except:
        pass


def Teardown(tc):
    cleanup(tc)
    flowutils.clearFlowTable(api.GetRemoteWorkloadPairs())
    return api.types.status.SUCCESS
