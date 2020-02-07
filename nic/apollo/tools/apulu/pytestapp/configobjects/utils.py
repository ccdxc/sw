import enum

def getmac2num(mac,reorder=False):
    by8 = mac.split(":")
    if len(by8) != 6:
       by8 = mac.split("-")
    if reorder:
       return (int(by8[5], 16) << 40) +\
           (int(by8[4], 16) << 32) +\
           (int(by8[3], 16) << 24) +\
           (int(by8[2], 16) << 16) +\
           (int(by8[1], 16) << 8) +\
           (int(by8[0], 16))
    else:
       return (int(by8[0], 16) << 40) +\
           (int(by8[1], 16) << 32) +\
           (int(by8[2], 16) << 24) +\
           (int(by8[3], 16) << 16) +\
           (int(by8[4], 16) << 8) +\
           (int(by8[5], 16))

PENSANDO_NIC_MAC = 0x022222111111
PDS_UUID_MAGIC_BYTE_VAL = 0x4242
PDS_UUID_BYTE_ORDER = "little"

PDS_UUID_LEN = 16
PDS_UUID_ID_LEN = 8
PDS_UUID_MAGIC_BYTE_LEN = 2
PDS_UUID_SYSTEM_MAC_LEN = 6

PDS_UUID_ID_OFFSET_START = 0
PDS_UUID_ID_OFFSET_END = PDS_UUID_ID_OFFSET_START + PDS_UUID_ID_LEN
PDS_UUID_MAGIC_BYTE_OFFSET_START = PDS_UUID_ID_OFFSET_END
PDS_UUID_MAGIC_BYTE_OFFSET_END = PDS_UUID_MAGIC_BYTE_OFFSET_START + PDS_UUID_MAGIC_BYTE_LEN
PDS_UUID_SYSTEM_MAC_OFFSET_START = PDS_UUID_MAGIC_BYTE_OFFSET_END

PDS_UUID_MAGIC_BYTE = PDS_UUID_MAGIC_BYTE_VAL.to_bytes(PDS_UUID_MAGIC_BYTE_LEN, PDS_UUID_BYTE_ORDER)
PDS_UUID_SYSTEM_MAC = PENSANDO_NIC_MAC.to_bytes(PDS_UUID_SYSTEM_MAC_LEN, "big")

class PdsUuid:

    def __init__(self, value, node_uuid=None):

        if node_uuid:
            node_uuid = int(node_uuid, 16)
            node_uuid = node_uuid.to_bytes(6, "big")

        if isinstance(value, int):
            self.Id = value
            self.Uuid = PdsUuid.GetUUIDfromId(self.Id, node_uuid)
        elif isinstance(value, bytes):
            self.Uuid = value
            self.Id = PdsUuid.GetIdfromUUID(self.Uuid)
        elif isinstance(value, list):
            self.Uuid = bytes(value)
            self.Id = PdsUuid.GetIdfromUUID(self.Uuid)
        else:
            logger.error(f"{type(value)} is NOT supported for PdsUuid class")
            assert(0)
        self.UuidStr = PdsUuid.GetUuidString(self.Uuid)


    def __str__(self):
        return f"ID:{self.Id} UUID:{self.UuidStr}"

    def GetUuid(self):
        return self.Uuid

    def GetId(self):
        return self.Id

    @staticmethod
    def GetUuidString(uuid):
        uuid_id = PdsUuid.GetIdfromUUID(uuid)
        uuid_magic = int.from_bytes(uuid[PDS_UUID_MAGIC_BYTE_OFFSET_START:PDS_UUID_MAGIC_BYTE_OFFSET_END], PDS_UUID_BYTE_ORDER)
        uuid_mac = int.from_bytes(uuid[PDS_UUID_SYSTEM_MAC_OFFSET_START:], "big")
        uuidstr = hex(uuid_id) + "-" + hex(uuid_magic) + "-" + hex(uuid_mac)
        return uuidstr

    @staticmethod
    def GetIdfromUUID(uuid):
        return int.from_bytes(uuid[PDS_UUID_ID_OFFSET_START:PDS_UUID_ID_OFFSET_END], PDS_UUID_BYTE_ORDER)

    @staticmethod
    def GetUUIDfromId(id, node_uuid=None):
        if not node_uuid:
            node_uuid = PDS_UUID_SYSTEM_MAC
        # uuid is of 16 bytes
        uuid = bytearray(PDS_UUID_LEN)
        # first 8 bytes ==> id
        uuid[PDS_UUID_ID_OFFSET_START:PDS_UUID_ID_OFFSET_END] = id.to_bytes(PDS_UUID_ID_LEN, PDS_UUID_BYTE_ORDER)
        # next 2 bytes ==> magic byte (0x4242)
        uuid[PDS_UUID_MAGIC_BYTE_OFFSET_START:PDS_UUID_MAGIC_BYTE_OFFSET_END] = PDS_UUID_MAGIC_BYTE
        # next 6 bytes ==> system mac (0x022222111111)
        uuid[PDS_UUID_SYSTEM_MAC_OFFSET_START:] = node_uuid
        return bytes(uuid)

class InterfaceTypes(enum.IntEnum):
    NONE = 0
    ETH = 1
    ETH_PC = 2
    TUNNEL = 3
    MGMT = 4
    UPLINK = 5
    UPLINKPC = 6
    L3 = 7
    LIF = 8

def PortToEthIfIdx(port):
    ifidx = InterfaceTypes.ETH << 28
    ifidx = ifidx | (1 << 24)
    ifidx = ifidx | (port << 16)
    ifidx = ifidx | 1
    return ifidx
