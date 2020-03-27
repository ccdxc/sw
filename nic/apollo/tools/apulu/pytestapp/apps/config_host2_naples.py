# Import all the config objects

import api
import vpc
import device
import batch
import tunnel
import route
import vnic
import mapping
import dhcp
import subnet
import policy
import mirror
import interface
import nh
import node

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
parser.add_argument("--grpc_port", help="naples grpc port (default=50054)", default=50054, type=str)
args = parser.parse_args()
naplesip = args.naples_ip
naplesport = args.grpc_port
os.environ['AGENT_GRPC_IP'] = naplesip
os.environ['AGENT_GRPC_PORT'] = naplesport

#Variables

# node uuid
node_uuid=node.NodeObject().GetNodeMac()

# Device object inputs
local_tep_ip='1.0.0.3'
gateway_ip='1.0.0.1'
local_tep_mac='00:00:bb:bb:bb:b1'

# VPC object inputs
vpc1_id=1
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
tunnel_id=1
tunnel_local_ip='1.0.0.3'
tunnel_remote_ip='1.0.0.2'
tunnel_mac='00:00:bb:bb:bb:b1'
tunnel_vnid=0
tunnel_nhid=2

# VCN objects
vcn_vnic_id = 100
vcn_subnet_id = 100
vcn_subnet_pfx='11.0.0.0/8'
vcn_host_if_idx='0x80000046'
vcn_intf_prefix='11.1.2.3/8'
vcn_intf_ip=ipaddress.IPv4Address('11.1.2.3')
vcn_v4_router_ip=ipaddress.IPv4Address('11.1.2.1')
vcn_vpc_encap=11
vcn_subnet_encap=12
vcn_vpc_id=11
vcn_virt_router_mac='00:66:01:00:00:01'
vcn_vnic_mac='fe:ff:0b:01:02:03'
vcn_route_prefix1='11.1.1.0/24'
vcn_route_table_id=100

# Subnet 1 object inputs
ipv4_subnet1='2.1.0.0/24'
# The host_if_idx is an encoding for PF1
subnet1_host_if_idx='0x80000047'
subnet1_fabric_encap=202
subnet1_v4_router_ip='2.1.0.1'
subnet1_virt_router_mac='00:55:01:00:00:01'
subnet1_gw_ip_addr=ipaddress.IPv4Address('2.1.0.1')

# VNIC and mapping (local and remote) table objects 
subnet1_local_vnic_mac='00:ae:cd:00:00:0a'
subnet1_remote_vnic_mac='00:ae:cd:00:08:11'
subnet1_local_host_ip=ipaddress.IPv4Address('2.1.0.3')
subnet1_remote_host_ip=ipaddress.IPv4Address('2.1.0.2')

# subnet 2 object inputs
ipv4_subnet2='3.1.0.0/24'
# the host_if_idx is an encoding for pf1
subnet2_fabric_encap=203
subnet2_v4_router_ip='3.1.0.1'
subnet2_virt_router_mac='00:55:02:00:00:01'

subnet2_remote_vnic_mac='00:ae:cd:00:08:12'
subnet2_remote_host_ip=ipaddress.IPv4Address('3.1.0.2')

# subnet 3 object inputs
ipv4_subnet3='64.0.0.0/24'
# the host_if_idx is an encoding for pf1
subnet3_host_if_idx='0x80000049'
subnet3_fabric_encap=204
subnet3_v4_router_ip='64.0.0.1'
subnet3_virt_router_mac='00:55:03:00:00:01'
subnet3_id = 3

subnet3_vnic_mac='00:ae:cd:00:00:0c'
subnet3_vnic_ip=ipaddress.IPv4Address('64.0.0.2')
subnet3_route_prefix1='50.0.0.0/24'
subnet3_route_table_id=3

# Initialize objects.

api.Init()

batch1=batch.BatchObject()

# Create device object
# local_tep_ip, gatewayip, local_tep_mac
device1=device.DeviceObject(ipaddress.IPv4Address(local_tep_ip),ipaddress.IPv4Address(gateway_ip),local_tep_mac)

