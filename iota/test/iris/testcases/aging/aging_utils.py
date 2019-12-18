#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as netagent_cfg_api
import yaml
import pdb

def timetoseconds(time):
    seconds = 0
    h = time.find("h")
    if (h != -1):
        seconds += int(time[0:h])*3600
        h += 1
    else:
        h = 0

    m = time.find("m")
    if (m != -1):
        seconds += int(time[h:m])*60
        m += 1
    else:
        m = 0

    s = time.find("s")
    if (s != -1):
        seconds += int(time[m:s])

    return (seconds)

def get_timeout(timeout):

    #Query will get the reference of objects on store
    store_profile_objects = netagent_cfg_api.QueryConfigs(kind='SecurityProfile')
    if len(store_profile_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return api.types.status.FAILURE

    for object in store_profile_objects:
        if (timeout == 'tcp-timeout'):
            return timetoseconds(object.spec.timeouts.tcp)
        if (timeout == 'udp-timeout'):
            return timetoseconds(object.spec.timeouts.udp)
        if (timeout == 'icmp-timeout'):
            return timetoseconds(object.spec.timeouts.icmp)
        if (timeout == 'tcp-half-close'):
            return timetoseconds(object.spec.timeouts.tcp_half_close)
        if (timeout == 'tcp-close'):
            return timetoseconds(object.spec.timeouts.tcp_close)
        if (timeout == 'tcp-connection-setup'):
            return timetoseconds(object.spec.timeouts.tcp_connection_setup)
        if (timeout == 'tcp-drop'):
            return timetoseconds(object.spec.timeouts.tcp_drop)
        if (timeout == 'udp-drop'):
            return timetoseconds(object.spec.timeouts.udp_drop)
        if (timeout == 'icmp-drop'):
            return timetoseconds(object.spec.timeouts.icmp_drop)

    return 0

def get_timeout_val(timeout):

    #Query will get the reference of objects on store
    store_profile_objects = netagent_cfg_api.QueryConfigs(kind='SecurityProfile')
    val = ""
    if len(store_profile_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return val

    for object in store_profile_objects:
        if (timeout == 'tcp-timeout'):
            val = object.spec.timeouts.tcp
        if (timeout == 'udp-timeout'):
            val = object.spec.timeouts.udp
        if (timeout == 'icmp-timeout'):
            val = object.spec.timeouts.icmp
        if (timeout == 'tcp-half-close'):
            val = object.spec.timeouts.tcp_half_close
        if (timeout == 'tcp-close'):
            val = object.spec.timeouts.tcp_close
        if (timeout == 'tcp-connection-setup'):
            val = object.spec.timeouts.tcp_connection_setup
        if (timeout == 'tcp-drop'):
            val = object.spec.timeouts.tcp_drop
        if (timeout == 'udp-drop'):
            val = object.spec.timeouts.udp_drop
        if (timeout == 'icmp-drop'):
            val = object.spec.timeouts.icmp_drop

    return val

def update_timeout(timeout, val):
    #Query will get the reference of objects on store
    store_profile_objects = netagent_cfg_api.QueryConfigs(kind='SecurityProfile')
    if len(store_profile_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return api.types.status.FAILURE

    for object in store_profile_objects:
        if (timeout == 'tcp-timeout'):
            object.spec.timeouts.tcp = val
        if (timeout == 'udp-timeout'):
            object.spec.timeouts.udp = val
        if (timeout == 'icmp-timeout'):
            object.spec.timeouts.icmp = val
        if (timeout == 'tcp-half-close'):
            object.spec.timeouts.tcp_half_close = val
        if (timeout == 'tcp-close'):
            object.spec.timeouts.tcp_close = val
        if (timeout == 'tcp-connection-setup'):
            object.spec.timeouts.tcp_connection_setup = val
        if (timeout == 'tcp-drop'):
            object.spec.timeouts.tcp_drop = val
        if (timeout == 'udp-drop'):
            object.spec.timeouts.udp_drop = val
        if (timeout == 'icmp-drop'):
            object.spec.timeouts.icmp_drop = val

    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_profile_objects)

    return api.types.status.SUCCESS

def update_field(field, val):
    #Query will get the reference of objects on store
    store_profile_objects = netagent_cfg_api.QueryConfigs(kind='SecurityProfile')
    if len(store_profile_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return api.types.status.FAILURE

    for object in store_profile_objects:
        if (field == 'enable-connection-tracking'):
            object.spec.enable_connection_tracking = val
   
    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_profile_objects)

    return api.types.status.SUCCESS

def update_sgpolicy(src, dst, proto, dport, action="DENY"):
    #Query will get the reference of objects on store
    store_policy_objects = netagent_cfg_api.QueryConfigs(kind='NetworkSecurityPolicy')
    if len(store_policy_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return api.types.status.FAILURE

    for object in store_policy_objects:
        for rule in object.spec.policy_rules:
            if (rule.action == action and rule.destination.proto_ports != None):
                for app_config in rule.destination.proto_ports:
                    if app_config.protocol == proto:
                       app_config.port = str(dport)

    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_policy_objects)

    return api.types.status.SUCCESS

def get_tickleinfo(cmd):
    if cmd.stdout == '' or cmd.stdout.find('API_STATUS_NOT_FOUND') != -1:
       return 0, 0, 0, 0

    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
            stats = data['stats']
            init_flow = stats['initiatorflowstats']
            itickles = init_flow['numtcpticklessent']
            iresets = init_flow['numtcprstsent']
             
            resp_flow = stats['responderflowstats']
            rtickles = resp_flow['numtcpticklessent']
            rresets = resp_flow['numtcprstsent']
            return itickles, iresets, rtickles, rresets

    return 0, 0, 0, 0

def get_haltimeout(timeout, cmd):
    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
           if (timeout == 'tcp-timeout'):
               return int(data['spec']['tcptimeout'])
           if (timeout == 'udp-timeout'):
               return int(data['spec']['udptimeout'])
           if (timeout == 'icmp-timeout'):
               return int(data['spec']['icmptimeout'])
           if (timeout == 'tcp-half-close'):
               return int(data['spec']['tcphalfclosedtimeout'])
           if (timeout == 'tcp-close'):
               return int(data['spec']['tcpclosetimeout'])
           if (timeout == 'tcp-connection-setup'):
               return int(data['spec']['tcpcnxnsetuptimeout'])
           if (timeout == 'tcp-drop'):
               return int(data['spec']['tcpdroptimeout'])
           if (timeout == 'udp-drop'):
               return int(data['spec']['udpdroptimeout'])
           if (timeout == 'icmp-drop'):
               return int(data['spec']['icmpdroptimeout'])
           return 0

def get_sess_handle(cmd):
    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
             return data['status']['sessionhandle'] 
