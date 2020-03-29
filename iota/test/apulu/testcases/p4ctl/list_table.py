#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.p4ctl_test as test
import random

def Setup(tc):
    tc.node_name = random.choice(api.GetNaplesHostnames())
    return api.types.status.SUCCESS

def Trigger(tc):
    tc.ret = test.test_list_cmd(tc.node_name)
    return api.types.status.SUCCESS

def Verify(tc):
    return tc.ret

def Teardown(tc):
    return api.types.status.SUCCESS
