#! /usr/bin/python3
import os
import pdb
import requests
import json
import grpc

import iota.harness.api as api

AGENT_URL=None

def Init(agent_ip):
    global AGENT_URL
    AGENT_URL = 'http://%s:9007/' % agent_ip
    return

def __rest_api_handler(url, obj):
    # check if the object has an overridden to_JSON method
    if hasattr(obj, "to_JSON"):
        json_data = obj.to_JSON()
    else:
        json_data = json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o)))
    api.Logger.info("JSON Data = ", json_data)
    headers = {'Content-type': 'application/json'}
    response = requests.post(url, data=json_data, headers=headers)
    api.Logger.info("REST response = ", response.text)
    assert(response.status_code == requests.codes.ok)
    return

def __config(objlist, url):
    agent_objlist = []
    for obj in objlist:
        __rest_api_handler(url, obj)
    return

def ConfigureTenants(objlist):
    url = AGENT_URL + 'api/tenants/'
    __config(objlist, url)
    return

def ConfigureSecurityProfiles(objlist):
    api.Logger.info(" - Skipping - AGENT TBD.")
    return

def ConfigureSecurityGroupPolicies(objlist):
    url = AGENT_URL + 'api/security/policies/'
    __config(objlist, url)
    return

def ConfigureSecurityGroups(objlist):
    url = AGENT_URL + 'api/sgs/'
    #__config(objlist, url)
    return

def ConfigureNetworks(objlist):
    url = AGENT_URL + 'api/networks/'
    __config(objlist, url)
    return

def ConfigureEndpoints(objlist):
    url = AGENT_URL + 'api/endpoints/'
    __config(objlist, url)
    return
