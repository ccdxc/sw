import random

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")
    
def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)

def random_appid():
    choices = ["DNS", "MYSQL", "HTTP", "HTTPS", "POSTGRES", "MONGO", "CASSANDRA", "ANY"]
    return random.choice(choices)

def SecGroupPolicyPreCreateCb(data, req_spec, resp_spec):
    req_spec.request[0].policy_rules.in_fw_rules[0].apps[0] = random_appid()
    req_spec.request[0].policy_rules.eg_fw_rules[0].apps[0] = random_appid()


def SecGroupPolicyPreUpdateCb(data, req_spec, resp_spec):
    SecGroupPolicyPreCreateCb(data, req_spec, resp_spec)
