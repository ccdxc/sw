
from ctypes import *
from pprint import pformat
import os.path
from enum import Enum
from collections import defaultdict
from common import get_bits, get_bit

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

        ("phv_timestamp_capture", c_uint64, 48),
        ("__pad1", c_uint64, 2),
        ("pkt_size", c_uint64, 14),

        ("mpu_processing_table_addr", c_uint64),

        ("__pad2", c_uint64, 1),
        ("entry_pc", c_uint64, 31),
        ("hash", c_uint64, 32),

        ("mpu_processing_table_latency", c_uint64, 16),
        ("mpu_processing_table_id", c_uint64, 4),

        ("sdp_pkt_id", c_uint64, 8),
        ("ring_nonempty", c_uint64, 8),
        ("table_hit", c_uint64, 1),
        ("table_error", c_uint64, 1),
        ("phv_error", c_uint64, 1),
        ("__pad3", c_uint64, 9),
        ("__pad4", c_uint64, 16),

        ("__pad5", c_uint64),

        ("__pad6", c_uint64),

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
        ("end_DS", c_uint64, 1),
        ("predicate", c_uint64, 1),
        ("pc", c_uint64, 34),
        ("C", c_uint64, 7),
        ("c_result", c_uint64, 1),
        ("__pad0", c_uint64, 3),
        ("dual_issue", c_uint64, 1),
        ("inst_count", c_uint64, 16),
        ("rsrcB", c_uint64),
        ("alu_result", c_uint64),
        ("opcode", c_uint64),

        ("tsrc_src2b", c_uint64),
        ("src1b", c_uint64),

        ("__pad1", c_uint64, 48),
        ("__pad2", c_uint64, 13),
        ("inst_sel", c_uint64, 2),
        ("exception_level", c_uint64, 1),
        ("trace_debug_generation", c_uint64, 1),
        ("__pad3", c_uint64, 7),
        ("sdp_pkt_id", c_uint64, 8),
        ("timestamp", c_uint64, 48)
    ]

#todo: shd be 64B aligned
class TraceFileHeader(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pipeline_num", c_uint8),
        ("stage_num", c_uint32),
        ("mpu_num", c_uint32),
        ("enable", c_uint8),
        ("trace_enable", c_uint8),
        ("phv_debug", c_uint8),
        ("phv_error", c_uint8),
        ("trace_addr", c_uint64),
        ("table_key", c_uint8),
        ("instructions", c_uint8),
        ("wrap", c_uint8),
        ("reset", c_uint8),
        ("trace_size", c_uint32),
        ("wpc_trace", c_uint8),
        ("wpc_count", c_uint8),
        ("wpc_intr", c_uint8),
        ("wpc_stop", c_uint8),
        ("wpc_exception", c_uint8),
        ("wpc_addr_lo", c_uint64),
        ("wpc_addr_hi", c_uint64),
        ("wdata_rtrace", c_uint8),
        ("wdata_wtrace", c_uint8),
        ("wdata_rintr", c_uint8),
        ("wdata_wintr", c_uint8),
        ("wdata_rstop", c_uint8),
        ("wdata_wstop", c_uint8),
        ("wdata_rexception", c_uint8),
        ("wdata_wexception", c_uint8),
        ("wdata_addr_lo", c_uint64),
        ("wdata_addr_hi", c_uint64),
        ("debug_index", c_uint32),
        ("debug_generation", c_uint8),
        ("__pad", c_int8 * 49)
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

        #print(sizeof(TraceFileHeader))
        print("\n>>> justina Trace HDR : 0x{:0128x}\n".format(int.from_bytes(fhdr, byteorder='big')))
        for fld in (fhdr._fields_):
            if not fld[0].startswith('_'):
                print("{:50} {:#x}".format(fld[0], getattr(fhdr, fld[0])))

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
        print("Empty trace \n")
        return

    # print(','.join(hex(x) for x in bytez[:64]))

    print("Found magic at offset ", i)
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
                        thdr.sdp_pkt_id == ent.sdp_pkt_id):
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

###################
##### DMA Trace 
###################

