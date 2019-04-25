#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.iris.utils.naples  as naples
import iota.test.iris.config.netagent.hw_push_config as hw_config

LinuxDriverPath = "/naples/drivers-linux-eth/drivers/eth/ionic/ionic.ko"
FreeBSDDriverPath = "/naples/drivers-freebsd-eth/sys/modules/ionic/ionic.ko"


def __UnloadDriver (tc, node, init=False):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True) 

    if tc.os == 'linux':
        if init == True:
            api.Trigger_AddHostCommand(req, node, "rmmod ionic_rdma")
        else: 
            api.Trigger_AddHostCommand(req, node, "rmmod ionic")
    elif tc.os == 'freebsd':
        if init == True:
            api.Trigger_AddHostCommand(req, node, "kldunload ionic_rdma")
        else:
            api.Trigger_AddHostCommand(req, node, "kldunload ionic")
    
    tc.resp = api.Trigger(req)
    if tc.resp is None:
        api.Logger.info("NONE")
        return api.types.status.FAILURE
    
    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            if tc.os == 'linux':
                if cmd.stdout.find ("is not currently loaded") != -1:
                    api.Logger.info("Unload Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
            elif tc.os == 'freebsd':
                if cmd.stdout.find ("can't find file") != -1:
                    api.Logger.info("Unload Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
                else:
                    api.Logger.info ("Driver was NOT loaded. Unload is expected to fail")
            else:
                api.Logger.info ("Unknown OS")
                return api.types.status.FAILURE
            
    return api.types.status.SUCCESS
  
def  __LoadDriver (tc,node):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    
    if tc.os == 'linux':
        api.Trigger_AddHostCommand(req, node, "insmod " + LinuxDriverPath)
    elif tc.os == 'freebsd':
        api.Trigger_AddHostCommand(req, node, "kldload " + FreeBSDDriverPath)
    else:
        api.Logger.info ("Unknown OS")
        return api.types.status.FAILURE

    tc.resp = api.Trigger(req)
    if tc.resp is None:
        return api.types.status.FAILURE
    
    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            if tc.os == 'Linux':
                if cmd.stdout.find ("File exists") != -1:
                    api.Logger.info("Load Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
            elif tc.os == 'freebsd':
                if cmd.stdout.find ("already loaded") != -1:
                    api.Logger.info("Load Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
                else:
                    api.Logger.info ("Driver was already loaded. Unload is expected to fail")
            else:
                api.Logger.info ("Unknown OS")
                return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def Setup(tc):
 
    api.Logger.info ("Driver Load/Unload x %s" % tc.args.loops)
 
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    
    return api.types.status.SUCCESS 

def Trigger(tc):
    loops = getattr(tc.args, "loops", 1)

    # TOOD: add traffice to the test case

    # first run through unload to init nodes
    for n in tc.nodes:
        __UnloadDriver (tc, n, init=True)

    # for every node, cycle through unload/load sequence
    for n in tc.nodes:
        for i in range (0, loops):
            if __UnloadDriver (tc, n) is api.types.status.FAILURE:
                return api.types.status.FAILURE
            if __LoadDriver (tc,n) is api.types.status.FAILURE:
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):

    # TODO: verify driver load/unload timing. Should never cross magic # of sec
 
    for node in tc.nodes:
        # this is required to bring the testbed into operation state
        # after driver unload interfaces need to be initialized
        hw_config.ReAddWorkloads(node)

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
           return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def Teardown(tc):
 
    return api.types.status.SUCCESS
