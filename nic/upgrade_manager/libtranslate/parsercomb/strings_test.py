import unittest

from stream import *
from parsers import *
from strings import *

class TestStrings(unittest.TestCase):
    def test_string(self):
        s = Stream('abcd')
        (t, s) = String('abcd')(s)
        self.assertTrue(t == 'abcd')

        s = Stream('abcde')
        self.assertRaises(NoMatch, String('abcdef'), s)
        