#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config
import iota.test.iris.utils.traffic as traffic

#
# LIF reset test, specific to FreeBSD
# Basic:
#  Filter list and VLAN  list must be unchanged after LIF reset
# Stress test:
#  Use sysctl dev.ionic.0.reset=1 to do LIF reset and from other shell
#  do reload of driver. System shouldn't panic.


def getVlanList(node, intf):
    host_cmd = "sysctl dev." + host.GetNaplesSysctl(intf) + ".vlans | grep VLAN |  cut -d '[' -f 2 | cut -d ']' -f 1" 

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, host_cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run host cmd: %s on host: %s"
                         %(host_cmd, node))
        return None

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("HOST CMD: %s failed on host: %s,"
                         %(host_cmd, node))
        api.PrintCommandResults(cmd)
        return None

    output = cmd.stdout.splitlines()
    vlan_list = list(map(int, output))
    
    return vlan_list

def getFilterList(node, intf):
    host_cmd = "sysctl dev." + host.GetNaplesSysctl(intf) + ".filters  | grep MAC | cut -d ' ' -f 2" 

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddHostCommand(req, node, host_cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run host cmd: %s on host: %s"
                         %(host_cmd, node))
        return None

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("HOST CMD: %s failed on host: %s,"
                         %(host_cmd, node))
        api.PrintCommandResults(cmd)
        return None

    output = cmd.stdout.splitlines()
    filter_list = list(map(str, output))
    
    return filter_list

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    api.Logger.info("LIF reset with driver reload test")
    return api.types.status.SUCCESS

def do_lif_reset_test(node, os):
    for i in range(3):
        api.Logger.info("LIF reset and driver reload test loop %d" % i)
        if host.UnloadDriver(os, node, "all") is api.types.status.FAILURE:
            api.Logger.error("ionic unload failed loop %d" % i)
            return api.types.status.FAILURE

        if host.LoadDriver(os, node) is api.types.status.FAILURE:
            api.Logger.error("ionic load failed loop %d" % i)
            return api.types.status.FAILURE
        
        hw_config.ReAddWorkloads(node)

        if api.GetNaplesHostInterfaces(node) is None:
            api.Logger.error("No ionic interface after loop %d" % i)
            return api.types.status.FAILURE

        for intf in api.GetNaplesHostInterfaces(node):
            req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
            vlan_list = getVlanList(node, intf)
            filter_list = getFilterList(node, intf)
            # Single LIF reset
            api.Trigger_AddHostCommand(req, node, "sysctl dev.%s.reset=1" %
                (host.GetNaplesSysctl(intf)))
            resp = api.Trigger(req)
            time.sleep(5)
            vlan_list1 = getVlanList(node, intf)
            filter_list1 = getFilterList(node, intf)
            if vlan_list != vlan_list1:
                api.Logger.error("VLAN list doesn't match for %s, before: %s after: %s" 
                                 %(intf, str(vlan_list), str(vlan_list1)))
                return api.types.status.FAILURE
            
            if filter_list != filter_list1:
                api.Logger.error("Filter list doesn't match for %s, before: %s after: %s" 
                                 %(intf, str(filter_list), str(filter_list1)))
                return api.types.status.FAILURE
                        
            api.Logger.info("Success running LIF reset test on %s VLAN: %s, Filters; %s"
                            %(intf, str(vlan_list), str(filter_list)))
        # Now stress test LIF reset
        for intf in api.GetNaplesHostInterfaces(node):
            req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
            api.Trigger_AddHostCommand(req, node, "for ((i=0;i<10;i++)); do sysctl dev.%s.reset=1; done &" %
                (host.GetNaplesSysctl(intf)))
	        # Some of LIF reset will fill fail since it will be running in background
            # with reload of driver.
            resp = api.Trigger(req)
 
    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    for node in tc.nodes:
        if api.IsNaplesNode(node):
            status = do_lif_reset_test(node, tc.os)
            if status != api.types.status.SUCCESS:
                api.Logger.error("lif reset test failed")
                return api.types.status.FAILURE


    return status

def Verify(tc):
    ping_cookies, ping_resp = traffic.pingAllRemoteWloadPairs()
    return traffic.verifyPing(ping_cookies, ping_resp)

def Teardown(tc):
    return api.types.status.SUCCESS

