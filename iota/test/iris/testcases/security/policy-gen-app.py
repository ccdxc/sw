#! /usr/bin/python3
import argparse
import json
import os
import collections
import random

object_template = {
    "type" : "netagent",
    "rest-endpoint" : "api/security/policies/",
    "object-key" : "meta.tenant/meta.namespace/meta.name",
    "objects": [],
}

default_policy = {"action": "PERMIT"}

sgpolicy_template = {
    "kind"                  : "NetworkSecurityPolicy",

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

def reset():
    object_template['objects'] = []
    sgpolicy_template['spec']['policy-rules'] = []

def get_ip_addr(a, b, c, d):
    return str(a) + "." + str(b) + "." + str(c) + "." + str(d) + "/24"

def generate_ip_list(count, cur_ip, EP):
    gen_count = 0
    ip_list = []
    a = 1
    b = 0
    c = 0
    d = 2
    i = 1

    while gen_count < count and i < len(EP) :
        if EP[i] != cur_ip and EP[i] != "any":
            ip_list.append(EP[i])
            gen_count = gen_count + 1
        i = i + 1

    while(gen_count < count):
        ip_list.append(get_ip_addr(a, b, c, d))
        d = d + 1
        if d == 128:
            d = 2
            c = c + 1
            if c == 128:
                c = 0
                b = b + 1
                if b == 128:
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

parser = argparse.ArgumentParser(description = "Policy Rules Generator.")
parser.add_argument('-s', '--scale', help='delimited list of scale', type=str, required=True)
parser.add_argument('-t', '--topology', help='Path of topology directory', default="/sw/iota/test/iris/topologies/container/", type=str)
parser.add_argument('-e', '--endpoints', help='Relative path of endpoints.json', default="endpoints.json", type=str)
parser.add_argument('-i', '--ips', help='Destination EPs for every rule in the policy', default=4, type=int)
parser.add_argument('-x', '--expansion', help='Generate expansion policies', default=True, type=bool)
args = parser.parse_args()


topology_dir = args.topology
endpoint_file = topology_dir + args.endpoints
protocols = ["udp", "tcp", "icmp"]
directories = ["udp", "tcp", "icmp", "mixed", "scale", "halcfg-expansion", "netagent-expansion"]
ports = ["10", "22", "24", "30", "50-100", "101-200", "201-250", "10000-20000", "65535"]
target_dir = "gen"
actions = ["PERMIT", "DENY", "REJECT"]
total_policies = 0

def StripIpMask(ip_address):
    ret = ""
    if '/' in ip_address:
        ret = ip_address[:-3]
    ret = ip_address
    print("Stripping IP mask {}".format(ret))
    return str(ret)

def GetIpRange(ip_address):
    stripped_ip = ip_address[:ip_address.rfind('.')]
    ret = "{}.1-{}.100".format(stripped_ip, stripped_ip)
    return str(ret)

def GetIpCidr(ip_address):
    stripped_ip = ip_address[:ip_address.rfind('.')]
    ret = "{}.0/24".format(stripped_ip)
    return str(ret)

def Main():
    with open(endpoint_file, 'r') as fp:
        print("Reading endpoint file from : {}".format(endpoint_file))
        obj = json.load(fp)
    EP = []
    FILENAME = []

    for i in range(0, len(obj["objects"])):
        if obj["objects"][i]["kind"] == "Endpoint" :
            print("EP[%d] : %s" % (i, obj["objects"][i]["spec"]["ipv4-addresses"][0]))
            EP.append(StripIpMask((obj["objects"][i]["spec"]["ipv4-addresses"][0])))
            FILENAME.append("endpoint")

    EP.append("any")
    EP.append("123.123.123.123")

    if not os.path.exists("gen"):
          os.makedirs("gen")

    for dir in directories:
        if not os.path.exists(topology_dir + "/" +target_dir + "/{}".format(dir)):
            os.makedirs(topology_dir + "/" +target_dir + "/{}".format(dir))

    total_policies = 0

    # One big policy
    for protocol in protocols:
        for action in actions:
            reset()
            policy_objects = object_template
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['spec']['policy-rules']

            verif =[]
            for i in range(0, len(EP) - 1):
                for j in range(i, len(EP)):
                    for k in ports:
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        policy_rules.append(rule)
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        policy_rules.append(rule)
                        verif.append(get_verif(protocol, k, action))
            total_policies = total_policies + 1
            policy_rules.append(default_policy)
            policy_objects["objects"].append(sgpolicy)

            json.dump(policy_objects, open(topology_dir + "/" +target_dir +"/{}/{}_{}_policy.json".format(protocol, protocol, action), "w"), indent=4)
            json.dump(verif, open(topology_dir + "/" +target_dir +"/{}/{}_{}_verif.json".format(protocol, protocol, action), "w"))

    # Generic policy
    for protocol in protocols:
        for action in actions:
            reset()
            policy_objects = object_template
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['spec']['policy-rules']
            verif =[]
            for i in range(0, len(EP) - 1):
                for k in ports:
                    rule = get_rule(EP[i], "any", protocol, k, action)
                    policy_rules.append(rule)
                    verif.append(get_verif(protocol, k, action))

            total_policies = total_policies + 1
            policy_rules.append(default_policy)
            policy_objects["objects"].append(sgpolicy)

            json.dump(policy_objects, open(topology_dir + "/" +target_dir +"/{}/{}_{}_any_policy.json".format(protocol, protocol, action), "w"), indent=4)
            json.dump(verif, open(topology_dir + "/" +target_dir + "/{}/{}_{}_any_verif.json".format(protocol, protocol, action), "w"), indent=4)

    # Specific Policy
    for protocol in protocols:
        for action in actions:
            reset()
            policy_objects = object_template
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['spec']['policy-rules']
            verif =[]
            for i in range(0, len(EP) - 4):
                for j in range(i, len(EP) - 3):
                    for k in ports:
                        rule = get_rule(EP[i], EP[j], protocol, k, action)
                        policy_rules.append(rule)
                        rule = get_rule(EP[j], EP[i], protocol, k, action)
                        policy_rules.append(rule)
                        verif.append(get_verif(protocol, k, action))
            total_policies = total_policies + 1
            policy_rules.append(default_policy)
            policy_objects["objects"].append(sgpolicy)

            json.dump(policy_objects, open(topology_dir + "/" +target_dir + "/{}/{}_{}_specific_policy.json".format(protocol, protocol, action), "w"), indent=4)
            json.dump(verif, open(topology_dir + "/" +target_dir + "/{}/{}_{}_specific_verif.json".format(protocol, protocol, action), "w"), indent=4)



    scale_list = [int(item) for item in args.scale.split(',')]
    for count in scale_list:
        reset()
        policy_objects = object_template
        sgpolicy = sgpolicy_template
        policy_rules = sgpolicy_template['spec']['policy-rules']
        verif =[]

        rule_count = 0
        i = 0
        j = 1
        k = 1
        proto_i = 0
        protocol = protocols[proto_i]

        while(rule_count < count):
            action = actions[random.randint(0, len(actions) - 1)]
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
                protocol = protocols[proto_i]

                j = j + 1
                if (j == len(EP)):
                    i = i + 1
                    j = i + 1

                if (i == len(EP) - 1):
                    print("Breaking from here I = {} J = {} RULE_CNT = {}".format(i, j, rule_count))
                    break

        print("Writing rule for scale {}".format(count))
        policy_rules.append(default_policy)
        total_policies = total_policies + 1
        policy_objects["objects"].append(sgpolicy)
        json.dump(policy_objects, open(topology_dir + "/" +target_dir + "/scale/{}_scale_policy.json".format(count), "w"), indent=4)
        json.dump(verif, open(topology_dir + "/" +target_dir + "/scale/{}_scale_verif.json".format(count), "w"), indent=4)

        #pow_2 = [4 ** x for x in range(6)]

    for count in scale_list:
        rules_per_ep = count / len(EP)
        if rules_per_ep <= 0:
            rules_per_ep = 1

        reset()
        policy_objects = object_template
        sgpolicy = sgpolicy_template
        policy_rules = sgpolicy_template['spec']['policy-rules']
        verif =[]
        rule_count = 0
        i = 0
        k = 1
        proto_i = 0
        protocol = protocols[proto_i]

        while(rule_count < count):
            rules_generated = 0
            ip_list = generate_ip_list(args.ips, EP[i], EP)
            while rules_generated < rules_per_ep:
                action = actions[random.randint(0, len(actions) - 1)]
                rule = get_rule(EP[i], ip_list, protocol, str(k), action)
                policy_rules.append(rule)
                verif.append(get_verif(protocol, str(k), action))
                k = k + 1
                if (k >= 65536):
                    k = 1
                    proto_i = proto_i + 1
                    protocol = protocols[proto_i]

                rules_generated = rules_generated + 1
                rule_count += 1

            i = i + 1
            if (i == len(EP)):
                break

        print("Writing rule for scale {}".format(count))
        policy_rules.append(default_policy)
        total_policies = total_policies + 1
        policy_objects["objects"].append(sgpolicy)
        json.dump(policy_objects, open(topology_dir + "/" + target_dir + "/scale/{}_multipleips_policy.json".format(protocol, protocol, action), "w"), indent=4)
        json.dump(verif, open(topology_dir + "/" + target_dir + "/scale/{}_multipleips_verif.json".format(protocol, protocol, action), "w"), indent=4)

    if args.expansion :
        pow_2 = [4 ** x for x in range(6)]
        ip_list = generate_ip_list(1024, "1.1.1.", EP)

        # HAL expansion policy
        for n in pow_2:
            for m in pow_2:
                reset()
                policy_objects = object_template
                sgpolicy = sgpolicy_template
                policy_rules = sgpolicy_template['spec']['policy-rules']
                verif =[]

                src_ip = ip_list[0 : n]
                dst_ip = ip_list[512 : 512 + m]

                for port in range(1, 9):
                    action = actions[random.randint(0, len(actions) - 1)]
                    rule = get_rule(src_ip, dst_ip, protocol, str(port), action)
                    policy_rules.append(rule)

                    policy_rules.append(default_policy)
                    total_policies = total_policies + 1
                    policy_objects["objects"].append(sgpolicy)
                    print("Writing rule for halcfg expansion {}_{}_{}".format(n, m, port))
                    json.dump(policy_objects, open(topology_dir + "/" + target_dir + "/halcfg-expansion/{}_{}_{}_expansion_policy.json".format(n, m, port), "w"), indent=4)

        # Netagent Expansion
        for port_len in pow_2:
            reset()
            policy_objects = object_template
            sgpolicy = sgpolicy_template
            policy_rules = sgpolicy_template['spec']['policy-rules']
            verif =[]

            src_ip = ip_list[0]
            dst_ip = ip_list[512]

            for port in range(1, port_len):
                action = actions[random.randint(0, len(actions) - 1)]
                rule = get_rule(src_ip, dst_ip, protocol, str(port), action)
                policy_rules.append(rule)

            policy_objects["objects"].append(sgpolicy)
            print("Writing rule for netagent expansion PORT Count : {}".format(port_len))
            total_policies = total_policies + 1
            json.dump(policy_objects, open(topology_dir + "/" +target_dir + "/netagent-expansion/{}_expansion_policy.json".format(port_len), "w"), indent=4)

if __name__=="__main__":
    Main()
