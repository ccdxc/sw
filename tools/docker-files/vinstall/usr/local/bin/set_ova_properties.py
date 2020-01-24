#!/usr/bin/env python
"""
    This script retrieves information from guestInfo.ovfEnv and
    print the OVF properties.
"""

import subprocess
import os
import sys
import pycurl
import json
import re
import datetime
import md5
import time
from StringIO import StringIO
from xml.dom.minidom import parseString
from pprint import pprint

state_file = "/data/var/init_ova.done"
log_file = "/data/var/log/init_ova.log"
rpctool_path = "/usr/bin/vmware-rpctool"
ovfenv_cmd = "%s 'info-get guestinfo.ovfEnv'" % rpctool_path

nmcli_intf_tmpl = 'nmcli con add type ethernet con-name {conname} ifname {ifname} ip4 {ipaddress}/{masklen} gw4 {gateway} ipv4.dns "{dnslist}" ipv4.dns-search {domain}'

dns_tmpl = ""

network_tmpl = "{hostname}\n"

hosts_tmpl = "{ipaddress} {hostname}.{domain} {hostname}\n"

def calculate_cidr_len(mask):
    return sum(bin(int(x)).count('1') for x in mask.split('.'))

def calcDottedNetmask(masklen):
    bits = 0
    for i in xrange(32-masklen,32):
        bits |= (1 << i)
    return "%d.%d.%d.%d" % ((bits & 0xff000000) >> 24, (bits & 0xff0000) >> 16, (bits & 0xff00) >> 8 , (bits & 0xff))

def is_vmware_env():
    if not os.path.exists(rpctool_path):
        return False
    cmd = "lspci | grep -i vmware | wc -l"
    output = subprocess.check_output(cmd, shell=True)
    if ( int(output.strip()) == 0 ):
        return False
    return True

def get_ovf_properties():
    global dns_tmpl
    # Return a dict of OVF properties in the ovfenv
    properties = {}
    try:
        xml_parts = subprocess.check_output(ovfenv_cmd, shell=True)
    except subprocess.CalledProcessError as e:
        write_log("* error getting ova properties: " + e.output)
        return None
    # Quit if we do not have the OVF properties
    if ( xml_parts.strip() == "No value found" ):
        write_log("* ova properties is empty")
        return None
    raw_data = parseString(xml_parts)
    # Get all the vApp properties
    for property in raw_data.getElementsByTagName('Property'):
        key, value = [ property.attributes['oe:key'].value,
                       property.attributes['oe:value'].value ]
        properties[key] = value
    # Before we start, we need to make sure network manager is running
    for i in range(10):
      cmd = "systemctl is-active NetworkManager"
      try:
        output = subprocess.check_output(cmd, shell=True)
        if ( output.strip() == "active" ):
          break
      except subprocess.CalledProcessError as e:
        write_log("* network manager check return code: " + str(e.returncode))
        write_log("* network manager check error: " + e.output)
      time.sleep(30)
    # Get the interface name and MAC
    cmd = "nmcli device status | grep ethernet | awk '{print $1}' | head -n 1"
    output = subprocess.check_output(cmd, shell=True)
    properties["ifname"] = output.strip()
    cmd = "ip link show dev " + properties["ifname"] + " | grep link | awk '{print $2}'"
    output = subprocess.check_output(cmd, shell=True)
    properties["ifmac"] = output.strip()
    # Need to check if the OVF properties is blank then it means DHCP is used
    # If it is DHCP, we will need to determine what the IP is so we can use to bootstrap venice
    if ( properties["ipaddress"] == "" ):
        properties["addrconf"] = "dhcp"
        cmd = "ip addr show dev " + properties["ifname"] + " | grep 'inet ' | awk '{print $2}'"
        output = subprocess.check_output(cmd, shell=True)
        ( addr, cidr_len ) = output.split('/')
        properties["ipaddress"] = addr
        properties["masklen"] = cidr_len
        properties["netmask"] = calcDottedNetmask(int(cidr_len))
    else:
        properties["addrconf"] = "static"
        properties["masklen"] = calculate_cidr_len(properties["netmask"])
        # properties["conname"] = "static-" + properties["ifname"]
        properties["conname"] = get_con_name()
        # Generate the DNS config
        dns_id = 1
        dns_list = [d.strip() for d in properties["dns"].split(',')]
        properties["dnslist"] = " ".join(dns_list)
        for dns in dns_list:
            dns_tmpl += "DNS%i=%s\n" % ( dns_id, dns )
            dns_id += 1
    return properties

def write_config_file( filename, content, flag ):
    fd = open(filename, flag)
    fd.write(content)
    fd.close() 

def config_networking():
    if ( properties["addrconf"] == "static" ):
        write_log("* Creating networking config..")
        # First we need to find the UUID of existing config
        cmd = "nmcli con | grep %s | awk '{print $(NF-2)}'" % properties["ifname"]
        output = subprocess.check_output(cmd, shell=True)
        old_uuid = output.strip()
        # Run the nmcli command to generate the new config
        cmd = nmcli_intf_tmpl.format(**properties)
        output = subprocess.check_output(cmd, shell=True)
        # Use the new network config
        cmd = "nmcli con down " + old_uuid + " && " + "nmcli con up " + properties["conname"]
        output = subprocess.check_output(cmd, shell=True)
    else:
        write_log("* Skip creating networking config since DHCP is used. This is not recommended..")
    # Create the hostname file
    write_config_file( "/etc/hostname", network_tmpl.format(**properties), "w+" )
    write_config_file( "/etc/hosts", hosts_tmpl.format(**properties), "a+" )
    # For some reason we still need to change the hostname
    subprocess.call("hostname " + properties['hostname'], shell=True)

def config_root_password():
    cmd = "echo '%s' | passwd --stdin root" % properties["password"] 
    output = subprocess.check_output(cmd, shell=True)

def write_log(msg):
    msg = str(datetime.datetime.now()) + ": " + msg + "\n"
    print msg 
    log_fd.write(str(datetime.datetime.now()) + ": " + msg)

def get_con_name():
    m = md5.new()
    m.update(str(time.time()))
    return m.hexdigest()

# Start here
if os.path.exists(state_file):
    sys.exit()
if not is_vmware_env():
    sys.exit()

log_fd = open(log_file, "w+")
properties = get_ovf_properties()
if ( properties is None ):
    write_log("* Skip creating networking config because ova properties not found")
    open(state_file, 'a').close()
    sys.exit()
pprint(properties, indent=4, width=80)
write_log(json.dumps(properties, indent=4) + "\n")
config_networking()
config_root_password()

# Execute the pensave command to make the change persistent
cmd = "/usr/local/bin/pensave-config.sh" 
output = subprocess.check_output(cmd, shell=True)

# Close all the log files and done
log_fd.close()
open(state_file, 'a').close()


sys.exit(0)
