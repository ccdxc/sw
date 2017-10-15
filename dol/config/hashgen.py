#! /usr/bin/python3
import binascii
import json
import re
import pdb
import subprocess
import os
from infra.common.glopts import GlobalOptions


MODEL_DEBUG = 'nic/gen/iris/dbg_out/model_debug.json'
CRCHACK = 'nic/third-party/crchack/bin/crchack -w32 -p04c11db7'

def run_cmd(cmd, inp):
    p1 = subprocess.Popen(cmd.split(), stdin=subprocess.PIPE, \
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p1.communicate(inp)
    return (out, err) 

class HashCollider:
    def __init__(self, model_json, crchack):
        dbg_out = None
        try:
            with open(os.path.join(os.environ['WS_TOP'], model_json), 'r') as mf:
                dbg_out = json.load(mf)
        except IOError as e:
            print('HashGen: %s does not exist' % model_json)
            if not GlobalOptions.dryrun:
                raise

        if dbg_out is None:
            self.fields = None
            return

        te = dbg_out['TableEngine']['INGRESS']

        flow_tbl = None
        for s in te.values():
            tbls = s['Tables']
            for tbl in tbls.values():
                if tbl['name'] == 'flow_hash':
                    flow_tbl = tbl
                    break
            if flow_tbl is not None:
                break
        start_off = flow_tbl['start_key_off']
        end_off = flow_tbl['end_key_off']

        # For each of the 512 bits, figure out what goes where
        fields = [None for _ in range(512)]

        start_byte = start_off >> 3
        end_byte = end_off >> 3

        for byte_ in range(start_byte, end_byte+1):
            if byte_ > 31:
                km = flow_tbl['key_maker1']
                byte = byte_-32
            else:
                km = flow_tbl['key_maker0']
                byte = byte_

            bit_extract = False
            val = km['key_maker_bytes'][str(byte)]
            if isinstance(val, str):
                if val == 'UNUSED':
                    continue
                elif 'BIT_EXTRACTION' in val:
                    bit_extract = True

            if bit_extract:
                # Use the bit extractor
                mg = re.search('BIT_EXTRACTION_(\d+)', val)
                bit_extractor_id = int(mg.groups()[0])

                for idx, data in km['key_maker_bits'].items():
                    bit = int(idx)
                    if bit//8 != bit_extractor_id:
                        continue
                    self.__extract_field(data, byte_, bit, bit+1, fields)
                continue
            else:
                self.__extract_field(val, byte_, 0, 8, fields)

        self.fields = fields
        self.default_widths = {
            'lkp_src'   : 128,
            'lkp_dst'   : 128,
            'lkp_proto' : 8,
            'lkp_sport' : 16,
            'lkp_dport' : 16,
            'lkp_dir'   : 1,
            'lkp_type'  : 4,
            'lkp_inst'  : 1,
            'lkp_vrf'   : 16,
        }
        self.crchack = os.path.join(os.environ['WS_TOP'], crchack)


    def __extract_field(self, val, byte, bit_pos_start, bit_pos_end, fields):
        for f, t in val.items():
            if t != 'K':
                continue
            # Got the field which goes into this byte
            mg = re.search('(.*)\.(.*)\[(\d+):(\d+)]', f)
            metadata, field, start, end = mg.groups()
            startbit = int(start)
            endbit = int(end)

            for bit in range(bit_pos_start, bit_pos_end):
                pos = byte * 8 + bit
                lkp = {}
                lkp['name'] = field
                assert startbit <= endbit
                lkp['bigbit'] = startbit
                startbit += 1
                assert fields[pos] == None
                fields[pos] = lkp
                
    def __encode(self, inp):
        val = 0
        for bit, lkp in enumerate(self.fields):
            if lkp is None:
                continue
            field_name = lkp['name']
            field_bit = lkp['bigbit']
            field_width = self.default_widths[field_name]
            bitpos = field_width - field_bit - 1
            
            field = inp[field_name]
            val |= ((field >> bitpos) & 0x1) << (511-bit)

        return val.to_bytes(64, 'big')

    def __decode(self, valbytes):

        outp = {}

        val = int.from_bytes(valbytes, 'big')
        for bit, lkp in enumerate(self.fields):
            if lkp is None:
                continue
            field_name = lkp['name']
            field_bit = lkp['bigbit']
            field_width = self.default_widths[field_name]
            bitpos = field_width - field_bit - 1

            field = outp.setdefault(field_name, 0) 
            field |= ((val >> (511-bit)) & 0x1) << bitpos
            outp[field_name] = field

        return outp

    def search(self, inp, expected_hash):
        if self.fields is None:
            return inp['data']

        val = self.__encode(inp['data'])

        # Call crchack with the right params
        bit_posn_args = []
        for bit, lkp in enumerate(self.fields):
            if lkp is None:
                continue
            field_name = lkp['name']
            field_bit = lkp['bigbit']

            if field_name in inp['modifiers']:
                for start, end in inp['modifiers'][field_name]:
                    if field_bit >= start and field_bit <= end:
                        bit_pos_arg = '-b%d.%d' % (bit//8, bit%8)
                        bit_posn_args.append(bit_pos_arg)
        if len(bit_posn_args) < 32:
            print('Need atleast 32 bits as modifiers')
            return {}

        cmd = [self.crchack] + bit_posn_args + ['-', '0x%x' % expected_hash]

        out,err = run_cmd(' '.join(cmd), val)
        if err:
            print('ERROR ERROR')
            print(err.decode('ascii'))
            return {}

        outp = self.__decode(out)
        return outp

hc = HashCollider(MODEL_DEBUG, CRCHACK)

class TcpUdpHashGen:
    def __init__(self, sip, dip, ipproto, iptype, instance = 0, vrf = 0x1001, dir = 1):
        self.__sip = sip
        self.__dip = dip
        if ipproto == 'TCP':
            self.__proto = 6
        elif ipproto == 'UDP':
            self.__proto = 17
        else:
            assert 0
            
        self.__vrf = vrf
        self.__dir = dir
        self.__instance = instance

        if iptype == 'IPV4':
            # type is 2 for IPv4
            self.__type = 2
        elif iptype == 'IPV6':
            # type is 3 for IPv6
            self.__type = 3
        else:
            assert 0

        self.__sport = 0
        self.__dport = 0
        return

    def Process(self, hashval):

        data = {
                'lkp_src'   : self.__sip,
                'lkp_dst'   : self.__dip,
                'lkp_proto' : self.__proto,
                'lkp_sport' : self.__sport,
                'lkp_dport' : self.__dport,
                'lkp_dir'   : self.__dir,
                'lkp_type'  : self.__type,
                'lkp_inst'  : self.__instance,
                'lkp_vrf'   : self.__vrf,
                }
        inp = { 
                'data':data,
                'modifiers': { 
                                'lkp_sport' : [(0, 15)],
                                'lkp_dport' : [(0, 15)],
                             } 
                }

        outp = hc.search(inp, hashval)

        self.__sport = outp['lkp_sport']
        self.__dport = outp['lkp_dport']
        return

    def GetSport(self):
        return self.__sport

    def GetDport(self):
        return self.__dport
