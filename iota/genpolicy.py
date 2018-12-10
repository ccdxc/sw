#! /usr/bin/python3
import argparse
import json
import os
import collections
import random

sgpolicy_template = { "sgpolicies" : [
    {
    "kind"                  : "SGPolicy",

    "meta": {
        "name"              : "default-policy",
        "tenant"            : "default",
        "namespace"         : "default",
        "creation-time"     : "1970-01-01T00:00:00Z",
        "mod-time"          : "1970-01-01T00:00:00Z"
    },

    "spec": {
        "attach-tenant"     : True,
        "policy-rules"      : [
        ]
    },
    "status"                : {}
    }
    ]
}

def get_ip_addr(a, b, c):
    return str(a) + "." + str(b) + "." + str(c) + ".0/24"

def generate_ip_list(count):
    gen_count = 0
    ip_list = [] 
    a = 1
    b = 0
    c = 0

    while(gen_count < count):
        ip_list.append(get_ip_addr(a, b, c))
        c = c + 1
        if (c == 128):
            c = 0
            b = b + 1
            if ( b == 128):
                b = 0
                a = a + 1

        gen_count = gen_count + 1

    return ip_list

def get_verif(protocol, port, result):
    verif = {}
    verif['protocol'] = protocol
    verif['port'] = port
    verif['result'] = result
    return verif

def get_appconfig(protocol, port):
    app_config = {}
    app_config['protocol'] = protocol
    app_config['port'] = port
    return app_config

def get_destination(dst_ip, protocol, port):
    dst = {}

    if(type(dst_ip) is list):
        dst['addresses'] = dst_ip
    else:
        dst['addresses'] = []
        dst['addresses'].append(dst_ip)

    dst['app-configs'] = []
    dst['app-configs'].append(get_appconfig(protocol, port))
    return dst

def get_source(src_ip):
    src = {}
    if(type(src_ip) is list):
        src['addresses'] = src_ip
    else:
        src['addresses'] = []
        src['addresses'].append(src_ip)
    return src

def get_rule(dst_ip, src_ip, protocol, port, action):
    rule = {}
    rule['destination'] = get_destination(dst_ip, protocol, port)
    rule['source'] = get_source(src_ip) 
    rule['action'] = action
    return rule

parser = argparse.ArgumentParser(description='Naples Security Policy Generator')
parser.add_argument('--topology', dest='topology_dir', required = dir,
                    default=None, help='Path to the JSON file having IOTA endpoint information.')

parser.add_argument('--expansion', dest='generate_expansion', action='store_true', default=False,
                     help='Generate expansion cases.')

