#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import iris.config.resmgr            as resmgr
from iris.config.store               import Store
from infra.common.logging       import logger

import iris.config.hal.api           as halapi
import iris.config.hal.defs          as haldefs
import rdma_pb2                 as rdma_pb2

import iris.config.objects.aq        as aq
import iris.config.objects.qp        as qp
import iris.config.objects.mr        as mr
import iris.config.objects.mw        as mw
import iris.config.objects.key       as key
import iris.config.objects.cq        as cq
import iris.config.objects.eq        as eq

from infra.common.glopts import GlobalOptions

class PdObject(base.ConfigObjectBase):
    def __init__(self, ep, spec, pd_id):
        super().__init__()
        self.ep = ep
        self.id = pd_id 
        self.GID("PD%04d" % self.id)
        self.spec = spec
        self.remote = ep.remote
        self.last_type1_mw_id = 0
        self.last_type2_mw_id = 0
        self.last_type1_2_mw_id = 0
        self.last_key_id = 0

        #MRs
        self.mrs = objects.ObjectDatabase()
        self.obj_helper_mr = mr.MrObjectHelper()
        mr_spec = spec.mr.Get(Store)
        self.obj_helper_mr.Generate(self, mr_spec)
        if len(self.obj_helper_mr.mrs):
            self.mrs.SetAll(self.obj_helper_mr.mrs)

        #EQs
        self.eqs = objects.ObjectDatabase()
        self.obj_helper_eq = eq.EqObjectHelper()
        eq_spec = spec.eq.Get(Store)
        self.obj_helper_eq.Generate(self, eq_spec)
        if len(self.obj_helper_eq.eqs):
            self.eqs.SetAll(self.obj_helper_eq.eqs)

        if self.id == 0:
           logger.info("Lite PD. Skipping other config than EQ")
           return

        #AQ
        #Pinning Admin queue to PD 1
        if pd_id == 1:
            self.aqs = objects.ObjectDatabase()
            self.obj_helper_aq = aq.AqObjectHelper()
            aq_spec = spec.aq.Get(Store)
            self.obj_helper_aq.Generate(self, aq_spec)
            if len(self.obj_helper_aq.aqs):
                self.aqs.SetAll(self.obj_helper_aq.aqs)

        #CQs
        self.cqs = objects.ObjectDatabase()
        self.obj_helper_cq = cq.CqObjectHelper()
        cq_spec = spec.cq.Get(Store)
        self.obj_helper_cq.Generate(self, cq_spec)
        if len(self.obj_helper_cq.cqs):
            self.cqs.SetAll(self.obj_helper_cq.cqs)

        #QPs for PD 1 are generated. If CQs are generated in PD, QPs must also be
        #generated to avoid these CQs from being used by QPs of another PD.
        #QPs
        self.qps = objects.ObjectDatabase()
        self.perf_qps = objects.ObjectDatabase()
        self.udqps = objects.ObjectDatabase()
        self.obj_helper_qp = qp.QpObjectHelper()
        qp_spec = spec.qp.Get(Store)
        self.obj_helper_qp.Generate(self, qp_spec)
        if len(self.obj_helper_qp.qps):
            self.qps.SetAll(self.obj_helper_qp.qps)
        if len(self.obj_helper_qp.perf_qps):
            self.perf_qps.SetAll(self.obj_helper_qp.perf_qps)
        if len(self.obj_helper_qp.udqps):
            self.udqps.SetAll(self.obj_helper_qp.udqps)
        logger.info('PD: %s Total UdQps in the PD : %d ' % (self.GID(), len(self.udqps)))
        #pdudqps = self.udqps.GetAll()
        #for tmpqp in pdudqps:
        #    logger.info('   Qps: %s' % (tmpqp.GID()))

        if self.id == 1:
           logger.info("Admin PD. Skipping other config than CQ, EQ")
           return

        #MWs
        self.mws = objects.ObjectDatabase()
        self.obj_helper_mw = mw.MwObjectHelper()
        mw_spec = spec.mw.Get(Store)
        self.obj_helper_mw.Generate(self, mw_spec)
        if len(self.obj_helper_mw.mws):
            self.mws.SetAll(self.obj_helper_mw.mws)

        #Keys
        self.keys = objects.ObjectDatabase()
        self.obj_helper_key = key.KeyObjectHelper()
        key_spec = spec.key.Get(Store)
        self.obj_helper_key.Generate(self, key_spec)
        if len(self.obj_helper_key.keys):
            self.keys.SetAll(self.obj_helper_key.keys)

        self.Show()
        return

    def AddMr(self, mr):
        self.obj_helper_mr.AddMr(mr)
        self.mrs.Add(mr)

    def Configure(self):

        if len(self.obj_helper_mr.mrs):
            self.obj_helper_mr.Configure()
        if len(self.obj_helper_eq.eqs):
            self.obj_helper_eq.Configure()

        if self.id == 0:
           logger.info("Lite PD. Skipping other config than EQ")
           return

        if self.id == 1 and len(self.obj_helper_aq.aqs):
            self.obj_helper_aq.Configure()
        if len(self.obj_helper_cq.cqs):
            self.obj_helper_cq.Configure()

        if self.id == 1:
           logger.info("Admin PD. Config for EQ, CQ and AQ only")
           return

        if len(self.obj_helper_mw.mws):
            self.obj_helper_mw.Configure()
        if len(self.obj_helper_key.keys):
            self.obj_helper_key.Configure()
        if len(self.obj_helper_qp.qps):
            self.obj_helper_qp.Configure()

    def Show(self):
        logger.info('PD: %s EP: %s Remote: %s' %(self.GID(), self.ep.GID(), self.remote))
        logger.info('Qps: %d Perf QPs: %d Mrs: %d Mws: %d Keys: %d' %(len(self.obj_helper_qp.qps), len(self.obj_helper_qp.perf_qps), len(self.obj_helper_mr.mrs), len(self.obj_helper_mw.mws), len(self.obj_helper_key.keys)))
        logger.info('UDQps: %d ' % (len(self.obj_helper_qp.udqps)))
        logger.info('CQs: %d EQs: %d' % (len(self.obj_helper_cq.cqs), len(self.obj_helper_eq.eqs)))

    def GetNewType1_2MW(self):
        i = 0
        for mw in self.obj_helper_mw.mws:
            if (mw.mw_type == 0):
                if self.last_type1_2_mw_id == i:
                    self.last_type1_2_mw_id+=1
                    return mw
                i+=1
        return None

    def GetNewType1MW(self):
        i = 0
        for mw in self.obj_helper_mw.mws:
            if (mw.mw_type == 1):
                if self.last_type1_mw_id == i:
                    self.last_type1_mw_id+=1
                    return mw
                i+=1
        return None

    def GetNewType2MW(self):
        i = 0
        for mw in self.obj_helper_mw.mws:
            if (mw.mw_type == 2):
                if self.last_type2_mw_id == i:
                    self.last_type2_mw_id+=1
                    return mw
                i+=1
        return None

    def GetNewKey(self):
        new_key = self.obj_helper_key.keys[self.last_key_id]
        self.last_key_id += 1
        return new_key

class PdObjectHelper:
    def __init__(self):
        self.pds = []

    def Generate(self, ep, spec):
        count = spec.count
        logger.info("Creating %d Pds. for EP:%s" %\
                       (count, ep.GID()))
        for i in range(count):
            pd_id = i if ep.remote else ep.intf.lif.GetPd()
            pd = PdObject(ep, spec, pd_id)
            self.pds.append(pd)

    def Configure(self):
        logger.info("Configuring %d Pds." % len(self.pds)) 
        for pd in self.pds:
            pd.Configure()
