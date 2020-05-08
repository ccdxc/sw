
from ctypes import *
from pprint import pformat
import os.path

def libcapisa_so_load():
    # so_container path will be used if customers run captrace in a container
    so_container_path = "/sw/nic/sdk/third-party/asic/captrace/x86_64/libcapisa.so"
    so_rel_path = "/../../third-party/asic/captrace/x86_64/libcapisa.so"
    if os.path.exists(so_container_path):
        libcapisa = cdll.LoadLibrary(so_container_path)
    else:
        dir_path = os.path.dirname(os.path.realpath(__file__))
        so_path = dir_path + so_rel_path
        libcapisa = cdll.LoadLibrary(so_path)
    return libcapisa

libcapisa = libcapisa_so_load()
libcapisa.c_libcapisa_init()

def to_dict(obj):
    if isinstance(obj, Array):
        return [to_dict(x) for x in obj]
    elif isinstance(obj, Structure) or isinstance(obj, Union):
        return {x[0]: to_dict(getattr(obj, x[0])) for x in obj._fields_ if not x[0].startswith('__')}
    elif isinstance(obj, int):
        return "0x%x" % obj
    else:
        return obj


def ctypes_pformat(cstruct):
    return pformat(to_dict(cstruct))


class MpuTraceHeader(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("magic", c_uint64, 56),
        ("__pad0", c_uint64, 1),
        ("stg", c_uint64, 5),       # sge = 0-5, sgi = 6-11, txdma = 16-23, rxdma = 23-31
        ("mpu", c_uint64, 2),

        ("__pad1", c_uint64, 16),
        ("phv_timestamp_capture", c_uint64, 32),
        ("__pad2", c_uint64, 2),
        ("pkt_size", c_uint64, 14),

        ("mpu_processing_table_addr", c_uint64),

        ("__pad3", c_uint64, 1),
        ("entry_pc", c_uint64, 28),
        ("__pad4", c_uint64, 3),
        ("hash", c_uint64, 32),

        ("mpu_processing_table_latency", c_uint64, 16),
        ("mpu_processing_table_id", c_uint64, 4),
        ("mpu_processing_pkt_id_next", c_uint64, 8),
        ("ring_nonempty", c_uint64, 8),
        ("table_hit", c_uint64, 1),
        ("table_error", c_uint64, 1),
        ("phv_error", c_uint64, 1),
        ("__pad5", c_uint64, 9),
        ("__pad6", c_uint64, 16),

        ("__pad7", c_uint64),

        ("__pad8", c_uint64),

        ("trace_debug_generation", c_uint64, 1),
        ("trace_table_and_key", c_uint64, 1),
        ("__pad9", c_uint64, 14),
        ("timestamp", c_uint64, 48),
    ]


class MpuTraceKD(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("key_data", c_uint8 * 64),
        ("table_data", c_uint8 * 64),
    ]


class MpuTraceInstructionEntry(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("table_valid_bytes_next", c_uint64),

        ("inst_count", c_uint64, 16),
        ("mpu_processing_pkt_id_next", c_uint64, 8),
        ("ex_c_vector", c_uint64, 7),
        ("final_id", c_uint64, 1),
        ("ex_predicate", c_uint64, 1),
        ("ex_pc", c_uint64, 31),

        ("ex_inst", c_uint64),
        ("alu_src1", c_uint64),
        ("alu_src2", c_uint64),
        ("alu_src3", c_uint64),
        ("debug_rdst", c_uint64),

        ("trace_debug_generation", c_uint64, 1),
        ("__pad0", c_uint64, 15),
        ("timestamp", c_uint64, 48)
    ]


class TraceFileHeader(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pipeline_type", c_uint8),
        ("stage_id", c_uint32),
        ("mpu", c_uint32),
        ("enable", c_uint8),
        ("trace_enable", c_uint8),
        ("phv_debug", c_uint8),
        ("phv_error", c_uint8),
        ("watch_pc", c_uint64),
        ("trace_addr", c_uint64),
        ("table_key", c_uint8),
        ("instructions", c_uint8),
        ("wrap", c_uint8),
        ("reset", c_uint8),
        ("trace_size", c_uint32),
        ("__pad", c_int8 * 27)
    ]


