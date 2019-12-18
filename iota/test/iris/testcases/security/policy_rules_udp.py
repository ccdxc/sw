#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.objects.sgpolicy as sgpolicy_obj
import iota.test.iris.config.netagent.api as agent_api
import pdb
import random

proto_list  = [ 'udp']
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
    dst['proto-ports'] = []
    dst['proto-ports'].append(get_appconfig(protocol, port))
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

def Setup(tc):
    tc.loop_count = 0
    tc.policy_db = {}
    
    tc.nodes = api.GetNaplesHostnames()
    #netagent_cfg_api.ReadJsons()
    tc.sgpolicy = sgpolicy_obj.gl_sg_json_template
    #get rules list - it is list of dictionary
    policy_rules = tc.sgpolicy['sgpolicies'][0]['spec']['policy-rules']
    meta = tc.sgpolicy['sgpolicies'][0]['meta']
    meta['name'] = 'default_policies'
    meta['namespace'] = 'default'
    meta['tenant'] = 'default'
    print('Initial policy rules {}'.format(policy_rules))
    print(api.GetWorkloads())
    tc.workload_pairs = api.GetLocalWorkloadPairs()
    print(tc.workload_pairs)
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        for proto in proto_list:
            if proto is not 'icmp':
                port = str(random.randint(1,65535))
            else:
                port = '1'
            action = random.choice(action_list) 
            rule = Rule((w1,w2,proto, port),(action))
            tc.policy_db[rule.key] = rule.action

    for match, action_tuple in tc.policy_db.items():
        src,dst,proto,dst_port = match
        action      = action_tuple
        policy_rules.append(get_rule(dst.ip_address, src.ip_address, proto, dst_port, action))
    print(tc.sgpolicy)
    json_str   = json.dumps(tc.sgpolicy)
    obj = api.parser.ParseJsonStream(json_str)
    agent_api.ConfigureSecurityGroupPolicies(obj.sgpolicies, oper= agent_api.CfgOper.UPDATE)
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting")
    tc.cmd_cookies = []
    print(tc.policy_db) 
    #local_to_remote_deny - L3 Ranz
    req = api.Trigger_CreateExecuteCommandsRequest()
    for rule, action in tc.policy_db.items():
        src_w, dst_w, proto, dst_port = rule
        cmd_cookie = (src_w.ip_address, dst_w.ip_address, proto, dst_port, action)
        tc.cmd_cookies.append(cmd_cookie)
        #hping3 has default proto as tcp not a good design
        if proto == 'tcp':
            cmd = "hping3 -p {} -c 1 {}".format(int(dst_port), dst_w.ip_address)
        elif proto == 'udp':
            cmd = "hping3 --{} -p {} -c 1 {}".format(proto.lower(), int(dst_port), dst_w.ip_address)
        else:
            cmd = "hping3 --{} -c 1 {}".format(proto.lower(), dst_w.ip_address)
        api.Trigger_AddCommand(req, src_w.node_name, src_w.workload_name,cmd)
    tc.resp = api.Trigger(req)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE
    result = api.types.status.SUCCESS
    cookie_idx = 0
    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (str(tc.cmd_cookies[cookie_idx])))
        api.PrintCommandResults(cmd)
        action = tc.cmd_cookies[cookie_idx][-1]
        proto = tc.cmd_cookies[cookie_idx][2]
        if proto == 'udp':
            if action == 'PERMIT' and cmd.exit_code != 0:
                result = api.types.status.FAILURE
            elif action == 'DENY' and cmd.exit_code == 0:
                result = api.types.status.FAILURE
        cookie_idx += 1
    
    return api.types.status.SUCCESS

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
