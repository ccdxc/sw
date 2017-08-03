#! /usr/bin/python3

import infra.common.objects as objects

FlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
InterfaceIdAllocator    = objects.TemplateFieldObject("range/1/32768")
LifIdAllocator          = objects.TemplateFieldObject("range/1025/2047")
QueueIdAllocator        = objects.TemplateFieldObject("range/1/16384")
TenIdAllocator          = objects.TemplateFieldObject("range/1/1000")
SegIdAllocator          = objects.TemplateFieldObject("range/1/8192")
SegVlanAllocator        = objects.TemplateFieldObject("range/2/4095")
EncapVlanAllocator      = objects.TemplateFieldObject("range/3001/4000")
SegVxlanAllocator       = objects.TemplateFieldObject("range/0xFF0000/0xFFFFFF")

EpIdAllocator           = objects.TemplateFieldObject("range/1/4096")
SessionIdAllocator      = objects.TemplateFieldObject("range/1/65535")
FlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
TcpProxyCbIdAllocator   = objects.TemplateFieldObject("range/1/32768")
TlsProxyCbIdAllocator   = objects.TemplateFieldObject("range/1/32768")
SecProfIdAllocator      = objects.TemplateFieldObject("range/1/256")

RouterMacAllocator      = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
LifMacAllocator         = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
EnicMacAllocator        = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
RemoteEpMacAllocator    = objects.TemplateFieldObject("macstep/00EE.FF00.0001/0000.0000.0001")
IpSubnetAllocator       = objects.TemplateFieldObject("ipstep/64.0.0.0/0.1.0.0")
Ipv6SubnetAllocator     = objects.TemplateFieldObject("ipv6step/2000::0:0/0::1:0:0")
TepIpSubnetAllocator    = objects.TemplateFieldObject("ipstep/10.0.0.0/0.1.0.0")
TepIpv6SubnetAllocator  = objects.TemplateFieldObject("ipv6step/1000::0:0/0::1:0:0")

def CreateIpAddrAllocator(subnet):
    allocator = objects.TemplateFieldObject("ipstep/" + subnet + "/0.0.0.1")
    # Dont use the Subnet/32 address
    allocator.get()
    return allocator

def CreateIpv6AddrAllocator(subnet):
    allocator = objects.TemplateFieldObject("ipv6step/" + subnet + "/::1")
    # Dont use the Subnet/128 address
    allocator.get()
    return allocator
