#! /usr/bin/python3
import pdb
import infra.api.api as infra_api

def GetDstPythonArgs(testcase):
    src_ip = testcase.config.src_endpoints[0].ipaddrs[0].get()
    return "/bin/tftpy_client.py -H " + src_ip + " -D /sw/dol/test/e2e/tftp/tftp_file.txt"

def GetSrcPythonArgs(testcase):
    src_ip = testcase.config.src_endpoints[0].ipaddrs[0].get()
    return "/bin/tftpy_server.py -i " + src_ip + " -r /sw"
    
