#!/usr/bin/python
#
import os
import sys
import math
import re
import pdb
import itertools
from string import Template

# globals
hash_field_cnt = 0

# key: split_field_name, value: list of tuples (field_name, sbit, ebit)
split_fields_dict = {}

# key: split_field_name, value: split field width
split_fields_width_dict = {}

hash_hint_fields_dict = {}

# list of fields other than key/data
hash_field_list = ['hash', 'more_hashes']
hint_field_list = ['hint', 'more_hints']
other_fields_list = ['hash', 'hint', 'more_hashes', 'more_hints', 'pad', 'entry_valid']
#other_fields_list = ['pad'].extend(hash_hint_field_list)

# constants
field_bit_unit = 64
field_bit_arr_unit = 8
field_bit_arr_shift = int(math.log(field_bit_arr_unit, 2))

def get_field_bit_unit():
    return field_bit_unit

def get_field_bit_arr_unit():
    return field_bit_arr_unit

def get_field_bit_arr_shift():
    return field_bit_arr_shift

# convert into bit array
def get_bit_arr_length(field_width):
    return (field_width + get_field_bit_arr_unit()-1) >> get_field_bit_arr_shift()

# TODO identify field
def is_field_hash(field_name):
    return 'hash' in field_name;

def is_table_ftl_gen(table, pddict):
    # generate for all tables for apulu
    if pddict['pipeline'] == 'apulu' and not pddict['p4plus']:
        return True

    # generate only for tables with @pragma capi_bitfields_struct
    for annotation_dict in pddict['tables'][table]['annotations']:
        if 'capi_bitfields_struct' in annotation_dict:
            return True
    return False

# hbm based table
def is_table_hbm_table(table, pddict):
    if pddict['tables'][table]['location'] == 'HBM':
       return True
    return False

# index based table
def is_table_index_based(table, pddict):
    return pddict['tables'][table]['type'] == 'Index'

# tcam based table
def is_table_tcam_based(table, pddict):
    return pddict['tables'][table]['type'] == 'Ternary'

# Hash based table
def is_table_hash_based(table, pddict):
    return pddict['tables'][table]['type'] == 'Hash'

# dont generate key struct for index based tables
def is_table_gen_key(table, pddict):
    return not is_table_index_based(table, pddict)

# for hash based tables, generate key along with data
def is_table_gen_key_with_data(table, pddict):
    return is_table_hash_based(table, pddict)

# generate hashes/hints for hash based table
def is_table_gen_hints(table, pddict):
    return is_table_hash_based(table, pddict)

# generate c compatible struct
def is_table_gen_c_compatible(table, pddict):
    return is_table_index_based(table, pddict)

# TODO use pragmas
def is_table_pad_256(table, pipeline):
    if 'v4' in str(table):
        return True

    # TODO nexthop and session_track is 256 bits in Apulu
    if pipeline == 'apulu' and ('nexthop' in str(table) or 'session_track' in str(table) or 'nat' in str(table) or 'session' in str(table)):
        return True

    if pipeline == 'athena' and ('session_rewrite' == str(table)):
        return True

    return False

# TODO use pragmas
def is_table_pad_128(table, pipeline):
    # TODO nexthop and session_track is 256 bits in Apulu
    if pipeline == 'apulu' and ('ip_mac_binding' in str(table)):
        return True

    return False

# TODO use pragmas
def is_table_pad_64(table, pipeline):
    if pipeline == 'apulu' and (('local_mapping_tag' in str(table)) or ('mapping_tag' in str(table))):
        return True
    return False

# TODO use pragmas
def is_table_pad_32(table, pipeline):
    if pipeline == 'athena' and ('conntrack' in str(table)):
        return True
    return False

# TODO use pragmas
def is_table_pad_16(table, pipeline):
    if pipeline == 'athena' and (('to_vnic' in str(table)) or 'config2' == str(table) or 'config1' == str(table)):
        return True
    return False

def is_hash_field(field_name):
    for field in hash_field_list:
        if field_name.startswith(field):
            return True
    return False

def is_hint_field(field_name):
    for field in hint_field_list:
        if field_name.startswith(field):
            return True
    return False

def is_hash_hint_field(field_name):
    for field in itertools.chain(hash_field_list, hint_field_list):
        if field_name.startswith(field):
            return True
    return False

def is_key_appdata_field(field_name):
    for field in other_fields_list:
        if field in field_name:
            return False
    return True

