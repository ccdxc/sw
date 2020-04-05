#! /usr/bin/python3
import iota.harness.api as api
import iota.test.common.utils.naples_upgrade.utils as utils
import iota.test.iris.testcases.naples_upgrade.testupgapp_utils as testupgapp_utils
import iota.test.iris.testcases.penctl.common as pencommon
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

def Main(step):
    if GlobalOptions.skip_setup:
        return api.types.status.SUCCESS

    naplesHosts = api.GetNaplesHostnames()

    assert(len(naplesHosts) != 0)

    for naplesHost in naplesHosts:
        testupgapp_utils.stopTestUpgApp(naplesHost, True)
        ret = utils.installBinary(naplesHost, "build/aarch64/iris/out/" + \
                    testupgapp_utils.UPGRADE_TEST_APP + "_bin/" + \
                    testupgapp_utils.UPGRADE_TEST_APP + ".bin")
        if ret != api.types.status.SUCCESS:
            return ret
        ret = utils.copyNaplesFwImage(naplesHost, testupgapp_utils.UPGRADE_TEST_APP + ".bin", "/update/")
        if ret != api.types.status.SUCCESS:
            return ret

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in naplesHosts:
        pencommon.AddPenctlCommand(req, n, "update time")

    api.Trigger(req)

    return api.types.status.SUCCESS
