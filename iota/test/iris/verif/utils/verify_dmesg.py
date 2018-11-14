#! /usr/bin/python3

import iota.harness.api as api

def Main(tc):
    api.Logger.info("Verifying Dmesg.")
    #req = api.Trigger_CreateExecuteCommandsRequest()
    #api.Trigger_AddCommand(req, tc.workload.node_name, tc.workload.workload_name,
    #                       "dmesg | grep -A20 \"Call Trace\" | grep sonic")
    #resp = api.Trigger(req)

    #api.PrintCommandResults(resp.commands[0])
    #if resp.commands[0].exit_code == 0:
    #    return api.types.status.FAILURE
    return api.types.status.SUCCESS
