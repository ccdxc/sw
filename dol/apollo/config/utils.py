#! /usr/bin/python3
import sys
import uuid
import ipaddress
import random
import socket
from scapy.layers.l2 import Dot1Q
import time
import yaml
from collections import OrderedDict
import os
import struct
import pdb

import types_pb2 as types_pb2
import tunnel_pb2 as tunnel_pb2
import infra.common.defs as defs
import apollo.config.agent.api as api
from apollo.config.agent.api import ObjectTypes as ObjectTypes
import apollo.config.topo as topo
from infra.common.logging import logger
from apollo.config.store import EzAccessStore
from apollo.config.store import client as EzAccessStoreClient
from infra.common.glopts import GlobalOptions

IP_VERSION_6 = 6
IP_VERSION_4 = 4

L3PROTO_MIN = 0

L4PORT_MIN = 0
L4PORT_MAX = 65535

ICMPTYPE_MIN = 0
ICMPTYPE_MAX = 255

ICMPCODE_MIN = 0
ICMPCODE_MAX = 255

ETHER_HDR_LEN = 14
DOT1Q_HDR_LEN = 4

IPV4_MINADDR = ipaddress.ip_address("0.0.0.0")
IPV4_MAXADDR = ipaddress.ip_address("255.255.255.255")

IPV6_MINADDR = ipaddress.ip_address("0::0")
IPV6_MAXADDR = ipaddress.ip_address("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff")

IPV4_DEFAULT_ROUTE = ipaddress.ip_network("0.0.0.0/0")
IPV6_DEFAULT_ROUTE = ipaddress.ip_network("0::/0")

NAT_ADDR_TYPE_PUBLIC = 0
NAT_ADDR_TYPE_SERVICE = 1

IPPROTO_TO_NAME_TBL = {num:name[8:] for name,num in vars(socket).items() if name.startswith("IPPROTO")}

IF_TYPE_SHIFT = 28
LIF_IF_LIF_ID_MASK = 0xFFFFFF

INVALID_BATCH_COOKIE = 0

def LifId2LifIfIndex(lifid):
    return ((topo.InterfaceTypes.LIF << IF_TYPE_SHIFT) | (lifid))

def LifIfindex2LifId(lififindex):
    return (lififindex & LIF_IF_LIF_ID_MASK)

PENSANDO_NIC_MAC = 0x022222111111
PDS_UUID_MAGIC_BYTE_VAL = 0x4242
PDS_UUID_BYTE_ORDER = "little"
PDS_NODE_UUID_BYTE_ORDER = "big"

PDS_UUID_LEN = 16
PDS_UUID_ID_LEN = 4
PDS_UUID_OBJTYPE_LEN = 2
PDS_UUID_RESERVED_LEN = 2
PDS_UUID_MAGIC_BYTE_LEN = 2
PDS_UUID_SYSTEM_MAC_LEN = 6

PDS_UUID_ID_OFFSET_START = 0
PDS_UUID_ID_OFFSET_END = PDS_UUID_ID_OFFSET_START + PDS_UUID_ID_LEN

PDS_UUID_OBJTYPE_OFFSET_START = PDS_UUID_ID_OFFSET_END
PDS_UUID_OBJTYPE_OFFSET_END = PDS_UUID_OBJTYPE_OFFSET_START + PDS_UUID_OBJTYPE_LEN

PDS_UUID_RESERVED_OFFSET_START = PDS_UUID_OBJTYPE_OFFSET_END
PDS_UUID_RESERVED_OFFSET_END = PDS_UUID_RESERVED_OFFSET_START + PDS_UUID_RESERVED_LEN

PDS_UUID_MAGIC_BYTE_OFFSET_START = PDS_UUID_RESERVED_OFFSET_END
PDS_UUID_MAGIC_BYTE_OFFSET_END = PDS_UUID_MAGIC_BYTE_OFFSET_START + PDS_UUID_MAGIC_BYTE_LEN

PDS_UUID_SYSTEM_MAC_OFFSET_START = PDS_UUID_MAGIC_BYTE_OFFSET_END

PDS_UUID_MAGIC_BYTE = PDS_UUID_MAGIC_BYTE_VAL.to_bytes(PDS_UUID_MAGIC_BYTE_LEN, PDS_UUID_BYTE_ORDER)
# only for SIM
PDS_UUID_SYSTEM_MAC = PENSANDO_NIC_MAC.to_bytes(PDS_UUID_SYSTEM_MAC_LEN, PDS_NODE_UUID_BYTE_ORDER)

