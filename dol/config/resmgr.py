#! /usr/bin/python3

import ctypes
from infra.common.glopts import GlobalOptions

import infra.common.objects as objects
from ctypes import *
import os


HostMemoryAllocator     = None

FlowIdAllocator         = objects.TemplateFieldObject("range/1/65535")
L4LbServiceIdAllocator  = objects.TemplateFieldObject("range/1/4096")
L4LbBackendIdAllocator  = objects.TemplateFieldObject("range/1/16384")
InterfaceIdAllocator    = objects.TemplateFieldObject("range/5/32768")
LifIdAllocator          = objects.TemplateFieldObject("range/1025/2047")
QueueIdAllocator        = objects.TemplateFieldObject("range/0/16384")
TenIdAllocator          = objects.TemplateFieldObject("range/1/1000")
SegIdAllocator          = objects.TemplateFieldObject("range/1/8192")
OifListIdAllocator      = objects.TemplateFieldObject("range/1/65535")
RdmaOifListIdAllocator  = objects.TemplateFieldObject("range/1/65535")
MulticastGroupIdAllocator   = objects.TemplateFieldObject("range/1/65535")

SegVlanAllocator        = objects.TemplateFieldObject("range/2/1000")
EncapVlanAllocator      = objects.TemplateFieldObject("range/3001/4000")
SegVxlanAllocator       = objects.TemplateFieldObject("range/0xFF0000/0xFFFFFF")

BlackHoleSegVlanAllocator   = objects.TemplateFieldObject("range/4090/4095")
BlackHoleSegVxlanAllocator  = objects.TemplateFieldObject("range/0xDDDDD0/0xDDDDDF")

AclIdAllocator          = objects.TemplateFieldObject("range/1/512")
QosClassIdAllocator     = objects.TemplateFieldObject("range/1/16")
SpanSessionIdAllocator  = objects.TemplateFieldObject("range/1/8")
ColelctorIdAllocator    = objects.TemplateFieldObject("range/0/31")
EpIdAllocator           = objects.TemplateFieldObject("range/1/4096")
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
CryptoKeyIdAllocator    = objects.TemplateFieldObject("range/1/256")
IpsecCbIdAllocator      = objects.TemplateFieldObject("range/0/32768")
SecProfIdAllocator      = objects.TemplateFieldObject("range/1/256")

RouterMacAllocator      = objects.TemplateFieldObject("macstep/00CC.0000.0001/0000.0000.0001")
LifMacAllocator         = objects.TemplateFieldObject("macstep/00DD.0000.0001/0000.0000.0001")
EnicMacAllocator        = objects.TemplateFieldObject("macstep/00EE.0000.0001/0000.0000.0001")
RemoteEpMacAllocator    = objects.TemplateFieldObject("macstep/00EE.FF00.0001/0000.0000.0001")
L4LbVMacAllocator       = objects.TemplateFieldObject("macstep/0000.9999.0001/0000.0000.0001")

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


class MemHandle(object):

    def __init__(self, va, pa):
        self.va = va
        self.pa = pa

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        return self.va == other.va and self.pa == other.pa

    def __str__(self):
        if self.va is None or self.pa is None:
            return '<None>'
        return '<va=0x%x, pa=0x%x>' % (self.va, self.pa)

    def __add__(self, other):
        assert isinstance(other, int)
        return MemHandle(self.va + other, self.pa + other)


NullMemHandle = MemHandle(0, 0)


class HostMemory(object):
    def __init__(self):
        if not GlobalOptions.dryrun and GlobalOptions.hostmem:
            path = os.path.join(os.environ['WS_TOP'], "bazel-bin/nic/utils/host_mem/libhost_mem.so")
            self.lib = ctypes.CDLL(path, mode=ctypes.RTLD_GLOBAL)
            self.lib.alloc_host_mem.argtypes = [ctypes.c_uint64]
            self.lib.alloc_host_mem.restype = ctypes.c_void_p
            self.lib.alloc_page_aligned_host_mem.argtypes = [ctypes.c_uint64]
            self.lib.alloc_page_aligned_host_mem.restype = ctypes.c_void_p
            self.lib.host_mem_v2p.argtypes = [ctypes.c_void_p]
            self.lib.host_mem_v2p.restype = ctypes.c_uint64
            self.lib.host_mem_p2v.argtypes = [ctypes.c_uint64]
            self.lib.host_mem_p2v.restype = ctypes.c_void_p
            self.lib.free_host_mem.argtypes = [ctypes.c_void_p]
            self.lib.free_host_mem.restype = None
            assert self.lib.init_host_mem() == 0

    def get(self, size, page_aligned=True):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return NullMemHandle
        assert isinstance(size, int)
        if page_aligned:
            ptr = self.lib.alloc_page_aligned_host_mem(size)
        else:
            ptr = self.lib.alloc_host_mem(size)
        return MemHandle(ptr, self.lib.host_mem_v2p(ptr))

    def p2v(self, pa):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return NullMemHandle.pa
        assert isinstance(pa, int)
        return self.lib.host_mem_p2v(pa)

    def v2p(self, va):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return NullMemHandle.va
        assert isinstance(va, int)
        return self.lib.host_mem_v2p(va)

    def write(self, memhandle, data):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return
        assert isinstance(memhandle, MemHandle)
        assert isinstance(data, bytes)
        va = memhandle.va
        ba = bytearray(data)
        arr = c_char * len(ba)
        arr = arr.from_buffer(ba)
        memmove(va, arr, sizeof(arr))

    def read(self, memhandle, size):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return bytes()
        assert isinstance(memhandle, MemHandle)
        assert isinstance(size, int)
        ba = bytearray([0x0]*size)
        va = memhandle.va
        arr = c_char * size
        arr = arr.from_buffer(ba)
        memmove(arr, va, sizeof(arr))
        return bytes(ba)

    def zero(self, memhandle, size):
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return
        assert isinstance(memhandle, MemHandle)
        assert isinstance(size, int)
        va = memhandle.va
        memset(va, 0, c_uint64(size))

    def __del__(self):
        if GlobalOptions == None:
            return
        if GlobalOptions.dryrun or not GlobalOptions.hostmem: return
        self.lib.delete_host_mem()

def init():
    global HostMemoryAllocator
    HostMemoryAllocator = HostMemory()
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
