#!/usr/bin/env python

import os
import sys
import pycurl
import json
import re
import time
import datetime
import time
from StringIO import StringIO
import argparse
import subprocess
import ast

def curl_send(**data):
    # Note that code 503 is Server Unavailable
    code = 503 
    c = pycurl.Curl()
    hdr = StringIO()
    buf = StringIO()
    c.setopt(c.URL, data["url"])
    c.setopt(c.WRITEFUNCTION, buf.write)
    c.setopt(c.HEADERFUNCTION, hdr.write)
    # Disable SSL cert validation
    if re.search('https:', data["url"]):
        c.setopt(c.SSL_VERIFYPEER, False)
        c.setopt(c.SSL_VERIFYHOST, False)
    write_log("* sending: " + data["method"] + " " + data["url"], 1)
    if ( data["method"] == "POST" ):
        c.setopt(c.POST, 1)
        c.setopt(c.POSTFIELDS, data["postdata"])
        write_log(data["postdata"], 1)
        c.setopt(c.HTTPHEADER, ["Content-Type: application/json", "Accept: application/json"]) 
    elif ( data["method"] == "PUT" ):
        c.setopt(c.UPLOAD, 1)
        putbuf = StringIO(data["postdata"])
        c.setopt(c.READFUNCTION, putbuf.read)
        write_log(data["postdata"], 1)
    elif ( data["method"] == "GET" ):
        c.setopt(c.HTTPHEADER, ["Accept: application/json"]) 
    start_time = int(time.time())
    req_error = ""
    while True:
        if ( ( int(time.time()) - start_time ) > opts.timeout ):
            break
        try: 
            c.perform()
            code = c.getinfo(pycurl.HTTP_CODE)
            req_error = ""
            break
        except Exception, e:
            req_error = str(e)
            write_log("* unable to send request to " + data["url"] +": " + str(e), 1)
        write_log("* service may not be up yet. retrying....", 1)
        time.sleep(opts.waittime)
    if req_error == "":
        # we were able to contact server
        header = hdr.getvalue()
        body = buf.getvalue()
        write_log("* receiving: " + header + body, 1)
        if code not in ( 200, 409 ):
            # server returned an unexpected error
            write_log("* error performing REST operation: " + data["method"] + " " + data["url"])
            write_log("* server returned: " + str(code) + " " + str(body))
    else:
       # we were not able to contact server, log exact error
       write_log("* error sending request to " + data["url"] +": " + req_error, 1)
    return code

def is_apigw_running():
    cmd = "docker ps | grep apigw | wc -l"
    output = subprocess.check_output(cmd, shell=True)
    if int(output) > 0:
        write_log("* venice apigw is already running")
        return True
    else:
        return False

def is_pencmd_running():
    cmd = "docker ps | grep pen-cmd | wc -l"
    start_time = int(time.time())
    while True:
        if ( ( int(time.time()) - start_time ) > opts.timeout ):
            write_log("* pen-cmd is not running. aborting")
            break
        output = subprocess.check_output(cmd, shell=True)
        if int(output) > 0:
            return True
        write_log("* waiting for pen-cmd to start")
        time.sleep(opts.waittime)
    return False

def check_reachability():
    for ip in opts.VENICE_IP:
        cmd = "ping -c 5 " + ip
        try:
            output = subprocess.check_output(cmd, shell=True) 
        except Exception, e:
            write_log("* connectivity check to %s failed" % ip)
            write_log("* error: " + str(e), 1) 
            return False
        write_log("* connectivity check to %s passed" % ip)
    return True

def pingOk(sHost):
    try:
        output = subprocess.check_output("ping -{} 1 {}".format('n' if platform.system().lower()=="windows" else 'c', sHost), shell=True)

    except Exception, e:
        return False

    return True

def create_cluster():
    ctx = { "method": "POST", "url": "http://localhost:9001/api/v1/cluster" }
    ctx["postdata"] = json.dumps({
        "kind": "Cluster",
        "api-version": "v1",
        "meta": {
            "name": opts.clustername
        },
        "spec": {
            "auto-admit-dscs": opts.autoadmit,
            "ntp-servers": opts.ntpservers,
            "quorum-nodes": opts.VENICE_IP
        }
    })
    return curl_send(**ctx)

def create_tenant():
    ctx = { "method": "POST", "url": "https://localhost/configs/cluster/v1/tenants" }
    ctx["postdata"] = json.dumps({"kind": "Tenant", "meta": {"name": "default"}})
    return curl_send(**ctx)

def create_auth_policy():
    ctx = { "method": "POST", "url": "https://localhost/configs/auth/v1/authn-policy" }
    ctx["postdata"] = json.dumps({
        "Kind": "AuthenticationPolicy", 
        "meta": {
            "Name": "AuthenticationPolicy"
        }, 
        "spec": {
            "authenticators": {
                "authenticator-order": [
                    "LOCAL"
                ], 
                "local": {
                    "enabled": True
                }, 
                "ldap": {
                    "enabled": False
                }
            }
        }, 
        "APIVersion": "v1"
    })
    return curl_send(**ctx)
    
