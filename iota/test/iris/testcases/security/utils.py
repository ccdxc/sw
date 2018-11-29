#! /usr/bin/python3
import json
import glob
import iota.harness.api as api
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.objects.sgpolicy as sgpolicy_obj

def GetProtocolDirectory(proto):
    api.Logger.info("BARUN GetProtocolDirectory {}".format(proto))
    return api.GetTopologyDirectory() + "/{}".format(proto)

def GetTargetJsons(proto):
    api.Logger.info("BARUN GetTargetJsons {}".format(proto))
    return glob.glob(GetProtocolDirectory(proto) + "/*_policy.json")

def GetTargetVerifJsons(proto):
    api.Logger.info("BARUN GetTargetVerifJsons {}".format(proto))
    return glob.glob(GetProtocolDirectory(proto) + "/*_verif.json")

def ReadJson(filename):
    return api.parser.JsonParse(filename)

def GetHping3Cmd(protocol, destination_ip, destination_port):
    if protocol == 'tcp':
        cmd = "hping3 -p {} -c 1 {}".format(int(destination_port), destination_ip)
    elif protocol == 'udp':
        cmd = "hping3 --{} -p {} -c 1 {}".format(protocol.lower(), int(destination_port), destination_ip)
    else:
        cmd = "hping3 --{} -c 1 {}".format(protocol.lower(), destination_ip)
        
    return cmd

def GetVerifJsonFromPolicyJson(policy_json):
    return policy_json.replace("_policy", "_verif")

def VerifyCmd(cmd, action):
    api.PrintCommandResults(cmd)
    result = api.types.status.SUCCESS
    if action == 'PERMIT' and cmd.exit_code != 0:
        result = api.types.status.FAILURE
    elif action == 'DENY' and cmd.exit_code ==0:
        result = api.types.status.FAILURE

    return result

def GetDestPort(port):
    if ',' in port:
        return "100"
    if '-' in port:
        return "120"
    return port 

def RunCmd(workload, protocol, destination_ip, destination_port, action):
    req = api.Trigger_CreateExecuteCommandsRequest() 
    result = api.types.status.SUCCESS

    cmd = GetHping3Cmd(protocol, destination_ip, destination_port)
    api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
    api.Logger.info("Running COMMAND {}".format(cmd))

    resp = api.Trigger(req)

    for cmd in resp.commands:
        result = VerifyCmd(cmd, action)
        if(result == api.types.status.FAILURE):
            break;

    return result

def RunAll(src_w, dest_w, verif_json):
    res = api.types.status.SUCCESS
    api.Logger.info("VERIFY JSON FILE {}".format(verif_json))

    verif = []
    with open(verif_json, 'r') as fp:
        verif = json.load(fp)

    api.Logger.info("VERIF = {}".format(verif))

    for i in range(0, len(verif)):
        protocol = verif[i]['protocol'] 
        #protocol = "udp"
        destination_port = GetDestPort(verif[i]['port'])
        #destination_port = "90"
        action = verif[i]['result']
        #action = "DENY"
 
        res = RunCmd(src_w, protocol, dest_w.ip_address, destination_port, action)
        #if(res == api.types.status.FAILURE):
        #    return res

    return res
