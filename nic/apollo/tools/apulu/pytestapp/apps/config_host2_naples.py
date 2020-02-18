# Import all the config objects

import api
import vpc
import device
import batch
import tunnel
import route
import vnic
import mapping
import subnet
import policy
import mirror
import interface
import nh

import netaddr
import argparse
import os
import time
import sys
import pdb

# Import types and ipaddress

import types_pb2
import interface_pb2
import vpc_pb2
import tunnel_pb2
import ipaddress

# Parse argument
parser = argparse.ArgumentParser()
parser.add_argument("naples_ip", help="naples ip address")
parser.add_argument("--grpc_port", help="naples grpc port (default=9999)", default=9999, type=str)
args = parser.parse_args()
naplesip = args.naples_ip
naplesport = args.grpc_port
os.environ['AGENT_GRPC_IP'] = naplesip
os.environ['AGENT_GRPC_PORT'] = naplesport

#Variables

# node uuid
# this is the mac address of the lowest numbered naples device on the server
# (example enp20s0)
node_uuid="0xaecd012228"

# Device object inputs
local_tep_ip='1.0.0.3'
gateway_ip='1.0.0.1'
local_tep_mac='00:00:bb:bb:bb:b1'

# VPC object inputs
vpc1_id=2
vpc1_vxlan_encap=2002
vpc2_id=100
vpc2_vxlan_encap=1002

# L3-interface (underlay) objects
intf1_prefix='1.1.0.103/24'
intf2_prefix='1.2.0.103/24'
intf1_mac='00:ae:cd:00:14:ce'
intf2_mac='00:ae:cd:00:14:cd'
intf1_underlay_mac='00:ae:cd:00:4f:de'
intf2_underlay_mac='00:ae:cd:00:4f:dd'

# Tunnel object inputs (Underlay)
tunnel_local_ip='1.0.0.3'
tunnel_remote_ip='1.0.0.2'
tunnel_mac='00:00:bb:bb:bb:b1'
tunnel_vnid=0
tunnel_nhid=2

# Subnet object inputs
ipv4_subnet1='2.1.0.0/24'
ipv6_subnet1='2001::2:1:0:0/112'
# The host_if_idx is an encoding for PF1
host_if_idx='0x8000004f'
fabric_encap=202
v4_router_ip='2.1.0.0'
v6_router_ip='2001::2:1:0:2'
virt_router_mac='00:55:01:00:00:01'

# VNIC and mapping (local and remote) table objects 
local_mapping_id=2
remote_mapping_id=1
local_vnic_mac='00:ae:cd:00:00:0a'
remote_vnic_mac='00:ae:cd:00:08:11'
local_host_ip='2.1.0.3'
remote_host_ip='2.1.0.2'

# Initialize objects.

api.Init()

batch1=batch.BatchObject()

# Create device object
# local_tep_ip, gatewayip, local_tep_mac
device1=device.DeviceObject(ipaddress.IPv4Address(local_tep_ip),ipaddress.IPv4Address(gateway_ip),local_tep_mac)

# Create VPC object 
# id, v4prefix, type = vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue
vpc2=vpc.VpcObject(vpc1_id, type=vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vpc1_vxlan_encap )

# Create VPC for Infra
vpc100=vpc.VpcObject(vpc2_id, type=vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vpc2_vxlan_encap )

# Create L3 Interfaces ..
# id, iftype, ifadminstatus, vpcid, prefix, portid, encap, macaddr
intf1=interface.InterfaceObject( 1, interface_pb2.IF_TYPE_L3, interface_pb2.IF_STATUS_UP, vpc2_id, intf1_prefix, 1, types_pb2.ENCAP_TYPE_NONE,intf1_mac, node_uuid=node_uuid )
intf2=interface.InterfaceObject( 2, interface_pb2.IF_TYPE_L3, interface_pb2.IF_STATUS_UP, vpc2_id, intf2_prefix, 2, types_pb2.ENCAP_TYPE_NONE,intf2_mac, node_uuid=node_uuid )


# Create NH objects ..
#self, id, type, l3intfid, underlaymac, vpcid=None, nhip=None, vlanid=None, macaddr=None
nh1 = nh.NexthopObject( 1, 'underlay', 1, intf1_underlay_mac, vpc2_id )
nh2 = nh.NexthopObject( 2, 'underlay', 2, intf2_underlay_mac, vpc2_id )


# Create Tunnel Objects ..
# id, vpcid, localip, remoteip, macaddr, encaptype, vnid, nhid
tunnel1 = tunnel.TunnelObject( 1,vpc1_id, tunnel_local_ip, tunnel_remote_ip,tunnel_mac, tunnel_pb2.TUNNEL_TYPE_NONE, types_pb2.ENCAP_TYPE_VXLAN, tunnel_vnid,tunnel_nhid) 


# Create Subnets
# id, vpcid, v4prefix, v6prefix, hostifindex, v4virtualrouterip, v6virtualrouterip, virtualroutermac, v4routetableid, v6routetableid, ingv4securitypolicyid, egrv4securitypolicyid, ingv6securitypolicyid, egrv6securitypolicyid, fabricencap='VXLAN', fabricencapid=1
subnet1 = subnet.SubnetObject( 1, vpc1_id, ipaddress.IPv4Network(ipv4_subnet1), ipaddress.IPv6Network(ipv6_subnet1),host_if_idx, ipaddress.IPv4Address(v4_router_ip), ipaddress.IPv6Address(v6_router_ip), virt_router_mac, 0, 0, 0, 0, 0, 0, 'VXLAN', fabric_encap, node_uuid=node_uuid )


# Create VNIC object
# id, subnetid, vpcid, macaddr, hostifindex, sourceguard=False, fabricencap='NONE', fabricencapid=1, rxmirrorid = [], txmirrorid = []
vnic1 = vnic.VnicObject(1, 1, local_vnic_mac, host_if_idx, False, 'VXLAN', fabric_encap, node_uuid=node_uuid )


# Create Mapping Objects 1 for local vnic and another for remote IP
# self, key_type, macaddr, ip, vpcid, subnetid, tunnelid, encaptype, encapslotid, nexthopgroupid, publicip, providerip, vnicid = 0

map1_l = mapping.MappingObject( local_mapping_id, 'l3', local_vnic_mac, ipaddress.IPv4Address(local_host_ip), vpc1_id, subnetid=1, vnicid=1 )

map1_r = mapping.MappingObject( remote_mapping_id, 'l3', remote_vnic_mac, ipaddress.IPv4Address(remote_host_ip), vpc1_id, subnetid=1, tunnelid=1 )

# Push the configuration
api.client.Start(api.ObjectTypes.BATCH, batch1.GetGrpcMessage())

# Create configs
api.client.Create(api.ObjectTypes.SWITCH, [device1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.VPC, [vpc2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VPC, [vpc100.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.INTERFACE, [intf1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.INTERFACE, [intf2.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.NH, [nh1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.NH, [nh2.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.TUNNEL, [tunnel1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.SUBNET, [subnet1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.VNIC, [vnic1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map1_l.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map1_r.GetGrpcCreateMessage()])

sys.exit(1)