GlobalOptions = parser.parse_args()
GlobalOptions.endpoint_file = GlobalOptions.topology_dir + "/endpoints.json"
GlobalOptions.protocols = ["udp", "tcp", "icmp"]
GlobalOptions.directories = ["udp", "tcp", "icmp", "mixed", "scale", "expansion"]
GlobalOptions.ports = ["10", "22", "24", "30", "50-100", "101-200", "201-250","10000-20000", "65535"]
GlobalOptions.actions = ["PERMIT", "DENY", "REJECT"]

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
    with open(GlobalOptions.endpoint_file, 'r') as fp:
        obj = json.load(fp)
    EP = [] 
    FILENAME = []

    for i in range(0, len(obj["endpoints"])):
        print("EP[%d] : %s" % (i, obj["endpoints"][i]["spec"]["ipv4-address"]))
        EP.append(StripIpMask(obj["endpoints"][i]["spec"]["ipv4-address"]))
        FILENAME.append("endpoint")

    EP.append(GetIpRange(EP[0]))
    EP.append(GetIpCidr(EP[0]))
    EP.append("any")

    json.dump(EP, open("EP.json", "w"))
    for dir in GlobalOptions.directories:
        if not os.path.exists(GlobalOptions.topology_dir + "/{}".format(dir)):
            os.makedirs(GlobalOptions.topology_dir + "/{}".format(dir))

    # One big policy
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['sgpolicies'][0]['spec']['policy-rules']
            del policy_rules[:]
            verif =[] 
            for i in range(0, len(EP) - 1):
                for j in range(i, len(EP)):
                    for k in GlobalOptions.ports:
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        policy_rules.append(rule)
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        policy_rules.append(rule)
                        verif.append(get_verif(protocol, k, action))
            json.dump(verif, open(GlobalOptions.topology_dir +"/{}/{}_{}_verif.json".format(protocol, protocol, action), "w"))
            json.dump(sgpolicy, open(GlobalOptions.topology_dir + "/{}/{}_{}_policy.json".format(protocol, protocol, action), "w"), indent=4)

    # Generic policy
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['sgpolicies'][0]['spec']['policy-rules']
            del policy_rules[:]
            verif =[] 
            for i in range(0, len(EP) - 1):
                for k in GlobalOptions.ports:
                    rule = get_rule(EP[i], "any", protocol, k, action)
                    policy_rules.append(rule)
                    verif.append(get_verif(protocol, k, action))
            json.dump(sgpolicy, open(GlobalOptions.topology_dir +"/{}/{}_{}_any_policy.json".format(protocol, protocol, action), "w"), indent=4)
            json.dump(verif, open(GlobalOptions.topology_dir + "/{}/{}_{}_any_verif.json".format(protocol, protocol, action), "w"), indent=4)
    
    # Specific Policy
    for protocol in GlobalOptions.protocols:
        for action in GlobalOptions.actions:
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['sgpolicies'][0]['spec']['policy-rules']
            del policy_rules[:]
            verif =[] 
            for i in range(0, len(EP) - 4):
                for j in range(i, len(EP) - 3):
                    for k in GlobalOptions.ports:
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        policy_rules.append(rule)
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        policy_rules.append(rule)
                        verif.append(get_verif(protocol, k, action))
            json.dump(sgpolicy, open(GlobalOptions.topology_dir + "/{}/{}_{}_specific_policy.json".format(protocol, protocol, action), "w"), indent=4)
            json.dump(verif, open(GlobalOptions.topology_dir + "/{}/{}_{}_specific_verif.json".format(protocol, protocol, action), "w"), indent=4)

    # Mixed Config
    for count in range(1,5):
        for protocol in GlobalOptions.protocols:
            for k in GlobalOptions.ports:
                sgpolicy = sgpolicy_template
                policy_rules = sgpolicy_template['sgpolicies'][0]['spec']['policy-rules']
                del policy_rules[:]
                verif =[] 
                for i in range(0, len(EP) - 4):
                    for j in range(i + 1, len(EP)):
                        action = GlobalOptions.actions[random.randint(0, len(GlobalOptions.actions) - 1)]
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        policy_rules.append(rule)
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        policy_rules.append(rule)
                        verif.append(get_verif(protocol, k, action))

        json.dump(sgpolicy, open(GlobalOptions.topology_dir + "/mixed/{}_mixed_policy.json".format(count), "w"), indent=4)
        json.dump(verif, open(GlobalOptions.topology_dir + "/mixed/{}_mixed_verif.json".format(count), "w"), indent=4)

    # Scale Config
    for count in [10000, 20000, 30000, 40000, 50000, 55000, 56000, 57000, 58000, 59000, 60000, 70000, 75000]:
    #for count in [10000]:
        sgpolicy = sgpolicy_template
        policy_rules = sgpolicy_template['sgpolicies'][0]['spec']['policy-rules']
        del policy_rules[:]
        verif =[]
        rule_count = 0
        i = 0
        j = 1
        k = 1
        proto_i = 0
        protocol = GlobalOptions.protocols[proto_i]

        while(rule_count < count):
            action = GlobalOptions.actions[random.randint(0, len(GlobalOptions.actions) - 1)]
            rule = get_rule(EP[i], EP[j], protocol, str(k), action)
            policy_rules.append(rule)
            rule = get_rule(EP[j], EP[i], protocol, str(k), action)
            policy_rules.append(rule)
            verif.append(get_verif(protocol, str(k), action))
            rule_count += 2
            k = k + 1
            if (k >= 65536):
                k = 1
                proto_i = proto_i + 1              
                protocol = GlobalOptions.protocols[proto_i]
           
                j = j + 1
                if (j == len(EP)):
                    i = i + 1 
                    j = i + 1
 
                if (i == len(EP) - 1):
                    print("Breaking from here I = {} J = {} RULE_CNT = {}".format(i, j, rule_count))
                    break
   
        print("Writing rule for scale {}".format(count))     
        json.dump(sgpolicy, open(GlobalOptions.topology_dir + "/scale/{}_scale_policy.json".format(count), "w"), indent=4)
        json.dump(verif, open(GlobalOptions.topology_dir + "/scale/{}_scale_verif.json".format(count), "w"), indent=4)

    #Expansion Config
    if GlobalOptions.generate_expansion : 
        pow_2 = [4 ** x for x in range(5)]
        ip_list = generate_ip_list(1024) 

        for count in [5000, 10000, 20000, 30000, 40000, 50000]:
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['sgpolicies'][0]['spec']['policy-rules']
            del policy_rules[:]
            verif =[]
            for n in pow_2:
                for m in pow_2:
                    rule_count = 1 
                    src_ip = ip_list[0 : n]
                    dst_ip = ip_list[512 : 512 + m]
                     
                    while(rule_count <= count):
                        action = GlobalOptions.actions[random.randint(0, len(GlobalOptions.actions) - 1)]
                        rule = get_rule(src_ip, dst_ip, protocol, str(rule_count), action)
                        policy_rules.append(rule)
                        rule_count = rule_count + 1

                    print("Writing rule for scale {}_{}_{}".format(count, n, m))
                    json.dump(sgpolicy, open(GlobalOptions.topology_dir + "/expansion/{}_{}_{}_expansion_policy.json".format(count, n, m), "w"), indent=4)

if __name__ == '__main__':
    Main()
