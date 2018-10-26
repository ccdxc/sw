#! /usr/bin/python3
import ipaddress

import iota.harness.resmgr as resmgr

TenantIdAllocator = resmgr.IntegerStep(start = 1, step = 1, stop = 32)
NetworkSubnetAllocator = resmgr.IpAddressStep("192.168.0.0", "0.0.0.0")
EndpointIpAllocator = resmgr.IpAddressStep("192.168.100.1", "0.0.0.1")
