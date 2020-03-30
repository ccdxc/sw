#!/usr/bin/env python

import argparse
import sys
import subprocess
import md5
import time
import pprint
import re
import datetime

def calculate_cidr_len(mask):
    return sum(bin(int(x)).count('1') for x in mask.split('.'))

def save_config():
    cmd = "/usr/local/bin/pensave-config.sh"
    output = subprocess.check_output(cmd, shell=True)

def write_config_file( filename, content, flag ):
    fd = open(filename, flag)
    fd.write(content)
    fd.close()

def config_networking(properties):
    if ( properties["addrtype"] == "static" ):
        old_con_uuid = get_con_uuid(properties["ifname"])
        if old_con_uuid is None:
            return False        
        # Build the nmcli command to configure the interface
        nmcli_intf_tmpl = ['nmcli con add type ethernet con-name {conname} ifname {ifname} ip4 {ipaddress}/{masklen}']
        if "gateway" in properties.keys():
            nmcli_intf_tmpl.append('gw4 {gateway}')
        if "dnslist" in properties.keys():
            nmcli_intf_tmpl.append('ipv4.dns "{dnslist}"')
        if "domain" in properties.keys():
            nmcli_intf_tmpl.append('ipv4.dns-search "{domain}"')
        if "routes" in properties.keys():
            nmcli_intf_tmpl.append('ipv4.routes "{routes}"')
        nmcli_intf_tmpl = ' '.join(nmcli_intf_tmpl)
        # Run the nmcli command to generate the new config
        cmd = nmcli_intf_tmpl.format(**properties)
        try:
            output = subprocess.check_output(cmd, shell=True)
        except subprocess.CalledProcessError as error:
            write_log("command '%s' failed with return code %d" % (cmd, error.returncode))
            return False
        # Use the new network config
        cmd = "nmcli con down " + old_con_uuid + " && " + "nmcli con up " + properties["conname"]
        output = subprocess.check_output(cmd, shell=True)
        # Delete the old con
        cmd = "nmcli con del " + old_con_uuid 
        output = subprocess.check_output(cmd, shell=True)
        save_config()
        return True

def modify_routes(properties):
    old_con_uuid = get_con_uuid(properties["ifname"])
    if old_con_uuid is None:
        return False        
    # Parse the routes information and modify existing connection
    write_log("modifying routes on interface %s" % properties["ifname"])
    for entry in properties["routes"].split(","):
        entry = entry.strip()
        m = re.match("^(\+|\-)(.*)$", entry)
        if not m:
            write_log("cannot parse route entry '%s'" % entry)
            write_log("expected format of [+/-][Route Prefix in CIDR format] [Gateway IP]")
            continue
        cmd = """nmcli con modify %s %sipv4.routes '%s'""" % ( old_con_uuid, m.group(1), m.group(2) )
        print(cmd)
        output = subprocess.check_output(cmd, shell=True)
    # Use the new network config
    cmd = "nmcli con down " + old_con_uuid + " && " + "nmcli con up " + old_con_uuid
    output = subprocess.check_output(cmd, shell=True)
    save_config()
    return True

def config_password(properties):
    cmd = "echo '%s' | passwd --stdin root" % properties["password"]
    output = subprocess.check_output(cmd, shell=True)
    save_config()

def config_hostname(properties):
    network_tmpl = "{hostname}\n"
    write_config_file( "/etc/hostname", network_tmpl.format(**properties), "w+" )
    subprocess.call("hostname " + properties['hostname'], shell=True)
    save_config()

def get_con_name():
    m = md5.new()
    m.update(str(time.time()))
    return m.hexdigest()

def get_if_name():
    cmd = "nmcli device status | grep ethernet | awk '{print $1}'"
    output = subprocess.check_output(cmd, shell=True)
    iflist = output.strip().split("\n")
    if len(iflist) > 0:
        return iflist[0]
    else:
        return None

def get_con_uuid(ifname):
    # First we need to find the UUID of existing config
    cmd = """nmcli con | egrep 'ethernet.*%s' | awk '{print $1","$(NF-2)}'""" % ifname
    output = subprocess.check_output(cmd, shell=True)
    # Validate the value
    if not re.match("^\S+,\S+$", output.strip()):
        write_log("Linux network manager cannot determine connection uuid of interface %s. Interface may be down." % properties["ifname"])
        write_log("Please run command \"%s\" to check" % cmd)
        return None
    # con_name , con_uuid = output.strip().split(",")
    return output.strip().split(",")[1]

