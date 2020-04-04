#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.testcases.qos.qos_utils as qos
import re

def Setup(tc):
 
    tc.desc = '''
    Test        :   QoS Switch Config
    Opcode      :   Config, Verify
    '''

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.cmd_cookies = []

    return api.types.status.SUCCESS

def Trigger(tc):

    result = api.types.status.SUCCESS
    #Enable Service Policy on the interfaces
    if getattr(tc.args, 'intf_qos', False) == True:
        result = api.EnableQosPorts(tc.nodes, "pmap-iota")
        if result != api.types.status.SUCCESS:
            api.Logger.error("QoS Service policy configure failed. Ignoring.")
    else:
        result = api.DisableQosPorts(tc.nodes, "pmap-iota")
        if result != api.types.status.SUCCESS:
            api.Logger.error("QoS Service policy unconfigure failed. Ignoring.")

    pause_enable = getattr(tc.args, 'pause_enable', False)
    pause_type = getattr(tc.args, 'pause_type', 0)

    api.Logger.info("pause_enable {}, pause_type {}".format(pause_enable, pause_type))
    if pause_type == 0: #LLFC
        if pause_enable:
            result = api.EnablePausePorts(tc.nodes)
            if result != api.types.status.SUCCESS:
                api.Logger.error("Port pause type LLFC configure failed. Ignoring.")
        else:
            result = api.DisablePausePorts(tc.nodes)
            if result != api.types.status.SUCCESS:
                api.Logger.error("Port pause type LLFC unconfigure failed. Ignoring.")
    else: #PFC
        if pause_enable:
            result = api.EnablePfcPorts(tc.nodes)
            if result != api.types.status.SUCCESS:
                api.Logger.error("Port pause type PFC configure failed. Ignoring.")
        else:
            result = api.DisablePfcPorts(tc.nodes)
            if result != api.types.status.SUCCESS:
                api.Logger.error("Port pause type PFC unconfigure failed. Ignoring.")

    #==============================================================
    # trigger the commands
    #==============================================================
    return api.types.status.SUCCESS

def Verify(tc):

    #==============================================================
    # verify the output
    #==============================================================

    result = api.types.status.SUCCESS
    return result

def Teardown(tc):

    #==============================================================
    # cleanup
    #==============================================================

    return api.types.status.SUCCESS