class PdsUuid:

    def __init__(self, value, objtype=None, node_uuid=None):

        if node_uuid:
            node_uuid = node_uuid.to_bytes(PDS_UUID_SYSTEM_MAC_LEN, PDS_NODE_UUID_BYTE_ORDER)

        if isinstance(value, int):
            assert (value > 0), "ID value must be greater than zero"
            self.Id = value
            self.Type = objtype
            self.Uuid = PdsUuid.GetUUIDfromId(self.Id, self.Type, node_uuid)
        elif isinstance(value, bytes):
            self.Uuid = value
            self.Id = PdsUuid.GetIdfromUUID(self.Uuid)
        elif isinstance(value, list):
            self.Uuid = bytes(value)
            self.Id = PdsUuid.GetIdfromUUID(self.Uuid)
        else:
            assert 0, f"{type(value)} is NOT supported for PdsUuid class"
        self.UuidStr = PdsUuid.GetUuidString(self.Uuid)


    def __str__(self):
        return f"ID:{self.Id} UUID:{self.UuidStr}"

    def GetUuid(self):
        return self.Uuid

    def GetId(self):
        return self.Id

    @staticmethod
    def GetUuidString(uuid):
        # keep this inline with pds_obj_key_s.str()
        uuid_id = PdsUuid.GetIdfromUUID(uuid, "big")
        uuid_objtype = PdsUuid.GetObjTypefromUUID(uuid, "big")
        uuid_rsvd = PdsUuid.GetReservedValfromUUID(uuid, "big")
        uuid_magic = int.from_bytes(uuid[PDS_UUID_MAGIC_BYTE_OFFSET_START:PDS_UUID_MAGIC_BYTE_OFFSET_END], PDS_UUID_BYTE_ORDER)
        uuid_mac = int.from_bytes(uuid[PDS_UUID_SYSTEM_MAC_OFFSET_START:], PDS_NODE_UUID_BYTE_ORDER)
        uuidstr = f"{uuid_id:08x}-{uuid_objtype:04x}-{uuid_rsvd:04x}-{uuid_magic:04x}-{uuid_mac:012x}"
        return uuidstr

    @staticmethod
    def GetIdfromUUID(uuid, byte_order=PDS_UUID_BYTE_ORDER):
        return int.from_bytes(uuid[PDS_UUID_ID_OFFSET_START:PDS_UUID_ID_OFFSET_END], byte_order)

    @staticmethod
    def GetObjTypefromUUID(uuid, byte_order=PDS_UUID_BYTE_ORDER):
        return int.from_bytes(uuid[PDS_UUID_OBJTYPE_OFFSET_START:PDS_UUID_OBJTYPE_OFFSET_END], byte_order)

    @staticmethod
    def GetReservedValfromUUID(uuid, byte_order=PDS_UUID_BYTE_ORDER):
        return int.from_bytes(uuid[PDS_UUID_RESERVED_OFFSET_START:PDS_UUID_RESERVED_OFFSET_END], byte_order)

    @staticmethod
    def GetUUIDfromId(id, objtype=None, node_uuid=None):
        # uuid is of 16 bytes
        uuid = bytearray(PDS_UUID_LEN)
        if id == 0:
            return bytes(uuid)
        # first 4 bytes ==> id
        uuid[PDS_UUID_ID_OFFSET_START:PDS_UUID_ID_OFFSET_END] = id.to_bytes(PDS_UUID_ID_LEN, PDS_UUID_BYTE_ORDER)
        # next 2 bytes ==> object type (except HAL created objects like lifs & ports)
        if objtype:
            uuid[PDS_UUID_OBJTYPE_OFFSET_START:PDS_UUID_OBJTYPE_OFFSET_END] = objtype.to_bytes(PDS_UUID_OBJTYPE_LEN, PDS_UUID_BYTE_ORDER)
        # next 2 bytes ==> reserved (0x0000)
        # next 2 bytes ==> magic byte (0x4242)
        uuid[PDS_UUID_MAGIC_BYTE_OFFSET_START:PDS_UUID_MAGIC_BYTE_OFFSET_END] = PDS_UUID_MAGIC_BYTE
        # next 6 bytes ==> system mac (0x022222111111)
        if not node_uuid:
            node_uuid = PDS_UUID_SYSTEM_MAC
        uuid[PDS_UUID_SYSTEM_MAC_OFFSET_START:] = node_uuid
        return bytes(uuid)

def Int2IPAddrStr(ip_int):
    return str(ipaddress.ip_address(ip_int))

def Int2MacStr(mac_int):
    mac_hex = "{:012x}".format(mac_int)
    return ':'.join(format(s, '02x') for s in bytes.fromhex(mac_hex))

def List2UuidStr(uuid_list):
    return str(uuid.UUID(bytes=bytes(uuid_list)))

def GetRandomObject(objList):
    return random.choice(objList)

def GetRandomSamples(objList, num):
    return random.sample(objList, k=num)

def GetFilteredObjects(objs, maxlimits, randomize=False):
    if maxlimits == None or maxlimits == 0 or maxlimits >= len(objs):
        num = len(objs)
    else:
        num = maxlimits
    if randomize:
        return GetRandomSamples(objs, num)
    return objs[0:num]

def IsSkipSetup():
    skip_setup = getattr(GlobalOptions, 'skip_setup', None)
    return skip_setup

def IsNetAgentMode():
    return GlobalOptions.netagent

def IsDryRun():
    return GlobalOptions.dryrun

def Sleep(timeout=1):
    if IsDryRun():
        return
    time.sleep(timeout)
    return

def GetYamlSpecAttr(spec, attr='id', convert2uuid=True):
    val = spec[attr]
    return PdsUuid(val).GetUuid() if convert2uuid else val

def ValidateRpcIPAddr(srcaddr, dstaddr):
    if srcaddr.version == IP_VERSION_6:
        if dstaddr.Af != types_pb2.IP_AF_INET6:
            return False
        if dstaddr.V6Addr != srcaddr.packed:
            return False
    else:
        if dstaddr.Af != types_pb2.IP_AF_INET:
            return False
        if dstaddr.V4Addr != int(srcaddr):
            return False
    return True

