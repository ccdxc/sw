#! /usr/bin/python3
import iota.harness.api as api
import iota.harness.infra.resmgr as resmgr
import iota.test.iris.utils.address as address_utils
import iota.test.iris.utils.debug as debug_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.utils.host as host_utils
import iota.test.iris.testcases.filters.filters_utils as filters_utils
from collections import defaultdict
import operator
import time
import yaml

ipv4_allocator = resmgr.IpAddressStep("239.1.5.1", "0.0.0.1")
__HPING_COUNT = 8

def getAllmcastEndPointsView(naples_node):
    # Host interface mcast endpoints
    host_mc_ep_set = filters_utils.getHostIntfMcastEndPoints(naples_node)

    # Naples intf mcast endpoints
    naples_mc_ep_set = filters_utils.getNaplesIntfMcastEndPoints(naples_node)

    # HAL view of endpoints
    hal_mc_ep_set = filters_utils.getNaplesHALmcastEndPoints(naples_node)

    #Keeping them separate as it is useful for debugging in scale
    return host_mc_ep_set, naples_mc_ep_set, hal_mc_ep_set

def verifyMCEndPoints(tc):
    host_mc_ep_view = tc.host_mc_ep_set
    naples_mc_ep_view = tc.naples_mc_ep_set
    hal_mc_ep_view = tc.hal_mc_ep_set

    # HAL's view of endpoints = Union of workload + Host + Naples Intf
    host_view = host_mc_ep_view | naples_mc_ep_view
    return filters_utils.verifyEndpoints(host_view, hal_mc_ep_view)

def verifyMCTrafficStats(tc):
    result = True
    statsCount = tc.statsCount
    preStatsCount = tc.preStatsCount
    postStatsCount = tc.postStatsCount

    for intf, mcStats in statsCount.items():
        #MC Frame stats before trigger
        preStats = preStatsCount[intf]
        #MC Frame stats after trigger
        postStats = postStatsCount[intf]
        #Expected MC Frame stats increase because of trigger
        expectedStats = statsCount[intf]
        #Actual MC Frame stats increase, actualStats = postStats - preStats
        actualStats = list(map(operator.sub, postStats, preStats))
        api.Logger.verbose("verifyMCTrafficStats info for ", intf, expectedStats, actualStats, preStats, postStats)
        # Interested in multicast rx stats only
        if actualStats[1] != expectedStats[1]:
            result = False
            api.Logger.error("verifyMCTrafficStats failed for ", intf, expectedStats, actualStats, preStats, postStats)
    return result

def GetLifMCFramesTxRxStats(naples_node, lif_id):
    args = "--id " + str(lif_id)
    resp, result = hal_show_utils.GetHALShowOutput(naples_node, "lif", args)
    mcframesok = [0, 0]
    if not result:
        api.Logger.critical("unknown response from Naples")
        return bcframesok
    cmd = resp.commands[0]

    perLifOutput = cmd.stdout.split("---")

    for lif in perLifOutput:
        lifObj = yaml.load(lif, Loader=yaml.FullLoader)
        if lifObj is not None:
            lifid = lifObj['spec']['keyorhandle']['keyorhandle']['lifid']
            if lif_id == lifid:
                txmcframesok = lifObj['stats']['lifs']['datalifstats']['txstats']['multicastframesok']
                rxmcframesok = lifObj['stats']['lifs']['datalifstats']['rxstats']['multicastframesok']
                mcframesok = [txmcframesok, rxmcframesok]
                return mcframesok

    api.Logger.error("GetMCFramesRxStats failed for ", lif_id)

    return mcframesok

def GetMCFramesTxRxStats(naples_node, statsObj, intfName2lifId_dict):
    for intf in statsObj.keys():
        lif_id = intfName2lifId_dict[intf]
        statsObj[intf] = GetLifMCFramesTxRxStats(naples_node, lif_id)
    return

