#! /usr/bin

import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import infra.config.base        as base

import config.hal.defs          as haldefs
import config.hal.api           as halapi

from config.store               import Store
from infra.common.logging       import cfglogger
from config.objects.tcp_proxy_cb        import TcpCbHelper

class ProxyCbServiceObject(base.ConfigObjectBase):
    def __init__(self, session):
        super().__init__()
        self.Clone(Store.templates.Get("PROXYCB"))
        self.session = session
        return

    def Show(self):
        cfglogger.info("Service List for %s" % self.GID())
        return

    def PrepareHALRequestSpec(self, req_spec):
        if self.session.iflow.label == 'TCP-PROXY':
            req_spec.meta.tenant_id = self.session.initiator.ep.tenant.id
            req_spec.spec.key_or_handle.proxy_id = 0
            req_spec.spec.proxy_type = 1
            if req_spec.__class__.__name__ == 'ProxyFlowConfigRequest':
                req_spec.proxy_en = True
            self.session.iflow.PrepareHALRequestSpec(req_spec)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        if resp_spec.__class__.__name__ == 'ProxyGetFlowInfoResponse':
            cfglogger.info("Received response %s qid1 %d qid2 %d qtype %d lif_id %d" % (haldefs.common.ApiStatus.Name(resp_spec.api_status), resp_spec.qid1, resp_spec.qid2, resp_spec.qtype, resp_spec.lif_id))
            self.qid = resp_spec.qid1
            self.other_qid = resp_spec.qid2
        else:
            cfglogger.info("Received response %s" % (haldefs.common.ApiStatus.Name(resp_spec.api_status)))
        return

class ProxyCbServiceObjectHelper:
    def __init__(self):
        self.proxy_service_list = []
        return

    def Configure(self):
        for proxycb in self.proxy_service_list:
            if proxycb.session.iflow.label == 'TCP-PROXY':
                lst = []
                lst.append(proxycb)
                halapi.ConfigureProxyCbService(lst)
                halapi.GetQidProxycbGetFlowInfo(lst)
                #TcpCbHelper.main(proxycb.qid)
        return

    def GetFlowInfo(self, session):
        lst = []
        proxyFlowObj = ProxyCbServiceObject(session)
        lst.append(proxyFlowObj)
        halapi.GetQidProxycbGetFlowInfo(lst)
        return proxyFlowObj.qid 

    def GetOtherFlowInfo(self, session):
        lst = []
        proxyFlowObj = ProxyCbServiceObject(session)
        lst.append(proxyFlowObj)
        halapi.GetQidProxycbGetFlowInfo(lst)
        return proxyFlowObj.other_qid 
   
    def main(self, sessions):
        for session in sessions:
            proxyFlowObj = ProxyCbServiceObject(session)
            self.proxy_service_list.append(proxyFlowObj)
        self.Configure()

ProxyCbServiceHelper = ProxyCbServiceObjectHelper()

