#!/usr/bin/env python

import argparse
import sys
import subprocess
import md5
import time

def calculate_cidr_len(mask):
    return sum(bin(int(x)).count('1') for x in mask.split('.'))

def save_config():
    cmd = "/usr/local/bin/pensave-config.sh"
    output = subprocess.check_output(cmd, shell=True)

def write_config_file( filename, content, flag ):
    fd = open(filename, flag)
    fd.write(content)
    fd.close()

def config_networking():
    if ( properties["addrconf"] == "static" ):
        # First we need to find the UUID of existing config
        cmd = """nmcli con | egrep 'ethernet.*%s' | awk '{print $1","$(NF-2)}'""" % properties["ifname"]
        output = subprocess.check_output(cmd, shell=True)
        old_con, old_uuid = output.strip().split(",")
        # Run the nmcli command to generate the new config
        cmd = nmcli_intf_tmpl.format(**properties)
        output = subprocess.check_output(cmd, shell=True)
        # Use the new network config
        cmd = "nmcli con down " + old_uuid + " && " + "nmcli con up " + properties["conname"]
        output = subprocess.check_output(cmd, shell=True)

def get_con_name():
    m = md5.new()
    m.update(str(time.time()))
    return m.hexdigest()

network_tmpl = "{hostname}\n"

# Parse tha command line argument
parser = argparse.ArgumentParser()
parser.add_argument("-hostname", "-m", help="Venice hostname")
parser.add_argument("-password", "-p", help="Venice console or SSH password")
parser.add_argument("-addrtype", "-a", choices=["static"], help="Addesssing type (only 'static' is allowed)", type=str)
parser.add_argument("-ipaddr", "-i", help="ip address")
parser.add_argument("-netmask", "-n", help="netmask")
parser.add_argument("-gateway", "-g", help="default gateway")
parser.add_argument("-dns", "-d", help="list of dns servers (multiple servers are comma separated)")
opts = parser.parse_args()
properties = dict()

if opts.hostname is not None:
    properties['hostname'] = opts.hostname
    write_config_file( "/etc/hostname", network_tmpl.format(**properties), "w+" )
    subprocess.call("hostname " + properties['hostname'], shell=True)

if opts.password is not None:
    properties['password'] = opts.password
    cmd = "echo '%s' | passwd --stdin root" % properties["password"]
    output = subprocess.check_output(cmd, shell=True)

if opts.addrtype is not None:
    # Determine the interface
    cmd = "nmcli device status | grep ethernet | awk '{print $1}' | head -n 1"
    output = subprocess.check_output(cmd, shell=True)
    properties['ifname'] = output.strip()
    cmd = "ip link show dev " + properties["ifname"] + " | grep link | awk '{print $2}'"
    output = subprocess.check_output(cmd, shell=True)
    properties['ifmac'] = output.strip() 
    properties['addrconf'] = opts.addrtype
    if opts.ipaddr is None or opts.netmask is None or opts.gateway is None:
        print("For addrtype = static, -ipaddr, -netmask, and -gateway are mandatory")
        save_config()
        sys.exit()        
    properties['ipaddress'] = opts.ipaddr
    properties['netmask'] = opts.netmask
    properties['gateway'] = opts.gateway
    properties['masklen'] = calculate_cidr_len(properties['netmask'])
    # properties['conname'] = "static-" + properties['ifname']
    properties['conname'] = get_con_name()
    if opts.dns is None:
        nmcli_intf_tmpl = 'nmcli con add type ethernet con-name {conname} ifname {ifname} ip4 {ipaddress}/{masklen} gw4 {gateway}'
    else:
        properties['dnslist'] = opts.dns
        nmcli_intf_tmpl = 'nmcli con add type ethernet con-name {conname} ifname {ifname} ip4 {ipaddress}/{masklen} gw4 {gateway} ipv4.dns "{dnslist}"'
    config_networking() 

save_config()
sys.exit()
