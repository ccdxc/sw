#! /usr/bin/python3

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr       as resmgr

from infra.common.logging   import logger
from infra.factory.store    import FactoryStore

import model_sim.src.model_wrap as model_wrap

import iris.config.objects.nvme.ns as ns

from infra.common.glopts import GlobalOptions

class NvmeLifObject(base.ConfigObjectBase):
    def __init__(self, lif, spec):
        super().__init__()
        self.lif = lif  
        self.spec = spec
        self.GID("Nvme%s" %lif.GID())

        #Namespaces
        self.nsdb = objects.ObjectDatabase()
        self.obj_helper_ns = ns.NsObjectHelper()
        self.obj_helper_ns.Generate(self, spec.max_ns)
        if len(self.obj_helper_ns.ns_list):
            self.nsdb.SetAll(self.obj_helper_ns.ns_list)
        
        self.Show()
        return

    def Show(self):
        logger.info("- NVMe LIF   : %s" % self.GID())
        logger.info("   enable: %s max_ns: %d max_sess: %d host_page_size: %d" \
                     %(self.spec.enable, self.spec.max_ns, self.spec.max_sess, self.spec.host_page_size))
        return

    def Configure(self):
        if len(self.obj_helper_ns.ns_list):
            self.obj_helper_ns.Configure()
        return
