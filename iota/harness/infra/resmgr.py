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

class Ipv6AddressStep(object):
     def __init__(self, subnet, step, count = 1):
         super().__init__()
         self.start = ipaddress.IPv6Address(subnet)
         self.step = ipaddress.IPv6Address(step)
         self.curr = self.start
         return

     def __getone(self):
         out = self.curr
         self.curr = ipaddress.IPv6Address(int(self.curr) + int(self.step))
         return out

     def Alloc(self):
         return self.__getone()


class MacAddressBase(object):
     def __init__(self, string=None, integer=None):
         super().__init__()
         if string:
             if "." in string:
                 words16 = string.split(".")
                 assert(len(words16) == 3)
                 self.num = (int(words16[0], 16) << 32) +\
                            (int(words16[1], 16) << 16) +\
                            (int(words16[2], 16))
             elif ":" in string:
                 by8 = string.split(":")
                 assert(len(by8) == 6)
                 self.num = (int(by8[0], 16) << 40) +\
                            (int(by8[1], 16) << 32) +\
                            (int(by8[2], 16) << 24) +\
                            (int(by8[3], 16) << 16) +\
                            (int(by8[4], 16) << 8) +\
                            (int(by8[5], 16))
             else:
                 assert 0
         elif integer is not None:
             #Assume its converted.
             self.num = integer
         return

     def __str__(self):
         return self.get()


     def get(self):
         by = [0] * 6
         by[0] = (self.num >> 40) & 0xFF
         by[1] = (self.num >> 32) & 0xFF
         by[2] = (self.num >> 24) & 0xFF
         by[3] = (self.num >> 16) & 0xFF
         by[4] = (self.num >> 8) & 0xFF
         by[5] = (self.num >> 0) & 0xFF
         string = "%02x:%02x:%02x:%02x:%02x:%02x" %\
                  (by[0], by[1], by[2], by[3], by[4], by[5])
         return string

     def getnum(self):
         return self.num

     def update(self, macincr):
         self.num += macincr
         return

     def __eq__(self, other):
         return other and self.getnum() == other.getnum()

class MacAddressStep(object):
     def __init__(self, mac, step):
         super().__init__()
         self.start = MacAddressBase(mac)
         self.step = MacAddressBase(step)
         self.curr = self.start
         return

     def __getone(self):
         out = self.curr
         self.curr.num += self.step.num
         return out

     def Alloc(self):
         out = self.__getone()
         return MacAddressBase(out.get())

ControlIpAllocator = IpAddressStep("172.16.100.1", "0.0.0.1")
WorkloadIpAllocator = IpAddressStep("192.168.100.1", "0.0.0.1")

TESTBED_NUM_VLANS         = 100
TESTBED_NUM_VLANS_HOSTPIN = 50
TESTBED_NUM_VLANS_CLASSIC = 50
class TestbedVlanAllocator(object):
    def __init__(self, vlan_base, mode):
        self.__start = vlan_base
        self.__count = TESTBED_NUM_VLANS_CLASSIC

        if mode == 'hostpin':
            self.__start += TESTBED_NUM_VLANS_HOSTPIN
            self.__count = TESTBED_NUM_VLANS_HOSTPIN

        self.__pool = iter(range(self.__start, self.__start + self.__count))
        return

    def Alloc(self):
        try:
            return next(self.__pool)
        except:
            self.__pool = iter(range(self.__start, self.__start + self.__count))
            return next(self.__pool)

    def Reset(self):
        self.__pool = iter(range(self.__start, self.__start + self.__count))
        return

    def Count(self):
        return self.__count

    def Vlans(self):
        return copy.deepcopy(range(self.__start, self.__start + self.__count))

    def VlanRange(self):
        return str(self.__start) + "-" + str(self.__start + self.__count)

    def VlanNative(self):
        return self.__start


class TestbedVlanManager(object):
    def __init__(self, vlans):
        self.__vlans = vlans
        self.__pool = None
        self.__count = 0
        self.Reset()

    def Alloc(self):
        return next(self.__pool)

    def Count(self):
        return self.__count

    def Reset(self):
        self.__pool = iter(self.__vlans)
        self.__count = len(self.__vlans)


TESTBED_NUM_PORTS = 32000
class TestbedPortAllocator():
    def __init__(self, start = 6000):
        self.__start = start
        self.__count = TESTBED_NUM_PORTS
        self.__pool = iter(range(self.__start, self.__start + self.__count))
        return

    def Alloc(self):
        return next(self.__pool)

TcpPortAllocator = TestbedPortAllocator()
UdpPortAllocator = TestbedPortAllocator()