def hash_hint_field_slot(field_name):
    _list = re.findall(r'[0-9]+', field_name)
    if not _list:
        return 0
    else:
        return _list[0]

def insert_hash_hint_fields_db(field_obj):
    global hash_hint_fields_dict

    slot = int(hash_hint_field_slot(field_obj.name()))
    if slot in hash_hint_fields_dict:
        hash_hint_fields_dict[slot].append(field_obj)
    else:
        hash_hint_fields_dict[slot] = [field_obj]

def get_hash_hint_fields_db(slot):
    if slot in hash_hint_fields_dict:
        return hash_hint_fields_dict[slot]
    else:
        return None

def hash_hint_fields_dict_reset():
    global hash_hint_fields_dict
    hash_hint_fields_dict = {}

def find_field(field, field_list):
    for _field in field_list:
        if _field.name() == field.name():
            return True
    return False

######################
# split field helpers
######################
def is_field_split(field_name):
    return 'sbit' in field_name and 'ebit' in field_name

def get_split_field_name(field_name):
    return field_name.split('_sbit')[0]

def get_split_field_sbit_ebit(field_name):
    _list = re.findall(r'[0-9]+', field_name)
    sbit = int(_list[-2])
    ebit = int(_list[-1])
    return sbit, ebit

def get_split_field(field_name):
    if field_name in split_fields_dict:
        return split_fields_dict[field_name]
    else:
        return None

def insert_split_fields_db(field_obj):
    global split_fields_dict

    split_field_name = field_obj.split_name()
    if split_field_name in split_fields_dict:
        split_fields_dict[split_field_name].append(field_obj)
    else:
        split_fields_dict[split_field_name] = [field_obj]

    ebit = field_obj.ebit()

    if split_field_name in split_fields_width_dict:
        if split_fields_width_dict[split_field_name] < (ebit + 1):
            split_fields_width_dict[split_field_name] = ebit + 1
    else:
        split_fields_width_dict[split_field_name] = ebit + 1

def get_split_field_width(field_name):
    if field_name in split_fields_width_dict:
        return split_fields_width_dict[field_name]
    else:
        return 0

def split_fields_dict_reset():
    global split_fields_dict
    global split_fields_width_dict

    split_fields_dict = {}
    split_fields_width_dict = {}

def next_pow_2(num):
    # return same number if already power of 2
    if num & (num-1) == 0:
        return num

    i = 0
    while (num != 0):
        num = num >> 1
        i += 1
    return (1 << i)

def validate_field_width_arr(field_name, field_width):
    # if the field width (in bits) is not a multiple of 8 (bits),
    # then creating uint8_t array needs special handling
    if field_width % get_field_bit_arr_unit() != 0:
        raise Exception('field ' + field_name + ' width ' + str(field_width) + ' not multiple of '+ str(get_field_bit_arr_unit()))

def get_byte_from_bit (bit):
    return bit/8

set_field_template_1 = Template(
"""\
${field_name} = (${field_arg} >> ${shift}) & ${mask_str};\
""")

set_field_template_2 = Template(
"""\
memcpy(${field_name}, ${field_arg}, ${arr_len});\
""")

set_field_template_3 = Template(
"""\
memset(${field_name}, ${field_arg}, ${arr_len});\
""")

get_field_template_1 = Template(
"""\
${field_arg} |= ((${field_name} & ${mask_str}) << ${shift});\
""")

get_field_template_2 = Template(
"""\
memcpy(${field_arg}, ${field_name}, ${arr_len});\
""")

