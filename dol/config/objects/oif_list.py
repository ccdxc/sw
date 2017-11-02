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
        self.enic_list = []
        self.uplink_list = []
        self.Show()
        return

    def Show(self):
        cfglogger.info("- OifList:%s NumOifs:%d" %\
                       (self.GID(), len(self.oifs)))
        for intf in self.oifs.GetAllInList():
            cfglogger.info("  - Oif: %s" % (intf.Summary()))
        return

    def addOif(self, oif, remote):
        for int in self.oifs.GetAllInList():
            if int == oif:
                return
        self.oifs.Add(oif)
        if remote:
            self.uplink_list.append(oif)
        else:
            self.enic_list.append(oif)
        return

# Helper Class to Generate/Configure/Manage OifList Objects.
class OifListObjectHelper:
    def __init__(self):
        self.oiflist = None
        return

    def Generate(self, segment):
        oiflist = OifListObject()
        oiflist.Init()
        for endpoint in segment.GetEps():
            if endpoint.remote:
                if segment.pinnedif is not None and\
                   segment.pinnedif != endpoint.GetInterface():
                    continue
            oiflist.addOif(endpoint.GetInterface(), endpoint.remote)
        self.oiflist = oiflist
        return

    def GetOifList(self):
        return self.oiflist
