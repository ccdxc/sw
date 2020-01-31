#! /usr/bin/python3

from infra.common.glopts import GlobalOptions

import infra.common.objects as objects

HostMemoryAllocator     = None

FlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
GftFlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
L4LbServiceIdAllocator  = objects.TemplateFieldObject("range/1/4096")
L4LbBackendIdAllocator  = objects.TemplateFieldObject("range/1/16384")
InterfaceIdAllocator    = objects.TemplateFieldObject("range/128/32768")
LifIdAllocator          = objects.TemplateFieldObject("range/1025/2047")
QueueIdAllocator        = objects.TemplateFieldObject("range/0/16384")
TenIdAllocator          = objects.TemplateFieldObject("range/11/1000")
SegIdAllocator          = objects.TemplateFieldObject("range/1/8192")
OifListIdAllocator      = objects.TemplateFieldObject("range/1/65535")
RdmaOifListIdAllocator  = objects.TemplateFieldObject("range/1/65535")
MulticastGroupIdAllocator   = objects.TemplateFieldObject("range/1/65535")

SegVlanAllocator            = objects.TemplateFieldObject("range/2/1000")
SegProviderVlanAllocator    = objects.TemplateFieldObject("range/1001/2000")
EncapVlanAllocator          = objects.TemplateFieldObject("range/3001/4000")
SegVxlanAllocator           = objects.TemplateFieldObject("range/0xFF0000/0xFFFFFF")

BlackHoleSegVlanAllocator   = objects.TemplateFieldObject("range/4090/4095")
BlackHoleSegVxlanAllocator  = objects.TemplateFieldObject("range/0xDDDDD0/0xDDDDDF")

AclIdAllocator          = objects.TemplateFieldObject("range/100/512")
QosClassIdAllocator     = objects.TemplateFieldObject("range/1/16")
SpanSessionIdAllocator  = objects.TemplateFieldObject("range/1/8")
CollectorIdAllocator    = objects.TemplateFieldObject("range/0/15")
EpIdAllocator           = objects.TemplateFieldObject("range/1/4096")
E2EIdAllocator       = objects.TemplateFieldObject("range/1/4096")
SessionIdAllocator      = objects.TemplateFieldObject("range/1/65535")
RdmaSessionIdAllocator  = objects.TemplateFieldObject("range/1/65535")
FlowIdAllocator         = objects.TemplateFieldObject("range/1/999999")
TcpCbIdAllocator        = objects.TemplateFieldObject("range/0/32768")
TlsCbIdAllocator        = objects.TemplateFieldObject("range/0/32768")
CpuIdAllocator          = objects.TemplateFieldObject("range/1/4")
RawrIdAllocator         = objects.TemplateFieldObject("range/1/32768")
RawcIdAllocator         = objects.TemplateFieldObject("range/1/32768")
ProxyrIdAllocator       = objects.TemplateFieldObject("range/0/32768")
ProxycIdAllocator       = objects.TemplateFieldObject("range/0/32768")
CryptoKeyIdAllocator    = objects.TemplateFieldObject("range/1/32768")
CryptoCertIdAllocator   = objects.TemplateFieldObject("range/1/32768")
CryptoAsymKeyIdAllocator   = objects.TemplateFieldObject("range/1/32768")
IpsecCbIdAllocator      = objects.TemplateFieldObject("range/0/32768")
SecProfIdAllocator      = objects.TemplateFieldObject("range/10/256")

RouterMacAllocator      = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
LifMacAllocator         = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
EnicMacAllocator        = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
RemoteEpMacAllocator    = objects.TemplateFieldObject("macstep/00EE.FF00.0001/0000.0000.0001")
L4LbVMacAllocator       = objects.TemplateFieldObject("macstep/0000.9999.0001/0000.0000.0001")
NicMacAllocator         = objects.TemplateFieldObject("macstep/00AA.0000.0001/0000.0000.0001")

AclIPv4Allocator         = objects.TemplateFieldObject("ipstep/150.0.0.0/0.0.0.1")
AclIPv6Allocator         = objects.TemplateFieldObject("ipv6step/a000::1/0::1")
AclMacAllocator          = objects.TemplateFieldObject("macstep/00AA.0000.0001/0000.0000.0001")
AclEtypeAllocator        = objects.TemplateFieldObject("range/0x1000/0x2000")
AclL4PortAllocator       = objects.TemplateFieldObject("range/1000/2000")
AclICMPTypeCodeAllocator = objects.TemplateFieldObject("range/10/100")

IpSubnetAllocator               = objects.TemplateFieldObject("ipstep/64.0.0.0/0.1.0.0")
Ipv6SubnetAllocator             = objects.TemplateFieldObject("ipv6step/2000::0:0/0::1:0:0")
GIPoAddressAllocator            = objects.TemplateFieldObject("ipstep/239.200.0.0/0.0.0.1")
TepIpSubnetAllocator            = objects.TemplateFieldObject("ipstep/10.0.0.0/0.1.0.0")
TepIpv6SubnetAllocator          = objects.TemplateFieldObject("ipv6step/1000::0:0/0::1:0:0")
L4LbServiceIpAllocator          = objects.TemplateFieldObject("ipstep/200.0.0.1/0.0.0.1")
L4LbServiceIpv6Allocator        = objects.TemplateFieldObject("ipv6step/3333::0:1/0::1")
L4LbBackendIpSubnetAllocator    = objects.TemplateFieldObject("ipstep/172.16.0.0/0.1.0.0")
L4LbBackendIpv6SubnetAllocator  = objects.TemplateFieldObject("ipv6step/4444::0/0::1:0:0")

QosCosAllocator     = objects.TemplateFieldObject("range/1/6")
QosDscpAllocator    = objects.TemplateFieldObject("range/32/48")

SecurityGroupAllocator  = objects.TemplateFieldObject("range/1/128")
DosPolicyAllocator      = objects.TemplateFieldObject("range/1/128")
InterruptAllocator      = objects.TemplateFieldObject("range/1/4096")

GftExmProfileIdAllocator = objects.TemplateFieldObject("range/1/32768")
GftTranspositionProfileIdAllocator = objects.TemplateFieldObject("range/1/32768")

SecurityPolicyIDAllocator = objects.TemplateFieldObject("range/1/32768")

NvmeSessionIdAllocator  = objects.TemplateFieldObject("range/1/65535")

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

def init():
    global HostMemoryAllocator
    HostMemoryAllocator = objects.GetHostMemMgrObject()
    assert HostMemoryAllocator is not None

def InitQos(topospec):
    global QosCosAllocator
    global QosDscpAllocator
    qos = getattr(topospec, "qos", False)
    if qos is False:
        QosCosAllocator = objects.TemplateFieldObject("range/7/7")
        QosDscpAllocator = objects.TemplateFieldObject("range/7/7")
    QosCosAllocator.EnableWrapAround()
    QosDscpAllocator.EnableWrapAround()