#############
# Field class
#############
class Field:
    def __init__(self, name, width, little_str):
        self._name = name
        self._width = width
        self._little_str = little_str

    def name(self):
        return self._name

    def width(self):
        return self._width

    def little_str(self):
        return self._little_str

    def field_type_str(self):
        field_width = self.width()
        if field_width > get_field_bit_unit():
            return 'uint' + str(get_field_bit_arr_unit()) + "_t *"
        else:
            bit_len = next_pow_2(field_width)
            if bit_len < 8:
                bit_len = 8
            return 'uint' + str(bit_len) + '_t'

    def is_hash_hint_field(self):
        return is_hash_hint_field(self.name())

    def is_hash_field(self):
        return is_hash_field(self.name())

    def is_hint_field(self):
        return is_hint_field(self.name())

    def is_key_appdata_field(self):
        return is_key_appdata_field(self.name())

    def is_field_hash(self):
        return is_field_hash(self.name())

    def is_field_split(self):
        return is_field_split(self.name())

    def set_field_str(self, field_arg, handle_split = True):
        field_name = self.name()
        field_width = self.width()
        field_type_str = self.field_type_str()
        field_str_list = []
        # do not generate for split fields in cases where
        # destination is a struct
        if self.is_field_split() and handle_split == True:
            split_field_name = self.split_name()
            split_fields_list = get_split_field(split_field_name)
            split_field_width = get_split_field_width(split_field_name)
            # handle case if total width is > 64
            if split_field_width > 64:
                for split_field in split_fields_list:
                    field_name = split_field.name()
                    sbit = split_field.sbit()
                    ebit = split_field.ebit()
                    # if total width > 64, then argument is uint8_t array
                    if (ebit - sbit + 1) < 8:
                        mask = 0
                        for i in range(ebit-sbit+1):
                            mask |= (1 << i)
                        # starting bit for this field in the argument
                        start_bit = split_field_width - (ebit + 1)
                        # calculate the corresponding byte to index in argument
                        start_byte = get_byte_from_bit(start_bit)
                        field_arg_index_str = field_arg + '[' + str(start_byte) + ']'
                        # bit shift within the corresponding byte
                        shift = 8 - (ebit + 1)
                        field_str_list.append(set_field_template_1.substitute(field_name=field_name, field_arg=field_arg_index_str, shift=shift, mask_str=str(hex(mask))))
                    else:
                        # starting bit for this field in the argument
                        start_bit = split_field_width - (ebit + 1)
                        # calculate the corresponding byte to index in argument
                        start_byte = get_byte_from_bit(start_bit)
                        # ending bit for this field in argument
                        end_bit = split_field_width - (sbit + 1)
                        # calculate the corresponding byte to index in argument
                        end_byte = get_byte_from_bit(end_bit)
                        # offset from which memcpy should start
                        start_offset =  '&' + field_arg + '[' + str(start_byte) + ']'
                        field_str_list.append(set_field_template_2.substitute(field_name=field_name, field_arg=start_offset, arr_len=str(end_byte - start_byte + 1)))
            else:
                for split_field in split_fields_list:
                    field_name = split_field.name()
                    sbit = split_field.sbit()
                    ebit = split_field.ebit()
                    mask = 0
                    for i in range(ebit-sbit+1):
                        mask |= (1 << i)
                    shift = split_field_width - (ebit + 1)
                    field_str_list.append(set_field_template_1.substitute(field_name=field_name, field_arg=field_arg, shift=shift, mask_str=str(hex(mask))))
        else:
            # clear/copy methods can set the fields individually and hence handle_split=False.
            # if the split field width > 64, then use memcpy/memset when handle_split=False
            if field_width > get_field_bit_unit() or (self.is_field_split() and get_split_field_width(self.split_name()) > get_field_bit_unit()):
                validate_field_width_arr(field_name, field_width)
                arr_len = get_bit_arr_length(field_width)
                if field_arg != '0':
                    field_str_list.append(set_field_template_2.substitute(field_name=field_name, field_arg=field_arg, arr_len=arr_len))
                else:
                    field_str_list.append(set_field_template_3.substitute(field_name=field_name, field_arg=field_arg, arr_len=arr_len))
            else:
                field_str_list.append(field_name + ' = ' + field_arg + ';')
        return field_str_list

    def get_field_str(self, field_arg, handle_split = True):
        field_name = self.name()
        field_width = self.width()
        field_type_str = self.field_type_str()
        field_str_list = []
        # do not generate for split fields in cases where
        # destination is a struct
        if self.is_field_split() and handle_split == True:
            split_field_name = self.split_name()
            split_fields_list = get_split_field(split_field_name)
            split_field_width = get_split_field_width(split_field_name)
            # handle case if total width is > 64
            if split_field_width > 64:
                if split_field_width % 8 != 0:
                    raise Exception(split_field + ' is not a multiple of 8')
                for split_field in split_fields_list:
                    field_name = split_field.name()
                    sbit = split_field.sbit()
                    ebit = split_field.ebit()
                    # if total width > 64, then argument is uint8_t array
                    if (ebit - sbit + 1) < 8:
                        mask = 0
                        for i in range(ebit-sbit+1):
                            mask |= (1 << i)
                        # starting bit for this field in the argument
                        start_bit = split_field_width - (ebit + 1)
                        # calculate the corresponding byte to index in argument
                        start_byte = get_byte_from_bit(start_bit)
                        field_arg_index_str = field_arg + '[' + str(start_byte) + ']'
                        # bit shift within the corresponding byte
                        shift = 8 - (ebit + 1)
                        field_str_list.append(get_field_template_1.substitute(field_name=field_name, field_arg=field_arg_index_str, shift=shift, mask_str=str(hex(mask))))
                    else:
                        # starting bit for this field in the argument
                        start_bit = split_field_width - (ebit + 1)
                        # calculate the corresponding byte to index in argument
                        start_byte = get_byte_from_bit(start_bit)
                        # ending bit for this field in argument
                        end_bit = split_field_width - (sbit + 1)
                        # calculate the corresponding byte to index in argument
                        end_byte = get_byte_from_bit(end_bit)
                        # offset from which memcpy should start
                        start_offset =  '&' + field_arg + '[' + str(start_byte) + ']'
                        field_str_list.append(get_field_template_2.substitute(field_name=field_name, field_arg=start_offset, arr_len=str(end_byte - start_byte + 1)))
            else:
                for split_field in split_fields_list:
                    field_name = split_field.name()
                    sbit = split_field.sbit()
                    ebit = split_field.ebit()
                    mask = 0
                    for i in range(ebit-sbit+1):
                        mask |= (1 << i)
                    shift = split_field_width - (ebit + 1)
                    field_str_list.append(get_field_template_1.substitute(field_name=field_name, field_arg=field_arg, shift=shift, mask_str=str(hex(mask))))
        else:
            if field_width > get_field_bit_unit():
                validate_field_width_arr(field_name, field_width)
                arr_len = get_bit_arr_length(field_width)
                field_str_list.append(get_field_template_2.substitute(field_name=field_name, field_arg=field_arg, arr_len=arr_len))
            else:
                field_str_list.append(field_arg + ' = ' + field_name + ';')
        return field_str_list

