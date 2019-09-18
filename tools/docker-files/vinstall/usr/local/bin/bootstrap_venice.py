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

def curl_send(**data):
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
    while True:
        if ( ( int(time.time()) - start_time ) > opts.timeout ):
            break
        try: 
            c.perform()
            break
        except Exception, e:
            write_log("* error: " + str(e), 1) 
        write_log("* retrying....", 1)
        time.sleep(opts.waittime)
    header = hdr.getvalue()
    body = buf.getvalue()
    write_log("* receiving: " + header + body, 1)
    return ( header, body )

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
            "name": "testCluster"
        },
        "spec": {
            "auto-admit-dscs": opts.autoadmit,
            "ntp-servers": opts.ntpservers,
            "quorum-nodes": opts.VENICE_IP
        }
    })
    ( header, body ) = curl_send(**ctx)

def create_tenant():
    ctx = { "method": "POST", "url": "https://localhost/configs/cluster/v1/tenants" }
    ctx["postdata"] = json.dumps({"kind": "Tenant", "meta": {"name": "default"}})
    ( header, body ) = curl_send(**ctx)

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
    ( header, body ) = curl_send(**ctx)
    
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
    ( header, body ) = curl_send(**ctx)

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
    ( header, body ) = curl_send(**ctx)

def complete_auth_bootstrap():
    ctx = { "method": "POST", "url": "https://localhost/configs/cluster/v1/cluster/AuthBootstrapComplete" }
    ctx["postdata"] = json.dumps({})
    ( header, body ) = curl_send(**ctx)

def bootstrap_venice():
    write_log("* creating venice cluster")
    create_cluster()
    write_log("* creating default tenant")
    create_tenant()
    write_log("* creating default authentication policy")
    create_auth_policy()
    write_log("* creating default admin user")
    create_admin_user()
    write_log("* assigning super admin role to default admin user")
    create_admin_role_binding()
    write_log("* complete venice bootstraping process")
    complete_auth_bootstrap()
    write_log("* venice cluster created successfully")
    print "\n"
    write_log("* you may access venice at https://" + opts.VENICE_IP[0])

def write_log(msg, verbose=0):
    if ( opts.verbose < verbose ):
        return
    msg = str(datetime.datetime.now()) + ": " + msg
    print msg 

# Parse tha command line argument
parser = argparse.ArgumentParser()
parser.add_argument("VENICE_IP", nargs="+", help="List of venice IPs")
parser.add_argument("-password", help="Venice gui password (Default=Penando0$)", default="Pensando0$", type=str)
parser.add_argument("-domain", help="Domain name for admin user", default="pensando.io", type=str)
parser.add_argument("-ntpservers", help="NTP servers (multiple needs to be separated by comma (,)", default="0.us.pool.ntp.org,1.us.pool.ntp.org", type=str)
parser.add_argument("-timeout", help="Total time to retry a transaction in seconds (default=300)", default=300, type=int)
parser.add_argument("-waittime", help="Total time to wait between each retry in seconds (default=30)", default=30, type=int)
parser.add_argument("-autoadmit", help="Auto admit DSC once it registers with Venice - 'True' or 'False' (default=True)", default="True", choices=["True", "False"], type=str)
parser.add_argument("-verbose", "-v", help="Verbose logging", action="count")
opts = parser.parse_args()
# Reformat the data to what each function expects
opts.ntpservers = [i.strip() for i in opts.ntpservers.split(',')]
opts.autoadmit = bool(opts.autoadmit)
if ( opts.verbose is None ):
    opts.verbose = 0
# print opts.__dict__
print "\n"
write_log("* start venice bootstrapping process")
write_log("* - list of venice ips: " + str(opts.VENICE_IP))
write_log("* - list of ntp servers: " + str(opts.ntpservers))
write_log("* - using domain name: " + str(opts.domain))
write_log("* - auto-admit dsc: " + str(opts.autoadmit))
write_log("* checking for reachability")
if not check_reachability():
    write_log("* aborting....")
    sys.exit()
bootstrap_venice()