def ValidateRpcIPPrefix(srcpfx, dstpfx):
    if dstpfx.Len != srcpfx.prefixlen:
        return False
    dstPfxAddr = dstpfx.Addr
    if srcpfx.version == IP_VERSION_6:
        if dstPfxAddr.Af != types_pb2.IP_AF_INET6:
            return False
        if dstPfxAddr.V6Addr != srcpfx.network_address.packed:
            return False
    else:
        if dstPfxAddr.Af != types_pb2.IP_AF_INET:
            return False
        if dstPfxAddr.V4Addr != int(srcpfx.network_address):
            return False
    return True

def ValidateTunnelEncap(node, srcencap, dstencap):
    if dstencap.type != EzAccessStoreClient[node].GetDeviceEncapType():
        return False
    if EzAccessStoreClient[node].IsDeviceEncapTypeMPLS():
         if dstencap.value.MPLSTag != srcencap:
             return False
    else:
        if dstencap.value.Vnid != srcencap:
            return False
    return True

def ValidateRpcEncap(encaptype, encapval, dstencap):
    if dstencap.type != encaptype:
        return False
    if encaptype == types_pb2.ENCAP_TYPE_DOT1Q:
        if encapval != dstencap.value.VlanId:
            return False
    return True

def ValidateGrpcResponse(resp, expApiStatus=types_pb2.API_STATUS_OK):
    return expApiStatus == resp.ApiStatus

def ValidateGrpcValues(obj, resp):
    return obj.ValidateSpec(resp.Spec) and\
           obj.ValidateStats(resp.Stats) and\
           obj.ValidateStatus(resp.Status)

# TODO: convert yaml 2 proto and remove the following
def ValidateYamlValues(obj, resp):
    spec = resp['spec']
    status = resp['status']
    stats = resp['stats']
    return obj.ValidateYamlSpec(spec) and\
           obj.ValidateYamlStats(stats) and\
           obj.ValidateYamlStatus(status)

def ValidateObject(obj, resp, yaml=False):
    if obj is None:
        logger.info(f"Read failed - No object found for the response {resp}")
        return False
    if yaml:
        return ValidateYamlValues(obj, resp)
    return ValidateGrpcValues(obj, resp)

def GetAttrFromResponse(obj, resp, attr):
    result = getattr(resp, attr, None)
    if result is None:
        logger.error(f"Error fetching {attr} from {resp}")
        assert(0)
    if obj.IsSingleton():
        return [result]
    return result

def SetObjectHwHabitantStatus(obj, oper, status):
    if oper == 'Create' or oper == 'Update':
        if status == types_pb2.API_STATUS_OK:
            obj.SetHwHabitant(True)
    elif oper == 'Delete':
        if status == types_pb2.API_STATUS_OK:
            obj.SetHwHabitant(False)
    return

def ValidateBatch(batchList, cookie):
    for item in batchList:
        oper = item[0]
        obj = item[1]
        operStatus = item[2]
        commitStatus = item[3]

        if IsDryRun():
            SetObjectHwHabitantStatus(obj, oper, types_pb2.API_STATUS_OK)
            return True

        is_obj_present = obj.IsHwHabitant()

        logger.info(f"ValidateBatch: operation {oper} for {obj} on {obj.Node} "
                    f"obj_present {is_obj_present}, cookie {cookie}")
        expApiStatus = types_pb2.API_STATUS_OK
        if is_obj_present == True:
            if oper == 'Create':
                expApiStatus = types_pb2.API_STATUS_EXISTS_ALREADY
                operResp = ValidateCreate(obj, operStatus, expApiStatus)
            elif oper == 'Update' or oper == 'Delete':
                expApiStatus = types_pb2.API_STATUS_OK
                if oper == 'Delete':
                    operResp = ValidateDelete(obj, operStatus, expApiStatus)
                else:
                    operResp = ValidateUpdate(obj, operStatus, expApiStatus)
        else:
            if oper == 'Create':
                expApiStatus = types_pb2.API_STATUS_OK
                operResp = ValidateCreate(obj, operStatus, expApiStatus)
            elif oper == 'Update' or oper == 'Delete':
                expApiStatus = types_pb2.API_STATUS_NOT_FOUND
                if oper == 'Delete':
                    operResp = ValidateDelete(obj, operStatus, expApiStatus)
                else:
                    operResp = ValidateUpdate(obj, operStatus, expApiStatus)

        logger.info(f"ValidateBatch: recvd {operStatus}, {commitStatus} "
                    f"Expected {expApiStatus}")

        # setting HwHabitant for an object only if operation succeeds
        # TODO: @amrita, hwhabitant gets set based on expected status
        # instead of actual status, please check & fix
        SetObjectHwHabitantStatus(obj, oper, expApiStatus)

        # if batch is created internally
        if cookie == INVALID_BATCH_COOKIE:
            logger.info(f"ValidateBatch: internal batch result {operResp}")
            return operResp
        else:
            commitResp = (expApiStatus == commitStatus)
            # objects which don't do DB lookup in pdsagent
            noDBObjs = [
                         ObjectTypes.DEVICE, ObjectTypes.LMAPPING,
                         ObjectTypes.RMAPPING, ObjectTypes.VNIC,
                         ObjectTypes.NEXTHOP, ObjectTypes.SUBNET,
                         ObjectTypes.ROUTE, ObjectTypes.POLICY,
                         ObjectTypes.BGP, ObjectTypes.BGP_PEER,
                         ObjectTypes.BGP_PEER_AF, ObjectTypes.BGP_NLRI_PREFIX,
                         ObjectTypes.BGP_EVPN_EVI, ObjectTypes.BGP_EVPN_EVI_RT,
                         ObjectTypes.BGP_EVPN_IP_VRF, ObjectTypes.BGP_EVPN_IP_VRF_RT
                       ]
            if oper != 'Delete':
                # vpc is being read before delete in agent
                noDBObjs.append(ObjectTypes.VPC)
            if obj.ObjType in noDBObjs:
                # TODO: @amrita, For these objects,
                # Validate on operStatus should be done against API_STATUS_OK
                logger.info(f"ValidateBatch: batch result {commitResp}")
                return commitResp
            else:
                # TODO: @amrita, agent dbs are now removed for all objects. Please check & fix
                # all stateful objects are validated in agent db currently, hence
                # return operation status. Once agent db is removed then change it to batch commit status
                logger.info(f"ValidateBatch: int batch result {operResp}")
                return operResp
        return True

