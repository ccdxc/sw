import unittest
import sys

from util.proto import *


class TestProto(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestProto, self).__init__(*args, **kwargs)
        self.protoList = ["icmp", "tcp", "udp", "any", "0", "6", "8", "256"]

    def test_proto(self):
        valid = ["icmp", "tcp", "udp", "any", "0", "6", "8", "256"]
        invalid_match = 23423
        no_match = 253

        for i in valid:
            p = Proto(i)
            self.assertEqual(str(p), i,
                             "should be same as %s"%i)
            self.assertEqual(p, Proto(i),
                             "Should be same as %s"%i)
            self.assertEqual(p != Proto(i), False, "Should not be True")
            if i.isalpha():
                self.assertIn(l3_proto_dict[i], p, "Shoud be in %s"%p)
            else:
                self.assertIn(i, p, "Shoud be in %s"%p)

            if p.isAny():
                self.assertIn(no_match, p, "Shoud match in %s"%p)
            else:
                self.assertNotIn(no_match, p, "Shoud not match in %s"%p)

            self.assertNotIn(invalid_match, p, "Shoud not match in %s"%p)
            self.assertNotIn(None, p, "Shoud not match in %s"%p)

        invalid = ["asd", "123asd", "-1", "277"]
        for i in invalid:
            self.assertRaisesRegex(ValueError,"Invalid l3 protocol %s"%i,
                                        Proto, i)

    def test_proto_getRandom(self):
        for proto in self.protoList:
            p = Proto(proto)
            rand = p.getRandom()
            self.assertIn(rand, p, "%s should match in %s"%(rand, p))

if __name__ == '__main__':
    unittest.main()