def decode_mpu_trace_file(bytez):
    assert (isinstance(bytez, bytes))
    s = 0

    while s < len(bytez):
        if (len(bytez) - s) < sizeof(TraceFileHeader):
            break

        # Read the file header
        fhdr = TraceFileHeader.from_buffer_copy(bytez[s: s + sizeof(TraceFileHeader)])
        # print(ctypes_pformat(fhdr))
        s += sizeof(TraceFileHeader)

        assert(fhdr.trace_size != 0)
        # Read Trace
        for thdr, key, data, instructions in decode_mpu_trace_kd(bytez[s: s + (fhdr.trace_size * 64)]):
            yield fhdr, thdr, key, data, instructions

        s += (fhdr.trace_size * 64)


def decode_mpu_trace_kd(bytez):
    assert (isinstance(bytez, bytes))

    magic = [0xc0, 0xde, 0x41, 0x1c, 0x0d, 0xe4, 0x11]
    for i in range(len(bytez)):
        for j in range(len(magic)):
            if bytez[i + j] != magic[j]:
                break
        else:
            break
    else:
        # Empty trace
        return

    # print(','.join(hex(x) for x in bytez[:64]))

    # print("Found magic at offset ", i)
    from collections import deque
    bytez = deque(bytez)
    bytez.rotate(-i)
    bytez = bytes(bytez)

    # print(','.join(hex(x) for x in bytez[:64]))

    s = 0
    while s < len(bytez):

        if (len(bytez) - s) < sizeof(MpuTraceHeader):
            break

        # Read Header
        thdr = MpuTraceHeader.from_buffer_copy(bytez[s: s + sizeof(MpuTraceHeader)])
        # print(ctypes_pformat(thdr))

        if thdr.magic == 0xc0de411c0de411:
            s += sizeof(MpuTraceHeader)

            if (len(bytez) - s) < sizeof(MpuTraceKD):
                break

            if thdr.trace_table_and_key:
                # Read KD
                kd = MpuTraceKD.from_buffer_copy(bytez[s: s + sizeof(MpuTraceKD)])
                # print(ctypes_pformat(kd))
                s += sizeof(MpuTraceKD)

                instructions = []
                # We need to figure out if there are instruction trace entries
                while s < len(bytez):
                    # Probe the next entry to see if there is valid header there.
                    ent = MpuTraceHeader.from_buffer_copy(bytez[s: s + sizeof(MpuTraceHeader)])
                    if ent.magic == 0xc0de411c0de411:
                        break
                    # It is either an instruction entry or uninitialized memory.
                    ent = MpuTraceInstructionEntry.from_buffer_copy(bytez[s: s + sizeof(MpuTraceInstructionEntry)])
                    # An entry is a valid instruction entry if it has the following relations
                    # with the header entry.
                    if (ent.timestamp >= thdr.timestamp and
                        thdr.trace_debug_generation == ent.trace_debug_generation and
                        thdr.mpu_processing_pkt_id_next == ent.mpu_processing_pkt_id_next):
                        instructions.append(ent)
                    s += sizeof(MpuTraceInstructionEntry)

                # Generate
                yield (
                        thdr,
                        int.from_bytes(kd.key_data, byteorder='big'),
                        int.from_bytes(kd.table_data, byteorder='big'),
                        instructions
                       )
            else:
                yield (
                    thdr,
                    None,
                    None,
                    []
                )
        else:
            s += 1

    else:
        if s != len(bytez):
            raise ValueError("Invalid Trace!")


def decode_instruction(pc, opcode):
    out = libcapisa.c_libcapisa_dasm(c_uint64(pc), c_uint64(opcode))
    s = c_char_p(out).value.decode('utf-8')
    libcapisa.c_libcapisa_freemem(out)
    return s
