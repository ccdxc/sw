#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.pdsctl as pdsctl

def __learn_endpoints(args):
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    
    for wl in api.GetWorkloads():
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "arping -c  5 -U %s -I %s" % (wl.ip_address, wl.interface))
        api.Logger.debug("ArPing from %s %s %s %s" % (wl.node_name, wl.workload_name, wl.interface, wl.ip_address))

    resp = api.Trigger(req)
    if resp is None:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def __verify_learn():
    api.Logger.verbose("Verifying if all VNIC and Mappings are learnt")
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

        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "learn statistics", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show learn statistics at node %s : %s" %(node, resp))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Main(args):
    api.Logger.verbose("Learn VNIC and Mappings")
    result = __learn_endpoints(args)
    if result != api.types.status.SUCCESS:
        return result
    result = __verify_learn()
    return result

if __name__ == '__main__':
    Main(None)