#shd be 64B aligned
class DmaTraceFileHeader(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pipeline_num", c_uint8),
        ("enable", c_uint8),
        ("phv_enable", c_uint8),
        ("capture_all", c_uint8),
        ("axi_err_enable", c_uint8),
        ("pkt_phv_sync_err_enable", c_uint8),
        ("wrap", c_uint8),
        ("reset", c_uint8),
        ("buf_size", c_uint32),
        ("base_addr", c_uint64),
        ("trace_index", c_uint32),
        ("__pad", c_int8 * 40),
]

#Each trace entry is {DmaTraceInfo, DmaLatInfo, DmaLatCmd} = 64B
class DmaTraceEntry(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("__pad0", c_uint64, 64),

        ("__pad1", c_uint64, 16),
        ("pkt_line_size", c_uint64, 6),
        ("pkt_seq_id", c_uint64, 11),
        ("pkt_linenum", c_uint64, 8),
        ("pkt_sop", c_uint64, 1),
        ("pkt_eop", c_uint64, 1),
        ("p_ff_empty", c_uint64, 1),
        ("trace_code", c_uint64, 4),
        ("txs_lif"         , c_uint64, 11), #LatInfo ends here
        ("u_axi_rd_addr" , c_uint64, 5),

        ("l_axi_rd_addr" , c_uint64, 59),
        ("cmdptr_cmd"     , c_uint64, 1 ),
        ("u_timestamp"  , c_uint64, 4),

        ("l_timestamp"  , c_uint64, 44),
        ("qtype" 	  , c_uint64, 3 ),
        ("u_qid"        , c_uint64, 17),

        ("l_qid"        , c_uint64, 7),
        ("phv_id"         , c_uint64, 10),
        ("cmd_flit_num"   , c_uint64, 4 ),
        ("dummy_nop"      , c_uint64, 1 ),
        ("debug_trace"    , c_uint64, 1 ),
        ("frame_size"     , c_uint64, 14),
        ("tm_iq"          , c_uint64, 5 ),
        ("tm_oq"          , c_uint64, 5 ),
        ("seq_id_no_data" , c_uint64, 11),
        ("u_seq_id_pkt" , c_uint64, 6),

        ("l_seq_id_pkt" , c_uint64, 5),
        ("seq_id_all"     , c_uint64, 11),
        ("no_data"        , c_uint64, 1 ),
        ("phv_upr"        , c_uint64, 3 ),
        ("lif_id"         , c_uint64, 11),
        ("rd_id_slot"     , c_uint64, 10),
        ("rd_id"          , c_uint64, 7 ),
        ("latcmd_upr", c_uint64, 16),           #lat_cmd ends here (128+16)

        ("u_cmd", c_uint64, 64),

        ("l_cmd", c_uint64, 60),
        ("cmdeop", c_uint64, 1),
        ("cmdtype", c_uint64, 3),
]

### DmaLatCmd maps to one below commands
#18B - round to 64B
class DmaLatPhv2Mem(BigEndianStructure):
    _pack_ = 1
    _fields_ = [

        ("__pad0", c_int8 * 48),

        ("__pad1", c_uint64, 6),
        ("pcie_msg", c_uint64, 1),
        ("barrier", c_uint64, 1),
        ("wr_round", c_uint64, 1),
        ("wr_data_fence", c_uint64, 1),
        ("wr_fence_fence", c_uint64, 1),
        ("u_wr_axi_addr", c_uint64, 53),

        ("l_wr_axi_addr", c_uint64, 45),
        ("wr_cache", c_uint64, 1),
        ("phv_size", c_uint64, 14),
        ("cmdeop", c_uint64, 1),
        ("cmdtype", c_uint64, 3),
]

