from scapy.all import TCP

from unit_test.infra.test_common import DolInfraTest
from infra.common.objects import MacAddressBase
from infra.common import objects
from infra.engine.trigger import ObjectCompare, Trigger
from infra.common.utils import convert_object_to_dict
from unit_test.infra.test_common import mock_modules
from infra.common.logging import logger

mock_modules()
factory_init_done = False


class DolPacketTest(DolInfraTest):

    class TestObj(object):
        pass

    def setUp(self):
        super(DolPacketTest, self).setUp()
        global PEN_REF
        from infra.engine.trigger import PEN_REF
        global PacketCompare
        from infra.engine.trigger import PacketCompare
        self._trig_obj = Trigger(connector='SocketDgram')
        self._trig_obj.start()
        logger.set_level(0)
        global factory
        import infra.factory.factory as factory

        global factory_init_done
        if not factory_init_done:
            factory.init()
            factory_init_done = True

    def tearDown(self):
        super(DolPacketTest, self).tearDown()
        import time
        self._trig_obj.stop()
        time.sleep(0.5)
        import mock
        mock.patch.stopall()

    def test_packet_compare_same(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)
        actual = self._construct_spkt(packet)
        result = PacketCompare(expected, actual)
        self.assertFalse(
            result.extra_hdrs or result.headers or result.missing_hdrs)

    def test_packet_compare_header_missing(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)
        del packet["Dot1Q"]
        actual = self._construct_spkt(packet)

        result = PacketCompare(expected, actual)
        self.assertTrue(result.missing_hdrs and
                        all(x in ["Dot1Q"] for x in result.missing_hdrs))
        self.assertFalse(result.extra_hdrs)

    def test_packet_compare_header_2_missing(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x800
                            },
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 17},
                  "UDP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)
        result = PacketCompare(expected, actual)
        self.assertTrue(result.missing_hdrs and
                        all(x in ["Dot1Q", "TCP"] for x in result.missing_hdrs))
        self.assertTrue(result.extra_hdrs and
                        all(x in ["UDP"] for x in result.extra_hdrs))

    def test_packet_compare_header_extra(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x800
                            },
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)
        result = PacketCompare(expected, actual)
        self.assertTrue(result.extra_hdrs and
                        all(x in ["Dot1Q"] for x in result.extra_hdrs))
        self.assertFalse(result.missing_hdrs)

    def test_packet_compare_eth_smac_mismatch(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0004").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)
        result = PacketCompare(expected, actual)
        #self.assertFalse(all(result[x] for x in ["missing_hdrs", "extra_hdrs"]))
        self.assertTrue(set(result.headers) == set(["Ether"]) and
                        set(result.headers["Ether"].mismatch_fields) == set(["src"]))

    def test_packet_compare_eth_smac_dmac_mismatch(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0005").get(),
                            "dst": MacAddressBase("0000.9999.0006").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)
        result = PacketCompare(expected, actual)
        #self.assertFalse(all(result[x] for x in ["missing_hdrs", "extra_hdrs"]))
        self.assertTrue(set(result.headers) == set(["Ether"]) and
                        set(result.headers["Ether"].mismatch_fields) ==
                        set(["src", "dst"]))

    def test_packet_compare_eth_sport_dport_mismatch(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 200, "dport": 100}}
        actual = self._construct_spkt(packet)
        result = PacketCompare(expected, actual)
        self.assertTrue(set(result.headers) == set(["TCP"]) and
                        set(result.headers["TCP"].mismatch_fields) ==
                        set(["sport", "dport"]))

    def test_packet_compare_only_ignore_ip_fields(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.65",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.67",
                         "dst": "10.0.0.68",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)

        ignore_pkt_cmp = objects.PacketComparePartial()
        hdr_ignore_result = objects.HeaderComparePartial()
        hdr_ignore_result.ignore_fields = ["src", "dst"]
        ignore_pkt_cmp.ignore_hdrs["IP"] = hdr_ignore_result
        result = PacketCompare(expected, actual, ignore_pkt_cmp)
        self.assertFalse(
            result.extra_hdrs or result.headers or result.missing_hdrs)

    def test_packet_compare_mult_ignore_ip_fields(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.68",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.69",
                         "dst": "10.0.0.70",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)
        ignore_pkt_cmp = objects.PacketComparePartial()
        hdr_ignore_result = objects.HeaderComparePartial()
        hdr_ignore_result.ignore_fields = ["src", "dst"]
        ignore_pkt_cmp.ignore_hdrs["IP"] = hdr_ignore_result
        result = PacketCompare(expected, actual, ignore_pkt_cmp)
        self.assertFalse(
            result.extra_hdrs or result.headers or result.missing_hdrs)

    def test_packet_compare_mult_header_mult_fields(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.68",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.69",
                         "dst": "10.0.0.70",
                         "proto": 6},
                  "TCP": {"sport": 101, "dport": 200}}
        actual = self._construct_spkt(packet)
        ignore_pkt_cmp = objects.PacketComparePartial()
        hdr_ignore_result = objects.HeaderComparePartial()
        hdr_ignore_result.ignore_fields = ["src", "dst"]
        ignore_pkt_cmp.ignore_hdrs["IP"] = hdr_ignore_result
        hdr_ignore_result = objects.HeaderComparePartial()
        hdr_ignore_result.ignore_fields = ["sport"]
        ignore_pkt_cmp.ignore_hdrs["TCP"] = hdr_ignore_result

        result = PacketCompare(expected, actual, ignore_pkt_cmp)
        self.assertFalse(
            result.extra_hdrs or result.headers or result.missing_hdrs)

        actual[TCP].dport = 9999

        result = PacketCompare(expected, actual, ignore_pkt_cmp)
        self.assertTrue(set(result.headers) == set(["TCP"]) and
                        set(result.headers["TCP"].mismatch_fields) == set(["dport"]))

        # Make sure other fields have no influence.
        actual[TCP].dport = 9999
        result = PacketCompare(expected, actual, ignore_pkt_cmp)
        self.assertTrue(set(result.headers) == set(["TCP"]) and
                        set(result.headers["TCP"].mismatch_fields) == set(["dport"]))

    def test_packet_compare_mult_ignore_header_missing(self):
        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x8100
                            },
                  "Dot1Q": {"vlan": 12, "type": 0x800},
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.68",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        expected = self._construct_spkt(packet)

        packet = {"Ether": {"src": MacAddressBase("0000.9999.0001").get(),
                            "dst": MacAddressBase("0000.9999.0002").get(),
                            "type": 0x800
                            },
                  "IP": {"src": "10.0.0.64",
                         "dst": "10.0.0.68",
                         "proto": 6},
                  "TCP": {"sport": 100, "dport": 200}}
        actual = self._construct_spkt(packet)

        ignore_pkt_cmp = objects.PacketComparePartial()
        hdr_ignore_result = objects.HeaderComparePartial()
        hdr_ignore_result.ignore_fields = ["vlan"]
        ignore_pkt_cmp.ignore_hdrs["Dot1Q"] = hdr_ignore_result
        result = PacketCompare(expected, actual, ignore_pkt_cmp)
        self.assertTrue(result.missing_hdrs and
                        all(x in ["Dot1Q"] for x in result.missing_hdrs))

    def object_compare_simple_one_level(self):
        obj1 = self.TestObj()
        setattr(obj1, "a", 1)
        setattr(obj1, "b", 2)
        setattr(obj1, "c", 3)
        obj2 = self.TestObj()
        setattr(obj2, "a", 4)
        setattr(obj2, "b", 5)
        setattr(obj2, "c", 3)
        result = ObjectCompare(obj1, obj2, None)
        print(convert_object_to_dict(result, ignore_keys=[]))
        self.assertTrue(result.mismatch_fields and (
            set(["a", "b"]) == set(result.mismatch_fields.keys())))

    def object_compare_simple_mult_level(self):
        def test_func():
            pass

        def test_func_1():
            pass
        obj1 = self.TestObj()
        obj11 = self.TestObj()
        obj111 = self.TestObj()
        setattr(obj1, "a", 1)
        setattr(obj1, "b", 2)
        setattr(obj1, "c", 3)
        setattr(obj1, "test_func", test_func)
        setattr(obj11, "d", 4)
        setattr(obj11, "e", 5)
        setattr(obj111, "g", 6)
        setattr(obj111, "f", 7)
        setattr(obj11, "ref1", obj111)
        setattr(obj1, "ref", obj11)
        obj2 = self.TestObj()
        obj22 = self.TestObj()
        obj222 = self.TestObj()
        setattr(obj2, "a", 4)
        setattr(obj2, "b", 5)
        setattr(obj2, "c", 3)
        setattr(obj1, "test_func", test_func_1)
        setattr(obj22, "d", 6)
        setattr(obj22, "e", 7)
        setattr(obj2, "ref", obj22)
        setattr(obj222, "g", 8)
        setattr(obj222, "f", 9)
        setattr(obj22, "ref1", obj222)
        result = ObjectCompare(obj1, obj2, None)
        self.assertTrue(result.mismatch_fields and (
            set(["a", "b", "ref"]) == set(result.mismatch_fields.keys())))

        self.assertTrue(result.mismatch_fields and (
            set(["d", "e", "ref1"]) == set(result.mismatch_fields["ref"].mismatch_fields.keys())))

        self.assertTrue(result.mismatch_fields and (
            set(["g", "f"]) == set(result.mismatch_fields["ref"].mismatch_fields["ref1"].mismatch_fields.keys())))

    def object_compare_simple_list_compare(self):
        obj1 = self.TestObj()
        setattr(obj1, "a", 1)
        setattr(obj1, "b", 2)
        setattr(obj1, "c", [1, 2, 3])
        obj2 = self.TestObj()
        setattr(obj2, "a", 4)
        setattr(obj2, "b", 5)
        setattr(obj2, "c", [1, 2, 3])
        result = ObjectCompare(obj1, obj2, None)
        self.assertTrue(result.mismatch_fields and (
            set(["a", "b"]) == set(result.mismatch_fields.keys())))

        setattr(obj1, "c", [1, 2, 4])
        result = ObjectCompare(obj1, obj2, None)
        self.assertTrue(result.mismatch_fields and (
            set(["a", "b", "c"]) == set(result.mismatch_fields.keys())))
        print(convert_object_to_dict(result))

    def object_compare_simple_one_list(self):
        obj1 = self.TestObj()
        objlist_1 = self.TestObj()
        setattr(obj1, "a", 1)
        setattr(obj1, "b", 2)
        setattr(obj1, "c", [objlist_1])
        obj2 = self.TestObj()
        objlist_2 = self.TestObj()
        setattr(obj2, "a", 4)
        setattr(obj2, "b", 5)
        setattr(obj2, "c", [objlist_2])
        result = ObjectCompare(obj1, obj2, None)
        self.assertTrue(result.mismatch_fields and (
            set(["a", "b"]) == set(result.mismatch_fields.keys())))

        setattr(objlist_1, "item1", 5)
        setattr(objlist_1, "item2", 6)
        setattr(objlist_2, "item1", 7)
        setattr(objlist_2, "item2", 8)
        setattr(obj1, "c", [objlist_1])
        setattr(obj2, "c", [objlist_2])
        result = ObjectCompare(obj1, obj2, None)
        self.assertTrue(result.mismatch_fields and (
            set(["a", "b", "c"]) == set(result.mismatch_fields.keys())))


if __name__ == '__main__':
    import unittest
    suite = unittest.TestSuite()
    suite.addTest(DolPacketTest(
        'object_compare_simple_list_compare'))
    unittest.TextTestRunner(verbosity=2).run(suite)
