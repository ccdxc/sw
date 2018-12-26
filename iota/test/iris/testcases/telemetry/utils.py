#! /usr/bin/python3
import json
import glob
import iota.harness.api as api
#import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import time
import re
import pdb

def GetProtocolDirectory(proto):
    return api.GetTopologyDirectory() + "/gen/telemetry/{}".format(proto)

def GetTargetJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_policy.json")

def GetTargetVerifJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_verif.json")

def ReadJson(filename):
    return api.parser.JsonParse(filename)

def GetHping3Cmd(protocol, destination_ip, destination_port):
    if protocol == 'tcp':
        cmd = "hping3 -S -p {} -c 1 {}".format(int(destination_port), destination_ip)
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
    if 'tcpdump' in cmd.command:
        matchObj = re.search( r'(.*) GREv0, length(.*)', cmd.stdout, 0)
        if matchObj is None:
            result = api.types.status.FAILURE
    return result

def GetDestPort(port):
    if ',' in port:
        return "100"
    elif '-' in port:
        return "120"
    elif 'any' in port:
        return '3000'
    return port 

def RunCmd(workload, protocol, destination_ip, destination_port, collector_w, action):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    result = api.types.status.SUCCESS

    api.Trigger_AddCommand(req, collector_w.node_name, collector_w.workload_name,
                           "tcpdump -nni %s ip proto gre" % (collector_w.interface), background=True)
    cmd = GetHping3Cmd(protocol, destination_ip, destination_port)
    api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
    api.Logger.info("Running from workload_ip {} COMMAND {}".format(workload.ip_address, cmd))

    trig_resp = api.Trigger(req)
    #time.sleep(1)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    for cmd in resp.commands:
        result = VerifyCmd(cmd, action)
        if (result == api.types.status.FAILURE):
            api.Logger.info("Testcase FAILED!! cmd: {}".format(cmd))
            break;
    return result

def GetSourceWorkload(verif, tc):
    workloads = api.GetWorkloads()
    sip = verif['src_ip']
    src_wl = None
    for wl in workloads:
        if '/24' in sip or 'any' in sip:
            if verif['dst_ip'] != wl.ip_address:
                src_wl = wl
                break
        else:
            if sip == wl.ip_address:
                src_wl = wl
                break
    if src_wl is None:
        api.Logger.info("Did not get a matching workload. Dump all workloads")
        api.Logger.info("sip: {}".format(sip))
        api.Logger.info("verif_dst_ip: {}".format(verif['dst_ip']))
        for wl in workloads:
            api.Logger.info("wl.ip_address: {}".format(wl.ip_address))
    return src_wl

def GetDestWorkload(verif, tc):
    workloads = api.GetWorkloads()
    dip = verif['dst_ip']
    dst_wl = None
    for wl in workloads:
        if '/24' in dip or 'any' in dip:
            if verif['src_ip'] != wl.ip_address:
                dst_wl = wl
                break
        else:
            if dip == wl.ip_address:
                dst_wl = wl
                break
    return dst_wl

def RunAll(collector_w, verif_json, tc):
    res = api.types.status.SUCCESS
    api.Logger.info("VERIFY JSON FILE {}".format(verif_json))

    verif = []
    ret = {}
    with open(verif_json, 'r') as fp:
        verif = json.load(fp)

    api.Logger.info("VERIF = {}".format(verif))
    count = 0
    for i in range(0, len(verif)):
        protocol = verif[i]['protocol'] 
        src_w = GetSourceWorkload(verif[i], tc)
        dest_w = GetDestWorkload(verif[i], tc)
        if src_w == None:
            continue
        if dest_w == None:
            continue
        dest_port = GetDestPort(verif[i]['port'])
        action = verif[i]['result']
        res = RunCmd(src_w, protocol, dest_w.ip_address, dest_port, collector_w, action)
        count = count + 1
    ret['res'] = res
    ret['count'] = count
    return ret
