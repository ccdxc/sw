#!/usr/bin/env python

import shutil
import subprocess
import os
import sys
import json
import xml.etree.ElementTree as ET
from xml.dom import minidom
from xml.dom.minidom import parseString
from pprint import pprint

def remove_rsrc_type( types, d ):
    for e in d.getElementsByTagName("rasd:ResourceType"):
        if e.childNodes[0].nodeValue in types:
            p = e.parentNode
            p.parentNode.removeChild(p)
    return d

if len(sys.argv) != 2:
    print("Syntax: " + sys.argv[0] + " [OVF FILE]")
    sys.exit(1)
filename = sys.argv[1]
fd = open(filename, "r")
data = fd.read()
fd.close()

xdata = parseString(data)

# There are several sections we need to modify
# 1. Add attribute ovf:transport to VirtualHardwareSection
#    This is to enable the OVF transport so the data is exposed in the guest 
e = xdata.getElementsByTagName("VirtualHardwareSection")
e[0].setAttribute("ovf:transport", "iso com.vmware.guestInfo")
# 2. Add a last child ProductSection in the VirtualSystem element
#    This is to add the custom vApp section
product_section = '''<ProductSection xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1">
      <Info>Information about the installed software</Info>
      <Product>Venice Appliance</Product>
      <Vendor>Pensando</Vendor>
      <VendorUrl>www.pensando.io</VendorUrl>
      <Category>Networking</Category>
      <Property ovf:key="hostname" ovf:type="string" ovf:userConfigurable="true" ovf:value="venice" ovf:required="true" ovf:qualifiers="MinLen(1),MaxLen(25)">
        <Label>Hostname</Label>
        <Description>Hostname</Description>
      </Property>
      <Property ovf:key="ipaddress" ovf:type="string" ovf:userConfigurable="true" ovf:value="">
        <Label>IP Address</Label>
        <Description>IP Address of eth0 (DHCP if left blank)</Description>
      </Property>
      <Property ovf:key="netmask" ovf:type="string" ovf:userConfigurable="true" ovf:value="">
        <Label>Netmask</Label>
        <Description>Netmask of eth0 (DHCP if IP Address is left blank)</Description>
      </Property>
      <Property ovf:key="gateway" ovf:type="string" ovf:userConfigurable="true" ovf:value="">
        <Label>Gateway</Label>
        <Description>Gateway of eth0 (DHCP if IP Address is left blank)</Description>
      </Property>
      <Property ovf:key="dns" ovf:type="string" ovf:userConfigurable="true" ovf:value="">
        <Label>DNS Server</Label>
        <Description>DNS server (Multiple servers need to be comma separated or DHCP if IP Address is left blank)</Description>
      </Property>
      <Property ovf:key="domain" ovf:type="string" ovf:userConfigurable="true" ovf:value="pensando.io" ovf:required="true" ovf:qualifiers="MinLen(3),MaxLen(255)">
        <Label>Domain Name</Label>
        <Description>Domain name, e.g. example.com</Description>
      </Property>
      <Category>Password</Category>
      <Property ovf:key="password" ovf:password="true" ovf:type="string" ovf:userConfigurable="true" ovf:value="" ovf:required="true" ovf:qualifiers="MinLen(8),MaxLen(15)">
        <Label>Console password (minimum 8 chacters with at least one upper case, one lower case, and one nunber)</Label>
        <Description>Console password</Description>
      </Property>
    </ProductSection>'''
ndata = parseString(product_section)
e = xdata.getElementsByTagName("VirtualSystem")
e[0].appendChild(ndata.documentElement)
# 3. The buildit tool assumes there are 4 interfaces, but Venice OVA requires just one. 
#    Need to get rid of the redundant one. Keep the VM Network one.
for e in xdata.getElementsByTagName("Network"):
    if e.getAttribute('ovf:name') != "VM Network":
       e.parentNode.removeChild(e)
for e in xdata.getElementsByTagName("rasd:Connection"):
    if e.childNodes[0].nodeValue != "VM Network":
        # Delete these nodes as these are interfaces we do not need or care about
        p = e.parentNode
        p.parentNode.removeChild(p)

# 4. Change the CPU and memory to match our spec
cpu_mem_section = [ '''
      <Item xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData" xmlns:vmw="http://www.vmware.com/schema/ovf" xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1">
        <rasd:AllocationUnits>hertz * 10^6</rasd:AllocationUnits>
        <rasd:Description>Number of Virtual CPUs</rasd:Description>
        <rasd:ElementName>12 virtual CPU(s)</rasd:ElementName>
        <rasd:InstanceID>1</rasd:InstanceID>
        <rasd:ResourceType>3</rasd:ResourceType>
        <rasd:VirtualQuantity>12</rasd:VirtualQuantity>
        <vmw:CoresPerSocket ovf:required="false">6</vmw:CoresPerSocket>
      </Item>
    ''',
    '''
      <Item xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData">
        <rasd:AllocationUnits>byte * 2^20</rasd:AllocationUnits>
        <rasd:Description>Memory Size</rasd:Description>
        <rasd:ElementName>32768MB of memory</rasd:ElementName>
        <rasd:InstanceID>2</rasd:InstanceID>
        <rasd:ResourceType>4</rasd:ResourceType>
        <rasd:VirtualQuantity>32768</rasd:VirtualQuantity>
      </Item>
    '''
]
# Remove the original CPU and memory resource and insert our spec
xdata = remove_rsrc_type(["3", "4"], xdata)
e = xdata.getElementsByTagName("Item")
for section in cpu_mem_section:
    ndata = parseString(section)
    e[0].parentNode.insertBefore(ndata.documentElement, e[0])

# Write the new OVF file
shutil.copy(filename, filename + ".save")
fd = open(filename, "w+")
fd.write(xdata.toxml())
fd.close()

# Re-package the OVA
dir = os.path.dirname(filename)
output_filename = os.path.join(os.path.dirname(filename), "venice.ova")
cmd = "ovftool %s %s" % ( filename, output_filename )
subprocess.check_output(cmd, shell=True)