def get_if_mac(ifname):
    cmd = "ip link show dev " + ifname + " | grep link | awk '{print $2}'"
    output = subprocess.check_output(cmd, shell=True)
    output = output.strip()
    # If this device does not exist, then we abort here
    if re.match("Device.*does not exist", output):
        write_log("Device %s does not exist" % "ifname")
        return ""
    if not re.match("([0-9a-f]{2}:){5}[0-9a-f]{2}", output):
        write_log("Cannot determine MAC address of device %s. Please try 'ip link show dev %s'" % ( properties["ifname"], properties["ifname"] ))
        return ""
    return output

def write_log(msg):
    msg = str(datetime.datetime.now()) + ": " + msg + "\n"
    print msg

def abort():
    write_log("Failed to configure networking....Abort")
    sys.exit(1)

if __name__ == "__main__":

    pp = pprint.PrettyPrinter(indent=4)
    
    # Parse tha command line argument
    parser = argparse.ArgumentParser()
    parser.add_argument("-hostname", "-m", help="Venice hostname")
    parser.add_argument("-password", "-p", help="Venice console or SSH password")
    parser.add_argument("-ifname", "-if", help="Interface name from running 'ip link', e.g. eth0, eth1", type=str)
    parser.add_argument("-addrtype", "-a", choices=["static"], help="Addesssing type (only 'static' is allowed)", type=str)
    parser.add_argument("-ipaddr", "-i", help="ip address")
    parser.add_argument("-netmask", "-n", help="netmask")
    parser.add_argument("-gateway", "-g", help="default gateway")
    parser.add_argument("-dns", "-d", help="list of dns servers separated by comma, e.g. '8.8.8.8, 8.8.4.4'")
    parser.add_argument("-search", "-s", help="list of domain to search separated by comma, e.g. 'example.com, test.com'", dest="domain")
    parser.add_argument("-routes", "-r", help="list of [route_cidr gateway] separated by comma, e.g. '192.168.68.0/22 10.0.0.1, 192.168.64.0/22 10.0.0.2'")
    opts = parser.parse_args()
    properties = dict()
    
    if opts.hostname is not None:
        properties['hostname'] = opts.hostname
        config_hostname(properties)
    
    if opts.password is not None:
        properties['password'] = opts.password
        config_password(properties)
   
    # Determine the interface
    if opts.ifname is None:
        properties["ifname"] = get_if_name() 
    else:
        properties["ifname"] = opts.ifname
    if properties["ifname"] is None:
        write_log("ifname is not specified but cannot auto detect the interface. Please specify -ifname")
        abort()
    # Get the route information (if any)
    if opts.routes is not None:
        properties['routes'] = opts.routes

    if opts.addrtype is not None:
        properties['addrtype'] = opts.addrtype
        # Get the interface MAC
        properties["ifmac"] = get_if_mac(properties["ifname"])
        if not properties["ifmac"].strip():
            write_log("cannot determine MAC address of %s" % properties["ifname"])
            abort()
        if opts.ipaddr is None or opts.netmask is None:
            write_log("For addrtype = static, -ipaddr and -netmask are mandatory")
            abort()
        properties['ipaddress'] = opts.ipaddr
        properties['netmask'] = opts.netmask
        properties['masklen'] = calculate_cidr_len(properties['netmask'])
        properties['conname'] = get_con_name()
        if opts.gateway is not None: 
            properties["gateway"] = opts.gateway
        if opts.dns is not None:
            properties['dnslist'] = opts.dns
        if opts.domain is not None:
            properties['domain'] = opts.domain
        
        # pp.pprint(properties)
        # print(nmcli_intf_tmpl.format(**properties))
        # sys.exit(0)
        
        if not config_networking(properties):
            abort()
    else:
        # May still want to modify the routes   
        if not modify_routes(properties):
            write_log("modify routes on interface %s failed" % properties["ifname"])
            abort()

    sys.exit(0)
