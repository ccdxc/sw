#! /usr/bin/python3

import infra.common.defs as defs
import infra.common.objects as objects

from infra.factory.store import FactoryStore

class RingObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def __get_address(self, index):
        return self.base_address + self.descrsz * index

    def Init(self, queue, spec, desc_class): 
        self.GID(spec.id)

        self.size       = spec.size
        self.pi         = spec.pi
        self.ci         = spec.ci
        self.template   = spec.template.Get(FactoryStore)
        self.descrsz    = self.template.meta.size
        
        self.hw_pi  = 0
        self.hw_ci  = 0

        self.base_address = 0
        self.entries = [ None ] * self.size

        self.desc_class = desc_class
        return

    def Configure(self):
        # 1) Call the backend API to allocate an address for this ring.
        # 2) Initialize all the elements of the ring to default value.
        return

    def Post(self, descriptor):
        # 1) Convert descriptor to backend format or bytes
        # 2) Call mem_write of descriptor bytes
        descriptor.SetAddress(self.__get_address(self.pi))
        descriptor.Write()
        self.pi += 1
        return

    def Read(self, index = None):
        # Read the HW state of the ring.
        assert(self.pi == hwstate.pi)
        self.hw_ci = hwstate.ci
        return

    def Consume(self):
        objs = []
        self.Read()
        while self.ci < self.hw_ci:
            obj = self.desc_class()
            obj.address = self.__get_address(self.ci)
            obj.Read()
            objs.append(obj)
        return objs

class RingObjectHelper:
    def __init__(self):
        self.rings = []
        return

    def Generate(self, queue, spec):
        for r in range(spec.count):
            ring = RingObject()
            ring.Init(queue, spec, None)
            self.rings.append(ring)
        return
