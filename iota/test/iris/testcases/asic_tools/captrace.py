#! /usr/bin/python3
import iota.harness.api as api
import re
import iota.test.iris.utils.traffic as traffic_utils
import iota.test.iris.testcases.asic_tools.utils as utils

g_req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

def Setup(tc):
    tc.skip = False
    return utils.naples_get(tc)

def conf_trace(tc):
    cmd = utils.g_path + 'captrace conf /nic/conf/captrace/pipeline-all.json'
    api.Trigger_AddNaplesCommand(g_req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)

def dump_trace(tc, num):
    cmd = utils.g_path + 'captrace dump /tmp/captrace%s.bin' % num
    api.Trigger_AddNaplesCommand(g_req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)
    cmd = 'ls -l /tmp/captrace%s.bin' % num
    api.Trigger_AddNaplesCommand(g_req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)

def show_trace(tc):
    cmd = utils.g_path + 'captrace show'
    api.Trigger_AddNaplesCommand(g_req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)

def base_test(tc, num):
    conf_trace(tc)
    dump_trace(tc, num)
    show_trace(tc)

def diff_test(tc, num1, num2):
    cmd = 'diff /tmp/captrace%s.bin /tmp/captrace%s.bin' % (num1, num2)
    api.Trigger_AddNaplesCommand(g_req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)

def traffic_test(tc):
    conf_trace(tc)
    dump_trace(tc, 1)
    tc.ping_cookies, tc.ping_resp = traffic_utils.pingAllRemoteWloadPairs()
    dump_trace(tc, 2)
    diff_test(tc, 1, 2)

def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS
    if tc.iterators.case == 'base':
        base_test(tc, 1)
    elif tc.iterators.case == 'traffic':
        traffic_test(tc)
    tc.resp = api.Trigger(g_req)
    return api.types.status.SUCCESS

def base_verify(tc):
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if 'conf' in cmd.command or 'show' in cmd.command:
            matchObj = re.search(r'success', cmd.stdout, 0)
            if matchObj is None:
                return api.types.status.FAILURE
        elif 'ls -l /tmp/captrace1.bin'  in cmd.command:
            matchObj = re.search(r'No such file', cmd.stdout, 0)
            if matchObj is not None:
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def post_traffic_verify(tc):
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if 'diff' in cmd.command:
            matchObj = re.search(r'differ', cmd.stdout, 0)
            #the files should differ
            if matchObj is None:
                result = api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    if tc.iterators.case == 'base':
        return base_verify(tc)
    elif tc.iterators.case == 'traffic':
        return post_traffic_verify(tc)
    return api.types.status.SUCCESS

def reset_trace(tc):
    cmd = utils.g_path + 'captrace reset'
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.info("api trigger failed : %s" % cmd)
        return api.types.status.FAILURE
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if 'reset' in cmd.command:
            matchObj = re.search(r'success', cmd.stdout, 0)
            if matchObj is None:
                api.Logger.info("cmd failed %s" % cmd)
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def rm_bins(tc):
    cmd = 'rm -f /tmp/captrace*bin'
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, tc.naples_node, cmd)
    api.Logger.info("cmd - %s" % cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.info("api trigger failed : %s" % cmd)
        return api.types.status.FAILURE
    for cmd in  resp.commands:
        api.PrintCommandResults(cmd)
        if 'rm -f' in cmd.command:
            matchObj = re.search(r'No such file or directory', cmd.stdout, 0)
            if matchObj is not None:
                api.Logger.info("cmd failed %s" % cmd)
                return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    if reset_trace(tc) is not api.types.status.SUCCESS:
        return api.types.status.FAILURE
    if rm_bins(tc) is not api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS
