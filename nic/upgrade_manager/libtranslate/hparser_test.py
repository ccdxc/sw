import unittest

from hparser import *

class TestPhase1(unittest.TestCase):
    def test_line_continuation(self):
        s = Stream('a\\\nb\n')
        (t, s) = Many(Choice([LineContinuation(), Any()]))(s)
        self.assertEqual(['a', None, 'b', '\n'], t)
        print(t)