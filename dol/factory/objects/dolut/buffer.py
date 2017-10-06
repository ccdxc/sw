#! /usr/bin/python3
import infra.factory.base   as base
import infra.common.objects as objects

class DolUtBufferObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, spec):
        super().Init(spec)
        self.address = 0x1234
        self.size = 1024
        self.data = [1,2,3,4]
        return

    def Write(self):
        return

    def Read(self):
        return
