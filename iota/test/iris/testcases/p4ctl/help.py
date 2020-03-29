#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.p4ctl_test as test
import random

def Setup(tc):
    tc.node_name = random.choice(api.GetNaplesHostnames())
    return api.types.status.SUCCESS

def Trigger(tc):
    failed = False
    ret = test.test_read_help_cmd(tc.node_name)
    if ret != api.types.status.SUCCESS:
        failed = True

    ret = test.test_write_help_cmd(tc.node_name)
    if ret != api.types.status.SUCCESS:
        failed = True

    ret = test.test_read_table_help_cmd(tc.node_name)
    if ret != api.types.status.SUCCESS:
        failed = True

    ret = test.test_write_table_help_cmd(tc.node_name)
    if ret != api.types.status.SUCCESS:
        failed = True

    tc.ret =  api.types.status.FAILURE if failed else api.types.status.SUCCESS
    return tc.ret

def Verify(tc):
    return tc.ret

def Teardown(tc):
    return api.types.status.SUCCESS