def create_admin_user():
    ctx = { "method": "POST", "url": "https://localhost/configs/auth/v1/tenant/default/users" }
    ctx["postdata"] = json.dumps({
        "api-version": "v1", 
        "kind": "User", 
        "meta": {
            "name": "admin"
        }, 
        "spec": {
            "fullname": "Admin User", 
            "password": opts.password,
            "type": "Local", 
            "email": "admin@" + opts.domain
        }
    })
    return curl_send(**ctx)

def create_admin_role_binding():
    ctx = { "method": "PUT", "url": "https://localhost/configs/auth/v1/tenant/default/role-bindings/AdminRoleBinding" }
    ctx["postdata"] = json.dumps({
        "api-version": "v1", 
        "kind": "RoleBinding", 
        "meta": {
            "name": "AdminRoleBinding", 
            "tenant": "default"
        }, 
        "spec": {
            "role": "AdminRole", 
            "users": [
                "admin"
            ]
        }
    })
    return curl_send(**ctx)

def enable_overlay_routing():
    ctx = { "method": "POST", "url": "https://localhost/configs/cluster/v1/licenses" }
    ctx["postdata"] = json.dumps({
    "kind": "License",
    "api-version": "v1",
    "meta": {
        "name": "default-license"
    },
    "spec": {
        "features": [
            {
                "feature-key": "OverlayRouting"
            }
        ]
    }
    })
    return curl_send(**ctx)

def complete_auth_bootstrap():
    ctx = { "method": "POST", "url": "https://localhost/configs/cluster/v1/cluster/AuthBootstrapComplete" }
    ctx["postdata"] = json.dumps({})
    return curl_send(**ctx)

def bootstrap_venice():
    write_log("* creating default tenant")
    if create_tenant() not in ( 200, 409 ):
        return False

    write_log("* creating default authentication policy")
    if create_auth_policy() not in ( 200, 409 ):
        return False

    write_log("* creating default admin user")
    if create_admin_user() not in ( 200, 409 ):
        return False

    write_log("* assigning super admin role to default admin user")
    if create_admin_role_binding() not in ( 200, 409 ):
        return False

    if opts.enablerouting:
        write_log("* enabling overlay routing")
        if enable_overlay_routing() not in ( 200, 409 ):
            return False

    write_log("* complete venice bootstraping process")
    if complete_auth_bootstrap() not in ( 200, 409 ):
        return False

    return True

def write_log(msg, verbose=0):
    if ( opts.verbose < verbose ):
        return
    msg = str(datetime.datetime.now()) + ": " + msg
    print msg 

# Parse tha command line argument
parser = argparse.ArgumentParser()
parser.add_argument("VENICE_IP", nargs="+", help="List of venice IPs")
parser.add_argument("-clustername", help="Venice cluster name (Default=cluster)", default="cluster", type=str)
parser.add_argument("-password", help="Venice gui password (Default=Pensando0$)", default="Pensando0$", type=str)
parser.add_argument("-domain", help="Domain name for admin user", default="pensando.io", type=str)
parser.add_argument("-ntpservers", help="NTP servers (multiple needs to be separated by comma (,)", default="0.us.pool.ntp.org,1.us.pool.ntp.org", type=str)
parser.add_argument("-timeout", help="Total time to retry a transaction in seconds (default=300)", default=300, type=int)
parser.add_argument("-waittime", help="Total time to wait between each retry in seconds (default=30)", default=30, type=int)
parser.add_argument("-autoadmit", help="Auto admit DSC once it registers with Venice - 'True' or 'False' (default=True)", default="True", choices=["True", "False"], type=str)
parser.add_argument("-enablerouting", help="Enable overlay routing on the cluster", action="store_true")
parser.add_argument("-verbose", "-v", help="Verbose logging", action="count")
parser.add_argument("-skip_create_cluster", "-s", help="Skip cluster creation step", action="store_true")
opts = parser.parse_args()

# Reformat the data to what each function expects
opts.ntpservers = [i.strip() for i in opts.ntpservers.split(',')]
opts.autoadmit = ast.literal_eval(opts.autoadmit)
if ( opts.verbose is None ):
    opts.verbose = 0
print "\n"
write_log("* start venice bootstrapping process")
write_log("* - list of venice ips: " + str(opts.VENICE_IP))
write_log("* - list of ntp servers: " + str(opts.ntpservers))
write_log("* - using domain name: " + str(opts.domain))
write_log("* - auto-admit dsc: " + str(opts.autoadmit))
write_log("* checking for reachability")

if not check_reachability():
    print "\n"
    write_log("* aborting....")
    sys.exit()

if not is_pencmd_running():
    print "\n"
    write_log("* aborting....")
    sys.exit()

if not opts.skip_create_cluster and not is_apigw_running():
    write_log("* creating venice cluster")
    if create_cluster() not in ( 200, 409 ):
        print "\n"
        write_log("* error creating cluster")
        write_log("* please correct the error and rerun %s with switch -v" % sys.argv[0])
        sys.exit()
if bootstrap_venice():
    print "\n"
    write_log("* venice bootstrap completed successfully")
    write_log("* you may access venice at https://" + opts.VENICE_IP[0])
else:
    print "\n"
    write_log("* venice bootstrap failed")
    write_log("* please correct the error and rerun %s with switch -v" % sys.argv[0])

sys.exit()
