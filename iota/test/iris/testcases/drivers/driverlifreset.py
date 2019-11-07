#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

#
# LIF reset test, specific to FreeBSD
# Use sysctl dev.ionic.0.reset=1 to do LIF reset and from other shell
# do reload of driver. System shouldn't panic.

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    api.Logger.info("LIF reset with driver reload test")
    return api.types.status.SUCCESS

def do_lif_reset_test(node, os):
    for i in range(10):
        api.Logger.info("LIF reset and driver reload test loop %d" % i)
        if host.UnloadDriver(os, node, "all") is api.types.status.FAILURE:
            api.Logger.error("ionic unload failed loop %d" % i)
            return -1

        if host.LoadDriver(os, node) is api.types.status.FAILURE:
            api.Logger.error("ionic load failed loop %d" % i)
            return -1
        hw_config.ReAddWorkloads(node)

        if api.GetNaplesHostInterfaces(node) is None:
            api.Logger.error("No ionic interface after loop %d" % i)
            return -1

        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        for intf in api.GetNaplesHostInterfaces(node):
            api.Trigger_AddHostCommand(req, node, "for ((i=0;i<10;i++)); do sysctl dev.%s.reset=1; done &" %
                (host.GetNaplesSysctl(intf)))
	# Some of LIF reset will fill fail since it will be running in background
        # with reload of driver.
        resp = api.Trigger(req)


    return 0

def Trigger(tc):
    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    for node in tc.nodes:
        if do_lif_reset_test(node, tc.os) !=0:
            api.Logger.info("lif reset test failed")
            return api.types.status.FAILURE


    return api.types.status.SUCCESS

def Verify(tc):

    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

