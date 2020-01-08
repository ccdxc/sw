#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.debug as debug_utils
import iota.test.utils.host as host_utils
import iota.test.utils.naples_host as naples_host
import iota.test.utils.traffic as traffic_utils
import random
import time

__RANDOM_MTU = -1
__MIN_MTU_FREEBSD = 72
__MIN_MTU = 68
__MAX_MTU = 9194
__DEF_MTU = 1500

__IS_FREEBSD = False

def verifyMTUchange(tc):
    result = api.types.status.SUCCESS
    expected_mtu = tc.new_mtu
    node_name = tc.naples_node
    workloads = api.GetWorkloads()
    for w in workloads:
        configured_mtu = host_utils.getInterfaceMTU(w.node_name, w.interface)
        if node_name != w.node_name:
            api.Logger.verbose("MTU filter : verifyMTUchange skipping peer node ", w.node_name, w.interface, configured_mtu, expected_mtu)
            continue
        if configured_mtu != expected_mtu:
            api.Logger.critical("MTU filter : verifyMTUchange failed for ", w.interface, configured_mtu, expected_mtu)
            host_utils.debug_dump_interface_info(w.node_name, w.interface)
            result = api.types.status.FAILURE
    return result

def changeWorkloadIntfMTU(new_mtu, node_name=None):
    result = api.types.status.SUCCESS
    workloads = api.GetWorkloads()
    for w in workloads:
        if node_name is not None:
            if node_name != w.node_name:
                api.Logger.debug("MTU filter : changeWorkloadIntfMTU skipping peer node ", w.node_name, w.interface, new_mtu)
                continue
        cmd = host_utils.setInterfaceMTU(w.node_name, w.interface, new_mtu)
        if cmd.exit_code != 0:
            api.Logger.critical("MTU filter : changeWorkloadIntfMTU failed for ", w.node_name, w.interface, new_mtu)
            api.PrintCommandResults(cmd)
            host_utils.debug_dump_interface_info(w.node_name, w.interface)
            result = api.types.status.FAILURE
    return result

def isFreeBSDTestbed():
    nodes = api.GetWorkloadNodeHostnames()
    for node in nodes:
        if api.GetNodeOs(node) == "freebsd":
            return True
    return False

def getRandomMTU():
    seed = time.time()
    api.Logger.verbose("MTU filter : seed used for random MTU ", seed)
    random.seed(seed)
    return random.randint(__MIN_MTU, __MAX_MTU)

def getMTUconfigs(tc):
    global __IS_FREEBSD
    new_mtu = int(tc.iterators.mtu)
    if new_mtu == __RANDOM_MTU:
        new_mtu = getRandomMTU()
    if new_mtu < __MIN_MTU_FREEBSD:
        if __IS_FREEBSD is True:
            # MIN_MTU supported by FreeBSD is 72
            new_mtu = __MIN_MTU_FREEBSD
    return new_mtu

def triggerMTUPings(tc):
    packet_size = tc.new_mtu
    #Trigger ping with exact MTU across all remote workload pairs
    api.Logger.verbose("MTU filter : ping workloads mtu (0) ", packet_size)
    tc.cmd_cookies_1, tc.resp_1 = traffic_utils.pingWorkloads(tc.workload_pairs, packet_size=packet_size, do_pmtu_disc=True)
    #Trigger ping with MTU - 1 across all remote workload pairs
    api.Logger.verbose("MTU filter : ping workloads mtu (-1)  ", packet_size-1)
    tc.cmd_cookies_2, tc.resp_2 = traffic_utils.pingWorkloads(tc.workload_pairs, packet_size=packet_size-1, do_pmtu_disc=True)
    #Trigger ping with MTU + 1 across all remote workload pairs
    api.Logger.verbose("MTU filter : ping workloads mtu (+1)  ", packet_size+1)
    tc.cmd_cookies_3, tc.resp_3 = traffic_utils.pingWorkloads(tc.workload_pairs, packet_size=packet_size+1, do_pmtu_disc=True)
    return

