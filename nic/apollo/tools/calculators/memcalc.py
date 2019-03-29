#! /usr/bin/python
import yaml
import argparse
import pdb
import math

ENTRY_SIZE_BYTES = 64
ENTRY_SIZE_BITS = 512

ScaleUnits = {
    'K': 1024,
    'M': 1024**2,
    'B': 1024**3,
    'T': 1024**4,
}

class FieldSizes:
    ipv4 = 32
    ipv6_64b = 64
    ipv6_128b = 128
    sport = 16
    dport = 16
    proto = 8
    macaddress = 48

class RfcConstants:
    max_ip_classes = 1024
    max_sport_classes = 128
    max_dport_proto_classes = 256
    p1_nbits = 17 # 10 bits from + 7 bits from SPORT
    p1_result_size = 10 # result is a 10-bit ClassID
    p2_nbits = 18 # 10 bits from P1 result + 8 bits from DPORT+PROTO
    p2_result_size = 2

# Pre-calculated RFC Block sizes indexed by keysize
LpmBlockSizes = {
    16: (1+32)*64,
    24: (1+16+16*16)*64,
    32: (1+16+16*16)*64,
    64: (1+(8)+(8*8)+(8*8*8))*64,
    128: (1+(4)+(4*4)+(4*4*4)+(4*4*4*4)+(4*4*4*4*4))*64,
}

LpmPackingFactor = {
    16: 32,
    24: 16,
    32: 16,
    64: 8,
    128: 4,
}

