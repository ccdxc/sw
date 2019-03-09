#! /usr/bin/python3

import iota.harness.api as api
import json

FILENAME = 'frequency.json'

__naples_clock_speeds = [833, 900, 957, 1033, 1100]

def __get_naples_file_path():
    return "/sysconfig/config0/"

def __get_local_file_fullpath():
    return api.GetTestDataDirectory() + '/' + FILENAME

def __get_local_file_directory():
    return api.GetTestDataDirectory()

def __get_local_file_name():
    return FILENAME

def __dump_to_json(content, filename='frequency.json'):
    with open(filename, 'w') as fp:
        json.dump(content, fp)
    return

def __generate_frequency_file(clock_frequency):
    config = {}
    config['frequency'] = clock_frequency
    __dump_to_json(config, __get_local_file_fullpath())

def __reboot_nodes():
    nodes = api.GetWorkloadNodeHostnames()
    api.RestartNodes(nodes)
    return api.types.status.SUCCESS

def __set_frequency_on_naples():
    result = api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    for naples_host in api.GetNaplesHostnames():
        api.CopyToNaples(naples_host, [__get_local_file_fullpath()], "") 
        api.Trigger_AddNaplesCommand(req, naples_host, "mv /%s %s" % (__get_local_file_name(), __get_naples_file_path()))
    resp = api.Trigger(req)
    
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code !=0 :
            result = api.types.status.FAILURE
    return result

def Setup(tc):
    tc.skip = False
    if tc.args.type == 'local_only':
        tc.workload_pairs = api.GetLocalWorkloadPairs()
    else:
        tc.workload_pairs = api.GetRemoteWorkloadPairs()
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True
    return api.types.status.SUCCESS

def Trigger(tc):
    __generate_frequency_file(tc.iterators.frequency)
    result = __set_frequency_on_naples()
    if result != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    result = __reboot_nodes()
    return result

def __generate_ping_command_cookies(tc):
    req = None
    interval = "0.2"
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
        interval = "3"
    
    tc.cmd_cookies = []
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        cmd_cookie = "%s(%s) --> %s(%s)" % (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,"ping -i %s -c 20 -s %d %s" % (interval, 128, w2.ip_address))
        api.Logger.info("Ping test from %s" % (cmd_cookie))
        tc.cmd_cookies.append(cmd_cookie)
    return api.Trigger(req)

def Verify(tc):
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result


def __remove_frequency_settings_on_naples():
    result = api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    for naples_host in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, naples_host, "rm -f /%s/%s" % (__get_naples_file_path(),__get_local_file_name()))
    resp = api.Trigger(req)
    
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code !=0 :
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    return __remove_frequency_settings_on_naples()

def Main(step):
    __generate_frequency_file(max(__naples_clock_speeds))
    result = __set_frequency_on_naples()
    if result != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return result