#18B - mem2pkt and mem2mem - round to 64B
class DmaLatMemRead(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("__pad0", c_int8 * 40),

        ("__pad1"           , c_uint64, 55 ),
        ("rd_id_req_64byte", c_uint64, 1 ),
        ("rd_id_inc"       , c_uint64, 1 ),
        ("rd_sz_64byte"    , c_uint64, 1 ),
        ("wr_round"        , c_uint64, 1 ),
        ("wr_data_fence"   , c_uint64, 1 ),
        ("wr_fence_fence"  , c_uint64, 1 ),
        ("wr_axi_addr_upr1" , c_uint64, 3),

        ("u_wr_axi_addr"   , c_uint64, 64),

        ("l_wr_axi_addr"   , c_uint64, 31),
        ("wr_cache"        , c_uint64, 1 ),
        ("rd_id"           , c_uint64, 7 ),
        ("psize"           , c_uint64, 14),
        ("start_offset"    , c_uint64, 6 ),
        ("pkteop"          , c_uint64, 1 ),
        ("cmdeop"          , c_uint64, 1 ),
        ("cmdtype"         , c_uint64, 3 ),
]

#18B - round to 64B
class DmaLatPkt2Mem(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("__pad0", c_int8 * 48),

        ("__pad1", c_uint64, 7),
        ("wr_round", c_uint64, 1),
        ("wr_data_fence", c_uint64, 1),
        ("wr_fence_fence", c_uint64, 1),
        ("u_wr_axi_addr", c_uint64, 54),

        ("l_wr_axi_addr", c_uint64, 44),
        ("wr_cache", c_uint64, 1),
        ("skip_to_eop", c_uint64, 1),
        ("psize", c_uint64, 14),
        ("cmdeop", c_uint64, 1),
        ("cmdtype", c_uint64, 3),
]

#18B - round to 64B
class DmaLatSkip(BigEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("__pad0", c_int8 * 56),

        ("__pad1", c_uint64, 45),
        ("skip_to_eop", c_uint64, 1),
        ("psize", c_uint64, 14),
        ("cmdeop", c_uint64, 1),
        ("cmdtype", c_uint64, 3),
]

class CMDTYPE(Enum):
    NOP     = 0
    mem2pkt = 1
    phv2pkt = 2
    phv2mem = 3
    pkt2mem = 4
    skip    = 5
    mem2mem = 6
    cmdptr  = 7 

class DMAPIPE(Enum):
    RxPDMA = 1
    TxPDMA = 0

def decode_dma_trace_file(bytez, print_type, sort_type):
    #print ("test_def\n")
    #print("\n>>> justina DMA Trace : numbytes in file : {}\n".format(len(bytez)))
    assert (isinstance(bytez, bytes))
    s = 0

    if (len(bytez) - s) < sizeof(DmaTraceFileHeader):
        print("\n>>> justina DMA Trace. Not enough bytes for DMA Trace Header.\n")
        
    #Create 2 fhdr for PRD and PTD. Each fhdr has a list of trace_dict_entries    
    while s < len(bytez):
        if (len(bytez) - s) < sizeof(TraceFileHeader):
            break

        # Read the file header
        fhdr = DmaTraceFileHeader.from_buffer_copy(bytez[s: s + sizeof(DmaTraceFileHeader)])
        # print(ctypes_pformat(fhdr))
        s += sizeof(DmaTraceFileHeader)

        print("\n>>> DMA Trace HDR : 0x{:0128x}\n".format(int.from_bytes(fhdr, byteorder='big')))
        for fld in (fhdr._fields_):
            if not fld[0].startswith('_'):
                if fld[0].startswith('pipeline'):
                    #print pipeline as RXPDMA/TXPDMA instead of 1/0
                    print("{:50} {:#x} ({})".format(fld[0], getattr(fhdr, fld[0]), DMAPIPE(getattr(fhdr, fld[0])).name))
                else:
                    print("{:50} {:#x}".format(fld[0], getattr(fhdr, fld[0])))
                
        assert(fhdr.buf_size != 0)
        #print("s0 is : {}\n".format(s))
        # decode entries. Each thdr has entire capture buffer for that pipe
        # thdr is a list of dictionaries. Each dictionary has key-pair values of DmaTraceEntry
        k = 0
        thdrList = decode_dmatrace_entry(bytez[s: s + (fhdr.buf_size * 64)])
        #print ("thdrList size is : {}\n".format(len(thdrList)))

        #sort the list by timestamp field before printing
        if not sort_type:
            sortedThdrList = thdrList
        else:
            sortedThdrList = sorted(thdrList, key=lambda x: x[sort_type])

        if (not print_type):
            dma_print_def(sortedThdrList)
        elif (print_type == "short"):
            dma_print_short(sortedThdrList)
        elif (print_type == "all"):
            dma_print_all(sortedThdrList)
        else:
            dma_print_def(sortedThdrList)

        # Jump to end of Trace entries for this pipe
        s += (fhdr.buf_size * 64)
        #print("s1 is : {}\n".format(s))

    return
    
