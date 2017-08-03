#! /usr/bin/python3

import infra.common.defs as defs
import infra.common.objects as objects

class PacketBufferObject(objects.FrameworkObject):
    def __init__(self, size):
        super.__init__()
        self.size = size
        self.buff = None
        return

    def get(self):
        return self.buff

    def set(self, buff, size, offset = 0):
        self.buff = bytes(self.size)

        start = offset
        end = self.size - offset
        self.buff[start:end] = buff
        return 

    def configure(self):
        return

