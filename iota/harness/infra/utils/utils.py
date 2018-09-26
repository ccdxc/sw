#! /usr/bin/python3
class Dict2Enum(object):
    def __init__(self, entries):
        self.str_enums = entries
        entries = dict((v.replace(" ", "_"), k) for k, v in entries.items())
        self.__dict__.update(entries)

    def valid(self, name):
        return name in self.__dict__

    def id(self, name):
        return self.__dict__[name]

    def str(self, v):
        return self.str_enums[v]

def List2Enum(entries):
    db = {}
    value = 0
    for entry in entries:
        db[value] = entry
        value += 1
    return Dict2Enum(db)

__config_db_types = {
    0: 'NONE',
    1: 'TENANT',
    2: 'SEGMENT',
    3: 'ENDPOINT',
    4: 'ENIC',
    5: 'SESSION',
    6: 'FLOW',
    7: 'TEMPLATE',
    8: 'SPEC',
    9: 'UPLINK',
    10: 'UPLINKPC',
}
cfgtypes = Dict2Enum(__config_db_types)

__config_field_scopes = {
    0: 'NONE',
    1: 'SRC',
    2: 'DST',
}
config_field_scopes = Dict2Enum(__config_field_scopes)

__filters = {
    'NONE',

    # Flow object attributes.
    'DOM',
    'SADDR',
    'DADDR',
    'PROTO',
    'SPORT',
    'DPORT',

    'PORT',
    'UPLINK',
    'LIF',
    'INPUT',
    'NAME',
    'ID',
    'ANY',
    'LOCAL',
    'REMOTE',
    'PARENT',
    'ADDR',
}
filters = List2Enum(__filters)

__auto_field_types = {
    0: 'NONE',
    1: 'SCAPY',
    2: 'INPUT',
}
autotypes = Dict2Enum(__auto_field_types)

__tspec_section_types = {
    0: 'NONE',
    1: 'CONFIG',
    2: 'TRIGGER',
    3: 'PACKETS',
    4: 'MEMORY',
    5: 'EXPECT',
}
tspec_section_types = Dict2Enum(__tspec_section_types)

__pkt_match_type = {
    0: 'FULL',
    1: 'PARTIAL'
}
pkt_match_type = Dict2Enum(__pkt_match_type)

__pkt_headers = [
    'BASE',
    'ETHERNET',
    'QTAG',
    'IPV4',
    'IPV6',
    'TCP',
    'UDP',
    'PAYLOAD',
    'PADDING',
    'PENDOL',
    'CRC',
    'METADATA',
    'END',
]
pkt_headers = List2Enum(__pkt_headers)

__ref_roots = [
    'TESTCASE',
    'FACTORY',
    'STORE',
    'TRACKERSTORE',
    'TRACKER',
    'STEP',
]
ref_roots = List2Enum(__ref_roots)

__ip_protos = {
    0   : 'NONE',
    1   : 'ICMP',
    4   : 'IPV4',
    6   : 'TCP',
    17  : 'UDP',
    47  : 'GRE',
    50  : 'ESP',
    51  : 'AH',
    58  : 'ICMPV6',
    255 : 'PROTO255',
}
ipprotos = Dict2Enum(__ip_protos)
