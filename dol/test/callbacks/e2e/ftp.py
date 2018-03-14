#! /usr/bin/python3
import pdb
import infra.api.api as infra_api

def GetWgetArgs(testcase):
    src_ip = testcase.config.src_endpoints[0].ipaddrs[0].get()
    return " ftp://admin:root@" + src_ip + "/test/e2e/ftp/ftp_file.txt"

def GetPythonArgs(testcase):
    src_ip = testcase.config.src_endpoints[0].ipaddrs[0].get()
    return "-m pyftpdlib -i " + src_ip + " -p 21 -u admin -P root"
    
