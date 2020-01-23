#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as netagent_cfg_api
import copy
import yaml
import pdb

APP_SVC_TFTP = 1
APP_SVC_FTP  = 2
APP_SVC_DNS  = 3
APP_SVC_SIP  = 4
APP_SVC_SUN_RPC = 5
APP_SVC_MSFT_RPC = 6
APP_SVC_RTSP = 7

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

def get_sess_handle(cmd):
    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    for data in yaml_out:
        if data is not None and isinstance(data, list):
             return data['status']['sessionhandle']
    return 0

###################Remove after rebase################


def GetThreeWorkloads():
    triplet = []
    for w1 in api.GetWorkloads():
        for w2 in api.GetWorkloads():
            for w3 in api.GetWorkloads():
                if id(w1) == id(w2) or \
                   id(w2) == id(w3) or id(w1) == id(w3): continue
                if w1.uplink_vlan != w2.uplink_vlan or \
                   w2.uplink_vlan != w3.uplink_vlan or \
                   w1.uplink_vlan != w3.uplink_vlan: continue
                triplet.append((w1, w2, w3))
    return triplet

def ForceReleasePort(port, node):
   req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
   api.Trigger_AddCommand(req, node.node_name, node.workload_name, "sudo fuser -k %s"%(port))
   trig_resp = api.Trigger(req)
   term_resp = api.Trigger_TerminateAllCommands(trig_resp)
   resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp) 
   for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

   return api.types.status.SUCCESS

def update_sgpolicy(app_name, allowDefault=False):
   #Query will get the reference of objects on store
    store_policy_objects = netagent_cfg_api.QueryConfigs(kind='NetworkSecurityPolicy')
    if len(store_policy_objects) == 0:
        api.Logger.error("No SG Policy objects in store")
        return api.types.status.FAILURE

    for object in store_policy_objects:
        rules = len(object.spec.policy_rules)
        if (rules == 0):
            continue
        #We dont want to keep updating the same policy
        defaultRule = object.spec.policy_rules.pop()
        if app_name != None:
            if (hasattr(object.spec.policy_rules[rules-2], 'app_name') and \
                object.spec.policy_rules[rules-2].app_name == app_name):
                continue
            newRule = copy.deepcopy(object.spec.policy_rules[0])
            newRule.source.addresses = ['any']
            newRule.destination.addresses = ['any']
            newRule.app_name = app_name
            newRule.destination.app_configs = None
            newRule.destination.proto_ports = None
            newRule.action = 'PERMIT'
            object.spec.policy_rules.append(newRule)

        if allowDefault == False:
            defaultRule.action = 'DENY'
        else:
            defaultRule.action = 'PERMIT'
        object.spec.policy_rules.append(defaultRule)

    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_policy_objects)

def update_app(app, timeout, field=None, val=None, isstring=False):
   #Query will get the reference of objects on store
    store_app_objects = netagent_cfg_api.QueryConfigs(kind='App')
    if len(store_app_objects) == 0:
        api.Logger.error("No App objects in store")
        return api.types.status.FAILURE

    for object in store_app_objects:
        if object.meta.name == app:
            object.spec.app_idle_timeout = timeout
            if field != None:
                obj = 'object.spec.alg' + '.' + app + '.' + field
                if isstring == True:
                    exec(obj + "=" + "\'%s\'"%(val))
                else:
                    exec(obj + "=" + val)

    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_app_objects)

    return api.types.status.SUCCESS

def get_meminfo(cmd, name):
    meminfo = []
    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
            if name in data['spec']['name']:
                stats = data['stats']
                meminfo.append({'inuse': stats['numelementsinuse'], 'allocs': stats['numallocs'], 'frees': stats['numfrees']})

    return meminfo

def get_alginfo(cmd, alg):
    alg_info = []
    yaml_out = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
    print(type(yaml_out))
    for data in yaml_out:
        if data is not None:
           status = data['status']
           if status['alg'] == alg:
              alg_info.append(status['alginfo'])

    return alg_info