def decode_dmatrace_entry(bytez):
    #print("\n>>> justina DMA Trace_entry : numbytes in file : {}\n".format(len(bytez)))
    assert (isinstance(bytez, bytes))
    s = 0
        
    #this is a list of data structures
    trace_entries = []
    #this is a list of dictionaries
    trace_dict_entries = []
    i = 0
    entryDict = {}
    while s < len(bytez):
        if (len(bytez) - s) < sizeof(DmaTraceEntry):
            break

        # Read the trace entry
        #Each trace entry is {DmaTraceInfo, DmaLatInfo, DmaLatCmd}
        #Each trace entry has a list associated with it

        Tinfo = DmaTraceEntry.from_buffer_copy(bytez[s: s + sizeof(DmaTraceEntry)])
        s += sizeof(DmaTraceEntry)
        
        #add DmaTraceEntry to the list. This is not used. Dictionary is used instead
        #since key-pair values combines u_* and l_* fields
        trace_entries.append(Tinfo)
        #create a dictionary with DmaTraceEntry and add it to the list
        entryDict = dma_dict_trace_entry(Tinfo)
        trace_dict_entries.append(entryDict)

        i = i + 1

    k=0
    for j in trace_dict_entries:
        #for s in j:
        #    print("{:50} {:#x}".format(s, j[s]))
        k = k+1
        #print("k is {}\n".format(k))

    #print("\n>> Length of TraceDictEntries list is : {:d}\n".format(len(trace_dict_entries)))
    return trace_dict_entries

#Use this function to print the datastructure which collapses u_* and l_* fields
def dma_print_trace_entry(bytez):
    #print("\n>>> DMA trace entry : 0x{:0128x}\n".format(int.from_bytes(bytez, byteorder='big')))
    
    #Some fields in struct had to be separated with l_ and u_ since 
    #the fields needs to split at 64b for packing to work. The code
    #below combines u_ and l_ fields by doing (u_* << l_*<width> + l_*) 
    #for printing log
    k=0
    for fld in (bytez._fields_):
        if not fld[0].startswith('_'):
            if fld[0].startswith('u_'):
                #print("{:50} {:#x} ".format(fld[0], getattr(bytez, fld[0])))
                k = getattr(bytez, fld[0])
            elif fld[0].startswith('l_'):
                #print("left shift {:#x} by {} ".format(k, fld[2]))
                j = getattr(bytez, fld[0]) | k << fld[2]
                k = 0
                #print("{:50} {:#x} ".format(fld[0], getattr(bytez, fld[0])))
                s = fld[0].split('l_')
                print("{:50} {:#x} ".format(s[1], j))

            else:
                print("{:50} {:#x}".format(fld[0], getattr(bytez, fld[0])))
    
    

    print ("\n")                
    return

#This function converts DmaTraceEntry datastructure to a dictionary of key-val pairs
def dma_dict_trace_entry(bytez):

    mydict = {}

    k=0
    for fld in (bytez._fields_):
        if not fld[0].startswith('_'):
            if fld[0].startswith('u_'):
                #print("{:50} {:#x} ".format(fld[0], getattr(bytez, fld[0])))
                k = getattr(bytez, fld[0])
            elif fld[0].startswith('l_'):
                #print("left shift {:#x} by {} ".format(k, fld[2]))
                j = getattr(bytez, fld[0]) | k << fld[2]
                k = 0
                #print("{:50} {:#x} ".format(fld[0], getattr(bytez, fld[0])))
                s = fld[0].split('l_')
                #print("{:50} {:#x} ".format(s[1], j))
                mydict[s[1]] = j

            else:
                mydict[fld[0]] = getattr(bytez, fld[0])
                #print("{:50} {:#x}".format(fld[0], getattr(bytez, fld[0])))
    
    
    #for key in (mydict):
        #print("{:50} {:#x}".format(key, mydict[key]))

    return mydict                


