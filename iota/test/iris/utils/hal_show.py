#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.address as address_utils
import yaml

def GetHALShowOutput(naples_node, show_cmd, args=None, yaml=True):
    #TODO -  instead of using show_cmd directly, get cmd_id and use that to retrieve the actual cmd
    cmd = '/nic/bin/halctl show ' + show_cmd
    if args is not None:
        cmd = cmd + ' ' + args
    if yaml:
        cmd = cmd + ' --yaml'
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, naples_node, cmd)
    resp = api.Trigger(req)

    return resp, api.IsApiResponseOk(resp)

def Getl2seg_vlan_mapping(naples_node):
    l2seg_vlan_dict = dict()
    resp, result = GetHALShowOutput(naples_node, "l2seg")
    if not result:
        api.Logger.critical("unknown response from Naples")
        return l2seg_vlan_dict
    cmd = resp.commands[0]

    perl2segOutput = cmd.stdout.split("---")

    for l2seg in perl2segOutput:
        l2segObj = yaml.load(l2seg)
        if l2segObj is not None:
            l2seg_id = l2segObj['spec']['keyorhandle']['keyorhandle']['segmentid'] 
            vlan_id = l2segObj['spec']['wireencap']['encapvalue']
            l2seg_vlan_dict.update({l2seg_id: vlan_id})

    return l2seg_vlan_dict

def GetIfId_lif_mapping(naples_node):
    ifId_lif_dict = dict()
    resp, result = GetHALShowOutput(naples_node, "interface")
    if not result:
        api.Logger.critical("unknown response from Naples")
        return ifId_lif_dict
    cmd = resp.commands[0]

    perifOutput = cmd.stdout.split("---")

    for intf in perifOutput:
        intfObj = yaml.load(intf)
        if intfObj is not None:
            if_id = intfObj['spec']['keyorhandle']['keyorhandle']['interfaceid']
            try:
                lif_id = intfObj['spec']['ifinfo']['ifenicinfo']['lifkeyorhandle']['keyorhandle']['lifid']
                ifId_lif_dict.update({if_id: lif_id})
            except:
                api.Logger.warn("GetIfId_lif_mapping: no lif_id for interfaceID", if_id)

    return ifId_lif_dict

def GetLifId_intfName_mapping(naples_node):
    lifId_intfName_dict = dict()
    resp, result = GetHALShowOutput(naples_node, "lif")
    if not result:
        api.Logger.critical("unknown response from Naples")
        return lifId_intfName_dict
    cmd = resp.commands[0]

    perLifOutput = cmd.stdout.split("---")

    for lif in perLifOutput:
        lifObj = yaml.load(lif)
        if lifObj is not None:
            lifid = lifObj['spec']['keyorhandle']['keyorhandle']['lifid'] 
            intfName = lifObj['spec']['name']
            #TODO: mnic interface names are appended with "/lif<lif_id>"
            # eg., inb_mnic0/lif67
            # so until that is fixed, temp hack to strip the "/lif<lif_id>" suffix
            intfName = intfName.split("/")[0]
            lifId_intfName_dict.update({lifid: intfName})

    return lifId_intfName_dict

def GetIntfName2LifId_mapping(naples_node):
    intfName2lifId_dict = dict()
    resp, result = GetHALShowOutput(naples_node, "lif")
    if not result:
        api.Logger.critical("unknown response from Naples")
        return intfName2lifId_dict
    cmd = resp.commands[0]

    perLifOutput = cmd.stdout.split("---")
    for lif in perLifOutput:
        lifObj = yaml.load(lif)
        if lifObj is not None:
            lifid = lifObj['spec']['keyorhandle']['keyorhandle']['lifid']
            intfName = lifObj['spec']['name']
            #TODO: mnic interface names are appended with "/lif<lif_id>"
            # eg., inb_mnic0/lif67
            # so until that is fixed, temp hack to strip the "/lif<lif_id>" suffix
            intfName = intfName.split("/")[0]
            intfName2lifId_dict.update({intfName: lifid})

    return intfName2lifId_dict

def IsNaplesForwardingModeClassic(naples_node):
    resp, result = GetHALShowOutput(naples_node, "system forwarding-mode", yaml=False)
    if not result:
        api.Logger.critical("unknown response from Naples")
        return True 
    cmd = resp.commands[0]

    api.Logger.info("Got Naples forwarding as {}".format(cmd.stdout))
    return cmd.stdout.find("CLASSIC") >= 0
