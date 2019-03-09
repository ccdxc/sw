#! /usr/bin/python3

import iota.harness.api as api
import iota.test.iris.testcases.frequency.clock_settings as cs

def __remove_frequency_settings_on_naples():
    result = api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    for naples_host in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, naples_host, "rm -f /%s/%s" % (cs.__get_naples_file_path(),cs.__get_local_file_name()))
    resp = api.Trigger(req)
    
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code !=0 :
            result = api.types.status.FAILURE
    return result

def Main(step):
    return __remove_frequency_settings_on_naples()
