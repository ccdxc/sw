#! /usr/bin/python3
import binascii
import json
import re
import pdb
import subprocess
import os
from infra.common.glopts import GlobalOptions
import infra.common.objects as objects

asic = os.environ.get('ASIC', 'capri')
MODEL_DEBUG = 'nic/build/x86_64/iris/' + asic + '/gen/p4gen/p4/dbg_out/model_debug.json'
CRCHACK = 'nic/hal/third-party/crchack/bin/crchack -w32 -p04c11db7'
RECIRC_LIMIT = 4

def run_cmd(cmd, inp):
    p1 = subprocess.Popen(cmd.split(), stdin=subprocess.PIPE, \
            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p1.communicate(inp)
    return (out, err) 

class HashValGenerator:
    def __init__(self):
        ''' Class which will give the next hash-value to use according to the 
            policy
        '''

        self.hash_lsb = objects.TemplateFieldObject('range/0x0/0x1fffff')
        self.hash_msb = objects.TemplateFieldObject('range/0x0/0x7ff')
        # For 32 bit collision, hints should be 1
        self.hash_policy = [
                # 2 Flows - 1 recirc
                {'name': '32BIT F1', 'hints': 0, 'hint_depth':1, 'skip':1},
                # 3 Flows - 2 recirc
                {'name': '32BIT F2', 'hints': 0, 'hint_depth':2, 'skip':1},
                # 5 Flows - 4 recirc
                {'name': '32BIT F3', 'hints': 0, 'hint_depth':4, 'skip':1},
                # 2 Flows - 1 recirc
                {'name': '21BIT F1', 'hints': 1, 'hint_depth':1, 'skip':1},
                # 3 Flows - 1 recirc
                {'name': '21BIT F2', 'hints': 2, 'hint_depth':1, 'skip':1},
                # 15 Flows - 2/3 recircs
                {'name': '21BIT F3', 'hints': 7, 'hint_depth':2, 'skip':1},
                ] 
        self.cur_hash_lsb = self.hash_lsb.get()
        self.cur_hash_msb = self.hash_msb.get()
        self.cur_iter = 0
        self.cur_step = 0

    def get(self):
        ''' Get the next Hash value to use '''

        hashval = (self.cur_hash_msb << 21) | self.cur_hash_lsb 


        # Figure out the movement
        it = self.hash_policy[self.cur_iter]
        no_hints = it['hints']
        if it['hints'] == 0:
            # Special case for 32 bit collisions where MSB shouldn't change
            no_hints = 1

        total_flows = 1 + (no_hints * it['hint_depth'])

        if self.cur_step == 0:
            no_recircs = 0
        else:
            no_recircs = (self.cur_step - 1) % it['hint_depth']
            no_recircs += 1

        self.cur_step += 1
        
        drop = False
        skip = False

        if no_recircs >= RECIRC_LIMIT:
            drop = True
        elif self.cur_step > (total_flows - it['skip']):
            skip = True

        if self.cur_step == total_flows:
            # Go to next iterator
            self.cur_step = 0
            self.cur_iter += 1
            if self.cur_iter >= len(self.hash_policy):
                self.cur_iter = 0
            # Change MSB and LSB while going to next iterator
            self.cur_hash_lsb = self.hash_lsb.get()
            self.cur_hash_msb = self.hash_msb.get()
        elif it['hints'] != 0 and (self.cur_step-1) % it['hint_depth'] == 0:
            # Change MSB while going to next hint
            self.cur_hash_msb = self.hash_msb.get()

        return (hashval, skip, drop)


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

        self.hashval_gen = HashValGenerator()
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

    def search(self, inp):
        outp = None
        hashval = None
        skip = None
        drop = None

        if self.fields is None:
            return (inp['data'], hashval, skip, drop)

        hashval,skip,drop = self.hashval_gen.get()

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
            return (outp, hashval, skip, drop)

        cmd = [self.crchack] + bit_posn_args + ['-', '0x%x' % hashval]

        out,err = run_cmd(' '.join(cmd), val)
        if err:
            print('ERROR ERROR')
            print(err.decode('ascii'))
            return (outp, hashval, skip, drop)

        outp = self.__decode(out)
        return (outp, hashval, skip, drop)


class TcpUdpHashGenObject:
    def __init__(self):
        pass

    def Process(self, sip, dip, ipproto, iptype, direction, vrf, instance = 0):
        if ipproto == 'TCP':
            proto = 6
        elif ipproto == 'UDP':
            proto = 17
        else:
            assert 0
            
        if iptype == 'IPV4':
            # type is 2 for IPv4
            lkp_type = 2
        elif iptype == 'IPV6':
            # type is 3 for IPv6
            lkp_type = 3
        else:
            assert 0

        if direction == 'FROM_ENIC':
            lkp_dir = 0
        elif direction == 'FROM_UPLINK':
            lkp_dir = 1
        else:
            assert 0

        if vrf is None:
            if GlobalOptions.dryrun:
                vrf = 0
            else:
                assert 0

        sport = 0
        dport = 0

        data = {
                'lkp_src'   : sip,
                'lkp_dst'   : dip,
                'lkp_proto' : proto,
                'lkp_sport' : sport,
                'lkp_dport' : dport,
                'lkp_dir'   : lkp_dir,
                'lkp_type'  : lkp_type,
                'lkp_inst'  : instance,
                'lkp_vrf'   : vrf,
                }
        inp = { 
                'data':data,
                'modifiers': { 
                                'lkp_sport' : [(0, 15)],
                                'lkp_dport' : [(0, 15)],
                             } 
                }

        outp,hashval,skip,drop = hc.search(inp)

        return (hashval, skip, drop, outp['lkp_sport'], outp['lkp_dport'])

hc = None
TcpUdpHashGen = None

if not GlobalOptions.gft:
    hc = HashCollider(MODEL_DEBUG, CRCHACK)
    TcpUdpHashGen = TcpUdpHashGenObject()

