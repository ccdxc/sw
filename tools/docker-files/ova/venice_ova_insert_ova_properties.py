#!/usr/bin/env python

import shutil
import subprocess
import os
import sys
import json
import argparse
import xml.etree.ElementTree as ET
from xml.dom import minidom
from xml.dom.minidom import parseString
from pprint import pprint

def remove_rsrc_type( types, d ):
    for e in d.getElementsByTagName("rasd:ResourceType"):
        if e.childNodes[0].nodeValue in types:
            p = e.parentNode
            remove_element(p)
    return d

# An element may have leading and trailing whitespaces which are
# parsed as text-node. When we remove an element we will check and
# remove both leading and trailing whitespaces with it. This is 
# just a wrapper to check the previous/next sibling and if it is an
# empty text-node, remove that along with the element
def remove_element(e):
    prev_e = e.previousSibling
    next_e = e.nextSibling
    e.parentNode.removeChild(e)
    for s in [ prev_e, next_e ]:
        if s.nodeType != minidom.Node.TEXT_NODE:
            continue
        if len(s.nodeValue.strip()) == 0:
            s.parentNode.removeChild(s)

parser = argparse.ArgumentParser()
parser.add_argument("ovf_file", help="Input OVF file")
parser.add_argument("--private_key_file", "-p", help="Path to the file containing both cert and private key in .pem format")
opts = parser.parse_args()
filename = opts.ovf_file
fd = open(filename, "r")
data = fd.read()
fd.close()

xdata = parseString(data)

# There are several sections we need to modify
# 1. Add attribute ovf:transport to VirtualHardwareSection
#    This is to enable the OVF transport so the data is exposed in the guest 
e = xdata.getElementsByTagName("VirtualHardwareSection")
e[0].setAttribute("ovf:transport", "iso com.vmware.guestInfo")
# 2. Add the DeploymentOptionSection so we can specify various VM spec
deployment_section = '''
    <DeploymentOptionSection xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1">
      <Info>The list of deployment options</Info>
      <Configuration ovf:id="lab">
        <Label ovf:msgid="config.lab.label">Lab</Label>
        <Description ovf:msgid="config.lab.description">
IMPORTANT: This configuration is only supported for lab testing

This configuration requires the following:
* 4 vCPU
* 16GB RAM
* 100GB Storage
        </Description>
      </Configuration>
      <Configuration ovf:id="production">
        <Label ovf:msgid="config.production.label">Production</Label>
        <Description ovf:msgid="config.production.description">
IMPORTANT: This configuration is required for production deployment

This configuration requires the following:
* 12 vCPU
* 32GB RAM
* 100GB Storage
        </Description>
      </Configuration>
    </DeploymentOptionSection>
'''
ndata = parseString(deployment_section)
e = xdata.getElementsByTagName("References")
e[0].parentNode.insertBefore(ndata.documentElement, e[0].nextSibling)
# 3. Add a last child ProductSection in the VirtualSystem element
#    This is to add the custom vApp section
product_section = '''
    <ProductSection xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1">
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
    </ProductSection>
'''
ndata = parseString(product_section)
e = xdata.getElementsByTagName("VirtualSystem")
e[0].appendChild(ndata.documentElement)
# 4. The buildit tool assumes there are 4 interfaces, but Venice OVA requires just one. 
#    Need to get rid of the redundant one. Keep the VM Network one.
for e in xdata.getElementsByTagName("Network"):
    if e.getAttribute('ovf:name') != "VM Network":
        remove_element(e)
for e in xdata.getElementsByTagName("rasd:Connection"):
    if e.childNodes[0].nodeValue != "VM Network":
        # Delete these nodes as these are interfaces we do not need or care about
        p = e.parentNode
        remove_element(p)

# 5. Change the CPU and memory to match our spec
cpu_mem_section = [ 
    '''
    <Item xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData" xmlns:vmw="http://www.vmware.com/schema/ovf" xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1" configuration="lab">
      <rasd:AllocationUnits>hertz * 10^6</rasd:AllocationUnits>
      <rasd:Description>Number of Virtual CPUs</rasd:Description>
      <rasd:ElementName>4 virtual CPU(s)</rasd:ElementName>
      <rasd:InstanceID>1</rasd:InstanceID>
      <rasd:ResourceType>3</rasd:ResourceType>
      <rasd:VirtualQuantity>4</rasd:VirtualQuantity>
      <vmw:CoresPerSocket ovf:required="false">2</vmw:CoresPerSocket>
    </Item>
    ''',
    '''
    <Item xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData" xmlns:vmw="http://www.vmware.com/schema/ovf" xmlns:ovf="http://schemas.dmtf.org/ovf/envelope/1" configuration="production">
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
    <Item xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData" configuration="lab">
      <rasd:AllocationUnits>byte * 2^20</rasd:AllocationUnits>
      <rasd:Description>Memory Size</rasd:Description>
      <rasd:ElementName>16384 of memory</rasd:ElementName>
      <rasd:InstanceID>2</rasd:InstanceID>
      <rasd:ResourceType>4</rasd:ResourceType>
      <rasd:VirtualQuantity>16384</rasd:VirtualQuantity>
    </Item>
    ''',
    '''
    <Item xmlns:rasd="http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ResourceAllocationSettingData" configuration="production">
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
# If the key file is provided and exists then sign the OVA
if opts.private_key_file is not None and os.path.exists(opts.private_key_file):
    cmd = "ovftool --privateKey=%s %s %s" % ( opts.private_key_file, filename, output_filename )
else:
    cmd = "ovftool %s %s" % ( filename, output_filename )
subprocess.check_output(cmd, shell=True)

