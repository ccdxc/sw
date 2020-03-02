#! /usr/bin/python3
import iota.harness.api as api

def __get_flap_cfg_cmd(intf, oper):
    flap_base_cmd = "ifconfig %s %s"
    flap_cmd = flap_base_cmd % (intf, oper)
    return flap_cmd

def __verify_response(resp, oper):
    if resp is None:
        api.Logger.critical("Failed to set state on host interface - No response")
        return False
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.critical("Failed to set state on interface")
            api.PrintCommandResults(cmd)
            #return False
    return True

def __set_state_all_host_intfs(oper):
    req = api.Trigger_CreateExecuteCommandsRequest()
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        intf_list = api.GetNaplesHostInterfaces(node)
        api.Logger.info("Host port admin state  %s %s in %s" % (oper, intf_list, node))
        for intf in intf_list:
            flap_cmd = __get_flap_cfg_cmd(intf, oper)
            api.Trigger_AddHostCommand(req, node, flap_cmd)
    resp = api.Trigger(req)
    return __verify_response(resp, oper)

def __flap_host_intf():
    if __set_state_all_host_intfs("down") == False:
        return api.types.status.FAILURE
    
    if __set_state_all_host_intfs("up") == False:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def flapHostIntf(tc): #use this api if invoking as a background task
    api.Logger.info("FLAPPING Host Intfs")
    return __flap_host_intf()

def Main(step): #directly use utils.hostflap as step in iota suite
    api.Logger.info("FLAPPING Host Intfs")
    return __flap_host_intf()

if __name__ == '__main__':
    Main(None)