def initiateMCtraffic(w1, w2, statsCount):
    result = api.types.status.SUCCESS

    #Get a new multicast IPv4 address
    mcast_ip = str(ipv4_allocator.Alloc())
    #Get corresponding multicast mac address
    mcast_mac = address_utils.convertMcastIP2McastMAC(mcast_ip)
    api.Logger.verbose("Mcast address ", mcast_ip, mcast_mac)

    #install rx filter for this mcast mac (derived from mcast IP) on w1.interface
    cmd = host_utils.AddMcastMAC(w1.node_name, w1.interface, mcast_mac)
    if cmd.exit_code != 0:
        api.Logger.critical("AddMcastMAC failed for w1 ", w1.node_name, w1.interface, mcast_mac)
        api.PrintCommandResults(cmd)
        result = api.types.status.FAILURE

    #install route for this mcast IP with w2.interface for hping3 to work
    cmd = host_utils.AddIPRoute(w2.node_name, w2.interface, mcast_ip)
    if cmd.exit_code != 0:
        api.Logger.critical("AddIPRoute failed for w2 ", w2.node_name, w2.interface, mcast_ip)
        api.PrintCommandResults(cmd)
        result = api.types.status.FAILURE

    # Without sleep here, mc packets are not getting received.
    # need to check if route / maddr install taking time for propagation.
    time.sleep(1)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    # hping from remote node
    # exit_code will be 1 but that's ok as the intention here is to generate packets from remote
    cmd_cookie = "hping3 -c %d --faster %s -I %s " % (__HPING_COUNT, mcast_ip, w2.interface)
    api.Trigger_AddHostCommand(req, w2.node_name, cmd_cookie)
    resp = api.Trigger(req)

    #Increment stats
    txrxmcframes = statsCount[w1.parent_interface]
    #increase rx by hping_count
    txrxmcframes[1] += __HPING_COUNT
    statsCount[w1.parent_interface] = txrxmcframes

    # rollback
    #remove route for this mcast IP with w1.interface
    cmd = host_utils.DeleteMcastMAC(w1.node_name, w1.interface, mcast_mac)
    if cmd.exit_code != 0:
        api.Logger.critical("DeleteMcastMAC failed for w1 ", w1.node_name, w1.interface, mcast_mac)
        api.PrintCommandResults(cmd)
        result = api.types.status.FAILURE

    #remove route for this mcast IP with w2.interface
    cmd = host_utils.DelIPRoute(w2.node_name, w2.interface, mcast_ip)
    if cmd.exit_code != 0:
        api.Logger.critical("DelIPRoute failed for w2 ", w2.node_name, w2.interface, mcast_ip)
        api.PrintCommandResults(cmd)
        result = api.types.status.FAILURE

    return result

"""
# For debug purpose
def printMCstats(w1, w2, tc, debugstr):
    lif_id = tc.intfName2lifId_dict[w1.parent_interface]
    mcframesok = GetLifMCFramesTxRxStats(tc.naples_node, lif_id)
    api.Logger.info("printMCstats ", debugstr, w1.parent_interface, mcframesok)
    return
"""

def triggerMCtraffic(tc):
    result = api.types.status.SUCCESS
    workload_pairs = tc.workloads
    naples_node = tc.naples_node
    #Get stats before trigger
    GetMCFramesTxRxStats(naples_node, tc.preStatsCount, tc.intfName2lifId_dict)

    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        api.Logger.verbose("workload1 : ", w1.workload_name, w1.node_name, w1.uplink_vlan, w1.interface, w1.parent_interface, w1.IsNaples())
        api.Logger.verbose("workload2 : ", w2.workload_name, w2.node_name, w2.uplink_vlan, w2.interface, w2.parent_interface, w2.IsNaples())
        if w1.node_name == naples_node:
            #printMCstats(w1, w2, tc, "BeforeHPING")
            result = initiateMCtraffic(w1, w2, tc.statsCount)
            #printMCstats(w1, w2, tc, "AfterHPING")
        else:
            #printMCstats(w2, w1, tc, "BeforeHPING")
            result = initiateMCtraffic(w2, w1, tc.statsCount)
            #printMCstats(w2, w1, tc, "AfterHPING")
        if result != api.types.status.SUCCESS:
            #In case of failure, bail out immediately
            api.Logger.info("triggerMCtraffic failed for workloads ", w1.workload_name, w2.workload_name)
            return result

    #Get stats after trigger
    GetMCFramesTxRxStats(naples_node, tc.postStatsCount, tc.intfName2lifId_dict)

    return result

