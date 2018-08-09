#! /usr/bin/python3

import ctypes
import os

import infra.common.objects     as objects

from infra.common.glopts    import GlobalOptions
from ctypes                 import *

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

def Init():
    return