def ValidateCreate(obj, resps, expApiStatus = types_pb2.API_STATUS_OK):
    if IsDryRun():
        # assume creation was fine in case of dry run
        obj.SetHwHabitant(True)
        return True
    for resp in resps:
        if not ValidateGrpcResponse(resp, expApiStatus):
            logger.error(f"[Re]Creation failed for {obj} on {obj.Node}, received resp {resp.ApiStatus} and expected resp {expApiStatus}")
            obj.Show()
            return False
        SetObjectHwHabitantStatus(obj, 'Create', expApiStatus)
    return True

def ValidateRead(obj, resps, expApiStatus = types_pb2.API_STATUS_OK):
    if IsDryRun(): return True
    for resp in resps:
        if ValidateGrpcResponse(resp, expApiStatus):
            if ValidateGrpcResponse(resp):
                readresponse = GetAttrFromResponse(obj, resp, 'Response')
                for response in readresponse:
                    if ValidateObject(obj, response):
                        if hasattr(obj, 'Status'):
                            obj.Status.Update(response.Status)
                    else:
                        logger.info(f"ValidateRead failed for {obj} on {obj.Node}, received resp {resp} & expected status {expApiStatus}")
                        return False
        else:
            logger.info(f"ValidateRead failed for {obj} on {obj.Node} with unexpected status, received resp {resp} & expected status {expApiStatus}")
            return False
    return True

def ValidateDelete(obj, resps, expApiStatus = types_pb2.API_STATUS_OK):
    if IsDryRun():
        # assume deletion was fine in case of dry run
        obj.SetHwHabitant(False)
        return True
    for resp in resps:
        respStatus = GetAttrFromResponse(obj, resp, 'ApiStatus')
        for status in respStatus:
            if status != expApiStatus:
                logger.error(f"Deletion failed for {obj} on {obj.Node}, received {status} but expected {expApiStatus}")
                obj.Show()
                return False
        SetObjectHwHabitantStatus(obj, 'Delete', expApiStatus)
    return True

def ValidateUpdate(obj, resps, expApiStatus = types_pb2.API_STATUS_OK):
    if IsDryRun(): return True
    for resp in resps:
        if ValidateGrpcResponse(resp, expApiStatus):
            if expApiStatus == types_pb2.API_STATUS_OK:
                SetObjectHwHabitantStatus(obj, 'Update', expApiStatus)
                InformDependents(obj, 'UpdateNotify')
        else:
            logger.error(f"Update failed for {obj} on {obj.Node}, received {resp.ApiStatus}, expected {expApiStatus}")
            obj.PrepareRollbackUpdate()
            obj.SetDirty(False)
            return False
    return True

def LoadYaml(cmdoutput):
    return yaml.load(cmdoutput, Loader=yaml.FullLoader)

def GetBatchCookie(node):
    batchClient = EzAccessStore.GetBatchClient()
    obj = batchClient.GetObjectByKey(node)
    return obj.GetBatchCookie()

def GetBatchCommitStatus(node):
    batchClient = EzAccessStore.GetBatchClient()
    obj = batchClient.GetObjectByKey(node)
    return obj.GetBatchCommitStatus()

def InformDependents(dependee, cbFn):
    # inform dependent objects
    for objType, ObjList in dependee.Deps.items():
        for depender in ObjList:
            getattr(depender, cbFn)(dependee)
    return

def TriggerCreate(obj):
    node = obj.Node
    if IsNetAgentMode():
        return api.client[node].Create(obj.ObjType, [obj])
    else:
        batchClient = EzAccessStore.GetBatchClient()
        batchClient.Start(node)
        cookie = GetBatchCookie(node)
        msg = obj.GetGrpcCreateMessage(cookie)
        operStatus = api.client[node].Create(obj.ObjType, [msg])
        batchClient.Commit(node)
        commitStatus = GetBatchCommitStatus(node)
        validate = ('Create', obj, operStatus, commitStatus)
        return ValidateBatch([validate], cookie)

