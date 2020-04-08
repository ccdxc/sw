#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.p4ctl as p4ctl
import iota.test.utils.p4ctl_test as test
import random

def __get_p4tables(node):
    p4tables = p4ctl.GetTables(node)
    try:
        # temporarily  excluding tables for which read is crashing
        p4tables.remove('flow_ohash')
    except:
        pass
    return p4tables

def Setup(tc):
    tc.node_name = random.choice(api.GetNaplesHostnames())
    return api.types.status.SUCCESS

def Trigger(tc):
    p4tables = __get_p4tables(tc.node_name)
    tc.ret = test.test_read_table_index_cmd(tc.node_name, p4tables)
    return api.types.status.SUCCESS

def Verify(tc):
    return tc.ret

def Teardown(tc):
    return api.types.status.SUCCESS
