#! /usr/bin/python3

import infra.common.objects as objects


class FactoryObjectBase(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        self.logger = None
        self.spec = None

    def __str__(self):
        return str(self.GID())

    def __copy__(self):
        obj = self.__class__()
        obj.GID(self.GID())
        obj.Init(self.spec)
        return obj

    def Logger(self, logger=None):
        if logger:
            self.logger = logger
        return self.logger

    def Bind(self, mem):
        self._mem = mem

    def Init(self, spec):
        self.spec = spec
        return

    def Read(self):
        raise NotImplementedError

    def Write(self):
        raise NotImplementedError