def initPeerNode(naples_node, new_mtu=__MAX_MTU):
    """ initialize MTU of interfaces on non 'naples_node' to __MAX_MTU """
    result = api.types.status.SUCCESS
    workloads = api.GetWorkloads()
    for w in workloads:
        if naples_node == w.node_name:
            api.Logger.debug("MTU filter : initPeerNode skipping naples node ", w.node_name, w.interface, new_mtu)
            continue
        cmd = host_utils.setInterfaceMTU(w.node_name, w.interface, new_mtu)
        if cmd.exit_code != 0:
            api.Logger.critical("MTU filter : initPeerNode failed for ", w.node_name, w.interface, new_mtu)
            api.PrintCommandResults(cmd)
            host_utils.debug_dump_interface_info(w.node_name, w.interface)
            result = api.types.status.FAILURE

    #TODO: Determine how much time to sleep
    time.sleep(40)
    api.Logger.info("MTU filter : hack - Slept for 40 secs")

    return result

def verifyMTUPings(tc):
    result = api.types.status.SUCCESS
    final_result = api.types.status.SUCCESS
    new_mtu = tc.new_mtu
    global __IS_FREEBSD

    # Verify ping with exact MTU is successful
    result = traffic_utils.verifyPing(tc.cmd_cookies_1, tc.resp_1)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings - exact MTU case ", new_mtu)
        final_result = result

    # Verify ping with (MTU - 1) is successful
    result = traffic_utils.verifyPing(tc.cmd_cookies_2, tc.resp_2)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings - MTU - 1 case ", new_mtu-1)
        final_result = result

    if __IS_FREEBSD is True:
        msg_too_long_exit_code = 2
    else:
        msg_too_long_exit_code = 1

    # Verify ping with (MTU + 1) is NOT successful
    result = traffic_utils.verifyPing(tc.cmd_cookies_3, tc.resp_3, exit_code=msg_too_long_exit_code)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings - MTU + 1 case ", new_mtu+1)
        final_result = result

    return final_result

def Setup(tc):
    api.Logger.verbose("MTU filter : Setup")
    tc.skip = False
    result = api.types.status.SUCCESS
    global __IS_FREEBSD

    if not api.RunningOnSameSwitch():
        tc.skip = True
        api.Logger.error("MTU filter : Setup -> Multi switch topology not supported yet - So skipping the TC")
        return api.types.status.IGNORED

    tc.naples_node, res = naples_host.GetNaplesNodeName()
    if res is False:
        tc.skip = True

    if tc.skip:
       api.Logger.error("MTU filter : Setup -> No Naples Topology - So skipping the TC")
       return api.types.status.IGNORED

    """
      # In Intel cards, post MTU change, need to wait for few sec before pinging
      # instead, set max MTU on peer node
    """
    result = initPeerNode(tc.naples_node)

    __IS_FREEBSD = isFreeBSDTestbed()
    tc.new_mtu = getMTUconfigs(tc)

    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    api.Logger.info("MTU filter : new MTU - ", tc.new_mtu)

    api.Logger.info("MTU filter : Setup final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Trigger(tc):
    api.Logger.verbose("MTU filter : Trigger")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED
    global __IS_FREEBSD

    #change workloads MTU
    result = changeWorkloadIntfMTU(tc.new_mtu, tc.naples_node)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Trigger failed for changeWorkloadIntfMTU ", result)
        debug_utils.collect_showtech(result)
        return result

    #Trigger ping across all remote workload pairs
    triggerMTUPings(tc)

    api.Logger.info("MTU filter : Trigger final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Verify(tc):
    api.Logger.verbose("MTU filter : Verify")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    result = verifyMTUchange(tc)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUchange ")
        debug_utils.collect_showtech(result)
        return result

    result = verifyMTUPings(tc)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings")
        debug_utils.collect_showtech(result)
        return result

    api.Logger.info("MTU filter : Verify final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Teardown(tc):
    api.Logger.verbose("MTU filter : Teardown")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    #rollback workloads MTU
    result = changeWorkloadIntfMTU(__DEF_MTU)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : rollback failed for changeWorkloadIntfMTU ", result)

    api.Logger.info("MTU filter : Teardown final result - ", result)
    debug_utils.collect_showtech(result)
    return result
