import unittest

from stream import *
from chars import *
from combinators import *

class TestCombinators(unittest.TestCase):
    def test_choice(self):
        s = Stream('a')
        (t, s) = Choice([Char('b'), Char('a')])(s)
        self.assertTrue(t == 'a')

        s = Stream('a')
        self.assertRaises(NoMatch, Sequence([Char('b'), Char('c')]), s)

    def test_many(self):
        s = Stream('')
        (t, s) = Many(Char('a'))(s)
        self.assertTrue(t == [])

        s = Stream('aaaa')
        (t, s) = Many(Char('a'))(s)
        self.assertSequenceEqual(t, ['a', 'a', 'a', 'a'])

    def test_many1(self):
        s = Stream('')
        self.assertRaises(NoMatch, Many1(Char('a')), s)

        s = Stream('b')
        self.assertRaises(NoMatch, Many1(Char('a')), s)

        s = Stream('aaaa')
        (t, s) = Many1(Char('a'))(s)
        self.assertSequenceEqual(t, ['a', 'a', 'a', 'a'])

    def test_manyTill(self):
        s = Stream('a')
        (t, s) = ManyTill(Char('a'))(s)
        self.assertSequenceEqual(t, [])

        s = Stream('bba')
        (t, s) = ManyTill(Char('a'))(s)
        self.assertSequenceEqual(t, ['b', 'b'])

        s = Stream('bbb')
        self.assertRaises(NoMatch, ManyTill(Char('a')), s)

    def test_sequence(self):
        s = Stream('abcd')
        (ts, s) = Sequence([Char('a'), Char('b'), Char('c')])(s)
        self.assertSequenceEqual(ts, ['a', 'b', 'c'])

        s = Stream('bbcd')
        self.assertRaises(NoMatch, Sequence([Char('a'), Char('b'), Char('c')]), s)

    def test_noneOf(self):
        s = Stream('a')
        (t, s) = NoneOf([Char('b'), Char('c')])(s)
        self.assertTrue(t == 'a')

        s = Stream('c')
        self.assertRaises(NoMatch, NoneOf([Char('b'), Char('c')]), s)

        s = Stream('')
        self.assertRaises(NoMatch, NoneOf([Char('b'), Char('c')]), s)

    def test_option(self):
        s = Stream('a')
        (t, s) = Option(Char('a'))(s)
        self.assertTrue(t == 'a')

        s = Stream('a')
        (t, s) = Option(Char('b'))(s)
        self.assertTrue(t == None)

    def test_map(self):
        s = Stream('a')
        (t, s) = Map(Char('a'), lambda _: 'b')(s)
        self.assertTrue(t == 'b')
    
    def test_between(self):
        s = Stream('adfdefaddb')
        (t, s) = Between(Char('a'), Char('b'))(s)
        self.assertEqual(t, ['d', 'f', 'd', 'e', 'f', 'a', 'd', 'd'])

        s = Stream('abcd')
        (t, s) = Between(Char('a'), Char('b'))(s)
        self.assertEqual(t, [])


    def test_separated_by(self):
        s = Stream('a,a,a,a')
        (t, s) = SeparatedBy(Char('a'), Char(','))(s)
        self.assertEqual(t, ['a', 'a', 'a', 'a'])

        s = Stream('a,a,a,a,')
        (t, s) = SeparatedBy(Char('a'), Char(','))(s)
        self.assertEqual(t, ['a', 'a', 'a', 'a'])

        s = Stream('a,a,a,a,b')
        (t, s) = SeparatedBy(Char('a'), Char(','))(s)
        self.assertEqual(t, ['a', 'a', 'a', 'a'])

        s = Stream('b,a,a,a,b')
        (t, s) = SeparatedBy(Char('a'), Char(','))(s)
        self.assertEqual(t, [])