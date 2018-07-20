from stream import * 
from parsers import * 

# Char parser is a group of parsers that can run against a string or char list 
# and return a char.

def Char(char):
    def apply(stream):
        s = stream.clone()
        c = s.consume()
        if c == char:
            return (c, s)
        raise NoMatch()
    return apply

def Digit():
    def apply(stream):
        s = stream.clone()
        c = s.consume()
        if c >= '0' and c <= '9':
            return (int(c) - int('0'), s)
        raise NoMatch()
    return apply

def AlphaNumeric():
    def apply(stream):
        s = stream.clone()
        c = s.consume()
        if c >= '0' and c <= '9':
            return (c, s)
        if c >= 'A' and c <= 'Z':
            return (c, s)
        if c >= 'a' and c <= 'z':
            return (c, s)
        if c == '_':
            return (c, s)
        raise NoMatch()
    return apply

def WhiteSpace():
    def apply(stream):
        s = stream.clone()
        c = s.consume()
        if c == ' ' or c == '\t':
            return (c, s)
        raise NoMatch()
    return apply