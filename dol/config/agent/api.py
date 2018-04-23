#! /usr/bin/python3

import os
import pdb
import requests
import json
import grpc

from infra.common.glopts import GlobalOptions
from infra.common.logging import logger as logger

#import netproto.tenant_pb2  as tenant_pb2

AGENT_URL='http://localhost:9007/'

def __rest_api_handler(url, obj):
    json_data = json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o)))
    logger.info("JSON Data = ", json_data)
    headers = {'Content-type': 'application/json'}
    response = requests.post(url, data=json_data, headers=headers)
    logger.info("REST response = ", response.text)
    assert(response.status_code == requests.codes.ok)
    return

def __config(objlist, url):
    agent_objlist = []
    for obj in objlist:
        agent_obj = obj.PrepareAgentObject()
        agent_objlist.append(agent_obj)
        __rest_api_handler(url, agent_obj)
    return

def IsConfigAllowed(objs):
    if GlobalOptions.dryrun: return False
    return True

def ConfigureTenants(objlist):
    if not IsConfigAllowed(objlist): return
    url = AGENT_URL + 'api/tenants/'
    __config(objlist, url)
    return

def ConfigureSecurityProfiles(objlist):
    logger.info(" - Skipping - AGENT TBD.")
    return

def ConfigureSecurityGroups(objlist):
    if not IsConfigAllowed(objlist): return
    url = AGENT_URL + 'api/sgs/'
    #__config(objlist, url)
    return

def ConfigureSegments(objlist):
    if not IsConfigAllowed(objlist): return
    url = AGENT_URL + 'api/networks/'
    __config(objlist, url)
    return

def ConfigureEndpoints(objlist):
    if not IsConfigAllowed(objlist): return
    url = AGENT_URL + 'api/endpoints/'
    __config(objlist, url)
    return

