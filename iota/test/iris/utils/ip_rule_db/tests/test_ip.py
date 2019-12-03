import unittest
import sys

from  util.ip import *
from ipaddress import ip_network, ip_address

class TestIP(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestIP, self).__init__(*args, **kwargs)
        self.ipList = ["any", "192.168.0.0 - 192.168.255.255", \
                       "10.10.10.10/32", "10.10.10.0/24", "0.0.0.0/32", "255.255.255.255/32"]

    def test_IP(self):
        ipany = "*"
        iprange = "192.168.0.0 - 192.168.255.255"
        iplist = "192.168.100.101, 192.168.100.102, 192.168.100.103"
        ipsubnet = "10.10.10.10/32"
        ipinvalid = "asd"

        self.assertEqual(IP(ipany), IPAny(ipany),
                            "Should be True")
        self.assertEqual(IP(iprange), IPRange(iprange),
                         "Should be True")
        self.assertEqual(IP(iplist), IPList(iplist),
                         "Should be True")
        self.assertEqual(IP(ipsubnet), IPSubnet(ipsubnet),
                         "Should be True")
        self.assertNotEqual(IP(ipsubnet), None,
                         "Should not be same")
        self.assertRaisesRegex(ValueError, "Can not parse IP from '%s'"%ipinvalid, IP, ipinvalid)

    def test_IPAny(self):
        self.assertEqual(str(IPAny("any")), "any", "should be any")
        self.assertEqual(IPAny("any"), IPAny("any"),
                         "should be True")
        self.assertNotEqual(IPAny("any"), IPSubnet("10.10.10.0/24"),
                         "asda")

    def test_IPRange(self):
        iprange = "192.168.0.0 - 192.168.255.255"
        self.assertEqual(str(IPRange(iprange)), iprange, "should be %s"%iprange)
        self.assertEqual(IPRange(iprange).ipType, IPType.IP_RANGE, "should be %s"%IPType.IP_RANGE.name)
        self.assertEqual(IPRange(iprange).version, 4, "should be 4")

        self.assertRaisesRegex(ValueError, "Invalid IP range 192.168.255.255 - 192.168.25500.0",
                               IPRange, "192.168.255.255 - 192.168.25500.0")
        self.assertRaisesRegex(ValueError, "Invalid IP range 192.168.255.255 - 2001::11",
                               IPRange, "192.168.255.255 - 2001::11")
        self.assertRaisesRegex(ValueError, "Invalid IP range, start 192.168.255.255 can not be greater than end 192.168.255.0",
                               IPRange, "192.168.255.255 - 192.168.255.0")

        iprange = "192.168.100.1 - 192.168.100.101"
        iprange2 = "192.168.100.1 - 192.168.100.102"
        in_host = ip_address("192.168.100.101")
        out_host = ip_address("192.168.100.105")

        self.assertEqual(IPRange(iprange), IPRange(iprange),
                         "should be same")
        self.assertNotEqual(IPRange(iprange), IPRange(iprange2),
                         "should not be same")
        self.assertNotEqual(IPRange(iprange), IPAny("any"),
                          "should not be same")
        self.assertIn(in_host, IPRange(iprange),
                      "%s should match in %s"%(in_host, IPRange(iprange)))
        self.assertNotIn(out_host, IPRange(iprange),
                          "%s should not match in %s"%(out_host, IPRange(iprange)))

    def test_IPList(self):
        iplist = "192.168.100.101, 192.168.100.102, 192.168.100.103"
        op = ', '.join(sorted(str(IPList(iplist)).split(', ')))
        self.assertEqual(IPList(iplist).ipType, IPType.IP_LIST, "should be same %s"%IPType.IP_LIST.name)
        self.assertEqual(IPList(iplist).version, 4, "should be same 4")

        iplist = "192.168.100.101, 192.168.100.102.123, 192.168.100.103"
        self.assertRaisesRegex(ValueError, "\'192.168.100.102.123\' does not appear to be an IPv4 or IPv6 address", IPList, iplist)

        iplist = "192.168.100.101, 192.168.100.102, 192.168.100.103, "
        self.assertRaisesRegex(ValueError, "\'\' does not appear to be an IPv4 or IPv6 address", IPList, iplist)

        iplist = "192.168.100.101, 192.168.100.102, 192.168.100.103, 2001::1"
        self.assertRaisesRegex(ValueError, "All addresses in iplist are not of same version 192.168.100.101, 192.168.100.102, 192.168.100.103, 2001::1", IPList, iplist)

        iplist = "192.168.100.101, 192.168.100.102, 192.168.100.103"
        iplist2 = "192.168.100.101, 192.168.100.102, 192.168.100.103, 192.168.100.104"
        in_host = ip_address("192.168.100.101")
        out_host = ip_address("192.168.100.105")

        self.assertEqual(IPList(iplist), IPList(iplist),
                         "should be same")
        self.assertNotEqual(IPList(iplist), IPList(iplist2),
                         "should not be same")
        self.assertNotEqual(IPList(iplist), IPAny("any"),
                         "should not be same")
        self.assertIn(in_host, IPList(iplist),
                      "%s should match in %s"%(in_host, IPList(iplist)))
        self.assertNotIn(out_host, IPList(iplist),
                         "%s should not match in %s"%(out_host, IPList(iplist)))


    def test_IPSubnet(self):
        ip = "10.10.10.10/32"
        net = IPSubnet(ip)
        self.assertEqual(net.subnet, ip_network(ip), "should be %s"%ip)
        self.assertRaisesRegex(ValueError, "\'10.10.10.10.asd\' does not appear to be an IPv4 or IPv6 network", IPSubnet, "10.10.10.10.asd")
        self.assertEqual(net.version, 4, "should be 4")
        self.assertEqual(net.ipType, IPType.IP_SUBNET, "should be %s"%IPType.IP_SUBNET.name)

        net = IPSubnet(ip)
        self.assertEqual(str(net), ip, "should match %s"%( ip))

        net = IPSubnet("10.10.10.10")
        self.assertEqual(net.subnet, ip_network(ip), "should be %s"%(ip_network(ip)))

        net1 = IPSubnet(ip)
        same_net = IPSubnet(ip)
        not_same_net = IPSubnet("20.20.20.20")
        self.assertEqual(net1, same_net, "%s and %s should be same"%(net1, same_net))
        self.assertNotEqual(net1, not_same_net,"%s and %s not should be same"%(net1, not_same_net))
        self.assertNotEqual(net1, IPAny("any"), "%s and %s not should be same"%(net1, not_same_net))

        net = IPSubnet("10.10.10.0/24")
        in_host = ip_address("10.10.10.10")
        out_host = ip_address("192.168.100.101")
        self.assertIn(in_host, net,"%s and %s should match"%(net, in_host))
        self.assertNotIn(out_host, net, "%s and %s should not match"%(net, out_host))

    def test_ip_getRandom(self):
        for ip in self.ipList:
            ip = IP(ip)
            ran = ip.getRandom()
            self.assertIn(ran, ip, "%s should match in %s"%(ran, ip))


if __name__ == '__main__':
    unittest.main()
