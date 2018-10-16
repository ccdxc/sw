#! /usr/bin/python3
import os
import pdb
import requests
import json
import grpc

import iota.harness.api as api

AGENT_URLS=[]

def Init(agent_ips):
    global AGENT_URLS
    for agent_ip in agent_ips:
        AGENT_URLS.append('http://%s:9007/' % agent_ip)
    return

def __rest_api_handler(url, obj):
    # check if the object has an overridden to_JSON method
    if hasattr(obj, "to_JSON"):
        json_data = obj.to_JSON()
    else:
        json_data = json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o)))
    api.Logger.info("URL = ", url)
    api.Logger.info("JSON Data = ", json_data)
    headers = {'Content-type': 'application/json'}
    response = requests.post(url, data=json_data, headers=headers)
    api.Logger.info("REST response = ", response.text)
    assert(response.status_code == requests.codes.ok)
    return

def __config(objlist, rest_api_path):
    agent_objlist = []
    for agent_url in AGENT_URLS:
        url = agent_url + rest_api_path
        for obj in objlist:
            __rest_api_handler(url, obj)
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
