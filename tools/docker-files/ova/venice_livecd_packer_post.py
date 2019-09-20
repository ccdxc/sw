import shutil
import subprocess
import os
import sys
import json
from xml.dom import minidom
from xml.dom.minidom import parseString
from pprint import pprint

filename = "output-venice-livecd/venice-livecd/venice-livecd.ovf"
fd = open(filename, "r")
data = fd.read()
fd.close()
# print data

xdata = parseString(data)
# There are two sections we need to modify
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

# Write the XML file
shutil.copy(filename, filename + ".save")
fd = open(filename, "w+")
fd.write(xdata.toxml())
fd.close()

# Remove the .mf file
os.remove("output-venice-livecd/venice-livecd/venice-livecd.mf")

# Re-package the ova
cmd = "ovftool output-venice-livecd/venice-livecd/venice-livecd.ovf output-venice-livecd/venice-livecd/venice-livecd.ova"
subprocess.check_output(cmd, shell=True)
cmd = "install -m 444 output-venice-livecd/venice-livecd/venice-livecd.ova /var/www/html/venice-livecd.ova"
subprocess.check_output(cmd, shell=True)