###################
# SplitField class
###################
class SplitField(Field):
    def __init__(self, name, width, little_str, split_name, sbit, ebit):
        Field.__init__(self, name, width, little_str)
        self._split_name = split_name
        self._sbit = sbit
        self._ebit = ebit

    def split_name(self):
        return self._split_name

    def sbit(self):
        return self._sbit

    def ebit(self):
        return self._ebit

    def field_type_str(self):
        return 'uint' + str(get_field_bit_unit()) + '_t'

####################
# hash field methods
####################
def ftl_hash_field_cnt():
    global hash_field_cnt
    return hash_field_cnt

def ftl_hash_field_cnt_reset():
    global hash_field_cnt

    hash_field_cnt = 0

##############
# print fields
##############
def ftl_field_print_str(field_obj):
    field_name = field_obj.name()
    field_width = field_obj.width()
    if field_width > get_field_bit_unit() or (field_obj.is_field_split() and get_split_field_width(field_obj.split_name()) > get_field_bit_unit()):
        args_list = []
        validate_field_width_arr(field_name, field_width)
        arr_len = get_bit_arr_length(field_width)
        for i in range(0, arr_len):
            args_list.append(field_name + '[' + str(i) + ']')
        return (field_name + ':%02x' * arr_len, args_list)
    else:
        return (field_name + ': %lu', [field_name])

def ftl_field_print(fields_list):
    format_list = []
    args_list = []
    for field_obj in fields_list:
        field_name = field_obj.name()
        if field_name == '__pad_key_bits' or field_name == '__pad_to_512b':
            continue
        format_str, _args_list = ftl_field_print_str(field_obj)
        format_list.append(format_str)
        args_list.extend(_args_list)
    return format_list, args_list

##############
# store fields
##############
def ftl_store_field(fields_dict, fields_list, field_name, field_width, little_str):
    global hash_field_cnt

    if is_field_split(field_name):
        split_field_name = get_split_field_name(field_name)
        sbit, ebit = get_split_field_sbit_ebit(field_name)
        field_obj = SplitField(field_name, field_width, little_str, split_field_name, sbit, ebit)
        insert_split_fields_db(field_obj)
    else:
        field_obj = Field(field_name, field_width, little_str)

    if field_obj.is_hash_hint_field():
        insert_hash_hint_fields_db(field_obj)

    fields_dict[field_name] = field_obj
    fields_list.append(field_obj)

    # count number of hash fields
    if field_obj.is_field_hash():
        hash_field_cnt += 1

