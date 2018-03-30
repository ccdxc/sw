#!/usr/bin/python3

#This moddule purely written to help agent to fill up rest of the configuration for E2E Testing
import os
import sys
import grpc
import time
import pdb
import json

paths = [
    '/../mbt/',
    '/../dol/', #Using some helpers in DOL
    '/cli/', #Tenjin wrapper
    '/../dol/third_party/', # Dol helper deps.
    '/gen/' #For generated code. 
]

nic_dir = os.getcwd()
ws_top = os.path.dirname(nic_dir)
ws_top = os.path.abspath(ws_top)
agent_helper_dir = os.path.abspath(os.path.dirname(sys.argv[0]))
print (agent_helper_dir)
e2e_dir = nic_dir + "/e2etests/"
mbt_dir = nic_dir + "/../mbt/"

for path in paths:
    fullpath = nic_dir + path
    sys.path.insert(0, fullpath)

import infra.common.parser  as parser
import infra.common.utils  as utils
import infra.common.objects as objects

import grpc_proxy
template_file = mbt_dir + '/hal_proto_gen_template.py'
outfile = nic_dir + "/gen/hal_proxy_gen.py"
grpc_proxy.genProxyServerMethods('proxy_handler', template_file, outfile, ws_top)
import hal_proxy_gen
import agent_config_mgr

def get_hal_channel():
    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    print("Creating GRPC channel to HAL on port %s" %(port))
    server = 'localhost:' + port
    hal_channel = grpc.insecure_channel(server)
    print("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(hal_channel).result()
    print("Connected to HAL!")
    return hal_channel

hal_channel = get_hal_channel()
hal_proxy_gen.set_grpc_forward_channel(hal_channel)

os.environ['WS_TOP'] = ws_top
config_specs = parser.ParseDirectory("../nic/e2etests/agent_helper/cfg/specs", "*.spec")
print (len(config_specs))
for config_spec in config_specs:
    print("Adding config spec for service : " , config_spec.Service)
    for sub_service in config_spec.objects:
        print("Adding config spec for service : ", config_spec.Service, sub_service.object.name)
        agent_config_mgr.ConfigObjectHelper(config_spec, hal_channel, sub_service.object)
    

#Add all the configuration to agent configuration manager.
agent_cfg = json.load(open(agent_helper_dir + "/cfg/sample_agent.cfg"))
for cfg in agent_cfg:
    agent_config_mgr.AddAgentConfig(cfg)

grpc_proxy.serve(hal_proxy_gen.proxyServer)
