#! /usr/bin/python3
import os
import pdb
import requests
import json
import grpc

import iota.harness.api as api

AGENT_URLS=[]
AGENT_IPS=[]
gl_hw = False

def Init(agent_ips, hw = False):
    global AGENT_URLS
    for agent_ip in agent_ips:
        AGENT_URLS.append('http://%s:9007/' % agent_ip)

    global AGENT_IPS
    AGENT_IPS = agent_ips

    global gl_hw
    gl_hw = hw
    return

def __rest_api_handler(rest_api_path, obj):
    for agent_url in AGENT_URLS:
        url = agent_url + rest_api_path
        json_data = json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o)))
        
        api.Logger.info("URL = ", url)
        api.Logger.info("JSON Data = ", json_data)
        headers = {'Content-type': 'application/json'}
        response = requests.post(url, data=json_data, headers=headers)
        api.Logger.info("REST response = ", response.text)
        assert(response.status_code == requests.codes.ok)
    return

def __hw_rest_api_handler(rest_api_path, obj):
    with open('temp_config.json', 'w') as outfile:
        outfile.write(json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o))))
    outfile.close()
    for agent_ip in AGENT_IPS:
        os.system("sshpass -p vm scp temp_config.json vm@%s:~" % agent_ip)
        ret = os.system("sshpass -p %s ssh %s@%s curl -X POST -d @temp_config.json -H \"Content-Type:application/json\" http://1.0.0.2:9007/%s" %\
                        (api.GetTestbedUsername(), api.GetTestbedPassword(), agent_ip, rest_api_path))
        assert(ret == 0)
    os.system("rm -f temp_config.json")
    return

def __config(objlist, rest_api_path):
    for obj in objlist:
        if gl_hw:
            __hw_rest_api_handler(rest_api_path, obj)
        else:
            __rest_api_handler(rest_api_path, obj)
    return

def ConfigureTenants(objlist):
    __config(objlist, 'api/tenants/')
    return

def ConfigureSecurityProfiles(objlist):
    api.Logger.info(" - Skipping - AGENT TBD.")
    return

def ConfigureSecurityGroupPolicies(objlist):
    __config(objlist, 'api/security/policies/')
    return

def ConfigureSecurityGroups(objlist):
    #__config(objlist, 'api/sgs/')
    return

def ConfigureNetworks(objlist):
    __config(objlist, 'api/networks/')
    return

def ConfigureEndpoints(objlist):
    __config(objlist, 'api/endpoints/')
    return
