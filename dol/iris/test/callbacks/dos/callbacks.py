#! /usr/bin/python3
import pdb

import infra.penscapy.penscapy as penscapy

def GetIpv4DstAddress(testcase, packet):
    return testcase.config.dst.segment.AllocIpv4Address()

def GetIpv6DstAddress(testcase, packet):
    return testcase.config.dst.segment.AllocIpv6Address()
