
from parsers import *
from stream import *

# Match any one of the list of parsers. Returns the result of the parser that 
# matched
def Choice(parsers):
    def apply(stream):
        for p in parsers:
            try:
                s = stream.clone()
                return p(s)
            except NoMatch:
                pass
        raise NoMatch()
    return apply

# Matches one or more times the parser. Returns a list of the results of
# the parser
def Many(parser):
    def apply(stream):
        s = stream
        rs = []
        while True:
            try:
                s = s.clone()
                (r, s) = parser(s)
                rs.append(r)
            except NoMatch:
                return (rs, s)
    return apply

# Match at least one or more times the parser. Returns a list of the results of
# the parser
def Many1(parser):
    def apply(stream):
        s = stream.clone()
        (r, s) = Many(parser)(s)
        if len(r) == 0:
            raise NoMatch()
        return (r, s)
    return apply

# Matches everything until the parser. Returns the tokens until the parser 
# fails. The result of the `parser` is lost
def ManyTill(parser):
    def apply(stream):
        s = stream.clone()
        rs = []
        while True:
            try:
                parser(s)
                return (rs, s)
            except NoMatch:
                t = s.consume()
                if not t:
                    raise NoMatch()
                rs.append(t)
    return apply

# Matches a sequence of parsers. Returns the list of the results of the parsers
def Sequence(parsers):
    def apply(stream):
        s = stream.clone()
        rs = []
        for p in parsers:
            try:
                (r, s) = p(s)
                rs.append(r)
            except NoMatch:
                raise NoMatch()
        return (rs, s)
    return apply

# Matchs if the parser doesn't match. Returns the token
def Not(parser):
    def apply(stream):
        s = stream.clone()
        try:
            parser(s)
        except NoMatch:
            c = s.consume()
            return (c, s)
        raise NoMatch()
    return apply

# Matches if none of the parsers match. Returns the token
def NoneOf(parsers):
    def apply(stream):
        s = stream.clone()
        for p in parsers:
            try:
                p(s)
            except NoMatch:
                continue
            raise NoMatch()
        c = s.consume()
        if c == None:
            raise NoMatch()
        return (c, s)
    return apply

# Optionally matches the parser. If it fails it returns None
def Option(parser):
    def apply(stream):
        s = stream.clone()
        try:
            return parser(s)
        except NoMatch:
            return (None, s)
    return apply

# Map is used to map the result of a parser
def Map(parser, func):
    def apply(stream):
        (r, s) = parser(stream)
        return (func(r), s)
    return apply


# Between
def Between(start, end):
    return Map(Sequence([start, Many(Not(end)), end]), lambda x:
        x[1])

def SeparatedBy(parser, sep):
    def apply(stream):
        s = stream.clone()
        ts = []
        while True:
            try:
                (t, s) = parser(s)
                ts.append(t)
                (_, s) = sep(s)
            except NoMatch:
                return (ts, s)
    return apply
