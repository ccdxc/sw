#! /usr/bin/python3

import infra.common.defs as defs
import infra.common.objects as objects
import config.objects.doorbell as doorbell

from infra.common.logging   import cfglogger
from infra.factory.store    import FactoryStore

class RingObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def Init(self, queue, spec): 
        self.GID(spec.id)
        self.id = spec.id
        self.size = spec.size
        self.queue = queue

        self.doorbell = doorbell.Doorbell()
        self.doorbell.Init(self, spec)

        self.descriptor_template = spec.desc.Get(FactoryStore)
        return

    def Configure(self):
        # 1) Call the backend API to allocate an address for this ring.
        # 2) Initialize all the elements of the ring to default value.
        cfglogger.warn("RingObject::Configure() - Base Class Implementation - DO NOT USE")
        return

    def Post(self, descriptor):
        # 1) Convert descriptor to backend format or bytes
        # 2) Call mem_write of descriptor bytes
        cfglogger.warn("RingObject::Post() - Base Class Implementation - DO NOT USE")
        return

    def Read(self, index=None):
        # Read the HW state of the ring.
        cfglogger.warn("RingObject::Read() - Base Class Implementation - DO NOT USE")
        return

    def Consume(self, descriptor):
        cfglogger.warn("RingObject::Consume() - Base Class Implementation - DO NOT USE")
        return

class RingObjectHelper:
    def __init__(self):
        self.rings = []
        return

    def Generate(self, queue, spec):
        for rspec in spec.rings:
            ring = RingObject()
            ring.Init(queue, rspec.ring)
            self.rings.append(ring)
        return