def CreateObject(obj):
    node = obj.Node
    if IsDryRun() and obj.IsHwHabitant():
        logger.info(f"Already restored {obj} on {node}")
        return True

    if not obj.IsOriginFixed():
        logger.info(f"Skipping creation of non-fixed object {obj} on {node}")
        return True

    def RestoreObj(robj):
        node = robj.Node
        if robj.IsOriginDiscovered():
            logger.info(f"Skip Recreating discovered object {robj} on {node}")
            return True
        logger.info(f"[Re]Creating object {robj} on {node}")
        if robj.Duplicate != None:
            #TODO: Ideally a new dependee object should be created first
            # and all dependents should be updated to point to the new object
            # before we delete this. Otherwise, for a moment, the dependent objects
            # are pointing to an object which has been deleted which leads to
            # inconsistency. Doing it this way in DOL, since IOTA scale test cases
            # uses the same code, and at full scale we can't create an object
            # without freeing another.
            logger.info(f"Deleting object {robj.Duplicate} on {node}")
            res = TriggerDelete(robj.Duplicate)
            if not res:
                logger.error(f"Failed to delete {robj} on {node}")
                return res
            confClient = EzAccessStoreClient[node].GetConfigClient(robj.ObjType)
            if confClient == None:
                return False
            confClient.DeleteObjFromDict(robj.Duplicate)
            robj.Duplicate = None
        res = TriggerCreate(robj)
        if not res:
            logger.error(f"Failed to restore {robj} on {node}")
            return res
        if IsUpdateSupported():
            InformDependents(robj, 'RestoreNotify')
        return True

    # create from top to bottom
    res = RestoreObj(obj);
    if not res:
        logger.error(f"Failed to restore {obj} on {node}, skip creating children")
        return res
    res = list(map(lambda childObj: not(CreateObject(childObj)), obj.Children))
    if any(res):
        logger.error(f"Failed to create children of {obj} on {node}, "
                     f"res {res} children {obj.Children}")
        return False
    return True

def ReadObject(obj, expApiStatus = types_pb2.API_STATUS_OK):
    if IsNetAgentMode():
        found = False
        ret = False
        resps = api.client[obj.Node].GetHttp(obj.ObjType)
        if not resps:
            return types_pb2.API_STATUS_NOT_FOUND
        for j in resps:
            if hasattr(j['meta'], 'uuid'):
                if j['meta']['uuid'] == obj.UUID.UuidStr:
                    found = True
                    ret = obj.ValidateJSONSpec(j)
                else:
                    logger.error('uuid not found in - ', j)
        # positive case. obj found and valid
        if expApiStatus == types_pb2.API_STATUS_OK:
            if found and ret:
                return expApiStatus
            else:
                return types_pb2.API_STATUS_NOT_FOUND
        # negative case. either object not found, or validation fail
        else:
            if not found or not ret:
                return expApiStatus
            else:
                return types_pb2.API_STATUS_OK
    else:
        msg = obj.GetGrpcReadMessage()
        resps = api.client[obj.Node].Get(obj.ObjType, [msg])
        return ValidateRead(obj, resps, expApiStatus)

def UpdateObject(obj):
    logger.info(f"Updating object {obj} on {obj.Node}")
    node = obj.Node
    if IsNetAgentMode():
        return api.client[node].Update(obj.ObjType, [obj])
    else:
        batchClient = EzAccessStore.GetBatchClient()
        batchClient.Start(node)
        cookie = GetBatchCookie(node)
        msg = obj.GetGrpcUpdateMessage(cookie)
        operStatus = api.client[node].Update(obj.ObjType, [msg])
        batchClient.Commit(node)
        commitStatus = GetBatchCommitStatus(node)
        validate = ('Update', obj, operStatus, commitStatus)
        return ValidateBatch([validate], cookie)

def TriggerDelete(obj):
    node = obj.Node
    if IsNetAgentMode():
        return api.client[node].Delete(obj.ObjType, [obj])
    else:
        batchClient = EzAccessStore.GetBatchClient()
        batchClient.Start(node)
        cookie = GetBatchCookie(node)
        msg = obj.GetGrpcDeleteMessage(cookie)
        operStatus = api.client[node].Delete(obj.ObjType, [msg])
        batchClient.Commit(node)
        commitStatus = GetBatchCommitStatus(node)
        validate = ('Delete', obj, operStatus, commitStatus)
        return ValidateBatch([validate], cookie)

