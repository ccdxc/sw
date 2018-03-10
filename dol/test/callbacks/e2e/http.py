#! /usr/bin/python3
import pdb
import infra.api.api as infra_api

def GetWgetArgs(testcase):
    src_ip = testcase.config.src_endpoints[0].ipaddrs[0].get()
    return src_ip + ":8000"
    
