import unittest
import sys
import random

from app.tcp import *

class TestTcp(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestTcp, self).__init__(*args, **kwargs)
        self.portList = ["any", "1000 - 20000", "6456", "4789", "0"]

    def test_tcp(self):
        inp_list = ["any", "1000 - 20000", "6456", "4789", "0"]
        for sp in inp_list:
            for dp in inp_list:
                kwargs = {"sp":sp, "dp":dp}
                tcp  = Tcp(**kwargs)
                same_tcp = Tcp(**kwargs)

            if tcp.sp.portType == PORT_TYPE.PORT_ANY:
                self.assertNotEqual(tcp.hint & HINT_MATCH_SP,
                                    HINT_MATCH_SP,
                                    "Should not be same")
            else:
                self.assertEqual(tcp.hint & HINT_MATCH_SP,
                                 HINT_MATCH_SP,
                                 "Should be same")

            if tcp.dp.portType == PORT_TYPE.PORT_ANY:
                self.assertNotEqual(tcp.hint & HINT_MATCH_DP,
                                    HINT_MATCH_DP,
                                    "Should not be same")
            else:
                self.assertEqual(tcp.hint & HINT_MATCH_DP,
                                 HINT_MATCH_DP,
                                 "Should be same")

            self.assertEqual(tcp.sp, Port(sp),
                             "should be same as %s"%tcp.sp)
            self.assertEqual(tcp.dp, Port(dp),
                             "should be same as %s"%tcp.dp)

            self.assertEqual(tcp, same_tcp, "Should be same %s"%tcp)
            self.assertEqual(str(tcp), "sp=%s, dp=%s"%(sp,dp),
                             "Should be same %s"%tcp)

        inp_list2 = ["124", "124-124", "65535"]

        for i in range(len(inp_list2), len(inp_list)):
            sp1, sp2 = random.choice(inp_list), random.choice(inp_list2)
            dp1, dp2 = random.choice(inp_list), random.choice(inp_list2)
            kwargs1 = {"sp": sp1, "dp": dp1}
            kwargs2 = {"sp": sp2, "dp": dp2}
            tcp1 = Tcp(**kwargs1)
            tcp2 = Tcp(**kwargs2)

            self.assertNotEqual(tcp1, tcp2,
                                "%s and %s should not be same"%
                                (tcp1, tcp2))

    def test_tcp_match(self):
        for i in range(10):
            sp = random.choice(self.portList)
            dp = random.choice(self.portList)

            tcp = Tcp(**{"sp":sp, "dp":dp})
            rand = tcp.getRandom()
            self.assertEqual(tcp.match(**rand), True,
                             "%s Should match to %s"%(rand, tcp))

        self.assertNotEqual(tcp.match(**{"sp":sp}), True,
                            "Should not match")
        self.assertNotEqual(tcp.match(**{"dp":dp}), True,
                            "Should not match")
