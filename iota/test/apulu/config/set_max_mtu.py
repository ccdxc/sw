#! /usr/bin/python3
import iota.harness.api as api

__MAX_MTU = 9100

def __get_mtu_cfg_cmd(intf):
    mtu_base_cmd = "ifconfig %s mtu %d"
    mtu_cmd = mtu_base_cmd % (intf, __MAX_MTU)
    return mtu_cmd

def __verify_response(resp):
    if resp is None:
        api.Logger.critical("Failed to set max mtu on interface - No response")
        return False
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.critical("Failed to set max mtu on interface")
            api.PrintCommandResults(cmd)
            return False
    return True

def __config_max_mtu_on_host_intfs():
    #req = api.Trigger_CreateAllParallelCommandsRequest()
    req = api.Trigger_CreateExecuteCommandsRequest()
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        intf_list = api.GetNaplesHostInterfaces(node)
        api.Logger.verbose("Setting max mtu on %s in %s" % (intf_list, node))
        for intf in intf_list:
            mtu_cmd = __get_mtu_cfg_cmd(intf)
            api.Trigger_AddHostCommand(req, node, mtu_cmd)
    resp = api.Trigger(req)
    return __verify_response(resp)

def __config_max_mtu_on_workload_intfs():
    workloads = api.GetWorkloads()
    #req = api.Trigger_CreateAllParallelCommandsRequest()
    req = api.Trigger_CreateExecuteCommandsRequest()
    for w in workloads:
        api.Logger.verbose("Setting max mtu on %s in %s/%s" % (w.interface, w.workload_name, w.node_name))
        mtu_cmd = __get_mtu_cfg_cmd(w.interface)
        api.Trigger_AddCommand(req, w.node_name, w.workload_name, mtu_cmd)
    resp = api.Trigger(req)
    return __verify_response(resp)

def __config_max_mtu():
    if __config_max_mtu_on_host_intfs() == False or\
       __config_max_mtu_on_workload_intfs() == False:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Main(step):
    api.Logger.info("Configuring MAX MTU")
    return __config_max_mtu()

if __name__ == '__main__':
    Main(None)
