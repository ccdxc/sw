#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
import config.objects.segment   as segment
import config.objects.tenant    as tenant

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import cfglogger
from config.store               import Store

class GftExmHeaderGroupObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('GFT_EXM_HEADER_GROUP'))
        return

    def Init(self, spec):
        self.GID(spec.name)
        self.headers = objects.MergeObjects(getattr(spec, 'headers', None),
                                            self.headers)
        self.fields = objects.MergeObjects(getattr(spec, 'fields', None),
                                           self.fields)
        bases = getattr(spec, 'inherit', [])
        for base in bases:
            self.__inherit_base(base)
        return

    def __inherit_base(self, baseref):
        base = baseref.Get(Store)
        self.headers = objects.MergeObjects(base.headers, self.headers)
        self.fields = objects.MergeObjects(base.fields, self.fields)
        return

class GftExmHeaderGroupObjectHelper:
    def __init__(self):
        self.objlist = []
        return

    def Generate(self, topospec):
        gftexm_spec = getattr(topospec, 'gftexm', None)
        if gftexm_spec is None: return

        spec = getattr(gftexm_spec, 'hdrgroups', None)
        hgs = spec.Get(Store)

        cfglogger.info("Adding GFT Header Groups to Store.")
        for entry in hgs.header_groups:
            hdrgroup = entry.group
            obj = GftExmHeaderGroupObject()
            obj.Init(hdrgroup)
            self.objlist.append(obj)
            Store.objects.Set(obj.GID(), obj)
        return

    def main(self, topospec):
        self.Generate(topospec)
        return

    def GetAll(self):
        return self.objlist

GftHeaderGroupHelper = GftExmHeaderGroupObjectHelper()
