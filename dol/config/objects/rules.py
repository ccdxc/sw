import pdb
import infra.config.base        as base
import config.hal.defs          as haldefs

from infra.common.logging import cfglogger

class SvcObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.proto = spec.proto
        self.dst_port =  getattr(spec, 'dst_port',None)
        self.icmp_msg_type = getattr(spec, 'icmp_type', 0)
        self.alg = getattr(spec, 'alg',"NONE")
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.ip_protocol = self.proto
        req_spec.dst_port = self.dst_port
        req_spec.icmp_msg_type = self.icmp_msg_type

        alg_name = "APP_SVC_" + self.alg
        req_spec.alg = haldefs.nwsec.ALGName.Value(alg_name.upper())
        return

class RuleObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.svc_objs =[]

    def Init(self, spec):
        self.action = spec.action.upper()
        self.log    = spec.log
        for svc_spec in spec.svcs:
            svc_obj = SvcObject(svc_spec.svc)
            self.svc_objs.append(svc_obj)
        return

    def PrepareHALRequestSpec(self, req_spec):
        for svc_obj in self.svc_objs:
            svc_req_spec  = req_spec.svc.add()
            svc_obj.PrepareHALRequestSpec(svc_req_spec)
        action = "FIREWALL_ACTION_" + self.action
        req_spec.action = haldefs.nwsec.FirewallAction.Value(action)
        req_spec.log    = self.log
        return

