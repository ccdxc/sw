
# A stream object is essentially a sequence of tokens against which we run
# the parser
class Stream(object):
    def __init__(self, s):
        self._s = s
        self._loc = 0
        pass

    def clone(self):
        c = Stream(self._s)
        c._loc = self._loc
        return c

    def consume(self):
        c = self.peek()
        if not c:
            return None
        self._loc += 1
        return c

    def peek(self):
        if self._loc == len(self._s):
            return None
        return self._s[self._loc]

    def debug(self):
        return self._loc
