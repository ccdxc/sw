import unittest
import sys
import random

from app.udp import *

class TestUdp(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestUdp, self).__init__(*args, **kwargs)
        self.portList = ["any", "1000 - 20000", "6456", "4789", "0"]

    def test_udp(self):
        inp_list = ["any", "1000 - 20000", "6456", "4789", "0"]
        for sp in inp_list:
            for dp in inp_list:
                kwargs = {"sp":sp, "dp":dp}
                udp  = Udp(**kwargs)
                same_udp = Udp(**kwargs)

            if udp.sp.portType == PORT_TYPE.PORT_ANY:
                self.assertNotEqual(udp.hint & HINT_MATCH_SP, HINT_MATCH_SP,
                                    "Should not be same")
            else:
                self.assertEqual(udp.hint & HINT_MATCH_SP, HINT_MATCH_SP,
                                 "Should be same")

            if udp.dp.portType == PORT_TYPE.PORT_ANY:
                self.assertNotEqual(udp.hint & HINT_MATCH_DP, HINT_MATCH_DP,
                                    "Should not be same")
            else:
                self.assertEqual(udp.hint & HINT_MATCH_DP, HINT_MATCH_DP,
                                 "Should be same")

            self.assertEqual(udp.sp, Port(sp),
                             "should be same as %s"%udp.sp)
            self.assertEqual(udp.dp, Port(dp),
                             "should be same as %s"%udp.dp)

            self.assertEqual(udp, same_udp, "Should be same %s"%udp)
            self.assertEqual(str(udp), "sp=%s, dp=%s"%(sp,dp), "Should be same %s"%udp)

        inp_list2 = ["124", "124-124", "65535"]

        for i in range(len(inp_list2), len(inp_list)):
            sp1, sp2 = random.choice(inp_list), random.choice(inp_list2)
            dp1, dp2 = random.choice(inp_list), random.choice(inp_list2)
            kwargs1 = {"sp": sp1, "dp": dp1}
            kwargs2 = {"sp": sp2, "dp": dp2}
            udp1 = Udp(**kwargs1)
            udp2 = Udp(**kwargs2)

            self.assertNotEqual(udp1, udp2, "%s and %s should not be same"%(udp1, udp2))

    def test_udp_match(self):
        for i in range(10):
            sp = random.choice(self.portList)
            dp = random.choice(self.portList)

            udp = Udp(**{"sp":sp, "dp":dp})
            rand = udp.getRandom()
            self.assertEqual(udp.match(**rand), True,
                             "%s Should match to %s"%(rand, udp))

        self.assertNotEqual(udp.match(**{"sp":sp}), True,
                            "Should not match")
        self.assertNotEqual(udp.match(**{"dp":dp}), True,
                            "Should not match")
