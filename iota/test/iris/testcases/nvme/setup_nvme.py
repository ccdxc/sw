#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.host as host

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Setting up target {0} for nvme".format(tc.nodes[0]))
    api.Trigger_AddHostCommand(req, tc.nodes[0], "sh /root/chaitanya/intf.sh")
    api.Trigger_AddHostCommand(req, tc.nodes[0], "sh /root/chaitanya/spdk/scripts/nvmet-spdk.sh")

    api.Logger.info("Setting up naple at host  {0} for nvme".format(tc.nodes[0]?))
    api.Trigger_AddCommand(req, tc.nodes[0]?, ?, "echo s > /tmp/nvme_test_cfg_start")

    api.Logger.info("Setting up host {0} for nvme".format(tc.nodes[1]))
    api.Trigger_AddHostCommand(req, tc.nodes[1], "modprobe nvme")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("nvme_setup results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS
