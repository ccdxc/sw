#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.testcases.networking.utils as utils
import iota.test.iris.testcases.telemetry.utils as bond_utils

def trigger_ping_to_bond(tc):
    req = api.Trigger_CreateAllParallelCommandsRequest()

    for node, wl in utils.GetBondWorkloadPair():
        node_name, bond_ip = node.Name(), node.GetBondIp()

        # for baremetal, don't ping to workloads in the nodes where the uplink is brought down.
        if tc.is_bm_type and tc.uplink_fail_stage:
            if wl.node_name in tc.nodes:
                #api.Logger.info("Skipping ping b/w %s <--> %s"%(bond_ip, wl.ip_address))
                continue

        # Ping Bond ===> Workload
        cmd_cookie = "%s(%s) --> %s(%s)" %(node_name, bond_ip, wl.workload_name, wl.ip_address)
        tc.cmd_cookies.append(cmd_cookie)
        api.Logger.info("%s"%(cmd_cookie))
        cmd = "ping -i %s -c 20 -s %d %s" % (tc.interval, tc.iterators.pktsize, wl.ip_address)
        api.Trigger_AddNaplesCommand(req, node_name, cmd)

        # Ping Workload ===> Bond
        cmd_cookie = "%s(%s) --> %s(%s)" %(wl.workload_name, wl.ip_address, node_name, bond_ip)
        tc.cmd_cookies.append(cmd_cookie)
        api.Logger.info("%s"%(cmd_cookie))
        cmd = "ping -i %s -c 20 -s %d %s" % (tc.interval, tc.iterators.pktsize, bond_ip)
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)

    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def verify_ping_to_bond_result(tc):
    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            if api.GetNicMode() == 'hostpin' and tc.iterators.pktsize > 1024:
                result = api.types.status.SUCCESS
            else:
                result = api.types.status.FAILURE
        cookie_idx += 1
    tc.resp = None
    tc.cmd_cookies = []
    return result

def Setup(tc):
    tc.cmd_cookies = []
    tc.interval = 0.1
    tc.resp = None
    tc.skip = False
    tc.uplink_fail_stage = False

    tc.nodes = api.GetNaplesHostnames()
    tc.is_bm_type = False
    for node in tc.nodes:
        if api.IsBareMetalWorkloadType(node):
            # for baremetal topology, untag WL is configured on the uplink itself.
            # shutting down uplink on both nodes will bring down the WL.
            # so bring down uplink only on one naples
            tc.nodes = [ node ]
            tc.is_bm_type = True
            break

    tc.port_down_time = getattr(tc.args, "port_shut_time", 60)
    tc.failover_delay = getattr(tc.args, "failover_delay", 0)

    result = bond_utils.DetectUpLinkState(tc.nodes, bond_utils.PORT_OPER_STATUS_UP, all)
    if result != api.types.status.SUCCESS:
        api.Logger.error("All uplink on %s are not in UP state."%tc.nodes)
        tc.skip = True
        return api.types.status.SUCCESS

    api.Logger.info("All uplink on %s are UP!"%tc.nodes)

    # Bring up inband and reset the active link on bond.
    result = bond_utils.SetupInbandInterface()
    if result != api.types.status.SUCCESS:
        return result

    trigger_ping_to_bond(tc)

    result = verify_ping_to_bond_result(tc)
    if result != api.types.status.SUCCESS:
        api.Logger.error("Ping failed during setup stage")

    return result

def Trigger(tc):
    tc.cmd_cookies = []

    if tc.skip:
        return api.types.status.SUCCESS

    # Flap one uplink
    api.Logger.info("Bring down one uplink!")
    flapTask = bond_utils.GetSwitchPortFlapTask(tc.nodes, 1, tc.port_down_time)
    flapTask.start()
    time.sleep(tc.failover_delay)
    api.Logger.info("one uplink is brought down!")

    # Make sure up link is down
    result = bond_utils.DetectUpLinkState(tc.nodes, bond_utils.PORT_OPER_STATUS_DOWN, any)
    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to detect any uplink in DOWN state.")
        flapTask.join(tc.port_down_time)
        return result

    tc.uplink_fail_stage = True
    # Rerun ping test
    trigger_ping_to_bond(tc)
    result = verify_ping_to_bond_result(tc)
    if result != api.types.status.SUCCESS:
        api.Logger.error("Ping failed after Uplink failover")
        flapTask.join(tc.port_down_time)
        return result

    # wait until the uplink state is brought up
    flapTask.join(tc.port_down_time)
    # Flap both uplinks
    flapTask = bond_utils.GetSwitchPortFlapTask(tc.nodes, 2, tc.port_down_time)
    flapTask.start()

    # Make sure both uplinks are down
    result = bond_utils.DetectUpLinkState(tc.nodes, bond_utils.PORT_OPER_STATUS_DOWN, all)
    if result != api.types.status.SUCCESS:
        api.Logger.error("Failed to detect all uplink in DOWN state.")
        flapTask.join(tc.port_down_time)
        return result

    api.Logger.info("Waiting until {}secs to bring up both uplinks after flap!".format(tc.port_down_time))
    #join the background task to wait until the port_down_time before the uplinks are made UP
    flapTask.join(tc.port_down_time)
    time.sleep(tc.failover_delay)

    tc.uplink_fail_stage = False
    # Rerun ping test
    trigger_ping_to_bond(tc)

    return result

def Verify(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    if tc.resp is None:
        return api.types.status.FAILURE

    result = verify_ping_to_bond_result(tc)
    if result != api.types.status.SUCCESS:
        api.Logger.error("Ping failed after both Uplink failover")
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