# Create VPC object 
# id, v4prefix, type = vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue
vpc1=vpc.VpcObject(vpc1_id, type=vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vpc1_vxlan_encap )

# Create VPC for Infra
vpc100=vpc.VpcObject(vpc2_id, type=vpc_pb2.VPC_TYPE_TENANT, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vpc2_vxlan_encap )

# Create VPC for VCN
vcn_route1=route.RouteObject(ipaddress.IPv4Network(vcn_route_prefix1), "tunnel", tunnel_id)
vcn_route_table=route.RouteTableObject(vcn_route_table_id, types_pb2.IP_AF_INET, [vcn_route1])
vcn_vpc=vpc.VpcObject(vcn_vpc_id, type=vpc_pb2.VPC_TYPE_CONTROL, encaptype=types_pb2.ENCAP_TYPE_VXLAN, encapvalue=vcn_vpc_encap, v4routetableid=vcn_route_table_id )

# Create L3 Interfaces ..
# id, iftype, ifadminstatus, vpcid, prefix, portid, encap, macaddr
intf1=interface.InterfaceObject( 1, interface_pb2.IF_TYPE_L3, interface_pb2.IF_STATUS_UP, vpc2_id, intf1_prefix, 1, types_pb2.ENCAP_TYPE_NONE,intf1_mac, node_uuid=node_uuid )
intf2=interface.InterfaceObject( 2, interface_pb2.IF_TYPE_L3, interface_pb2.IF_STATUS_UP, vpc2_id, intf2_prefix, 2, types_pb2.ENCAP_TYPE_NONE,intf2_mac, node_uuid=node_uuid )

# Create VCN interface
vcn0=interface.InterfaceObject( 3, interface_pb2.IF_TYPE_CONTROL, interface_pb2.IF_STATUS_UP, prefix=vcn_intf_prefix, macaddr=vcn_vnic_mac, gateway=vcn_v4_router_ip )


# Create NH objects ..
#self, id, type, l3intfid, underlaymac, vpcid=None, nhip=None, vlanid=None, macaddr=None
nh1 = nh.NexthopObject( 1, 'underlay', 1, intf1_underlay_mac, vpc2_id )
nh2 = nh.NexthopObject( 2, 'underlay', 2, intf2_underlay_mac, vpc2_id )


# Create Tunnel Objects ..
# id, vpcid, localip, remoteip, macaddr, encaptype, vnid, nhid
tunnel1 = tunnel.TunnelObject( tunnel_id,vpc1_id, tunnel_local_ip, tunnel_remote_ip,tunnel_mac, tunnel_pb2.TUNNEL_TYPE_NONE, types_pb2.ENCAP_TYPE_VXLAN, tunnel_vnid,tunnel_nhid) 

# Create DHCP Policy
dhcp_policy1 = dhcp.DhcpPolicyObject(1, server_ip=subnet1_gw_ip_addr, mtu=9216,  gateway_ip=subnet1_gw_ip_addr)

# Create Subnets
# id, vpcid, v4prefix, hostifindex, v4virtualrouterip, virtualroutermac, v4routetableid, fabricencap='VXLAN', fabricencapid=1
subnet1 = subnet.SubnetObject( 1, vpc1_id, ipaddress.IPv4Network(ipv4_subnet1), subnet1_host_if_idx, ipaddress.IPv4Address(subnet1_v4_router_ip), subnet1_virt_router_mac, 0, 'VXLAN', subnet1_fabric_encap, node_uuid=node_uuid )
subnet2 = subnet.SubnetObject( 2, vpc1_id, ipaddress.IPv4Network(ipv4_subnet2), None, ipaddress.IPv4Address(subnet2_v4_router_ip), subnet2_virt_router_mac, 0, 'VXLAN', subnet2_fabric_encap, node_uuid=node_uuid )

