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
import time
import datetime
import md5
import time
from StringIO import StringIO
from xml.dom.minidom import parseString
import pprint
from config_PSM_networking import *

state_file = "/data/var/init_ova.done"
log_file = "/data/var/log/init_ova.log"
rpctool_path = "/usr/bin/vmware-rpctool"
ovfenv_cmd = "%s 'info-get guestinfo.ovfEnv'" % rpctool_path

# nmcli_intf_tmpl = 'nmcli con add type ethernet con-name {conname} ifname {ifname} ip4 {ipaddress}/{masklen} gw4 {gateway} ipv4.dns "{dnslist}" ipv4.dns-search {domain}'

# network_tmpl = "{hostname}\n"

# hosts_tmpl = "{ipaddress} {hostname}.{domain} {hostname}\n"

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
        properties[key] = value.strip()
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
    properties["ifname"] = get_if_name()
    properties["ifmac"] = get_if_mac(properties["ifname"])
    # Need to check if the OVF properties is blank then it means DHCP is used
    # If it is DHCP, we will need to determine what the IP is so we can use to bootstrap venice
    if ( properties["ipaddress"] == "" ):
        properties["addrtype"] = "dhcp"
        # Try to parse the IP address. Since DHCP may take a while we will need to loop 
        start_time = int(time.time())
        cmd = "ip addr show dev " + properties["ifname"] + " | grep 'inet ' | awk '{print $2}'"
        while True:
            if ( ( int(time.time()) - start_time ) > 300 ):
                write_log("* DHCP is specified but unable to determine the IP address of %s. Giving up." % properties["ifname"])
                sys.exit(1)
            output = subprocess.check_output(cmd, shell=True)
            # Check that it has the IP address
            if re.search("(\d+\.){3}\d+\/\d+", output):
                output = output.strip()
                break
            write_log("* DHCP is specified but unable to determine the IP address of %s. Retrying.." % properties["ifname"])
            time.sleep(30)
        ( addr, cidr_len ) = output.split('/')
        properties["ipaddress"] = addr
        properties["masklen"] = cidr_len
        properties["netmask"] = calcDottedNetmask(int(cidr_len))
    else:
        properties["addrtype"] = "static"
        if not properties["gateway"].strip():
            del properties["gateway"]
        properties["masklen"] = calculate_cidr_len(properties["netmask"])
        properties["conname"] = get_con_name()
        if properties["dns"].strip():
            dns_list = [d.strip() for d in properties["dns"].split(',')]
            properties["dnslist"] = " ".join(dns_list)
        # if properties["routes"].strip():
        #     properties['routes'] = opts.routes
    return properties

def write_config_file( filename, content, flag ):
    fd = open(filename, flag)
    fd.write(content)
    fd.close() 

def _config_networking():
    if ( properties["addrtype"] == "static" ):
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
    # write_config_file( "/etc/hostname", network_tmpl.format(**properties), "w+" )
    # write_config_file( "/etc/hosts", hosts_tmpl.format(**properties), "a+" )
    # For some reason we still need to change the hostname
    # subprocess.call("hostname " + properties['hostname'], shell=True)

def _config_root_password():
    cmd = "echo '%s' | passwd --stdin root" % properties["password"] 
    output = subprocess.check_output(cmd, shell=True)

def write_log(msg):
    msg = str(datetime.datetime.now()) + ": " + msg + "\n"
    print msg 
    log_fd.write(str(datetime.datetime.now()) + ": " + msg)

def _get_con_name():
    m = md5.new()
    m.update(str(time.time()))
    return m.hexdigest()

# Start here
if os.path.exists(state_file):
    sys.exit(0)
if not is_vmware_env():
    sys.exit(0)

log_fd = open(log_file, "w+")
properties = get_ovf_properties()
if ( properties is None ):
    write_log("* Skip creating networking config because ova properties not found")
    open(state_file, 'a').close()
    sys.exit(1)

# pp = pprint.PrettyPrinter(indent=4)
# pp.pprint(properties)
write_log(json.dumps(properties, indent=4) + "\n")
config_password(properties)
config_hostname(properties)
config_networking(properties)

# Close all the log files and done
log_fd.close()
open(state_file, 'a').close()

sys.exit(0)
