#! /usr/bin/python3
import ipaddress

class IpAddressStep(object):
    def __init__(self, subnet, step, count = 1):
        super().__init__()
        self.start = ipaddress.IPv4Address(subnet)
        self.step = ipaddress.IPv4Address(step)
        self.curr = self.start
        self.count = count
        return

    def __getone(self):
        out = self.curr
        self.curr = ipaddress.IPv4Address(int(self.curr) + int(self.step))
        return out

    def Alloc(self):
        return self.__getone()

    def GetCount(self):
        return self.count

    def GetLast(self):
        #Assuming /24
        return ipaddress.IPv4Address(int(self.start) + 254)

ControlIpAllocator = IpAddressStep("172.16.100.1", "0.0.0.1")
WorkloadIpAllocator = IpAddressStep("192.168.100.1", "0.0.0.1")

TESTBED_NUM_VLANS         = 100
TESTBED_NUM_VLANS_HOSTPIN = 50
TESTBED_NUM_VLANS_CLASSIC = 50
class TestbedVlanAllocator(object):
    def __init__(self, tbid, mode):
        self.__start = int(tbid) * TESTBED_NUM_VLANS
        self.__count = TESTBED_NUM_VLANS_CLASSIC

        if mode == 'hostpin':
            self.__start += TESTBED_NUM_VLANS_HOSTPIN
            self.__count = TESTBED_NUM_VLANS_HOSTPIN

        self.__pool = iter(range(self.__start, self.__start + self.__count))
        return

    def Alloc(self):
        return next(self.__pool)
