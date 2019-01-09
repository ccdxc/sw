#! /usr/bin/python3
import json
import glob
import yaml
import iota.harness.api as api
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.objects.sgpolicy as sgpolicy_obj
import subprocess
import threading
import os

proto_list  = [ 'tcp']
action_list  = ['PERMIT', 'DENY']

class Rule:
    def __init__(self, key, action):
        self.key = key
        self.action = action


def get_appconfig(protocol, port):
    app_config = {}
    app_config['protocol'] = protocol
    app_config['port'] = port
    return app_config

def get_destination(dst_ip, protocol, port):
    dst = {}
    dst['addresses'] = []
    dst['addresses'].append(dst_ip)
    dst['app-configs'] = []
    dst['app-configs'].append(get_appconfig(protocol, port))
    return dst

def get_source(src_ip):
    src = {}
    src['addresses'] = []
    src['addresses'].append(src_ip)
    return src

def get_rule(dst_ip, src_ip, protocol, port, action):
    rule = {}
    rule['destination'] = get_destination(dst_ip, protocol, port)
    rule['source'] = get_source(src_ip)
    rule['action'] = action
    return rule

class Command(object):
    def __init__(self, cmd):
        api.Logger.info("Running Command : {}".format(cmd))
        self.cmd = cmd
        self.process = None
        self.out = None

    def run_command(self, capture = False):
        if not capture:
            self.process = subprocess.Popen(self.cmd,shell=True)
            self.process.communicate()
            return
        # capturing the outputs of shell commands
        self.process = subprocess.Popen(self.cmd, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
        out,err = self.process.communicate()
        if len(out) > 0:
            self.out = out.splitlines()
        else:
            self.out = None

    # set default timeout to 2 minutes
    def run(self, capture = False, timeout = 120):
        thread = threading.Thread(target=self.run_command, args=(capture,))
        thread.start()
        thread.join(timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()
        return self.out

def GetSecurityPolicy(workload):
    mnicIP = api.GetNicIntMgmtIP(workload.node_name)

    cmd = 'sshpass -p vm ssh vm@{} curl -X GET -H "Content-Type:application/json" http://{}:9007/api/security/policies/'.format(workload.workload_name,mnicIP)
    return Command(cmd).run()

def GetProtocolDirectory(proto):
    return api.GetTopologyDirectory() + "/gen/{}".format(proto)

def GetTargetJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_policy.json")

def GetTargetVerifJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_verif.json")

def ReadJson(filename):
    api.Logger.info("Reading JSON file {}".format(filename))
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
    api.Logger.info("Parsing Verification json : \n{}".format(verif_str))
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
    cmd = 'curl -X GET -H "Content-Type:application/json" http://169.254.0.1:9007/api/security/policies/'
    req = api.Trigger_CreateExecuteCommandsRequest()
    result = api.types.status.SUCCESS
    verif = None

    api.Trigger_AddHostCommand(req, workload.node_name, cmd)
    api.Logger.info("Running COMMAND {}".format(cmd))

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.Logger.info("CMD = {}", cmd)
        if cmd.exit_code != 0:
            return verif
    #out = GetSecurityPolicy(workload)
    verif = ParseVerifStr(cmd.stdout)
    #verif = ParseVerifStr(out)

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

def DisableHalLogs(workload):
    cmd = 'halctl debug trace --level none'
    req = api.Trigger_CreateExecuteCommandsRequest()
    result = api.types.status.SUCCESS

    api.Trigger_AddNaplesCommand(req, workload.node_name, workload.workload_name, cmd)
    api.Logger.info("Running COMMAND {}".format(cmd))

    api.Trigger(req)