def ftl_store_field_splitted(fields_dict, fields_list, field_name, split_field_name, field_width, sbit, ebit):
    global hash_field_cnt

    field_obj = SplitField(field_name, field_width, '', split_field_name, sbit, ebit)
    insert_split_fields_db(field_obj)

    if field_obj.is_hash_hint_field():
        insert_hash_hint_fields_db(field_obj)

    fields_dict[field_name] = field_obj
    fields_list.append(field_obj)

    # count number of hash fields
    if field_obj.is_field_hash():
        hash_field_cnt += 1

def ftl_store_field_full(fields_dict, fields_list, field_name, field_width):
    global hash_field_cnt

    field_obj = Field(field_name, field_width, '')

    fields_dict[field_name] = field_obj
    fields_list.append(field_obj)

    if field_obj.is_hash_hint_field():
        insert_hash_hint_fields_db(field_obj)

    # count number of hash fields
    if field_obj.is_field_hash():
        hash_field_cnt += 1

################
# process field
################
def ftl_process_field(field_obj):
    field_name = field_obj.name()
    field_width = field_obj.width()
    little_str = field_obj.little_str()
    field_bit_arr_unit = get_field_bit_arr_unit()
    field_str_list = []
    if field_width > get_field_bit_unit() or (field_obj.is_field_split() and get_split_field_width(field_obj.split_name()) > get_field_bit_unit()):
        # if the width is not aligned to array unit, align it to previous
        # boundary and generate new field with the remaining bits
        if field_width % field_bit_arr_unit != 0:
            field_width_aligned = (field_width/field_bit_arr_unit) * field_bit_arr_unit
            remaining_bits = field_width - field_width_aligned
            field_str_list.append("uint" + str(field_bit_arr_unit) + "_t " + field_name + "[" + str(get_bit_arr_length(field_width_aligned)) + "];")
            field_str_list.append("uint" + str(get_field_bit_arr_unit()) + "_t " + field_name + "_align" + " : " + str(remaining_bits) + ";")
        else:
            field_str_list.append("uint" + str(field_bit_arr_unit) + "_t " + field_name + "[" + str(get_bit_arr_length(field_width)) + "];")
    else:
        width = str(field_width) + little_str
        field_str_list.append("uint" + str(get_field_bit_unit()) + "_t " + field_name + " : " + width + ";")
    return field_str_list

ftl_table_template_hpp = Template(
"""\
class ${table_name}: public ftl_base {
public:
    static ftl_base *factory(sdk_table_factory_params_t *params);
    static void destroy(ftl_base *f);
    virtual void set_thread_id(uint32_t id) override;

    ${table_name}() {}
    ~${table_name}(){}

    virtual base_table_entry_t *get_entry(int index) override;

protected:
    virtual sdk_ret_t genhash_(sdk_table_api_params_t *params) override;
    virtual uint32_t thread_id(void) override;
private:
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    static thread_local ${table_entry_name} entry_[FTL_MAX_RECIRCS];
    static thread_local uint32_t thread_id_;
};
""")

