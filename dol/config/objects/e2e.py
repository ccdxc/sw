#! /usr/bin/python3

import pdb
import copy
import os
import json

import infra.common.defs            as defs
import infra.common.objects         as objects
import infra.common.utils           as utils
import infra.config.base            as base

import config.resmgr                  as resmgr
import config.objects.endpoint        as endpoint
import config.objects.tenant          as tenant
import config.objects.collector       as collector

from config.store                       import Store
from infra.common.logging               import logger
import infra.e2e.main as E2E

class E2EObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('E2E'))
        return

    def Init(self, src_eps, dst_eps):
        self.id = resmgr.E2EIdAllocator.get()
        self.GID("E2E:%d" % self.id)
        self.src_eps = src_eps
        self.dst_eps = dst_eps

        self.Show()
        return defs.status.SUCCESS

    def __print_eps(self, eps):
        for ep in eps:
            logger.info("\t EP: %s, type : %s" % (ep.GID(),
                            "Remote" if ep.IsRemote() else "Local")) 
        
    def Show(self):
        logger.info("Created E2E with GID:%s" % self.GID())
        logger.info("SRC EPS:")
        self.__print_eps(self.src_eps)
        logger.info("DST EPS:")
        self.__print_eps(self.dst_eps)

    def SetupTestcaseConfig(self, obj):
        obj.src_endpoints = self.src_eps
        obj.dst_endpoints = self.dst_eps
        
    def __cleanup_eps(self):
        for ep in self.src_eps:
            E2E.CleanUp(ep.GID())
        for ep in self.dst_eps:
            E2E.CleanUp(ep.GID())
            
    def TearDownTestcaseConfig(self, obj):
        self.__cleanup_eps()

class E2EObjectHelper:
    def __init__(self):
        pass

    def __get_eps(self):
        eps = []
        tenants = Store.objects.GetAllByClass(tenant.TenantObject)
        for t in tenants:
            eps += t.GetEps()
        return eps

    def __get_matching_EpPairs(self, selectors = None):
        ssns = []
        for ssn in self.objs:
            if ssn.IsFilterMatch(selectors):
                ssns.append(ssn)
        if selectors.maxEpPairs == None:
            return ssns
        if selectors.maxEpPairs >= len(ssns):
            return ssns
        return ssns[:selectors.maxEpPairs]

    def GenerateMatchingConfigObject(self, selectors = None):
        eps = self.__get_eps()
        src_eps = []
        dst_eps = []
        for ep in eps:
            if len(src_eps) < selectors.src_endpoints.max and \
                ep.IsFilterMatch(selectors.src_endpoints.type) and \
                    ep.segment.IsFilterMatch(selectors.segment) and \
                    ep.segment.tenant.IsFilterMatch(selectors.tenant):
                src_eps.append(ep)
            elif len(dst_eps) < selectors.dst_endpoints.max and \
                ep.IsFilterMatch(selectors.dst_endpoints.type) and \
                    ep.segment.IsFilterMatch(selectors.segment) and \
                    ep.segment.tenant.IsFilterMatch(selectors.tenant):
                dst_eps.append(ep)
        if (src_eps and dst_eps):
            obj = E2EObject()
            obj.Init(src_eps, dst_eps)
            return obj

E2EHelper = E2EObjectHelper()

def GenerateMatchingConfigObject(selectors):
    return E2EHelper.GenerateMatchingConfigObject(selectors)
