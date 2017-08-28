#! /usr/bin/python3
import infra.factory.base as base

class EthBufferObject(base.FactoryObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, spec):
        self.spec = spec
        return

    def Write(self):
        return

    def Read(self):
        return
