#! /usr/bin/python3
import pdb
import json
import requests


import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions


PORT_NUM=8888

def __get(ip, path):
    url = f"http://{ip}:{PORT_NUM}{path}"
    api.Logger.info(f"Fetching from url: {url}")
    if GlobalOptions.dryrun:
        return []
    r=requests.get(url)
    return r.json()

def __del(ip, path, obj):
    delurl = "http://%s:%d%s%s/%s/%s"%(ip,PORT_NUM, path,
            obj['meta']['namespace'], obj['meta']['tenant'],
            obj['meta']['name'])
    api.Logger.info(f"Deleting from url {delurl}")
    if GlobalOptions.dryrun:
        return
    requests.delete(delurl)

def __cleanup_netagent():
    nodes = api.GetNaplesNodes()
    for node in nodes:
        addr = api.GetNicMgmtIP(node.Name())
        j = __get(addr, "/api/networks/");
        for obj in j: __del(addr, "/api/networks/", obj)
        j = __get(addr, "/api/security/policies/");
        for obj in j: __del(addr, "/api/security/policies/", obj)
        j = __get(addr, "/api/apps/");
        for obj in j: __del(addr, "/api/apps/", obj)
        j = __get(addr, "/api/vrfs/");
        for obj in j:
            if obj['spec']['vrf-type'] != 'CUSTOMER':
                continue
            __del(addr, "/api/vrfs/", obj)
        j = __get(addr, "/api/route-tables/");
        for obj in j: __del(addr, "/api/route-tables/", obj)
    return api.types.status.SUCCESS

def Main(step):
    api.Logger.info("Cleaning VRFs and Subnets in netagent")
    if not GlobalOptions.netagent:
        api.Logger.info("Nothing to be done")
        return api.types.status.SUCCESS
    return __cleanup_netagent()

if __name__ == '__main__':
    Main(None)
