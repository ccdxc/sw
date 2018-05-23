#! /usr/bin/python3
import pdb
import copy
import itertools
import infra.config.base           as base
import config.hal.api              as halapi
import config.hal.defs             as haldefs
import config.objects.rules        as rules
import config.hal.defs             as haldefs
import config.resmgr               as resmgr

from    config.store               import Store
from    infra.common.logging       import logger
from    config.objects.security_group import SecurityGroupObject
from    config.objects.tenant         import TenantObject

class SGPairObject(base.ConfigObjectBase):
    def __init__(self, sg_id, peer_sg_id, ten_id, obj):
        super().__init__()
        self.Clone(Store.templates.Get('SECURITY_GROUP_PAIR'))
        self.sg_id = sg_id
        self.peer_sg_id = peer_sg_id
        self.GID("TEN%04dSGPAIRSG%04dSG%04d" % (ten_id, sg_id,peer_sg_id))
        self.obj = obj
        return

    def PrepareHALRequestSpec(self, req_spec):
        for rule_obj in self.obj.in_rules:
            for svc_obj in rule_obj.svc_objs:
                rule_req_spec = req_spec.rule.add()
                rule_req_spec.match.src_sg.append(self.sg_id)
                rule_req_spec.match.dst_sg.append(self.peer_sg_id)
                rule_obj.PrepareHALRequestSpec(rule_req_spec)
                svc_obj.PrepareHALRequestSpec(rule_req_spec)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        return

    def PrepareHALGetRequestSpec(self, get_req_spec):
        #pl_id = haldefs.kh.SecurityGroupPolicyId()
        #pl_id.security_group_id = self.sg_id
        #pl_id.peer_security_group_id = self.peer_sg_id
        #get_req_spec.key_or_handle.security_group_policy_id = pl_id
        return

    def ProcessHALGetResponse(self, get_req_spec, get_resp):
        return

    def Get(self):
        halapi.GetSecurityGroupPolicies([self])

class SGPairObjectHelper:
    def __init__(self):
        self.sgpair_objlist = []

    def __get_sg_pair(self, sp):
            print("Tenant id: {}", sp.tenant.id)
            sglist = sp.tenant.GetSecurityGroups()
            for sg1,sg2 in itertools.combinations_with_replacement(sglist, 2):
                    yield ([sg1, sg2], sp.tenant.id)

    def Generate(self, sp):
        for sgpair,tenant_id in self.__get_sg_pair(sp):
            if (sgpair[0].id != sgpair[1].id):
                sgpair_obj = SGPairObject(sgpair[0].id, sgpair[1].id,tenant_id,
                                        sp)
                sgpair_obj_rev = SGPairObject(sgpair[1].id, sgpair[0].id,tenant_id,
                                        sp)
                self.sgpair_objlist.extend([sgpair_obj, sgpair_obj_rev])
            else:
                sgpair_obj = SGPairObject(sgpair[0].id, sgpair[1].id,tenant_id,
                                        sp)
                self.sgpair_objlist.extend([sgpair_obj])
        Store.objects.SetAll(self.sgpair_objlist)
    def PrepareHALRequestSpec(self, req_spec):
        for sg_obj in self.sgpair_objlist:
            sg_obj.PrepareHALRequestSpec(req_spec)


class SecurityGroupPolicyObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('SECURITY_POLICY'))
        self.in_rules=[]
        self.eg_rules=[]
        self.pol_id = resmgr.SecurityPolicyIDAllocator.get()
        return

    def Init(self, spec, ten):
        if spec.in_rules:
            for rule_spec in spec.in_rules:
                rule_obj = rules.RuleObject()
                rule_obj.Init(rule_spec.rule)
                self.in_rules.append(rule_obj)
        self.tenant = ten
        self.GID("TEN%04dPOL%04s" % (ten.id,spec.id))
        return

    def PrepareHALRequestSpec(self, req_spec):
        #Fill key
        pl_id = haldefs.kh.SecurityPolicyKey()
        pl_id.security_policy_id = self.pol_id
        pl_id.vrf_id_or_handle.vrf_id = self.tenant.id
        req_spec.policy_key_or_handle.security_policy_key.CopyFrom(pl_id)
        self.sg_pair.PrepareHALRequestSpec(req_spec)
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        self.hal_handle = resp_spec.policy_status.security_policy_handle
        logger.info(" - SecurityPolicy %s = %s (HDL = 0x%x)" %\
                       (self.GID(), \
                        haldefs.common.ApiStatus.Name(resp_spec.api_status),
                        self.hal_handle))
        return

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

# Helper Class to Generate/Configure/Manage Security Group Objects
class SecurityGroupPolicyObjectHelper:
    def __init__(self):
        self.sps = []
        self.sg_pair = None
        return

    def Configure(self):
        sgpolicylist  = Store.objects.GetAllByClass(SecurityGroupPolicyObject)
        if len(sgpolicylist):
            logger.info("Confguring %d SecurityGroupPolicies." %len(sgpolicylist))
            halapi.ConfigureSecurityGroupPolicies(sgpolicylist)
        return

    def GetSGType(self, flow_obj, sep, dep):
        src_sg_list = getattr(sep, 'sgs', None)
        dst_sg_list = getattr(dep, 'sgs', None)
        if src_sg_list is None or dst_sg_list is None:
            return 'INTRA'
        for src_sg, dst_sg in itertools.product(src_sg_list, dst_sg_list):
            if src_sg == dst_sg: return 'INTRA'
        return 'INTER'

    def __eval_policy(self, sec_policy, flow_obj):
        for rule_obj in sec_policy.in_rules:
            for svc_obj in rule_obj.svc_objs:
                if (svc_obj.proto == 0): return rule_obj.action
                if (svc_obj.proto == flow_obj.proto):
                    if (flow_obj.icmpcode and
                        flow_obj.icmpcode == svc_obj.icmp_msg_type or
                        (flow_obj.dport == svc_obj.dst_port)):
                            return rule_obj.action
        return None

    def GetAction(self, flow_obj, sep, dep):
        src_sg_list  = getattr(sep, 'sgs', None)
        dst_sg_list  = getattr(dep, 'sgs', None)
        if src_sg_list is None or dst_sg_list is None:
            return None
        if (sep.tenant.id != dep.tenant.id):
            print("What does this mean??")
        for src_sg, dst_sg in itertools.product(src_sg_list, dst_sg_list):
            if src_sg == dst_sg: continue
            try:
                sgpair_obj = Store.objects.Get("TEN" + str(sep.tenant.id) + "SGPAIR" + str(src_sg) + str(dst_sg))
            except KeyError:
                continue

            if (sgpair_obj):
                #Get Security Policy
                sp = sgpair_obj.obj
                return self.__eval_policy(sp, flow_obj)
        return None

    def Generate(self, topospec):
        sps = getattr(topospec, 'security_policy', None)
        if sps is None:
            return
        spec = topospec.security_policy.Get(Store)
        logger.info("Creating %d SecurityPolicies." % len(spec.policies))
        policy_list = spec.policies
        tenant_list = Store.objects.GetAllByClass(TenantObject)
        for tenant,p in itertools.product(tenant_list, policy_list):
            policy = SecurityGroupPolicyObject()
            policy.Init(p.policy, tenant)
            self.sps.append(policy)
            policy.sg_pair = SGPairObjectHelper()
            policy.sg_pair.Generate(policy)
        Store.objects.SetAll(self.sps)

    def main(self, topospec):
        self.Generate(topospec)
        self.Configure()
        return

SecurityGroupPolicyHelper = SecurityGroupPolicyObjectHelper()
