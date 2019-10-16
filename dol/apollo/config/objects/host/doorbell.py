#! /usr/bin/python3

import infra.config.base  as base
from infra.common.logging import logger

class Doorbell(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, ring, spec):
        self.GID(spec.id)
        self.ring = ring
        self.spec = spec

    def Ring(self, test_spec, lgh=logger):
        logger.warn("Doorbell::Ring() - Base Class Implementation - DO NOT USE")
        return
