from stream import *

# Raised when parser fails
class NoMatch(Exception):
    def __init__(self, location=0, message=''):
        self._location = location
        self._message = message


# Matches one token as long as it is not EOF. Returns the token
def Any():
    def apply(stream):
        t = stream.consume()
        if not t:
            raise NoMatch()
        return (t, stream)
    return apply

# Matches EOF only
def EOF():
    def apply(stream):
        s = stream.clone()
        if s.consume() == None:
            return (None, s) 
        
        raise NoMatch()
    return apply
