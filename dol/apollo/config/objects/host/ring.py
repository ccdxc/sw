#! /usr/bin/python3

import infra.common.objects as objects

from infra.common.logging   import logger
from infra.factory.store    import FactoryStore

class RingObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def Init(self, queue, spec):
        self.GID(spec.id)
        self.spec = spec
        self.id = spec.id
        self.queue = queue
        self.descriptor_template = spec.desc.Get(FactoryStore)
        return

    def GetLif(self):
        return self.queue.GetLif()

    def Configure(self):
        # 1) Call the backend API to allocate an address for this ring.
        # 2) Initialize all the elements of the ring to default value.
        logger.warn("RingObject::Configure() - Base Class Implementation - DO NOT USE")
        return

    def Post(self, descriptor):
        # 1) Convert descriptor to backend format or bytes
        # 2) Call mem_write of descriptor bytes
        logger.warn("RingObject::Post() - Base Class Implementation - DO NOT USE")
        return

    def Read(self, index=None):
        # Read the HW state of the ring.
        logger.warn("RingObject::Read() - Base Class Implementation - DO NOT USE")
        return

    def Consume(self, descriptor):
        logger.warn("RingObject::Consume() - Base Class Implementation - DO NOT USE")
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
