#! /usr/bin/python3
import argparse
import pexpect
import sys
import os
import time
import socket
import pdb
import requests

parser = argparse.ArgumentParser(description='CIMC Reset Script')
# Mandatory parameters
parser.add_argument('--host-ip', dest='host_ip', required = True,
                    default=None, help='Host IP Address.')
parser.add_argument('--cimc-ip', dest='cimc_ip', required = True,
                    default=None, help='CIMC IP Address.')
# Optional parameters
parser.add_argument('--host-username', dest='host_username',
                    default="root", help='Host Username')
parser.add_argument('--host-password', dest='host_password',
                    default="docker", help='Host Password.')
parser.add_argument('--cimc-username', dest='cimc_username',
                    default="admin", help='CIMC Username')
parser.add_argument('--cimc-password', dest='cimc_password',
                    default="N0isystem$", help='CIMC Password.')
parser.add_argument('--debug', dest='debug',
                    action='store_true', help='Enable Debug Mode')

GlobalOptions = parser.parse_args()

def CimcReset():
    session = requests.Session()
    session.auth = (GlobalOptions.cimc_username, GlobalOptions.cimc_password)
    resp = session.get("https://%s/redfish/v1/Systems" % GlobalOptions.cimc_ip, verify=False)
    obj = resp.json()
    print("Login Response =", obj)
    sysurl = "https://%s%s/Actions/System.Reset" % (GlobalOptions.cimc_ip, obj['Members'][0]['@odata.id'])
    print("SysURL = ", sysurl)
    session.post(sysurl, '{"ResetType":"ForceOff"}', verify=False)
    time.sleep(10)
    session.post(sysurl, '{"ResetType":"On"}', verify=False)
    return

def WaitForSsh():
    print("Waiting for host to be up.")
    for retry in range(60):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ret = sock.connect_ex(('%s' % GlobalOptions.host_ip, 22))
        sock.settimeout(1)
        if ret == 0: 
            return
        else:
            time.sleep(5)

    print("Host not up. Ret:%d" % ret)
    sys.exit(1)
    return

def Main():
    for i in range(50):
        CimcReset()
        WaitForSsh()
    return

if __name__ == '__main__':
    Main()
