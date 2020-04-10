# Flowmon Packet Analysis

from scapy.packet import *
from scapy.fields import *
from scapy.all import *

class _IpfixIpv4Fields(Packet):
    name = "IpfixIpv4Fields"
    fields_desc = [
        BitField("len",         0,      16),
        BitField("vrf",         0,      32),
        IPField("ip_sa",        "0.0.0.0"),
        IPField("ip_da",        "0.0.0.0"),
        BitField("proto",       0,      8),
        BitField("sport",       0,      16),
        BitField("dport",       0,      16),
    ]

class _IpfixIpFields(Packet):
    name = "IpfixIpFields"
    fields_desc = [
        BitField("flow_state_index",        0,      24),
        BitField("role",                    0,      8),
        BitField("ttl",                     0,      8),
        BitField("icmp_type_code",          0,      16),
        BitField("tcp_seq_num",             0,      32),
        BitField("tcp_ack_num",             0,      32),
        BitField("tcp_win_sz",              0,      16),
        BitField("tcp_win_scale",           0,      8),
        BitField("tcp_state",               0,      8),
        BitField("tcp_win_mss",             0,      16),
        BitField("tcp_exceptions",          0,      32),
        BitField("flit_pad1",               0,      16),
        BitField("tcp_rtt",                 0,      32),
    ]

class _IpfixCommonFields(Packet):
    name = "IpfixIpFields"
    fields_desc = [
        BitField("flow_id",                 0,      32),
        BitField("egress_interface",        0,      16),
        BitField("egress_interface_type",   0,      16),
        BitField("permit_packets",          0,      64),
        BitField("permit_bytes",            0,      64),
        BitField("delta_permit_packets",    0,      32),
        BitField("delta_permit_bytes",      0,      32),
        BitField("drop_packets",            0,      64),
        BitField("drop_bytes",              0,      64),
        BitField("delta_drop_packets",      0,      32),
        BitField("delta_drop_bytes",        0,      32),
        BitField("flit_pad2",               0,      32),
        BitField("start_timestamp",         0,      64),
        BitField("last_seen_timestamp",     0,      64),
        BitField("drop_vector",             0,      64),
    ]

class _IpfixRecordBase(Packet):
    fields_desc = [
        BitField("set_id",      0,      16),
    ]
class IpfixRecord(Packet):
    fields_desc = [ _IpfixRecordBase ]

class IpfixRecordIpv4Data(Packet):
    name = 'IpfixRecordIpv4Data'
    holds_packet = True
    fields_desc = [
        _IpfixIpv4Fields,
        _IpfixIpFields,
        _IpfixCommonFields
    ]
    def guess_payload_class(self, payload):
        return IpfixRecord

bind_layers(IpfixRecord, IpfixRecordIpv4Data, set_id=257)

class Ipfix(Packet):
    holds_packet = 1
    name = "Ipfix"
    fields_desc = [
        BitField("version",     0,      16),
        BitField("len",         0,      16),
        BitField("export_time", 0,      32),
        BitField("seq_num",     0,      32),
        BitField("domain_id",   0,      32),
        PacketListField("records", [], IpfixRecord)
    ]

bind_layers(UDP, Ipfix, dport=4000)
bind_layers(UDP, Ipfix, dport=4001)
bind_layers(UDP, Ipfix, dport=4002)
bind_layers(UDP, Ipfix, dport=4003)