def DeleteObject(obj):
    node = obj.Node
    if IsDryRun() and not obj.IsHwHabitant():
        logger.info(f"Already deleted {obj} on {node}")
        return True

    if not obj.IsOriginFixed():
        logger.info(f"Skipping deletion of non-fixed object {obj} on {node}")
        return True

    def DelObj(dobj):
        node = dobj.Node
        if dobj.IsOriginDiscovered():
            logger.info(f"Skip Deleting discovered object {dobj} on {node}")
            return True
        logger.info(f"Deleting object {dobj} on {node}")
        #TODO: Ideally a new dependee object should be created first
        # and all dependents should be updated to point to the new object
        # before we delete this. Otherwise, for a moment, the dependent objects
        # are pointing to an object which has been deleted which leads to
        # inconsistency. Doing it this way in DOL, since IOTA scale test cases
        # uses the same code, and at full scale we can't create an object
        # without freeing another.
        obj_present = dobj.IsHwHabitant()
        ret = TriggerDelete(dobj)
        if not ret:
            logger.error(f"Failed to delete {dobj} on {node}")
            return False

        # create duplicate object only if object is present
        if not obj_present:
            return True

        if IsUpdateSupported():
            if dobj.ObjType == ObjectTypes.TUNNEL or\
               dobj.ObjType == ObjectTypes.NEXTHOP or\
               dobj.ObjType == ObjectTypes.NEXTHOPGROUP or\
               dobj.ObjType == ObjectTypes.INTERFACE:
                dupObj = dobj.Dup()
                confClient = EzAccessStore.GetConfigClient(dobj.ObjType)
                if confClient == None:
                    return False
                confClient.AddObjToDict(dupObj)
                TriggerCreate(dupObj)
            InformDependents(dobj, 'DeleteNotify')
        return True

    # Delete from bottom to top
    res = list(map(lambda childObj: not(DeleteObject(childObj)), obj.Children))
    if any(res):
        logger.error(f"Failed to cleanup children of {obj} on {node}, "
                     f"res {res} children {obj.Children}")
        return False
    # Delete the final
    return DelObj(obj)

def GetIPProtoByName(protoname):
    """
        returns IP Protocol number for the given protocol name
    """
    return socket.getprotobyname(protoname)

def GetIPProtoName(proto):
    """
        returns IP Protocol name for the given protocol number
    """
    return IPPROTO_TO_NAME_TBL[proto]

def IsICMPProtocol(proto):
    """
        returns True if given proto is icmp/icmpv6
    """
    return 'ICMP' in GetIPProtoName(proto)

def __get_subnet(ip, prev=False):
    """
        returns next subnet of 'ip' if 'prev' is False else previous subnet
    """
    totalhosts = -1 if prev else 1
    totalhosts *= ip.num_addresses
    newpfx = str(ip.network_address + totalhosts) + '/' + str(ip.prefixlen)
    return ipaddress.ip_network(newpfx)

def GetNextSubnet(ip):
    return __get_subnet(ip)

def GetPreviousSubnet(ip):
    return __get_subnet(ip, prev=True)

def GetRandomPrefix(pfx):
    """
        return random prefix within the subnet of pfx
    """
    pfxLen = pfx.prefixlen
    maxPfxLen = pfx.max_prefixlen
    newPfxLen = random.randint(pfxLen, maxPfxLen)
    newPfxAddr = pfx.network_address + random.getrandbits(maxPfxLen - pfxLen)
    newPfxStr = f"{str(newPfxAddr)}/{newPfxLen}"
    return ipaddress.ip_network(newPfxStr, False)

def GetTunnelType(e):
    if e == 'internet-gateway':
        return tunnel_pb2.TUNNEL_TYPE_IGW
    elif e == 'workload':
        return tunnel_pb2.TUNNEL_TYPE_WORKLOAD
    elif e == 'service' or e == 'remoteservice':
        return tunnel_pb2.TUNNEL_TYPE_SERVICE
    else:
        return tunnel_pb2.TUNNEL_TYPE_NONE

def GetTunnelTypeString(e):
    if e == tunnel_pb2.TUNNEL_TYPE_IGW:
        return "internet-gateway"
    elif e == tunnel_pb2.TUNNEL_TYPE_WORKLOAD:
        return "workload"
    elif e == tunnel_pb2.TUNNEL_TYPE_SERVICE:
        return "service"
    elif e == tunnel_pb2.TUNNEL_TYPE_NONE:
        return "None"
    else:
        logger.error("ERROR: Invalid/Unknown Tunnel Type: %s" % e)
        sys.exit(1)
        return None

def GetEncapType(e):
    if e == 'vxlan':
        return types_pb2.ENCAP_TYPE_VXLAN
    elif e == 'mplsoudp':
        return types_pb2.ENCAP_TYPE_MPLSoUDP
    else:
        logger.error("ERROR: Invalid/Unknown Encap: %s" % e)
        sys.exit(1)
        return None

def GetEncapTypeString(e):
    if e == types_pb2.ENCAP_TYPE_VXLAN:
        return "vxlan"
    elif e == types_pb2.ENCAP_TYPE_MPLSoUDP:
        return "mplsoudp"
    else:
        logger.error("ERROR: Invalid/Unknown Encap: %s" % e)
        sys.exit(1)
        return None

def isDefaultRoute(ippfx):
    if ippfx == IPV4_DEFAULT_ROUTE or ippfx == IPV6_DEFAULT_ROUTE:
        return True
    return False

def isDefaultAddrRange(addrLow, addrHigh):
    if addrLow == IPV4_MINADDR and addrHigh == IPV4_MAXADDR:
        return True
    if addrLow == IPV6_MINADDR and addrHigh == IPV6_MAXADDR:
        return True
    return False

def isTagWithDefaultRoute(tag):
    tagpfxlist = tag.Prefixes if tag else None
    if tagpfxlist is None:
        return False
    for tagpfx in tagpfxlist:
        if isDefaultRoute(tagpfx):
            return True
    return False

