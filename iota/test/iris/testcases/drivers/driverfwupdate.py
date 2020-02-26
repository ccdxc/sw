#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

IONIC_DRV_PATH = api.HOST_NAPLES_DIR + "/drivers-freebsd-eth"
IONIC_FW_DRV_PATH = api.HOST_NAPLES_DIR + "/drivers-freebsd-eth/sys/modules/ionic_fw"

def Setup(tc):
    api.Logger.info("Firmware update using sysctl")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    return api.types.status.SUCCESS

#Load ionic_fw.ko
def LoadFwDriver(os_type, node):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if os_type != host.OS_TYPE_BSD:
        api.Logger.info("Unknown os_type - %s" % os_type)
        return api.types.status.FAILURE
    
    fullpath = api.GetTopDir() + '/nic/naples_fw.tar'
    api.Logger.info("Fullpath for firmware image: " + fullpath)
    resp = api.CopyToHost(node, [fullpath], "")
    if resp is None:
        api.Logger.info("Failed to copy naples_fw.tar")
        return api.types.status.FAILURE
    api.Trigger_AddHostCommand(req, node, "cp naples_fw.tar " + IONIC_FW_DRV_PATH)
    api.Trigger_AddHostCommand(req, node, "make -C " + IONIC_FW_DRV_PATH)
    api.Trigger_AddHostCommand(req, node, "kldload " + IONIC_FW_DRV_PATH + "/ionic_fw.ko")

    resp = api.Trigger(req)
    if resp is None:
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            if os_type == host.OS_TYPE_BSD:
                if cmd.stdout.find("already loaded") != -1:
                    api.Logger.info("ionic_fw load failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
                else:
                    api.Logger.info("ionic_fw already loaded, reload is expected to fail without unload")
            else:
                api.Logger.info("Unknown os_type - %s" % os_type)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    # Unload ionic and ionic_fw
    for node in tc.nodes:
        host.UnloadDriver(tc.os, node, "all")
        host.UnloadDriver(tc.os, node, "ionic_fw")

    for node in tc.nodes:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
	    # XXX: Find the Naples_fw.tar version.
        api.Trigger_AddHostCommand(req, node, "kenv hw.ionic.fw_update_ver=FILL_FW_VERSION")
        resp = api.Trigger(req)
        if resp is None:
            api.Logger.info("Failed kenv hw.ionic.fw_update_ver=X")
            return api.types.status.FAILURE      
        if host.LoadDriver(tc.os, node) is api.types.status.FAILURE:
            api.Logger.info("ionic already loaded")
            return api.types.status.FAILURE
        if LoadFwDriver(tc.os, node) is api.types.status.FAILURE:
            return api.types.status.FAILURE
        for i in api.GetNaplesHostInterfaces(node):
            #
            # In local testing, this step completes in 35-40s, but the default
            # timeout is 30s.  Therefore, increase the timeout to 60s.
            #
            # The iota logs may contain messages such as "CHECK_ERR: Nicmgr
            # crashed for host: node2?"  Please note, this is due to finding
            # the string "fw heartbeat stuck" in the host dmesg.  This is
            # currently the expected behavior when doing fw update.  If nicmgr
            # does crash, than expect subsequent tests to fail, otherwise the
            # CHECK_ERR message in the iota test logs may be ignored.
            #
            api.Trigger_AddHostCommand(req, node, "sysctl dev.%s.fw_update=1" % host.GetNaplesSysctl(i),
                    timeout=60)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.os != host.OS_TYPE_BSD:
        api.Logger.info("Not implemented")
        return api.types.status.IGNORED

    result = api.types.status.SUCCESS

    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    if tc.resp is None:
        return api.types.status.FAILURE

    if api.GetNicMode() == 'classic':
        expect_exit_code = 0
    else:
        # expect OS's EPERM (FreeBSD/Linux is 1) (IONIC_RC_EPERM is 4)
        expect_exit_code = 1

    for cmd in tc.resp.commands:
        if cmd.exit_code != expect_exit_code:
            api.PrintCommandResults(cmd)
            api.Logger.error("Expected exit code %d" % expect_exit_code)
            api.Logger.error("Actual exit code %d" % cmd.exit_code)
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

