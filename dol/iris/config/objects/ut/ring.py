#! /usr/bin/python3

import infra.common.defs as defs
import infra.common.objects as objects
from factory.objects.ut.descriptor import InfraUtTxDescriptorObject

RingIdAllocator = objects.TemplateFieldObject("range/1/256")


class RingObject(objects.FrameworkObject):
    def __init__(self, size, entrysz, pi=0, ci=0):
        super().__init__()
        self.ID(RingIdAllocator.get())
        self.size = size
        self.entrysz = entrysz
        self.pi = pi
        self.ci = ci

        self.list = [None] * self.size
        return

    def configure(self):
        return

    def Post(self, descriptor):
        descriptor.Write(self.GID(), self.pi)
        self.pi += 1
        return self.pi

    def Read(self, descriptor, index):
        descriptor.Read(self.GID(), index - 1)

    def Consume(self):
        while True:
            descriptor = InfraUtTxDescriptorObject()
            descriptor.size = 1024
            descriptor.Read(self.GID(), self.ci)
            if not descriptor.buffs:
                # No buffers for this descriptor, assuming no more descriptors
                # to consume.
                return
            self.ci += 1
            yield descriptor