def GetIPVersion(ipaf):
    if ipaf == 'IPV6':
        return IP_VERSION_6
    return IP_VERSION_4

def GetRpcIPAddrFamily(ipaddrfamily):
    if ipaddrfamily == 'IPV6':
        return types_pb2.IP_AF_INET6
    elif ipaddrfamily == 'IPV4':
        return types_pb2.IP_AF_INET
    else:
        return types_pb2.IP_AF_NONE

def GetRpcIPPrefix(srcpfx, dstpfx):
    dstpfx.Len = srcpfx.prefixlen
    if srcpfx.version == IP_VERSION_6:
        dstpfx.Addr.Af = types_pb2.IP_AF_INET6
        dstpfx.Addr.V6Addr = srcpfx.network_address.packed
    else:
        dstpfx.Addr.Af = types_pb2.IP_AF_INET
        dstpfx.Addr.V4Addr = int(srcpfx.network_address)

def GetRpcIPv4Prefix(srcpfx, dstpfx):
    dstpfx.Len = srcpfx.prefixlen
    dstpfx.Addr = int(srcpfx.network_address)

def GetRpcIPv6Prefix(srcpfx, dstpfx):
    dstpfx.Len = srcpfx.prefixlen
    dstpfx.Addr = srcpfx.network_address.packed

def GetRpcIfIPPrefix(srcpfx, dstpfx):
    dstpfx.Len = ipaddress.ip_network(srcpfx.network).prefixlen
    dstpfx.Addr.Af = types_pb2.IP_AF_INET
    dstpfx.Addr.V4Addr = int(ipaddress.ip_address(srcpfx.ip))

def GetRpcIPAddr(srcaddr, dstaddr):
    if srcaddr.version == IP_VERSION_6:
        dstaddr.Af = types_pb2.IP_AF_INET6
        dstaddr.V6Addr = srcaddr.packed
    else:
        dstaddr.Af = types_pb2.IP_AF_INET
        dstaddr.V4Addr = int(srcaddr)

def GetRpcIPRange(addrLow, addrHigh, addrRange):
    if addrLow.version != addrHigh.version:
        logger.error("ERROR: addrRange version mismatch: Low %s High %s" %(addrLow, addrHigh))
        sys.exit(1)
    if addrLow.version == IP_VERSION_6:
        GetRpcIPAddr(addrLow, addrRange.IPv6Range.Low)
        GetRpcIPAddr(addrHigh, addrRange.IPv6Range.High)
    else:
        GetRpcIPAddr(addrLow, addrRange.IPv4Range.Low)
        GetRpcIPAddr(addrHigh, addrRange.IPv4Range.High)
    return

def GetRpcEncap(node, mplsslot, vxlanid, encap):
    encap.type = EzAccessStoreClient[node].GetDeviceEncapType()
    if EzAccessStoreClient[node].IsDeviceEncapTypeMPLS():
         encap.value.MPLSTag  = mplsslot
    else:
         encap.value.Vnid  = vxlanid

def GetRpcDirection(direction):
    if direction == "ingress":
         return types_pb2.RULE_DIR_INGRESS
    elif direction == "egress":
         return types_pb2.RULE_DIR_EGRESS
    return types_pb2.RULE_DIR_NONE

def GetRpcSecurityRuleAction(action):
    if action == "allow":
        return types_pb2.SECURITY_RULE_ACTION_ALLOW
    elif action == "deny":
        return types_pb2.SECURITY_RULE_ACTION_DENY
    else:
        return types_pb2.SECURITY_RULE_ACTION_NONE

def GetPortIDfromInterface(interfaceid):
    return topo.INTF2PORT_TBL.get(interfaceid, topo.PortTypes.NONE)

def IsPipelineArtemis():
    if GlobalOptions.pipeline == 'artemis':
        return True
    return False

def IsPipelineApulu():
    if GlobalOptions.pipeline == 'apulu':
        return True
    return False

def IsPipelineApollo():
    if GlobalOptions.pipeline == 'apollo':
        return True
    return False

def GetPipelineName():
    return GlobalOptions.pipeline

def IsHostLifSupported():
    if IsPipelineArtemis():
        return False
    return True

def IsInterfaceSupported():
    if IsPipelineArtemis():
        return False
    return True

def IsL3InterfaceSupported():
    if IsPipelineApulu():
        return True
    return False

def IsIGWTunnelSupported():
    if IsPipelineApulu():
        return False
    return True

def IsWorkloadTunnelSupported():
    if IsPipelineApulu():
        return False
    return True

def IsServiceTunnelSupported():
    if IsPipelineArtemis():
        return True
    return False

def IsUnderlayTunnelSupported():
    if IsPipelineApulu():
        return True
    return False

def IsRouteTableSupported():
    return True

def IsVnicPolicySupported():
    if IsPipelineApulu():
        return True
    return False

def IsTagSupported():
    if IsPipelineArtemis():
        return True
    return False

def IsMeteringSupported():
    if IsPipelineArtemis():
        return True
    return False

def IsServiceMappingSupported():
    if IsPipelineArtemis():
        return True
    return False

def IsPfxRangeSupported():
    if IsPipelineApollo():
        return False
    return True

def IsFlowInstallationNeeded():
    if IsPipelineArtemis():
        return True
    return False

