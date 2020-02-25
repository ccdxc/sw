#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.utils.pdsctl as pdsctl

def Main(step):
    api.Logger.info("Verifying if all VNIC and Mappings are learnt")
    #TODO to invoke pdsctl show learn verification once it is done
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "learn mac", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show learn mac at node %s : %s" %(node, resp))
            return api.types.status.FAILURE

        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "learn ip", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show learn ip at node %s : %s" %(node, resp))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