def getinitStatsObjects(intf_list):
    statsCount = defaultdict(list)
    preStatsCount = defaultdict(list)
    postStatsCount = defaultdict(list)

    for intf in intf_list:
        #Init the stat objects [txstats:multicastframesok, rxstats:multicastframesok]
        statsCount[intf] = [0, 0]
        preStatsCount[intf] = [0, 0]
        postStatsCount[intf] = [0, 0]
    return statsCount, preStatsCount, postStatsCount

def getStatObjects(naples_node):
    intf_list = list(api.GetNaplesHostInterfaces(naples_node))
    api.Logger.verbose("getStatObjects : intf_list ", intf_list)
    return getinitStatsObjects(intf_list)

def Setup(tc):
    api.Logger.verbose("MC MAC filter : Setup")
    tc.skip = False
    result = api.types.status.SUCCESS

    if not api.RunningOnSameSwitch():
        tc.skip = True
        api.Logger.error("MC MAC filter : Setup -> Multi switch topology not supported yet - So skipping the TC")
        return api.types.status.IGNORED

    tc.skip, tc.workloads, tc.naples_node = filters_utils.getNaplesNodeandWorkloads()
    if tc.skip:
        api.Logger.error("MC MAC filter : Setup -> No Naples Topology - So skipping the TC")
        return api.types.status.IGNORED

    tc.intfName2lifId_dict = hal_show_utils.GetIntfName2LifId_mapping(tc.naples_node)
    tc.statsCount, tc.preStatsCount, tc.postStatsCount = getStatObjects(tc.naples_node)

    api.Logger.info("MC MAC filter : Setup final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Trigger(tc):
    api.Logger.verbose("MC MAC filter : Trigger")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # Triggers done - Now build endpoint view of Host and Naples
    tc.host_mc_ep_set, tc.naples_mc_ep_set, tc.hal_mc_ep_set = getAllmcastEndPointsView(tc.naples_node)
    api.Logger.debug("getAllmcastEndPointsView: host_mc_ep_set ", len(tc.host_mc_ep_set), tc.host_mc_ep_set)
    api.Logger.debug("getAllmcastEndPointsView: naples_mc_ep_set ", len(tc.naples_mc_ep_set), tc.naples_mc_ep_set)
    api.Logger.debug("getAllmcastEndPointsView: hal_mc_ep_set ", len(tc.hal_mc_ep_set), tc.hal_mc_ep_set)

    # Trigger multicast Traffic
    result = triggerMCtraffic(tc)

    api.Logger.info("MC MAC filter : Trigger final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Verify(tc):
    '''
    #TODO
        1. any HW table dump to check? - Not needed apparently
            Ideally, We can also check registered MAC table as follows
            halctl show table dump --entry-id 1840 --table-id 4
            Get the entry id from "halctl show multicast --yaml"
        2. Check for memleaks [will pull from Amrita's TC]
            2.1 "halctl show system memory slab | grep mc_entry"
    '''
    api.Logger.verbose("MC MAC filter : Verify")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # Check if MACs in "halctl show multicast" match with host & naples interface mcast MAC
    if not verifyMCEndPoints(tc):
        api.Logger.error("MC MAC filter : Verify failed for verifyMCEndPoints")
        result = api.types.status.FAILURE
    else:
        api.Logger.debug("MC MAC filter : Verify - verifyMCEndPoints SUCCESS ")

    # Check multicast traffic stats
    if not verifyMCTrafficStats(tc):
        api.Logger.error("MC MAC filter : Verify failed for verifyMCTrafficStats ")
        result = api.types.status.FAILURE
    else:
        api.Logger.debug("MC MAC filter : Verify - verifyMCTrafficStats SUCCESS ")

    api.Logger.info("MC MAC filter : Verify final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Teardown(tc):
    api.Logger.verbose("MC MAC filter : Teardown")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    api.Logger.info("MC MAC filter : Teardown final result - ", result)
    debug_utils.collect_showtech(result)
    return result
