from  app.tcp import *
from  app.udp import *
from  app.icmp import *
from  util.proto import *

def getAppInstance(appName=None, proto=None, **kwargs):
    if appName == None and proto == None:
        return None

    if appName:
        if appName.lower() == "tcp":
            return Tcp(**kwargs)
        elif appName.lower() == "udp":
            return Udp(**kwargs)
        elif appName.lower() == "icmp":
            return Icmp(**kwargs)

    if proto:
        if proto.l3Proto == l3_proto_dict['tcp']:
            return Tcp(**kwargs)
        elif proto.l3Proto == l3_proto_dict['udp']:
            return Udp(**kwargs)
        elif proto.l3Proto == l3_proto_dict['icmp']:
            return Icmp(**kwargs)

    return None
