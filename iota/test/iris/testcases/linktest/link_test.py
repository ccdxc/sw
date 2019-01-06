#! /usr/bin/python3
import time
import iota.harness.api as api
import yaml
import iota.test.iris.config.netagent.api as netagent_cfg_api

def Setup(tc):
#    api.Logger.info("RAMI SETUP")
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("start flapping all ports")
    netagent_cfg_api.FlapPorts()
    return api.types.status.SUCCESS

def Verify(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    command = "/nic/bin/halctl  show port  --yaml"
    for nodename in api.GetNaplesHostnames():
        api.Trigger_AddNaplesCommand(req, nodename, command)
#    trig_resp = api.Trigger(req)
#    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
#    tc.resp = api.Trigger_AggregateCommandsResponse(tri_resp)
    tc.resp = api.Trigger(req)

    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS
#inint results to be success

    for cmd in tc.resp.commands:
#traverese through all commands that were sent to the end points and check the status
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
#if command failed - then exit
#Below section is applicable for Napls. TBD - comamnds for other NIC and TOR side
        perPortOutput = cmd.stdout.split("---") #split output of halctl. Each entry will hold information about a port
        for portInfo in perPortOutput:            
            testobj = yaml.load(portInfo)#parse the YAML
            if bool(testobj):#last split is always empty - skip it. Otherwise process it
                 portId=testobj['spec']['keyorhandle']['keyorhandle']['portid']
                 portStatus=testobj['status']['operstatus']
                 if portStatus!=1 :
                      api.Logger.error("Node Name:[%s] Port:[%d] Status:[%d] - Port Is in bad state "% (cmd.node_name,portId,portStatus))
                      result = api.types.status.FAILURE
                 else :
                      api.Logger.info("Node Name:[%s] Port:[%d] Status:[%d]" % (cmd.node_name,portId,portStatus))

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

