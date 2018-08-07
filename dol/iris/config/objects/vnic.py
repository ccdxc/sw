#! /usr/bin/python3

import infra.common.defs as defs
import infra.common.objects as objects

class VnicObject(objects.FrameworkObject):
    def __init__(self, template):
        super().__init__()
        self.template = template
        self.__load()
        return

    def __load(self):
        return

    def configure(self):
        return

