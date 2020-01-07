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

# TODO use pragmas
def is_table_ftl_gen(table):
    # return 'flow' in str(table) and 'stats' not in str(table) and 'info' not in str(table)
    return 'flow' in str(table) and 'stats' not in str(table)

# index based table
def is_table_index_based(table):
    return 'flow_info' in str(table)

# generate key for table
def is_table_gen_key(table):
    return not is_table_index_based(table)

# generate hashes/hints for table
def is_table_gen_hints(table):
    return not is_table_index_based(table)

# TODO use pragmas
def is_table_pad_256(table):
    return 'v4' in str(table)

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

def split_fields_dict_reset():
    global split_fields_dict
    split_fields_dict = {}

def next_pow_2(num):
    # return same number if already power of 2
    if num & (num-1) == 0:
        return num

    i = 0
    while (num != 0):
        num = num >> 1
        i += 1
    return (1 << i)

set_field_template_1 = Template(
"""\
${field_name} = (${field_arg} >> ${sbit}) & ${mask_str};\
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
${field_arg} |= (${field_name} << ${sbit}) & ${mask_str};\
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
            for split_field in split_fields_list:
                field_name = split_field.name()
                sbit = split_field.sbit()
                ebit = split_field.ebit()
                mask = 0
                for i in range(ebit-sbit+1):
                    mask |= (1 << i)
                field_str_list.append(set_field_template_1.substitute(field_name=field_name, field_arg=field_arg, sbit=sbit, mask_str=str(hex(mask))))
        else:
            if field_width > get_field_bit_unit():
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
            for split_field in split_fields_list:
                field_name = split_field.name()
                sbit = split_field.sbit()
                ebit = split_field.ebit()
                mask = 0
                for i in range(ebit-sbit+1):
                    mask |= (1 << i)
                field_str_list.append(get_field_template_1.substitute(field_name=field_name, field_arg=field_arg, sbit=sbit, mask_str=str(hex(mask))))
        else:
            if field_width > get_field_bit_unit():
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
    if field_width > get_field_bit_unit():
        args_list = []
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

################
# process field
################
def ftl_process_field(field_obj):
    field_name = field_obj.name()
    field_width = field_obj.width()
    little_str = field_obj.little_str()
    if field_width > get_field_bit_unit():
        field_str  = "uint" + str(get_field_bit_arr_unit()) + "_t " + field_name + "[" + str(get_bit_arr_length(field_width)) + "];"
    else:
        width = str(field_width) + little_str
        field_str = "uint" + str(get_field_bit_unit()) + "_t " + field_name + " : " + width + ";"
    return field_str

ftl_table_template_hpp = Template(
"""\
class ${table_name}: public ftl_base {
public:
    static ftl_base *factory(sdk_table_factory_params_t *params);
    static void destroy(ftl_base *f);

    ${table_name}() {}
    ~${table_name}(){}

    virtual base_table_entry_t *get_entry(int index) override;

private:
    sdk_ret_t init_(sdk_table_factory_params_t *params);
    static thread_local ${table_entry_name} entry_[FTL_MAX_RECIRCS];
};

""")

ftl_table_template_cc = Template(
"""\
thread_local ${table_entry_name} ${table_name}::entry_[FTL_MAX_RECIRCS];

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
