#! /usr/bin/python3
import enum

import iota.harness.api as api
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.pdsctl as pdsctl
import learn_pb2 as learn_pb2


__nodes = api.GetNaplesHostnames()
__dirty = True
__stdout = {}
__store = {}

def __eventtype_list_to_dict(obj, event_name):
    obj[event_name] = { e['eventtype'] : e['count'] for e in obj[event_name] }

def __apioptype_list_to_dict(obj, apiop_name):
    obj[apiop_name] = { a['apioptype'] : a['count'] for a in obj[apiop_name] }

def __normalize_stats():
    for node in __nodes:
        for event_type in ['maclearnevents', 'iplearnevents']:
            __eventtype_list_to_dict(__store[node], event_type)
        for apiop_type in ['vnicops', 'remotel2mappings', 'locall3mappings', 'remotel3mappings']:
            __apioptype_list_to_dict(__store[node], apiop_type)

def Clear():
    learn_utils.ClearLearnStatistics(__nodes)
    global __dirty
    __dirty = True

def Show(nodes=[]):
    def __output_lines(name, output):
        lines = output.split('\n')
        if len(lines): api.Logger.header(name)
        for line in lines:
            api.Logger.info(line)

    if __dirty:
        Fetch()

    nodes = nodes if nodes else __nodes
    for node in nodes:
        if node not in __stdout:
            continue
        api.Logger.SetNode(node)
        __output_lines('LEARN STATISTICS', __stdout[node])
        api.Logger.SetNode(None)

def Fetch():
    global __store
    global __stdout
    global __dirty
    __store = learn_utils.GetLearnStatistics(__nodes)
    __normalize_stats()
    for node in __nodes:
        _, resp = pdsctl.ExecutePdsctlCommand(node, "show learn statistics", yaml=False, print_op=False)
        __stdout[node] = resp
    __dirty = False

#
# MAC learn and move counters
#
def GetNewLocalMacLearnEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_NEW_LOCAL]

def GetNewRemoteMacLearnEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_NEW_REMOTE]

def GetL2LMacMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_L2L_MOVE]

def GetR2LMacMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_R2L_MOVE]

def GetL2RMacMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_L2R_MOVE]

def GetR2RMacMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_R2R_MOVE]

def GetRemoteMacDeleteEventCount(node):
    if __dirty: Fetch()
    return __store[node]['maclearnevents'][learn_pb2.LEARN_EVENT_DELETE]

#
# IP learn and move counters
#
def GetNewLocalIpLearnEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_NEW_LOCAL]

def GetNewRemoteIpLearnEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_NEW_REMOTE]

def GetL2LIpMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_L2L_MOVE]

def GetR2LIpMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_R2L_MOVE]

def GetL2RIpMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_L2R_MOVE]

def GetR2RIpMoveEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_R2R_MOVE]

def GetRemoteIpDeleteEventCount(node):
    if __dirty: Fetch()
    return __store[node]['iplearnevents'][learn_pb2.LEARN_EVENT_DELETE]

#
# VNIC API
#
def GetVnicCreateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['vnicops'][learn_pb2.LEARN_API_OP_CREATE]

def GetVnicDeleteApiCount(node):
    if __dirty: Fetch()
    return __store[node]['vnicops'][learn_pb2.LEARN_API_OP_DELETE]

def GetVnicUpdateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['vnicops'][learn_pb2.LEARN_API_OP_UPDATE]

#
# Local IP mapping API
#
def GetLocalIpMappingCreateEventApiCount(node):
    if __dirty: Fetch()
    return __store[node]['locall3mappings'][learn_pb2.LEARN_API_OP_CREATE]

def GetLocalIpMappingDeleteEventApiCount(node):
    if __dirty: Fetch()
    return __store[node]['locall3mappings'][learn_pb2.LEARN_API_OP_DELETE]

def GetLocalIpMappingUpdateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['locall3mappings'][learn_pb2.LEARN_API_OP_UPDATE]

#
# Remote MAC mapping API
#
def GetRemoteMacMappingCreateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['remotel2mappings'][learn_pb2.LEARN_API_OP_CREATE]

def GetRemoteMacMappingDeleteApiCount(node):
    if __dirty: Fetch()
    return __store[node]['remotel2mappings'][learn_pb2.LEARN_API_OP_DELETE]

def GetRemoteMacMappingUpdateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['remotel2mappings'][learn_pb2.LEARN_API_OP_UPDATE]

#
# Remote IP mapping API
#
def GetRemoteIpMappingCreateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['remotel3mappings'][learn_pb2.LEARN_API_OP_CREATE]

def GetRemoteIpMappingDeleteApiCount(node):
    if __dirty: Fetch()
    return __store[node]['remotel3mappings'][learn_pb2.LEARN_API_OP_DELETE]

def GetRemoteIpMappingUpdateApiCount(node):
    if __dirty: Fetch()
    return __store[node]['remotel3mappings'][learn_pb2.LEARN_API_OP_UPDATE]

