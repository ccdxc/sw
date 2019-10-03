#! /usr/bin/python3
import time
import iota.harness.api as api


def Setup(tc):
    return api.types.status.SUCCESS


def Trigger(tc):

    ret = api.RunSubTestCase(tc, "testcases.triggers.switch_flap", parallel=True)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    ret = api.RunSubTestCase(tc, "testcases.config.update.policy")
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.WaitForTestCase(tc, "testcases.triggers.switch_flap")
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    return api.types.status.SUCCESS
