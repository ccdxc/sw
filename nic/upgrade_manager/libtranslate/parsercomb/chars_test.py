import unittest

from chars import *

class TestChar(unittest.TestCase):
    def test(self):
        s = Stream('a')
        (a, s) = Char('a')(s)
        self.assertTrue(a == 'a')

        s = Stream('b')
        self.assertRaises(NoMatch, Char('a'), s)

class TestWhiteSpace(unittest.TestCase):
    def test(self):
        s = Stream(' ')
        (w, s) = WhiteSpace()(s)
        self.assertTrue(w == ' ')

        s = Stream('\t')
        (w, s) = WhiteSpace()(s)
        self.assertTrue(w == '\t')

        s = Stream('b ')
        self.assertRaises(NoMatch, WhiteSpace(), s)
