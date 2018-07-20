from stream import *
from parsers import *
from chars import *
from combinators import *

def String(value):
    return Map(Sequence([Char(c) for c in value]), lambda r: ''.join(r))
