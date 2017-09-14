#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.resmgr            as resmgr
from config.store               import Store
from infra.common.logging       import cfglogger

import config.hal.api           as halapi
import config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

import config.objects.qp        as qp
import config.objects.mr        as mr

from infra.common.glopts import GlobalOptions

class PdObject(base.ConfigObjectBase):
    def __init__(self, ep, spec, pd_id):
        super().__init__()
        self.ep = ep
        self.id = pd_id 
        self.GID("PD%04d" % self.id)
        self.spec = spec
        self.remote = ep.remote

        self.mrs = objects.ObjectDatabase(cfglogger)
        self.obj_helper_mr = mr.MrObjectHelper()
        mr_spec = spec.mr.Get(Store)
        self.obj_helper_mr.Generate(self, mr_spec)
        if len(self.obj_helper_mr.mrs):
            self.mrs.SetAll(self.obj_helper_mr.mrs)

        self.qps = objects.ObjectDatabase(cfglogger)
        self.obj_helper_qp = qp.QpObjectHelper()
        qp_spec = spec.qp.Get(Store)
        self.obj_helper_qp.Generate(self, qp_spec)
        self.qps.SetAll(self.obj_helper_qp.qps)

        self.Show()
        return

    def AddMr(self, mr):
        self.obj_helper_mr.AddMr(mr)
        self.mrs.Add(mr)

    def Configure(self):
        self.obj_helper_mr.Configure()
        self.obj_helper_qp.Configure()

    def Show(self):
        cfglogger.info('PD: %s EP: %s Remote: %s' %(self.GID(), self.ep.GID(), self.remote))
        cfglogger.info('Qps: %d Mrs: %d' %(len(self.obj_helper_qp.qps), len(self.obj_helper_mr.mrs)))

class PdObjectHelper:
    def __init__(self):
        self.pds = []

    def Generate(self, ep, spec):
        count = spec.count
        cfglogger.info("Creating %d Pds. for EP:%s" %\
                       (count, ep.GID()))
        for i in range(count):
            pd_id = i if ep.remote else ep.intf.lif.GetPd()
            pd = PdObject(ep, spec, pd_id)
            self.pds.append(pd)

    def Configure(self):
        cfglogger.info("Configuring %d Pds." % len(self.pds)) 
        for pd in self.pds:
            pd.Configure()
