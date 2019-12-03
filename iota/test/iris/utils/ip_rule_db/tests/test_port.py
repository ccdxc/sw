import unittest
import sys

from util.port import *

class TestPort(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestPort, self).__init__(*args, **kwargs)
        self.portList = ["any", "1000 - 20000", "6456", "0-0"]

    def test_Port(self):
        pvalid = ["any", "1000 - 20000", "6456"]
        for inp in pvalid:
            p = Port(inp)
            self.assertEqual(str(p), inp,
                             "should be same")
            self.assertNotEqual(p, None,
                                "Should not be same")
            self.assertNotIn(None, p,
                             "Should not match IN %s"%p)
        pinvalid = "-234asd"
        self.assertRaisesRegex(ValueError,
                               "Invalid Port %s"%pinvalid,
                               Port, pinvalid)

    def test_PortAny(self):
        pnum = "any"
        p = PortAny(pnum)

        self.assertEqual(str(p), pnum,
                         "should be same as %s"%pnum)
        self.assertEqual(p.portType, PORT_TYPE.PORT_ANY,
                         "should be same as %s"%PORT_TYPE.PORT_ANY.name)
        self.assertEqual(p, PortAny(pnum),
                         "should be same ")
        self.assertEqual(p, PortAny("*"),
                         "should be same ")
        self.assertNotEqual(p, PortNum("10"),
                         "should not be same ")
        self.assertIn(1000, p,
                      "should match")
        self.assertNotIn(9999999, p,
                         "should not match")
        self.assertNotIn(-2001, p,
                         "should not match")

        self.assertRaisesRegex(ValueError,
                               "Invalid Port num -300",
                               PortNum, "-300")
    def test_PortNum(self):
        pnum = "1000"
        p = PortNum(pnum)

        self.assertEqual(str(p), pnum,
                         "should be same as %s"%pnum)
        self.assertEqual(p.portType, PORT_TYPE.PORT_NUM,
                         "should be same as %s"%PORT_TYPE.PORT_NUM.name)
        self.assertEqual(p, PortNum(pnum),
                         "should be same")
        self.assertNotEqual(p, PortNum("10"),
                         "should not be same ")
        self.assertIn(1000, p,
                         "should match")
        self.assertNotIn(999, p,
                         "should not match")
        self.assertNotIn(2001, p,
                         "should not match")

        self.assertRaisesRegex(ValueError,
                               "Invalid Port num -300",
                               PortNum, "-300")

    def test_PortRange(self):
        prange = "1000 - 2000"
        p = PortRange(prange)

        self.assertEqual(str(p), prange,
                         "should be same as %s"%prange)
        self.assertEqual(p.portType, PORT_TYPE.PORT_RANGE,
                         "should be same as %s"%PORT_TYPE.PORT_RANGE.name)
        self.assertEqual(p, PortRange(prange),
                         "should be same ")
        self.assertNotEqual(p, PortRange("10-20"),
                         "should not be same ")
        self.assertIn(1000, p,
                         "should match")
        self.assertIn(1500, p,
                         "should match")
        self.assertIn(2000, p,
                         "should match")

        self.assertNotIn(999, p,
                         "should not match")
        self.assertNotIn(15000, p,
                         "should not match")
        self.assertNotIn(2001, p,
                         "should not match")

        prange = "1000-1000"
        p = PortRange(prange)
        self.assertEqual(str(p), "1000 - 1000",
                         "should be same ")
        self.assertIn(1000, p,
                         "Should match")

        self.assertRaisesRegex(ValueError,
                               "Invalid Port range : 10000-300",
                               PortRange, "10000-300")
        self.assertRaisesRegex(ValueError,
                               "Invalid Port range : 10000000-31100",
                               PortRange, "10000000-31100")
        self.assertRaisesRegex(ValueError,
                               "Invalid Port range : 31100-10000000",
                               PortRange, "31100-10000000")
        self.assertRaisesRegex(ValueError,
                               "Invalid Port range : -31100-10000000",
                               PortRange, "-31100-10000000")

    def test_Port_getRandom(self):
        for p in self.portList:
            p = Port(p)
            rand = p.getRandom()
            self.assertIn(str(rand), p, "%s should match in %s"%(rand, p))

if __name__ == '__main__':
    unittest.main()
