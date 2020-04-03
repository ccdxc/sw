#!/usr/bin/python3

import pdb
import os
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions


def Main(args):
    if GlobalOptions.skip_setup:
        # No profile change is required for skip setup
        return api.types.status.SUCCESS

    newProfile = getattr(args, 'profile', 'FEATURE_PROFILE_BASE')
    if newProfile != 'FEATURE_PROFILE_BASE':
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        for n in api.GetNaplesHostnames():
            common.AddPenctlCommand(req, n, "update dsc --dsc-profile %s" % newProfile)
        resp = api.Trigger(req)

        if not api.Trigger_IsSuccess(resp):
            return api.types.status.FAILURE

        api.Logger.info("Naples profile changed to %s - Restarting nodes" % newProfile)
        return api.RestartNodes(api.GetNaplesHostnames())
    else:
        api.Logger.info("Profile unchanged, input: %s" % newProfile)

    return api.types.status.SUCCESS

