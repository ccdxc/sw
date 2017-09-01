#! /usr/bin/python3

import infra.common.objects as objects

FlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
L4LbServiceIdAllocator  = objects.TemplateFieldObject("range/1/4096")
L4LbBackendIdAllocator  = objects.TemplateFieldObject("range/1/16384")
InterfaceIdAllocator    = objects.TemplateFieldObject("range/1/32768")
LifIdAllocator          = objects.TemplateFieldObject("range/1025/2047")
QueueIdAllocator        = objects.TemplateFieldObject("range/1/16384")
TenIdAllocator          = objects.TemplateFieldObject("range/1/1000")
SegIdAllocator          = objects.TemplateFieldObject("range/1/8192")
SegVlanAllocator        = objects.TemplateFieldObject("range/2/4095")
EncapVlanAllocator      = objects.TemplateFieldObject("range/3001/4000")
SegVxlanAllocator       = objects.TemplateFieldObject("range/0xFF0000/0xFFFFFF")

AclIdAllocator          = objects.TemplateFieldObject("range/1/512")
SpanSessionIdAllocator  = objects.TemplateFieldObject("range/1/8")
EpIdAllocator           = objects.TemplateFieldObject("range/1/4096")
SessionIdAllocator      = objects.TemplateFieldObject("range/1/65535")
FlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
TcpCbIdAllocator        = objects.TemplateFieldObject("range/0/32768")
TlsCbIdAllocator        = objects.TemplateFieldObject("range/0/32768")
IpsecCbIdAllocator      = objects.TemplateFieldObject("range/0/32768")
SecProfIdAllocator      = objects.TemplateFieldObject("range/1/256")

RouterMacAllocator      = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
LifMacAllocator         = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
EnicMacAllocator        = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
RemoteEpMacAllocator    = objects.TemplateFieldObject("macstep/00EE.FF00.0001/0000.0000.0001")
L4LbVMacAllocator       = objects.TemplateFieldObject("macstep/0000.9999.0001/0000.0000.0001")

AclIPv4Allocator       = objects.TemplateFieldObject("ipstep/150.0.0.0/0.0.0.1")
AclIPv6Allocator       = objects.TemplateFieldObject("ipv6step/a000::1/0::1")
AclIPv4SubnetAllocator = objects.TemplateFieldObject("ipstep/151.0.0.0/0.1.0.0")
AclIPv6SubnetAllocator = objects.TemplateFieldObject("ipv6step/a001::1/0:0:0:1::")

IpSubnetAllocator               = objects.TemplateFieldObject("ipstep/64.0.0.0/0.1.0.0")
Ipv6SubnetAllocator             = objects.TemplateFieldObject("ipv6step/2000::0:0/0::1:0:0")
TepIpSubnetAllocator            = objects.TemplateFieldObject("ipstep/10.0.0.0/0.1.0.0")
TepIpv6SubnetAllocator          = objects.TemplateFieldObject("ipv6step/1000::0:0/0::1:0:0")
L4LbServiceIpAllocator          = objects.TemplateFieldObject("ipstep/200.0.0.1/0.0.0.1")
L4LbServiceIpv6Allocator        = objects.TemplateFieldObject("ipv6step/3333::0:1/0::1")
L4LbBackendIpSubnetAllocator    = objects.TemplateFieldObject("ipstep/172.16.0.0/0.1.0.0")
L4LbBackendIpv6SubnetAllocator  = objects.TemplateFieldObject("ipv6step/4444::0/0::1:0:0")

def CreateIpv4AddrPool(subnet):
    allocator = objects.TemplateFieldObject("ipstep/" + subnet + "/0.0.0.1")
    # Dont use the Subnet/32 address
    allocator.get()
    return allocator

def CreateIpv6AddrPool(subnet):
    allocator = objects.TemplateFieldObject("ipv6step/" + subnet + "/::1")
    # Dont use the Subnet/128 address
    allocator.get()
    return allocator
