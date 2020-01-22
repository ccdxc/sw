#! /usr/bin/python3
# {C} Copyright 2019 Pensando Systems Inc. All rights reserved\n

import json
import sys
import os
import apollo.config.resmgr as resmgr

class Config(object):
    def __init__(self):
        self.objects = []
        return;

    def SetPortParams(self, num_tcp, num_udp, num_icmp, sport_lo, sport_hi, dport_lo, dport_hi):
        obj = {}
        obj['kind'] = 'flow'
        obj['num_tcp'] = str(num_tcp)
        obj['num_udp'] = str(num_udp)
        obj['num_icmp'] = str(num_icmp)
        obj['sport_lo'] = str(sport_lo)
        obj['sport_hi'] = str(sport_hi)
        obj['dport_lo'] = str(dport_lo)
        obj['dport_hi'] = str(dport_hi)
        self.objects.append(obj)
        return

    def SetDeviceParams(self):
        obj = {}
        obj['kind'] = 'device'
        obj['dual-stack'] = 'true'
        self.objects.append(obj)
        return

    def SetSessionParams(self, num_nh_per_vpc, num_v4_meter_per_vpc, num_v6_meter_per_vpc, vpc_count):
        obj = {}
        obj['kind'] = 'session'
        obj['num_nh_per_vpc'] = []
        obj['num_nh_per_vpc']  = num_nh_per_vpc
        obj['num_v4_meter_per_vpc']  = num_v4_meter_per_vpc
        obj['num_v6_meter_per_vpc']  = num_v6_meter_per_vpc
        obj['vpc_count'] = str(vpc_count)
        self.objects.append(obj)
        return

class CfgJsonObjectHelper:
    def __init__(self):
        self.__obj = None
        self.__file = os.environ['CONFIG_PATH'] + '/gen/dol_agentcfg.json'
        self.__num_tcp = 100000
        self.__num_udp = 0
        self.__num_icmp = 0
        self.__sport_lo = resmgr.TransportSrcPort
        self.__sport_hi = resmgr.TransportSrcPort
        self.__dport_lo = resmgr.TransportDstPort
        self.__dport_hi = resmgr.TransportDstPort
        self.__num_nh_per_vpc = []
        self.__num_v4_meter_per_vpc = []
        self.__num_v6_meter_per_vpc = []
        self.__vpc_count = 0
        return

    def SetNumNexthopPerVPC(self, v):
        self.__num_nh_per_vpc = v
        return

    def SetNumMeterPerVPC(self, v4, v6):
        self.__num_v4_meter_per_vpc = v4
        self.__num_v6_meter_per_vpc = v6
        return

    def SetVPCCount(self, v):
        self.__vpc_count = v
        return

    def WriteConfig(self):
        self.__obj = Config()
        self.__obj.SetPortParams(self.__num_tcp, self.__num_udp, self.__num_icmp, \
                self.__sport_lo, self.__sport_hi, self.__dport_lo, self.__dport_hi)
        self.__obj.SetDeviceParams()
        self.__obj.SetSessionParams(self.__num_nh_per_vpc, self.__num_v4_meter_per_vpc, \
                self.__num_v6_meter_per_vpc, self.__vpc_count)
        with open(self.__file, "w") as file:
            json.dump(self.__obj.__dict__, file, indent=4)
        #s = json.dumps(self.__obj.__dict__, indent=4)
        #print(s)
        return
