#! /usr/bin/python3
import argparse
import json
import os
import collections
import random
import copy

flowmonpolicy_template = {
    "type" : "netagent",
    "rest-endpoint"    : "api/telemetry/flowexports/",
    "object-key" : "meta.tenant/meta.namespace/meta.name",
    "objects" : [
    {
    "kind"                  : "FlowExportPolicy",
    "meta": {
        "name"              : "flow-export-ipfix-0",
        "tenant"            : "default",
        "namespace"         : "default",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },
    "spec": {
        "interval": "1s",
        "template-interval": "5m",
        "format": "IPFIX",
        "exports": [
          {
            "destination": "192.168.100.103",
            "proto-port": {
              "protocol": "udp",
              "port": "2055"
            }
          }
        ],
        "match-rules": [
        ]
    },
    "status"                : {}
    }
    ]
}

def get_verif(dst_ip, src_ip, protocol, port, collector):
    verif = {}
    verif['dst_ip'] = dst_ip
    verif['src_ip'] = src_ip
    verif['protocol'] = protocol
    verif['port'] = port
    verif['result'] = "flowmon"
    verif['collector'] = collector
    return verif

def get_appconfig(protocol, port):
    app_config = {}
    app_config['protocol'] = protocol
    app_config['port'] = port
    return app_config

def get_app_proto(protocol, dport):
    proto_ports = []
    app = {}
    app['protocol'] = protocol
    if protocol == 'icmp':
        app['port'] = '0'
        #proto_str = str(protocol).upper() + '/' + '0' + '/' + '0'
    else:
        if dport == 'any':
            app['port'] = '0'
            #proto_str = str(protocol).upper() + '/' + '0'
        else:
            app['port'] = str(dport)
            #proto_str = str(protocol).upper() + '/' + str(dport)
#    app['port'] = str(dport)
#     app['proto-ports'] = []
#     app['proto-ports'].append(proto_str)
    proto_ports.append(app)
    return proto_ports

# def get_app_proto(protocol, dport):
#     if protocol == 'icmp':
#         proto_str = str(protocol).upper() + '/' + '0' + '/' + '0'
#     else:
#         if dport == 'any':
#             proto_str = str(protocol).upper() + '/' + '0'
#         else:
#             proto_str = str(protocol).upper() + '/' + str(dport)
#     app = {}
#     app['proto-ports'] = []
#     app['proto-ports'].append(proto_str)
#     return app

def get_destination(dst_ip, protocol, port):
    if dst_ip == 'any':
        dst_ip = '0.0.0.0/0'
    dst = {}
    dst['addresses'] = []
    dst['addresses'].append(dst_ip)
    dst['proto-ports'] = get_app_proto(protocol, port)
    #dst['app-protocol-selectors'] = []
    #dst['app-protocol-selectors'].append(get_appconfig(protocol, port))
    return dst

def get_source(src_ip):
    if src_ip == 'any':
        src_ip = '0.0.0.0/0'
    src = {}
    src['addresses'] = []
    src['addresses'].append(src_ip)
    return src

def get_rule(dst_ip, src_ip, protocol, port, action):
    rule = {}
    rule['destination'] = get_destination(dst_ip, protocol, port)
    rule['source'] = get_source(src_ip)
    #rule['app-protocol-selectors'] = get_app_proto(protocol, port)
    #rule['action'] = action
    return rule

parser = argparse.ArgumentParser(description='Naples Mirror Policy Generator')
parser.add_argument('--topology', dest='topology_dir', required = dir,
                    default=None, help='Path to the JSON file having IOTA endpoint information.')

GlobalOptions = parser.parse_args()
GlobalOptions.endpoint_file = GlobalOptions.topology_dir + "/endpoints.json"
GlobalOptions.protocols = ["udp", "tcp", "icmp"]
GlobalOptions.directories = ["udp", "tcp", "icmp", "mixed", "scale"]
#GlobalOptions.ports = ["10", "22", "24", "30", "50-100", "101-200", "201-250, 251-290", "10000-20000", "65535"]
GlobalOptions.ports = ["120"]
GlobalOptions.actions = ["flowmon"]

def StripIpMask(ip_address):
    if '/' in ip_address:
        return ip_address[:-3].encode("ascii")
    return ip_address.encode("ascii")

