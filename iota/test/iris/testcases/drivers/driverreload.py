#! /usr/bin/python3
import pdb
import time
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.config.netagent.hw_push_config as hw_config

def Setup(tc):

    api.Logger.info ("Driver Load/Unload x %s" % tc.args.loops)

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    if tc.os != host.OS_TYPE_BSD and tc.os != host.OS_TYPE_LINUX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED
     
    return api.types.status.SUCCESS 

def Trigger(tc):

    if tc.os != host.OS_TYPE_BSD and tc.os != host.OS_TYPE_LINUX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED
    
    loops = getattr(tc.args, "loops", 1)

    # TOOD: add traffic to the test case

    # first run through unload to init nodes
    for node in tc.nodes:
        host.UnloadDriver(tc.os, node, "all")

    # for every node, cycle through unload/load sequence
    for node in tc.nodes:
        for i in range (0, loops):
            if host.UnloadDriver(tc.os, node, "eth") is api.types.status.FAILURE:
                return api.types.status.FAILURE
            if host.LoadDriver(tc.os, node) is api.types.status.FAILURE:
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):

    # TODO: verify driver load/unload timing. Should never cross magic # of sec
 
    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    return api.types.status.SUCCESS

def Teardown(tc):
 
    return api.types.status.SUCCESS
