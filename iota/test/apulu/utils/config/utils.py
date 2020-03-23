#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.utils.pdsctl as pdsctl

def SetDeviceLearnTimeout(val):
    # Enabling Max age for all endpoints
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        ret, resp = pdsctl.ExecutePdsctlCommand(node, "debug device", "--learn-age-timeout %d" % val, yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute debug device at node %s : %s" %(node, resp))
            return ret
    return True

def ClearLearnData():
    # Clearing all learnt MAC/IP info
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        ret, resp = pdsctl.ExecutePdsctlCommand(node, "clear learn mac", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute clear learn mac at node %s : %s" %(node, resp))
            return ret
    return True
