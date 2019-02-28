#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.traffic as traffic_utils
import iota.test.iris.testcases.filters.filters_utils as filters_utils
import random
import time

__RANDOM_MTU = -1
__MIN_MTU_FREEBSD = 72
__MIN_MTU = 68
__MAX_MTU = 9194
__DEF_MTU = 1500

__IS_FREEBSD = False

def verifyMTUchange(expected_mtu):
    result = api.types.status.SUCCESS
    workloads = api.GetWorkloads()
    for w in workloads:
        configured_mtu = host_utils.getInterfaceMTU(w.node_name, w.interface)
        if configured_mtu != expected_mtu:
            api.Logger.critical("MTU filter : verifyMTUchange failed for ", w.interface, configured_mtu, expected_mtu)
            host_utils.debug_dump_interface_info(w.node_name, w.interface)
            result = api.types.status.FAILURE
    return result

def changeWorkloadIntfMTU(new_mtu):
    result = api.types.status.SUCCESS
    workloads = api.GetWorkloads()
    for w in workloads:
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
    if new_mtu == __MIN_MTU:
        if __IS_FREEBSD is True:
            # MIN_MTU supported by FreeBSD is 72
            new_mtu = __MIN_MTU_FREEBSD
    return new_mtu

def triggerMTUPings(tc):
    #Trigger ping with exact MTU across all remote workload pairs
    tc.cmd_cookies_1, tc.resp_1 = traffic_utils.pingAllRemoteWloadPairs(mtu=tc.new_mtu, do_pmtu_disc=True)
    #Trigger ping with MTU - 1 across all remote workload pairs
    tc.cmd_cookies_2, tc.resp_2 = traffic_utils.pingAllRemoteWloadPairs(mtu=tc.new_mtu-1, do_pmtu_disc=True)
    #Trigger ping with MTU + 1 across all remote workload pairs
    tc.cmd_cookies_3, tc.resp_3 = traffic_utils.pingAllRemoteWloadPairs(mtu=tc.new_mtu+1, do_pmtu_disc=True)
    return

def verifyMTUPings(tc):
    result = api.types.status.SUCCESS
    global __IS_FREEBSD

    # Verify ping with exact MTU is successful
    result = traffic_utils.verifyPing(tc.cmd_cookies_1, tc.resp_1)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings - exact MTU")
        return result

    # Verify ping with (MTU - 1) is successful
    result = traffic_utils.verifyPing(tc.cmd_cookies_2, tc.resp_2)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings - MTU - 1")
        return result

    if __IS_FREEBSD is True:
        msg_too_long_exit_code = 2
    else:
        msg_too_long_exit_code = 1

    # Verify ping with (MTU + 1) is NOT successful
    result = traffic_utils.verifyPing(tc.cmd_cookies_3, tc.resp_3, exit_code=msg_too_long_exit_code)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings - MTU + 1")
        return result

    return result

def Setup(tc):
    api.Logger.verbose("MTU filter : Setup")
    result = api.types.status.SUCCESS
    global __IS_FREEBSD

    __IS_FREEBSD = isFreeBSDTestbed()
    tc.new_mtu = getMTUconfigs(tc)
    api.Logger.debug("MTU filter : new MTU - ", tc.new_mtu)

    api.Logger.info("MTU filter : Setup final result - ", result)
    return result

def Trigger(tc):
    api.Logger.verbose("MTU filter : Trigger")
    result = api.types.status.SUCCESS
    global __IS_FREEBSD

    #change workloads MTU
    result = changeWorkloadIntfMTU(tc.new_mtu)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Trigger failed for changeWorkloadIntfMTU ", result)
        return result

    #FreeBSD hack
    if __IS_FREEBSD is True:
        time.sleep(60)
        api.Logger.info("MTU filter : FreeBSD hack - Slept for 60 secs")

    #Trigger ping across all remote workload pairs
    triggerMTUPings(tc)

    api.Logger.info("MTU filter : Trigger final result - ", result)
    return result

def Verify(tc):
    api.Logger.verbose("MTU filter : Verify")
    result = api.types.status.SUCCESS

    result = verifyMTUchange(tc.new_mtu)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUchange ")
        return result

    result = verifyMTUPings(tc)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : Verify failed for verifyMTUPings")
        return result

    api.Logger.info("MTU filter : Verify final result - ", result)

    return result

def Teardown(tc):
    api.Logger.verbose("MTU filter : Teardown")
    result = api.types.status.SUCCESS

    #rollback workloads MTU
    result = changeWorkloadIntfMTU(__DEF_MTU)
    if result is not api.types.status.SUCCESS:
        api.Logger.error("MTU filter : rollback failed for changeWorkloadIntfMTU ", result)

    api.Logger.info("MTU filter : Teardown final result - ", result)

    return result
