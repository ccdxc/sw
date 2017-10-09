# /usr/bin/python3
import pdb

import infra.config.base        as base
import config.resmgr            as resmgr
import infra.common.objects     as objects

from config.store               import Store
from infra.common.logging       import cfglogger

class OifListObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('OIF_LIST'))
        return

    def Init(self):
        self.id = resmgr.OifListIdAllocator.get()
        gid = "OifList%04d" % self.id
        self.GID(gid)
        self.oifs = objects.ObjectDatabase(cfglogger)
        self.Show()
        return

    def Show(self):
        cfglogger.info("Creating OifList = %s" % (self.GID()))
        return

    def Summary(self):
        summary = ''
        summary += 'GID: %s\n' % self.GID()
        num = 0
        for int in self.oifs.GetAllInList():
            summary += '                    Oifs[%d]: %s\n' % (num, int.Summary())
            num += 1
        return summary

    def addOif(self, oif):
        for int in self.oifs.GetAllInList():
            if int == oif:
                return

        self.oifs.Add(oif)
        return

# Helper Class to Generate/Configure/Manage OifList Objects.
class OifListObjectHelper:
    def __init__(self):
        self.oiflist = None
        return

    def Generate(self, segment):
        oiflist = OifListObject()
        oiflist.Init()
        for endpoint in segment.obj_helper_ep.eps:
            oiflist.addOif(endpoint.GetInterface())

        cfglogger.info("%s" % (oiflist.Summary()))
        self.oiflist = oiflist
        return

    def GetOifList(self):
        return self.oiflist