ftl_table_template_cc = Template(
"""\
thread_local ${table_entry_name} ${table_name}::entry_[FTL_MAX_RECIRCS];
thread_local uint32_t ${table_name}::thread_id_ = 0;

ftl_base*
${table_name}::factory(sdk_table_factory_params_t *params) {
    void *mem = NULL;
    ${table_name} *f = NULL;
    sdk_ret_t ret = SDK_RET_OK;

    mem = (ftl_base *) SDK_CALLOC(sdk::SDK_MEM_ALLOC_FTL, sizeof(${table_name}));
    if (mem) {
        f = new (mem) ${table_name}();
        ret = f->init_(params);
        if (ret != SDK_RET_OK) {
            f->~${table_name}();
            SDK_FREE(sdk::SDK_MEM_ALLOC_FTL, mem);
            f = NULL;
        }
    } else {
        ret = SDK_RET_OOM;
    }
    return f;
}

void
${table_name}::destroy(ftl_base *f) {
    ftl_base::destroy(f);
}

sdk_ret_t
${table_name}::init_(sdk_table_factory_params_t *params) {
    params->table_id = ${tableid};
    params->max_recircs = FTL_MAX_RECIRCS;
    params->num_hints = ${num_hints};
    return ftl_base::init_(params);
}

base_table_entry_t *
${table_name}::get_entry(int index) {
    return &entry_[index];
}

//---------------------------------------------------------------------------
// ftl Insert entry with hash value
//---------------------------------------------------------------------------
sdk_ret_t
${table_name}::genhash_(sdk_table_api_params_t *params) {
    static thread_local base_table_entry_t *hashkey;

    if (hashkey == NULL) {
        hashkey = ${table_entry_name}::alloc(64 + sizeof(base_table_entry_t));
        if (hashkey == NULL) {
            return SDK_RET_OOM;
        }
    }

    hashkey->build_key(params->entry);
    sdk::lib::swizzle(get_sw_entry_pointer(hashkey), params->entry->entry_size());

    if (!params->hash_valid) {
#ifdef SIM
        static thread_local char buff[512];
        params->entry->tostr(buff, sizeof(buff));
        FTL_TRACE_VERBOSE("Input Entry = [%s]", buff);
        params->hash_32b = sdk::utils::crc32(
                                    (uint8_t *)get_sw_entry_pointer(hashkey),
                                    64,
                                    props_->hash_poly);
#else
        params->hash_32b = crc32_aarch64(
                                (uint64_t *)get_sw_entry_pointer(hashkey));
#endif
        params->hash_valid = true;
    }

    FTL_TRACE_VERBOSE("[%s] => H:%#x",
                      ftlu_rawstr((uint8_t *)get_sw_entry_pointer(hashkey), 64),
                      params->hash_32b);
    return SDK_RET_OK;
}

void
${table_name}::set_thread_id(uint32_t id) {
    SDK_ASSERT(id < PDS_FLOW_HINT_POOLS_MAX);
    thread_id_ = id;
}

uint32_t
${table_name}::thread_id(void) {
    return thread_id_;
}

""")

#####################################
# generate derived class for ftl_base
#####################################
def ftl_table_gen(output_h_dir, output_c_dir, tableid, num_hints, table_name, table_entry_name):
    f = open(output_h_dir + '/ftl_table.hpp', "a+")
    output = ftl_table_template_hpp.substitute(table_name=table_name, table_entry_name=table_entry_name)
    f.write(output)

    f = open(output_c_dir + '/ftl.cc', "a+")
    output = ftl_table_template_cc.substitute(tableid=tableid, table_name=table_name, table_entry_name=table_entry_name, num_hints=num_hints)
    f.write(output)

getters_gen_template_1 = Template(
"""\
    uint8_t get_actionid(void) {
        return actionid;
    }

""")

getters_gen_template_2 = Template(
"""\
    void get_${split_field_name}(uint8_t *${split_field_name}) {
""")

getters_gen_template_3 = Template(
"""\
    ${field_type_str} get_${split_field_name}(void) {
        ${field_type_str} ${split_field_name} = 0x0;
""")

getters_gen_template_4 = Template(
"""\
        ${field_get_str}
""")

getters_gen_template_5 = Template(
"""\
        return ${split_field_name};
""")

getters_gen_template_6 = Template(
"""\
    void get_${field_name}(uint8_t *_${field_name}) {
        memcpy(_${field_name}, ${field_name}, ${arr_len});
        return;
""")

getters_gen_template_7 = Template(
"""\
    ${field_type_str} get_${field_name}(void) {
        return ${field_name};
""")

getters_gen_template_8 = Template(
"""\
    }

""")

getters_gen_template_9 = Template(
"""\
        return;
""")

