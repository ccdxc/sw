#! /usr/bin/python3
import grpc
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api
import iota.test.iris.config.netagent.objects.sgpolicy as sgpolicy_obj
import iota.test.iris.config.netagent.api as agent_api
import pdb

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
    api.Logger.info("Loop count set to ", tc.loop_count)
    
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
        rule = get_rule(w1.ip_address, w2.ip_address, "udp", "59379","PERMIT")
        tc.policy_db[(w1, w2,"udp", "59379")] = "PERMIT"
        policy_rules.append(rule)
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
        cmd = "hping3 --{} -p {} -c 1 {}".format(proto, int(dst_port), dst_w.ip_address)
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
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1
    return result

def Teardown(tc):
    api.Logger.info("Tearing down ...")
    return api.types.status.SUCCESS