def GetIpRange(ip_address):
    stripped_ip = ip_address[:ip_address.rfind('.')]
    return "{}.1-{}.100".format(stripped_ip, stripped_ip)

def GetIpCidr(ip_address):
    stripped_ip = ip_address[:ip_address.rfind('.')]
    return "{}.0/24".format(stripped_ip)

def Main():
    #import pdb; pdb.set_trace()
    with open(GlobalOptions.endpoint_file, 'r') as fp:
        obj = json.load(fp)
    EP = []
    EP_nodes = []

    for i in range(0, len(obj["objects"])):
        print("EP[%d] : %s" % (i, obj["objects"][i]["spec"]["ipv4-addresses"][0]))
        EP.append(StripIpMask(obj["objects"][i]["spec"]["ipv4-addresses"][0]))
        EP_nodes.append(obj["objects"][i]["spec"]["node-uuid"])

    GlobalOptions.topology_dir = GlobalOptions.topology_dir + '/gen/telemetry/flowmon'
    for dir in GlobalOptions.directories:
        if not os.path.exists(GlobalOptions.topology_dir + "/{}".format(dir)):
            os.makedirs(GlobalOptions.topology_dir + "/{}".format(dir))

    # Specific match policy
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            mirrorpolicy = flowmonpolicy_template
            match_rules = flowmonpolicy_template['objects'][0]['spec']['match-rules']
            del match_rules[:]
            verif =[]
            loop = 1
            for i in range(0, len(EP)):
                for j in range(i+1, len(EP)):
                    for k in GlobalOptions.ports:
                        if EP_nodes[i] == EP_nodes[j]:
                            continue
                        if loop == 0:
                            continue
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP[i], EP[j], protocol, k, action))
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP[j], EP[i], protocol, k, action))
                        loop = 0
            if len(verif) > 0:
                json.dump(verif, open(GlobalOptions.topology_dir +"/{}/{}_{}_specific_verif.json".format(protocol, protocol, action), "w"), indent=4)
                json.dump(mirrorpolicy, open(GlobalOptions.topology_dir + "/{}/{}_{}_specific_policy.json".format(protocol, protocol, action), "w"), indent=4)

    EP_SUBNET = []
    EP_SUBNET.append(GetIpCidr(EP[0]))
    # Subnet policy
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            mirrorpolicy = flowmonpolicy_template
            match_rules = flowmonpolicy_template['objects'][0]['spec']['match-rules']
            del match_rules[:]
            verif =[]
            loop = 1
            for i in range(0, len(EP_SUBNET)):
                for j in range(0, len(EP)):
                    for k in GlobalOptions.ports:
                        if EP_nodes[i] == EP_nodes[j]:
                            continue
                        if loop == 0:
                            continue
                        rule = get_rule(EP_SUBNET[i], EP[j], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP_SUBNET[i], EP[j], protocol, k, action))
                        rule = get_rule(EP[j], EP_SUBNET[i], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP[j], EP_SUBNET[i], protocol, k, action))
                        loop = 0
            if len(verif) > 0:
                json.dump(mirrorpolicy, open(GlobalOptions.topology_dir +"/{}/{}_{}_subnet_policy.json".format(protocol, protocol, action), "w"), indent=4)
                json.dump(verif, open(GlobalOptions.topology_dir + "/{}/{}_{}_subnet_verif.json".format(protocol, protocol, action), "w"), indent=4)

    EP_ANY = []
    EP_ANY.append("any")
    GlobalOptions.ports.append("any")
    # Generic (any) Policy
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            mirrorpolicy = flowmonpolicy_template
            match_rules = flowmonpolicy_template['objects'][0]['spec']['match-rules']
            del match_rules[:]
            verif =[]
            loop = 1
            for i in range(0, len(EP_ANY)):
                for j in range(i, len(EP)):
                    for k in GlobalOptions.ports:
                        if EP_nodes[i] == EP_nodes[j]:
                            continue
                        if loop == 0:
                            continue
                        rule = get_rule(EP_ANY[i], EP[j], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP_ANY[i], EP[j], protocol, k, action))
                        rule = get_rule(EP[j], EP_ANY[i], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP[j], EP_ANY[i], protocol, k, action))
                        loop = 0
            if len(verif) > 0:
                json.dump(mirrorpolicy, open(GlobalOptions.topology_dir + "/{}/{}_{}_any_policy.json".format(protocol, protocol, action), "w"), indent=4)
                json.dump(verif, open(GlobalOptions.topology_dir + "/{}/{}_{}_any_verif.json".format(protocol, protocol, action), "w"), indent=4)

    # Mixed Config
    del match_rules[:]
    verif =[]
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            mirrorpolicy = flowmonpolicy_template
            match_rules = flowmonpolicy_template['objects'][0]['spec']['match-rules']
            loop = 1
            for i in range(0, len(EP)):
                for j in range(i+1, len(EP)):
                    for k in GlobalOptions.ports:
                        if EP_nodes[i] == EP_nodes[j]:
                            continue
                        if loop == 0:
                            continue
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP[i], EP[j], protocol, k, action))
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        match_rules.append(rule)
                        verif.append(get_verif(EP[j], EP[i], protocol, k, action))
                        loop = 0
    if len(verif) > 0:
        json.dump(verif, open(GlobalOptions.topology_dir +"/{}/flowmon_mixed_verif.json".format('mixed'), "w"), indent=4)
        json.dump(mirrorpolicy, open(GlobalOptions.topology_dir + "/{}/flowmon_mixed_policy.json".format('mixed'), "w"), indent=4)

    # Scale Config
    verif = []
    for action in GlobalOptions.actions:
        mirrorpolicies = copy.deepcopy(flowmonpolicy_template)
        objects = mirrorpolicies['objects']
        mirorpolicy = copy.deepcopy(objects[0])
        export = copy.deepcopy(mirorpolicy['spec']['exports'][0])
        del objects[:]
        proto_index = 0
        ep_index = 0
        cnt = 0
        for collector_port in [2055, 3055]:
            exports = []
            for i in range(0, min(len(EP), 4)):   # Export loop
                export['destination'] = EP[i-7]
                export['proto-port']['port'] = collector_port
                exports.append(copy.deepcopy(export))
                objects.append(copy.deepcopy(mirorpolicy))
                objects[-1]['spec']['exports'] = copy.deepcopy(exports)
                objects[-1]['meta']['name'] = "flowmon-scale-%s"%cnt
                match_rules = objects[-1]['spec']['match-rules']
                del match_rules[:]
                protocol =  GlobalOptions.protocols[proto_index]
                for j in range(ep_index+1, len(EP)):
                    for k in GlobalOptions.ports:
                        rule = get_rule(EP[ep_index], EP[j], protocol, k, action)
                        match_rules.append(rule)
                        #verif.append(get_verif(EP[i], EP[j], protocol, k, export, action))
                        rule = get_rule(EP[j], EP[ep_index], protocol, k, action)
                        match_rules.append(rule)
                        #verif.append(get_verif(EP[j], EP[i], protocol, k, export, action))

                proto_index = (proto_index + 1) % len(GlobalOptions.protocols)
                if proto_index == 0:
                    ep_index += 1
                cnt += 1

            exports = []
            for i in range(4, 8):   # Export loop
                export['destination'] = EP[i-7]
                export['proto-port']['port'] = collector_port
                exports.append(copy.deepcopy(export))
                objects.append(copy.deepcopy(mirorpolicy))
                objects[-1]['spec']['exports'] = copy.deepcopy(exports)
                objects[-1]['meta']['name'] = "flowmon-scale-%s"%cnt
                match_rules = objects[-1]['spec']['match-rules']
                del match_rules[:]
                protocol =  GlobalOptions.protocols[proto_index]
                for j in range(ep_index+1, len(EP)):
                    for k in GlobalOptions.ports:
                        rule = get_rule(EP[ep_index], EP[j], protocol, k, action)
                        match_rules.append(rule)
                        #verif.append(get_verif(EP[i], EP[j], protocol, k, export, action))
                        rule = get_rule(EP[j], EP[ep_index], protocol, k, action)
                        match_rules.append(rule)
                        #verif.append(get_verif(EP[j], EP[i], protocol, k, export, action))

                proto_index = (proto_index + 1) % len(GlobalOptions.protocols)
                if proto_index == 0:
                    ep_index += 1
                cnt += 1
    json.dump(mirrorpolicies, open(GlobalOptions.topology_dir + "/{}/flowmon_scale_policy.json".format('scale'), "w"), indent=4)

if __name__ == '__main__':
    Main()