subnet3_route1=route.RouteObject(ipaddress.IPv4Network(subnet3_route_prefix1), "tunnel", tunnel_id)
subnet3_route_table=route.RouteTableObject(subnet3_route_table_id, types_pb2.IP_AF_INET, [subnet3_route1])
subnet3 = subnet.SubnetObject( 3, vpc1_id, ipaddress.IPv4Network(ipv4_subnet3), subnet3_host_if_idx, ipaddress.IPv4Address(subnet3_v4_router_ip), subnet3_virt_router_mac, subnet3_route_table_id, 'VXLAN', subnet3_fabric_encap, node_uuid=node_uuid )

# VCN subnet
vcn_subnet = subnet.SubnetObject( vcn_subnet_id, vcn_vpc_id, ipaddress.IPv4Network(vcn_subnet_pfx), vcn_host_if_idx, vcn_v4_router_ip, vcn_virt_router_mac, 0, 'VXLAN', vcn_subnet_encap, node_uuid=node_uuid )


# Create VNIC object
# id, subnetid, vpcid, macaddr, hostifindex, sourceguard=False, fabricencap='NONE', fabricencapid=1, rxmirrorid = [], txmirrorid = []
vnic1 = vnic.VnicObject(1, 1, subnet1_local_vnic_mac, subnet1_host_if_idx, False, 'VXLAN', subnet1_fabric_encap, node_uuid=node_uuid )
vnic3 = vnic.VnicObject(3, 3, subnet3_vnic_mac, subnet3_host_if_idx, False, 'VXLAN', subnet3_fabric_encap, node_uuid=node_uuid )

# Create VCN VNIC object
vcn_vnic = vnic.VnicObject(vcn_vnic_id, vcn_subnet_id, vcn_vnic_mac, vcn_host_if_idx, False, 'VXLAN', vcn_subnet_encap, node_uuid=node_uuid )


# Create Mapping Objects 1 for local vnic and another for remote IP
# self, key_type, macaddr, ip, vpcid, subnetid, tunnelid, encaptype, encapslotid, nexthopgroupid, publicip, providerip, vnicid = 0

map1 = mapping.MappingObject( 1, 'l3', vcn_vnic_mac, vcn_intf_ip, vcn_vpc_id, subnetid=vcn_subnet_id, vnicid=vcn_vnic_id )

map2 = mapping.MappingObject( 2, 'l3', subnet1_local_vnic_mac, subnet1_local_host_ip, vpc1_id, subnetid=1, vnicid=1 )
map3 = mapping.MappingObject( 3, 'l3', subnet1_remote_vnic_mac, subnet1_remote_host_ip, vpc1_id, subnetid=1, tunnelid=1 )

map5 = mapping.MappingObject( 5, 'l3', subnet2_remote_vnic_mac, subnet2_remote_host_ip, vpc1_id, subnetid=2, tunnelid=1 )

map6 = mapping.MappingObject( 6, 'l3', subnet3_vnic_mac, subnet3_vnic_ip, vpc1_id, subnetid=3, vnicid=3 )

# Push the configuration
api.client.Start(api.ObjectTypes.BATCH, batch1.GetGrpcMessage())

# Create configs
api.client.Create(api.ObjectTypes.SWITCH, [device1.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.VPC, [vpc1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VPC, [vpc100.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.INTERFACE, [intf1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.INTERFACE, [intf2.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.INTERFACE, [vcn0.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.NH, [nh1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.NH, [nh2.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.TUNNEL, [tunnel1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.ROUTE, [vcn_route_table.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VPC, [vcn_vpc.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.DHCP_POLICY, [dhcp_policy1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [subnet1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [subnet2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.ROUTE, [subnet3_route_table.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [subnet3.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.SUBNET, [vcn_subnet.GetGrpcCreateMessage()])

api.client.Create(api.ObjectTypes.VNIC, [vnic1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VNIC, [vnic3.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.VNIC, [vcn_vnic.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map1.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map2.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map3.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map5.GetGrpcCreateMessage()])
api.client.Create(api.ObjectTypes.MAPPING, [map6.GetGrpcCreateMessage()])

sys.exit(1)
