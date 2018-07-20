import unittest

from stream import *
from parsers import *

class TestParsers(unittest.TestCase):
    def test_any(self):
        s = Stream('a')
        (t, s) = Any()(s)
        self.assertTrue(t == 'a')

        self.assertRaises(NoMatch, Any(), s)

    def test_eof(self):
        s = Stream('a')
        self.assertRaises(NoMatch, EOF(), s)

        (_, s) = Any()(s)
        (t, s) = EOF()(s)
        self.assertTrue(t == None)