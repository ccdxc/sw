import unittest

from stream import *
from parsers import *
from chars import *
from combinators import *
from numbers import *

class TestNumbers(unittest.TestCase):
    def test_base10(self):
        s = Stream('12345')
        (t, s) = Base10()(s)
        self.assertEqual(t, 12345)

        s = Stream('12345x')
        (t, s) = Base10()(s)
        self.assertEqual(t, 12345)

        s = Stream('x12345')
        self.assertRaises(NoMatch, Base10(), s)