class Dict2Object(object):
    def __init__(self, d):
        for a, b in d.items():
            if isinstance(b, (list, tuple)):
                setattr(self, a, [Dict2Object(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, Dict2Object(b) if isinstance(b, dict) else b)

def YmlParse(filename):
    with open(filename, 'r') as f:
        obj = Dict2Object(yaml.load(f))
    f.close()
    return obj

def UnitStr2Value(valuestr):
    for k,v in ScaleUnits.items():
        if k in str(valuestr):
            return int(valuestr.strip(k))*v
    return int(valuestr)

def ParseParams():
    Params.vpc.count = UnitStr2Value(Params.vpc.count)
    Params.policy_tables.count = UnitStr2Value(Params.policy_tables.count)
    Params.policy_tables.ipv4.ingress = UnitStr2Value(Params.policy_tables.ipv4.ingress)
    Params.policy_tables.ipv4.egress = UnitStr2Value(Params.policy_tables.ipv4.egress)
    Params.policy_tables.ipv6.ingress = UnitStr2Value(Params.policy_tables.ipv6.ingress)
    Params.policy_tables.ipv6.egress = UnitStr2Value(Params.policy_tables.ipv6.egress)
    Params.route_tables.count = UnitStr2Value(Params.route_tables.count)
    Params.route_tables.routes.ipv4 = UnitStr2Value(Params.route_tables.routes.ipv4)
    Params.route_tables.routes.ipv6 = UnitStr2Value(Params.route_tables.routes.ipv6)
    Params.route_tables.nexthops = UnitStr2Value(Params.route_tables.nexthops)
    Params.flows.count = UnitStr2Value(Params.flows.count)
    Params.mappings.locals.ipv4 = UnitStr2Value(Params.mappings.locals.ipv4)
    Params.mappings.locals.ipv6 = UnitStr2Value(Params.mappings.locals.ipv6)
    Params.mappings.remotes.ipv4 = UnitStr2Value(Params.mappings.remotes.ipv4)
    Params.mappings.remotes.ipv6 = UnitStr2Value(Params.mappings.remotes.ipv6)
    Params.teps.count = UnitStr2Value(Params.teps.count)
    Params.nat.count = UnitStr2Value(Params.nat.count)
    return

def RoundPowN(value, n):
    if value:
        return math.pow(n, math.ceil(math.log(value,n)))
    return 0

def RoundPow2(value):
    if value:
        return math.pow(2, math.ceil(math.log(value,2)))
    return 0

def Round8(value):
    return int(math.ceil(value / 8) * 8)

def Round64(value):
    return int(math.ceil(value / 64) * 64)

def __ValueToXXCommon(value, units, decimal_points = True):
    s = 0
    while value >= 1024:
        value = value / 1024
        s += 1
    if s > len(units):
        pdb.set_trace()
    if decimal_points:
        return "%3.02f %s" % (value, units[s])
    else:
        return "%d %s" % (value, units[s])

def ValueToX(value):
    units = ["", "K", "M", "B", "T", "P", "Z" ]
    return __ValueToXXCommon(value, units, decimal_points = False)

def ValueToXB(value_bits):
    units = ["B", "KB", "MB", "GB", "TB", "PB", "ZB" ]
    value_bytes = float(value_bits) / 8
    return __ValueToXXCommon(value_bytes, units)

def CalculateLpmMemory(fldsize, count):
    # Block size is in Bytes and represents a 1K block
    multiplier = RoundPowN(math.ceil(float(count) / 1024), LpmPackingFactor[fldsize])
    return LpmBlockSizes[fldsize] * multiplier * 8

def CalculateRfcTableMemory(nbits, result_size):
    #pdb.set_trace()
    num_entries = math.pow(2, nbits)
    entries_per_line = ENTRY_SIZE_BITS / result_size
    num_lines = Round64(num_entries / entries_per_line)
    return num_lines * ENTRY_SIZE_BITS

def CalculatePolicyMemory(ip_field_size, count):
    #pdb.set_trace()
    # Phase0 LPM for IP Address
    mem = CalculateLpmMemory(ip_field_size,
                             RfcConstants.max_ip_classes)
    # Phase0 LPM for Sport
    mem += CalculateLpmMemory(FieldSizes.sport,
                              RfcConstants.max_sport_classes)
    # Phase0 LPM for Dport
    mem += CalculateLpmMemory(FieldSizes.dport + FieldSizes.proto,
                              RfcConstants.max_dport_proto_classes)

    # Phase1 Table
    mem += CalculateRfcTableMemory(RfcConstants.p1_nbits,
                                   RfcConstants.p1_result_size)

    # Phase2 Table
    mem += CalculateRfcTableMemory(RfcConstants.p2_nbits,
                                   RfcConstants.p2_result_size)
    return mem * count / 1024

def CalculateRouteTableMemory():
    ipv4_routes_memory = CalculateLpmMemory(FieldSizes.ipv4,
                                            Params.route_tables.routes.ipv4)
    ipv6_routes_memory = CalculateLpmMemory(FieldSizes.ipv6_64b,
                                            Params.route_tables.routes.ipv6)
    total_ipv4_routes_memory = ipv4_routes_memory * (Params.route_tables.count + 1)
    total_ipv6_routes_memory = ipv6_routes_memory * (Params.route_tables.count + 1)
    total = total_ipv4_routes_memory + total_ipv6_routes_memory

    #print("----------------------------------------------------------");
    print("Route Table Memory Calculation          IPv4       IPv6");
    print("----------------------------------------------------------")
    print("[ a] - Number of Route Tables         : %-10s %-10s" % (ValueToX(Params.route_tables.count), ValueToX(Params.route_tables.count)))
    print("[ b] - Number of Routes               : %-10s %-10s" % (ValueToX(Params.route_tables.routes.ipv4), ValueToX(Params.route_tables.routes.ipv6)))
    print("[ c] - LPM Memory (1 VPC)             : %-10s %-10s" % (ValueToXB(ipv4_routes_memory), ValueToXB(ipv6_routes_memory)))
    print("[T1] - Total LPM Memory (a*c)         : %-10s %-10s\n" % (ValueToXB(total_ipv4_routes_memory), ValueToXB(total_ipv6_routes_memory)))
    return total

def CalculatePolicyTableMemory():
    ing_ipv4_memory = CalculatePolicyMemory(FieldSizes.ipv4, Params.policy_tables.ipv4.ingress)
    egr_ipv4_memory = CalculatePolicyMemory(FieldSizes.ipv4, Params.policy_tables.ipv4.egress)
    vpc_ipv4_memory = ing_ipv4_memory + egr_ipv4_memory
    total_ipv4_memory = vpc_ipv4_memory * Params.policy_tables.count

    ing_ipv6_memory = CalculatePolicyMemory(FieldSizes.ipv6_128b,
                                            Params.policy_tables.ipv6.ingress)
    egr_ipv6_memory = CalculatePolicyMemory(FieldSizes.ipv6_128b,
                                            Params.policy_tables.ipv6.egress)
    vpc_ipv6_memory = ing_ipv6_memory + egr_ipv6_memory
    total_ipv6_memory = vpc_ipv6_memory * Params.policy_tables.count

    total_vpc_memory = vpc_ipv4_memory + vpc_ipv6_memory

    total = total_ipv4_memory + total_ipv6_memory

    #print("----------------------------------------------------------");
    print("Policy Table Memory Calculation         IPv4       IPv6");
    print("----------------------------------------------------------")
    print("[ a] - Number of VPCs                 : %s" % ValueToX(Params.policy_tables.count))
    print("[ b] - Ingress Policy Memory          : %-10s %-10s" % (ValueToXB(ing_ipv4_memory), ValueToXB(ing_ipv6_memory)))
    print("[ c] - Egress Policy Memory           : %-10s %-10s" % (ValueToXB(egr_ipv4_memory), ValueToXB(egr_ipv6_memory)))
    print("[ d] - Policy Memory(1 VPC)(b+c)      : %-10s %-10s" % (ValueToXB(vpc_ipv4_memory), ValueToXB(vpc_ipv6_memory)))
    print("[ e] - Total Policy Memory(d*a)       : %-10s %-10s\n" % (ValueToXB(total_ipv4_memory), ValueToXB(total_ipv6_memory)))
    return total

def __calculate_memhash_table_info(count, infosize, statsenable, collision_ratio, instances = 1):
    count = count * instances
    hash_mem = 2 * count * ENTRY_SIZE_BITS * instances
    oflow_mem = hash_mem * int(collision_ratio.strip('%')) / 100 * instances
    info_mem = count * ENTRY_SIZE_BITS if infosize else 0
    stats_mem = count * ENTRY_SIZE_BITS if statsenable else 0
    total = hash_mem + oflow_mem + info_mem + stats_mem
    return hash_mem,oflow_mem,info_mem,stats_mem,total

def CalculateFlowTableMemory():
    h,o,i,s,t = __calculate_memhash_table_info(Params.flows.count, 1, True, '25%')
    print("Flow Table Memory Calculation")
    print("----------------------------------------------------")
    print("[ a] - Count                          : %s" % ValueToX(Params.flows.count))
    print("[ b] - Hash Table Memory              : %s" % ValueToXB(h))
    print("[ c] - Overflow Table Memory          : %s" % ValueToXB(o))
    print("[ d] - Info Table Memory              : %s" % ValueToXB(i))
    print("[ e] - Stats Table Memory             : %s" % ValueToXB(s))
    print("[T3] - Total Memory [b+c+d+e]         : %s\n" % ValueToXB(t))
    return t

def CalculateMappingTableMemory():
    l_count = Params.mappings.locals.ipv4 + Params.mappings.locals.ipv6
    l_h,l_o,l_i,l_s,l_t = __calculate_memhash_table_info(l_count, 0, False, '20%')
    r_count = Params.mappings.remotes.ipv4 + Params.mappings.remotes.ipv6
    r_h,r_o,r_i,r_s,r_t = __calculate_memhash_table_info(r_count, 0, False, '20%',
                                                         Params.mappings.remotes.instances)

    #print("----------------------------------------------------------");
    print("Mapping Tables Memory Calculation       Locals     Remotes");
    print("----------------------------------------------------------")
    print("[ a] - Count (IPv4 + IPv6)            : %-10s %-10s" % (ValueToX(l_count), ValueToX(r_count)))
    print("[ b] - Instances                      : %-10s %-10s" % ('1', str(Params.mappings.remotes.instances)))
    print("[ c] - Hash Table Memory              : %-10s %-10s" % (ValueToXB(l_h), ValueToXB(r_h)))
    print("[ d] - Overflow Table Memory          : %-10s %-10s" % (ValueToXB(l_o), ValueToXB(r_o)))
    print("[T4] - Total Memory [c+d]             : %-10s %-10s\n" % (ValueToXB(l_t), ValueToXB(r_t)))
    return l_t+r_t

def CalculateTepMemory():
    count = Params.teps.count
    entry_size = (FieldSizes.ipv4 + FieldSizes.macaddress) / 8
    entries_per_line = RoundPow2(ENTRY_SIZE_BYTES / entry_size)
    num_lines = RoundPow2(Params.teps.count / entries_per_line)
    memory = num_lines * ENTRY_SIZE_BITS

    print("TEP Table Memory Calculation")
    print("----------------------------------------------------")
    print("[ a] - Count                          : %s" % ValueToX(count))
    print("[T5] - Total TEP Memory               : %s\n" % ValueToXB(memory))
    return memory

def CalculateNatMemory():
    count = Params.nat.count
    entry_size = FieldSizes.ipv6_128b / 8
    entries_per_line = RoundPow2(ENTRY_SIZE_BYTES / entry_size)
    num_lines = RoundPow2(Params.nat.count / entries_per_line)
    memory = num_lines * ENTRY_SIZE_BITS

    print("NAT Table Memory Calculation")
    print("----------------------------------------------------")
    print("[ a] - Count                          : %s" % ValueToX(count))
    print("[T6] - Total NAT Memory               : %s\n" % ValueToXB(memory))
    return memory

parser = argparse.ArgumentParser(description='Memory Calculator')
parser.add_argument('--params', dest='params_filename', default='params.yml',
                    help='Parameter File')
Args = parser.parse_args()
Params = YmlParse(Args.params_filename)
ParseParams()

total = 0
total += CalculateRouteTableMemory()
total += CalculatePolicyTableMemory()
total += CalculateFlowTableMemory()
total += CalculateMappingTableMemory()
total += CalculateTepMemory()
total += CalculateNatMemory()

print("Total Datapath Memory [T1+...+T6]     : %s" % ValueToXB(total))

linux_memory = 3*1024*1024*1024*8
print("Total Linux Memory                    : %s" % ValueToXB(linux_memory))
print("Total Memory                          : %s" % ValueToXB(linux_memory+total))