#this is the default print for DMA Trace
def dma_print_def(list_of_entries):
    
    flds_to_print = ["timestamp",
                     "trace_code", 
                     "cmdeop",
                     "cmdtype",
                     "no_data",
                     "lif_id",
                     "txs_lif",
                     "qtype",
                     "qid",
                     "phv_id",
                     "debug_trace",
                     "tm_iq",
                     "tm_oq",
                     "pkt_seq_id",
                     "seq_id_no_data",
                     "seq_id_pkt",
                     "seq_id_all",
                     "axi_rd_addr"]


    dma_print_row(flds_to_print, list_of_entries)
    return

#this is the default print for DMA Trace
def dma_print_short(list_of_entries):
    
    flds_to_print = ["timestamp",
                     "trace_code", 
                     "cmdeop",
                     "cmdtype",
                     "no_data",
                     "lif_id",
                     "qtype",
                     "qid",
                     "phv_id"]



    dma_print_row(flds_to_print, list_of_entries)
    return

def dma_print_all(list_of_entries):
    
    print("\n")
    
    for thdr in list_of_entries:
        for ent in thdr:
            if ent.startswith('cmdtype'):
                print("{:50} {} ({:#x})".format(ent, CMDTYPE(thdr[ent]).name, thdr[ent]))
            else:
                print("{:50} {:#x}".format(ent, thdr[ent]))
        print(" ")
        print("\tCommand({}) fields :".format(CMDTYPE(thdr['cmdtype']).name))
        
        cmd_bytez = 1 << 512 | thdr['cmd'] << 4 | thdr['cmdeop'] << 3 | thdr['cmdtype']
        print("\tCMD bytes: {}".format(hex(get_bits(cmd_bytez,0,511))))

        if (CMDTYPE(thdr[ent]).name == "skip"):
            t1 = DmaLatSkip()
        elif (CMDTYPE(thdr[ent]).name == "pkt2mem"):    
            t1 = DmaLatPkt2Mem()
        elif (CMDTYPE(thdr[ent]).name == "phv2mem"):    
            t1 = DmaLatPhv2Mem()
        else:    
            t1 = DmaLatMemRead()

        if ((CMDTYPE(thdr[ent]).name == "NOP") |    
            (CMDTYPE(thdr[ent]).name == "cmdptr")):    
            print("\n")
        else:     
            for fld in reversed(t1._fields_):
                if not fld[0].startswith('_'):
                    if (fld[2] == 1):
                        extract_fld = get_bit(cmd_bytez,0)
                    else:
                        extract_fld = get_bits(cmd_bytez,0,fld[2]-1)
                    if fld[0].startswith('l_'):
                        k = extract_fld
                        q = fld[2]
                    elif fld[0].startswith('u_'):
                        j = extract_fld << q | k
                        s = fld[0].split('u_')
                        print("\t{:50} {:#x} ".format(s[1], j))
                    else:
                        print("\t{:50} {:#x} ".format(fld[0], extract_fld))
                        cmd_bytez = cmd_bytez >> fld[2]        
                        #print(hex(cmd_bytez))
            print("\n")
        

    print("\n")
    return

#this is the default print for DMA Trace
def dma_print_row(flds_to_print, list_of_entries):
    
    HdrLine = " "
    for ent in flds_to_print: 
        HdrLine = HdrLine  + "{:<15}".format(ent)

    print("\n")    
    print(HdrLine.upper())
    print(" ")        
    k=0
    for thdr in list_of_entries:
        #k=k+1
        EntryLine = " "
        for ent in flds_to_print:
            if ent.startswith('cmdtype'):
                #print("{:50} {}".format(ent, CMDTYPE(thdr[ent]).name))
                EntryLine = EntryLine  + "{:<15}".format(CMDTYPE(thdr[ent]).name)
            else:
                #print("{:50} {:#x}".format(ent, thdr[ent]))
                EntryLine = EntryLine  + "{:<15}".format(str(hex(thdr[ent])))
        print(EntryLine)
        #print("k is {}\n".format(k))

    print("\n")    
    return
