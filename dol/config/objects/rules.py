import pdb
import infra.config.base        as base
import config.hal.defs          as haldefs
import infra.common.defs        as defs

from infra.common.logging import logger

class SvcObject(base.ConfigObjectBase):
    def __init__(self, spec):
        super().__init__()
        self.proto = spec.proto
        self.dst_port =  getattr(spec, 'dst_port',0)
        self.icmp_msg_type = getattr(spec, 'icmp_type', 0)
        self.alg = getattr(spec, 'alg',"NONE")
        return

    def PrepareHALRequestSpec(self, req_spec):
        req_spec.match.protocol = self.proto
        proto_str = 'IPPROTO_' + defs.ipprotos.str(self.proto)
        logger.info("prot %s" %(proto_str))
        if proto_str in ('IPPROTO_ICMP', 'IPPROTO_ICMPV6'):
            req_spec.match.app_match.icmp_info.icmp_type = self.icmp_msg_type
        else:
            app_match = req_spec.match.app_match.add()
            dst_port_range = app_match.port_info.dst_port_range.add()
            dst_port_range.port_low = self.dst_port
            dst_port_range.port_high= self.dst_port

        if self.alg is not "None":
            app_data = req_spec.action.app_data.add();
            alg_name = "APP_SVC_" + self.alg
            app_data.alg = haldefs.nwsec.ALGName.Value(alg_name)
        return

class RuleObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.svc_objs =[]
        self.apps =[]

    def Init(self, spec):
        self.action = spec.action.upper()
        self.log    = spec.log
        for svc_spec in spec.svcs:
            svc_obj = SvcObject(svc_spec.svc)
            self.svc_objs.append(svc_obj)
        if hasattr(spec, 'apps'):
            for app_spec in spec.apps:
                self.apps.append(app_spec.app.upper())
        return

    def PrepareHALRequestSpec(self, req_spec):
        if len(self.apps) > 0:
            req_spec.appid.extend(self.apps)

        action = "FIREWALL_ACTION_" + self.action
        req_spec.action.sec_action = haldefs.nwsec.FirewallAction.Value(action)
        req_spec.action.log_action = self.log
        return

