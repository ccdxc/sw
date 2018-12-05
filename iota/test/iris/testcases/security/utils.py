#! /usr/bin/python3
import json
import glob
import yaml
import iota.harness.api as api
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.objects.sgpolicy as sgpolicy_obj

def GetProtocolDirectory(proto):
    return api.GetTopologyDirectory() + "/{}".format(proto)

def GetTargetJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_policy.json")

def GetTargetVerifJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_verif.json")

def ReadJson(filename):
    return api.parser.JsonParse(filename)

def GetHping3Cmd(protocol, destination_ip, destination_port, source_port = 0, ):
    if protocol == 'tcp':
        cmd = "hping3 -p {} -c 1 {}".format(int(destination_port), destination_ip)
    elif protocol == 'udp':
        cmd = "hping3 --{} -p {} -c 1 {}".format(protocol.lower(), int(destination_port), destination_ip)
    else:
        cmd = "hping3 --{} -c 1 {}".format(protocol.lower(), destination_ip)
        
    return cmd

def GetVerifJsonFromPolicyJson(policy_json):
    return policy_json.replace("_policy", "_verif")

def GetNwsecYaml(workload):
    cmd = 'sleep 3 && /nic/bin/halctl show nwsec policy'
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, workload.node_name, cmd)
    resp = api.Trigger(req)
    yml = None

    cmd = resp.commands[0]
    if not cmd.stdout:
        return None

    yml_loaded = yaml.load_all(cmd.stdout)
    for yml in yml_loaded:
        if yml is not None:
            return yml
    return None

def GetRuleHit(workload, ruleid):
    d = GetNwsecYaml(workload)
    print(d)

    if d is None:
        return None
    for i in range(0, len(d['polstats']["rulestats"])):
        if(d['polstats']["rulestats"][i]["ruleid"] == ruleid):
            print("FOUND Rule id {}".format(ruleid))
            return None
    return None
        

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

def RunCmd(workload, protocol, destination_ip, destination_port, action, ruleid):
    req = api.Trigger_CreateExecuteCommandsRequest() 
    result = api.types.status.SUCCESS

    #GetRuleHit(workload, ruleid)

    for i in range(0, len(destination_port)):
        cmd = GetHping3Cmd(protocol[i], destination_ip, GetDestPort(destination_port[i]))
        api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
        api.Logger.info("Running COMMAND {}".format(cmd))

    resp = api.Trigger(req)

    for cmd in resp.commands:
        result = VerifyCmd(cmd, action)
        if(result == api.types.status.FAILURE):
            break;

    return result

def GetVerif(protocol, src_ip, src_port, dst_ip, dst_port, result, ruleid):
    verif = {}
    verif['ruleid'] = ruleid
    verif['protocol'] = protocol
    verif['src_ip'] = src_ip
    verif['src_port'] = src_port
    verif['dst_ip'] = dst_ip
    verif['dst_port'] = dst_port
    verif['result'] = result
    return verif

def ParseVerifStr(verif_str):
    js = json.loads(verif_str)
    verif = []

    for i in range(0, len(js[0]["spec"]["policy-rules"])):
        protocol = []
        for k in range(0, len(js[0]["spec"]["policy-rules"][i]["destination"]["app-configs"])):
            protocol.append(js[0]["spec"]["policy-rules"][i]["destination"]["app-configs"][k]["protocol"])
        src_ip = js[0]["spec"]["policy-rules"][i]["source"]["addresses"]
        src_port = "1234"
        dst_ip = js[0]["spec"]["policy-rules"][i]["destination"]["addresses"]

        dst_port = []
        for k in range(0, len(js[0]["spec"]["policy-rules"][i]["destination"]["app-configs"])):
            protocol.append(js[0]["spec"]["policy-rules"][i]["destination"]["app-configs"][k]["port"])
            dst_port.append(js[0]["spec"]["policy-rules"][i]["destination"]["app-configs"][k]["port"])
        result = js[0]["spec"]["policy-rules"][i]["action"]
        ruleid = js[0]["spec"]["policy-rules"][i]["rule-id"] 
        v = GetVerif(protocol, src_ip, src_port, dst_ip, dst_port, result, ruleid)
        verif.append(v)
    return verif

def GetVerifDict(workload):
    cmd = 'curl -X GET -H "Content-Type:application/json" http://1.0.0.2:9007/api/security/policies/'
    req = api.Trigger_CreateExecuteCommandsRequest()
    result = api.types.status.SUCCESS
    verif = None

    api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
    api.Logger.info("Running COMMAND {}".format(cmd))

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.Logger.info("CMD = {}", cmd)
        if cmd.exit_code != 0:
            return verif 

        verif = ParseVerifStr(cmd.stdout)
         
    return verif

def RunAll(src_w, dest_w):
    res = api.types.status.SUCCESS

    verif = GetVerifDict(src_w)
    if verif == None:
        return api.types.status.FAILURE

    for i in range(0, len(verif)):
        protocol = verif[i]['protocol']
        #protocol = "udp"
        destination_port = verif[i]['dst_port']
        #destination_port = "90"
        action = verif[i]['result']
        #action = "DENY"

        ruleid = verif[i]['ruleid']

        res = RunCmd(src_w, protocol, dest_w.ip_address, destination_port, action, ruleid)
        #if(res == api.types.status.FAILURE):
        #    return res

    return res