def getters_gen(gen_actionid, key_data_chain):
    getters_gen_str = ""
    if gen_actionid == True:
        getters_gen_str += getters_gen_template_1.substitute()

    split_field_dict = {}

    for key_data_field in key_data_chain:
        # ignore if field is hash/hint/padding
        if not key_data_field.is_key_appdata_field():
            continue

        field_name = key_data_field.name()
        field_width = key_data_field.width()
        field_type_str = key_data_field.field_type_str()
        if key_data_field.is_field_split():
            split_field_name = key_data_field.split_name()

            # ignore if method is already generated
            if split_field_name in split_field_dict:
                continue

            split_field_dict[split_field_name] = 1
            field_get_str_list = key_data_field.get_field_str(split_field_name)
            split_field_width = get_split_field_width(split_field_name)

            if split_field_width > 64:
                getters_gen_str += getters_gen_template_2.substitute(split_field_name=split_field_name)
                for field_get_str in field_get_str_list:
                    getters_gen_str += getters_gen_template_4.substitute(field_get_str=field_get_str)
                getters_gen_str += getters_gen_template_9.substitute()
            else:
                getters_gen_str += getters_gen_template_3.substitute(field_type_str=field_type_str, split_field_name=split_field_name)
                for field_get_str in field_get_str_list:
                    getters_gen_str += getters_gen_template_4.substitute(field_get_str=field_get_str)
                getters_gen_str += getters_gen_template_5.substitute(split_field_name=split_field_name)
        else:
            if field_width > get_field_bit_unit():
                arr_len = get_bit_arr_length(field_width)
                getters_gen_str += getters_gen_template_6.substitute(field_name=field_name, arr_len=arr_len)
            else:
                getters_gen_str += getters_gen_template_7.substitute(field_type_str=field_type_str, field_name=field_name)
        getters_gen_str += getters_gen_template_8.substitute(field_type_str=field_type_str, field_name=field_name)

    return getters_gen_str

setters_gen_template_1 = Template(
"""\
    void set_actionid(uint8_t actionid_) {
        actionid = actionid_;
    }

""")

setters_gen_template_2 = Template(
"""\
    void set_${split_field_name}(uint8_t *_${split_field_name}) {
""")

setters_gen_template_3 = Template(
"""\
    void set_${split_field_name}(${field_type_str} _${split_field_name}) {
""")

setters_gen_template_4 = Template(
"""\
    void set_${field_name}(${field_type_str} _${field_name}) {
""")

setters_gen_template_5 = Template(
"""\
        ${field_set_str}
""")

setters_gen_template_6 = Template(
"""\
    }

""")

def setters_gen(gen_actionid, key_data_chain):
    setters_gen_str = ""

    if gen_actionid == True:
        setters_gen_str += setters_gen_template_1.substitute()

    # To set fields if they are split
    split_field_dict = {}

    for key_data_field in key_data_chain:
        # ignore if field is hash/hint/padding
        if not key_data_field.is_key_appdata_field():
            continue

        field_name = key_data_field.name()
        field_width = key_data_field.width()
        field_type_str = key_data_field.field_type_str()
        if key_data_field.is_field_split():
            split_field_name = key_data_field.split_name()

            # ignore if method is already generated
            if split_field_name in split_field_dict:
                continue

            split_field_dict[split_field_name] = 1
            field_set_str_list = key_data_field.set_field_str('_' + split_field_name)
            split_field_width = get_split_field_width(split_field_name)

            if split_field_width > 64:
                setters_gen_str += setters_gen_template_2.substitute(split_field_name=split_field_name)
            else:
                setters_gen_str += setters_gen_template_3.substitute(split_field_name=split_field_name, field_type_str=field_type_str)
        else:
            field_set_str_list = key_data_field.set_field_str('_' + field_name)
            setters_gen_str += setters_gen_template_4.substitute(field_name=field_name, field_type_str=field_type_str)

        for field_set_str in field_set_str_list:
            setters_gen_str += setters_gen_template_5.substitute(field_set_str=field_set_str)

        setters_gen_str += setters_gen_template_6.substitute()

    return setters_gen_str

key_str_gen_template_1 = Template(
"""\
    int key2str(char *buff, uint32_t len) {
""")

key_str_gen_template_2 = Template(
"""\
        return snprintf(buff, len, "key: "
""")

key_str_gen_template_3 = Template(
"""\
                 "${format_str}",
""")

key_str_gen_template_4 = Template(
"""\
                 ${args});
    }

""")

def key2str_gen(key_fields_list):
    key_str = key_str_gen_template_1.substitute()

    format_list, args_list = ftl_field_print(key_fields_list)

    key_str += key_str_gen_template_2.substitute()

    if len(format_list) == 1:
        format_str = format_list[0]
    else:
        format_str = ', '.join(map(lambda format: format, format_list))

    key_str += key_str_gen_template_3.substitute(format_str=format_str)

    if len(args_list) == 1:
        args = args_list[0]
    else:
        args = ', '.join(map(lambda arg: arg, args_list))
    key_str += key_str_gen_template_4.substitute(args=args)

    return key_str