def IsNatSupported():
    if IsPipelineArtemis() or IsPipelineApollo():
        return True
    return False

def IsDualEcmp(spec):
    return getattr(spec, 'dual-ecmp', False)

def GetDefaultDeviceMode():
    if IsPipelineApulu():
        return 'host'
    return 'bitw'

def IsV4Stack(stack):
    return True if ((stack == "dual") or (stack == 'ipv4')) else False

def IsV6Stack(stack):
    return True if ((stack == "dual") or (stack == 'ipv6')) else False

def GetVlanHeaderSize(packet):
    pkt = packet.GetScapyPacket()
    if Dot1Q in pkt:
        return DOT1Q_HDR_LEN
    return 0

def IsUpdateSupported():
    if IsPipelineApulu():
        return True
    return False

def MergeFilteredObjects(objs, selected_objs):
    if topo.ChosenFlowObjs.select_objs is True:
        topo.ChosenFlowObjs.Add(objs)
    elif topo.ChosenFlowObjs.use_selected_objs is True:
        objs.extend(selected_objs)

def MergeDicts(objs1, objs2):
    objs = OrderedDict()
    for obj in objs1.values():
        objs.update({obj.Id: obj})
    for obj in objs2.values():
        objs.update({obj.Id: obj})
    return objs

# For debug purposes
def dump(obj):
   for attr in dir(obj):
       if hasattr( obj, attr ):
           logger.info( "obj.%s = %s" % (attr, getattr(obj, attr)))

def DumpObject(obj):
    parentObj = getattr(obj, 'Parent', None)
    if parentObj:
        DumpObject(parentObj)
    logger.info(" === Selected %s === " % (obj.ObjType.name))
    obj.Show()
    return

def DumpTestcaseConfig(obj):
    tcAttrs = [ 'route', 'tunnel', 'policy', 'localmapping', 'remotemapping']
    logger.info("========== Testcase config start ==========")
    for attr in tcAttrs:
        cfgObj = getattr(obj, attr, None)
        if cfgObj: DumpObject(cfgObj)
    ruleObj = getattr(obj, 'tc_rule', None)
    objs = list(filter(None, [ruleObj]))
    for cfgObj in objs:
        logger.info(" === Selected rule/route ===")
        cfgObj.Show()
    logger.info("========== Testcase config end ==========")
    return

WS_PATH = os.environ['WS_TOP']
def GetDeviceJsonPath():
    rel_path = "nic/conf/%s/device.json" % (GetPipelineName())
    abs_path = os.path.join(WS_PATH, rel_path)
    return abs_path

def GetNicmgrLogPath():
    rel_path = "nic/nicmgr.log"
    abs_path = os.path.join(WS_PATH, rel_path)
    return abs_path

class rrobiniter:
    def __init__(self, objs):
        assert len(objs) != 0
        self.objs = objs
        self.iterator = iter(objs)
        self.size = len(objs)
    def rrnext(self):
        while True:
            try:
                return next(self.iterator)
            except:
                self.iterator = iter(self.objs)
                continue
    def size(self):
        return self.size

def IsDol():
    return defs.TEST_TYPE == "DOL"

def RunPdsctlShowCmd(node, cmd, args=None, yaml=True):
    if IsDol():
        import apollo.test.utils.pdsctl as pdsctl
        ret, yaml_op = pdsctl.ExecutePdsctlShowCommand(cmd, args, yaml)
    else:
        import iota.test.apulu.utils.pdsctl as pdsctl
        ret, yaml_op = pdsctl.ExecutePdsctlShowCommand(node, cmd, args, yaml, print_op=False)
    return ret, yaml_op

def GetNodeLoopbackPrefix(node):
    loopback_ip = EzAccessStoreClient[node].GetLoopbackIp()
    if loopback_ip:
        return ipaddress.ip_network(loopback_ip + "/32")
    else:
        return None

def GetNodeLoopbackIp(node):
    loopback_ip = EzAccessStoreClient[node].GetLoopbackIp()
    if loopback_ip:
        return ipaddress.IPv4Address(loopback_ip)
    else:
        return None

def GetNodeLoopbackRemoteTEP(node):
    rtep = EzAccessStoreClient[node].GetLoopbackRemoteTEP()
    if rtep:
        return ipaddress.IPv4Address(rtep)
    else:
        return None

def GetNodeUnderlayIp(node, intf_name):
    ip = EzAccessStoreClient[node].GetUnderlayIp(intf_name)
    if ip:
        return(ipaddress.IPv4Address(ip))
    return None

def GetNodeUnderlayMaskLen(node, intf_name):
    return EzAccessStoreClient[node].GetUnderlayMaskLen(intf_name)

def GetNodeUnderlayBGPRemoteASN(node, intf_name):
    asn = EzAccessStoreClient[node].GetUnderlayBGPRemoteASN(intf_name)
    if asn:
         return(int(asn))
    return None

def GetNodeUnderlayNexthop(node, intf_name):
    nexthop = EzAccessStoreClient[node].GetUnderlayNexthop(intf_name)
    if nexthop:
        return(ipaddress.IPv4Address(nexthop))
    return None

def IsBridgingEnabled(node):
    return EzAccessStoreClient[node].IsBridgingEnabled()
