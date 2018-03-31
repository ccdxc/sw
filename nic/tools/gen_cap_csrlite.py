#!/usr/bin/python

import os
import copy
import yaml
import yaml.constructor
from collections import OrderedDict
from string import Template
from itertools import chain
import json
import re

block_name = ''

class OrderedDictYAMLLoader(yaml.Loader):
    """
    A YAML loader that loads mappings into ordered dictionaries.
    """

    def __init__(self, *args, **kwargs):
        yaml.Loader.__init__(self, *args, **kwargs)

        self.add_constructor(u'tag:yaml.org,2002:map', type(self).construct_yaml_map)
        self.add_constructor(u'tag:yaml.org,2002:omap', type(self).construct_yaml_map)

    def construct_yaml_map(self, node):
        data = OrderedDict()
        yield data
        value = self.construct_mapping(node)
        data.update(value)

    def construct_mapping(self, node, deep=False):
        if isinstance(node, yaml.MappingNode):
            self.flatten_mapping(node)
        else:
            raise yaml.constructor.ConstructorError(None, None,
                                                    'expected a mapping node, but found %s' % node.id, node.start_mark)

        mapping = OrderedDict()
        for key_node, value_node in node.value:
            key = self.construct_object(key_node, deep=deep)
            try:
                hash(key)
            except TypeError as exc:
                raise yaml.constructor.ConstructorError('while constructing a mapping',
                                                        node.start_mark, 'found unacceptable key (%s)' % exc,
                                                        key_node.start_mark)
            value = self.construct_object(value_node, deep=deep)
            mapping[key] = value
        return mapping

class component_info:
    Registry = {}

    def __init__(self):
        self.name = "foo"
        self.start = 0
        self.size = 0
        self.array = 0
        self.decoder = ""
        self.total_size = 0
        self.byte_size = 0
        self.reset_value = "0"
        self.subtype = "decoder"
        self.offset = 0
        self.large_array_thr = 1
        self.instance_done = 0
        self.fields = []

    c_helper_headerfile_class_declare = Template("""
class ${hlprclassname} {
private:
    uint32_t base_addr;
public:
    ${hlprclassname}()=default;
    ~${hlprclassname}()=default;
    void init(uint32_t _addr);
    ${csrclassname} *get_csr_instance(uint32_t chip_id);""")
    c_helper_headerfile_class_tail = Template("""
}; //  ${hlprclassname}()
""")
    c_helper_headerfile_decoder_array_info = Template("""
    $field_typename $field_name[$field_array];
    inline uint32_t get_depth_${field_name}() { return $field_array; }""")
    c_helper_headerfile_decoder_info = Template("""
    ${field_typename} $field_name;""")

    c_headerfile_class_declare = Template("""
class ${classname} : public cap_sw_${base}_base {
    public:
        ${classname}();
        virtual ~${classname}();

        void init(uint32_t chip_id, uint32_t _addr_base) override;
        void pack(uint8_t *bytes, uint32_t start=0) override;
        void unpack(uint8_t *bytes, uint32_t start=0) override;

        uint32_t get_width() const override;
        static uint32_t s_get_width();""")
    c_headerfile_class_tail = Template("""
}; // ${classname}
""")
    c_headerfile_decoder_array_info = Template("""
        $field_typename $field_name[$field_array];
        int get_depth_${field_name}() { return $field_array; }""")
    c_headerfile_decoder_info = Template("""
        ${field_typename} $field_name;""")
    c_headerfile_field_info = Template("""
        ${field_typename} ${field_name}${field_array}${field_wide};
        void pack_${field_name}(uint8_t *bytes, uint32_t start=0);
        void unpack_${field_name}(uint8_t *bytes, uint32_t start=0);""")

    c_helper_ccfile_init_declare = Template("""
void ${classname}::init(uint32_t _addr) {
    base_addr = _addr;""")
    c_helper_ccfile_init_tail = Template("""
} // ${hlprclassname}::init()

${csrclassname} *${hlprclassname}::get_csr_instance(uint32_t chip_id) {
    auto *inst = new ${csrclassname}();
    if (inst) {
        inst->init(chip_id, base_addr);
    }
    return inst;
} // ${hlprclassname}::get_csr_instance(void)
""")
    c_helper_ccfile_decoder_array_info = Template("""
    for (uint32_t ii = 0; ii < get_depth_${field_name}(); ii++) {
        ${field_name}[ii].init(base_addr + 0x${field_offset});
    }""")
    c_helper_ccfile_decoder_info = Template("""
    $field_name.init(base_addr + 0x${field_offset});""")

    c_ccfile_class_declare = Template("""
${classname}::${classname}()=default;
${classname}::~${classname}()=default;""")
    c_ccfile_class_declare_add_byte_size = Template("""
${classname}::${classname}():cap_sw_${base}_base()  { 
    set_byte_size(${field_byte_size});
}
${classname}::~${classname}()=default;
""")

    c_ccfile_s_width_declare = Template("""
uint32_t ${classname}::s_get_width() {
    int _count = 0;""")
    c_ccfile_s_width_declare_end = Template("""
    return _count;
}
""")
    c_ccfile_s_width_decoder_array_impl = Template("""
    _count += (${field_typename}_t::s_get_width() * ${field_array}); // ${field_name}""")
    c_ccfile_s_width_decoder_impl = Template("""
    _count += ${field_typename}_t::s_get_width(); // ${field_name}""")
    c_ccfile_s_width_array_impl = Template("""
    _count += ${field_size} * ${field_array}; // ${field_name}""")
    c_ccfile_s_width_impl = Template("""
    _count += ${field_size}; // ${field_name}""")

    c_ccfile_width_declare = Template("""
uint32_t ${classname}::get_width() const {
    return ${classname}::s_get_width();""")
    c_ccfile_width_declare_end = Template("""
}
""")
    c_ccfile_width_decoder_array_impl = Template("")
    c_ccfile_width_decoder_impl = Template("")
    c_ccfile_width_array_impl = Template("")
    c_ccfile_width_impl = Template("")
    c_ccfile_init_reset_val = Template("""""")
    c_ccfile_init_register_set_get = Template("""""")
    c_ccfile_init_register_set_get_array = Template("""""")
    c_ccfile_init_decoder_array_impl = Template("""
    for(int ii = 0; ii < ${field_array}; ii++) {
        ${field_name}[ii].init(_chip_id, _addr_base + 0x${field_offset} + (${field_name}[ii].get_byte_size()*ii));
    }""")
    c_ccfile_init_declare = Template("""
void ${classname}::init(uint32_t _chip_id, uint32_t _addr_base) {
    set_chip_id(_chip_id);
    set_offset(_addr_base);""")
    c_ccfile_init_decoder_impl = Template("""
    ${field_name}.init(_chip_id, _addr_base + 0x${field_offset});""")
    c_ccfile_init_array_impl = Template("""
""")
    c_ccfile_init_impl = Template("""
    base_offset = ${offsetname}[inst_id] + ${fieldoffset};
""")
    c_ccfile_init_declare_end = Template("""
}
""")

    c_ccfile_unpack_declare = Template("""
void ${classname}::unpack(uint8_t *bytes, uint32_t start) {""")
    c_ccfile_unpack_declare_end = Template("""
}
""")
    c_ccfile_unpack_decoder_array_impl = Template("""
    for(uint32_t ii = 0; ii < ${field_array}; ii++) {
        ${field_name}[ii].unpack(bytes, start);
        start += ${field_name}[ii].get_width();
    }""")
    c_ccfile_unpack_decoder_impl = Template("""
    ${field_name}.unpack(bytes, start);""")
    c_ccfile_unpack_array_impl = Template("""
    for(uint32_t ii = 0; ii < ${field_array}; ii++) {
        ${field_name}[ii].unpack(bytes, start);
        start += ${field_size};
    }""")
    c_ccfile_unpack_impl = Template("""
    unpack_${field_name}(bytes, start);""")

    c_ccfile_pack_declare = Template("""
void ${classname}::pack(uint8_t *bytes, uint32_t start) 
{""")
    c_ccfile_pack_declare_end = Template("""
}
""")
    c_ccfile_pack_decoder_array_impl = Template("""
    for(uint32_t ii = 0; ii < ${field_array}; ii++) {
        ${field_name}[ii].pack(bytes, start);
        start += ${field_name}[ii].get_width();
    }""")
    c_ccfile_pack_decoder_impl = Template("""
    ${field_name}.pack(bytes, start);""")
    c_ccfile_pack_array_impl = Template("""
    // ${field_name}
    for(uint32_t ii = 0; ii < ${field_array}; ii++) {
        ret_val = hlp.set_slc(ret_val, static_cast<cpp_int>(int_var__${field_name}[ii]) , _count, _count -1 + ${field_size} );
        _count += ${field_size};
    }""")
    c_ccfile_pack_impl = Template("""
    pack_${field_name}(bytes, start);""")

    c_ccfile_set_field_array_impl = Template("""
// ${field_name}
void ${classname}::${field_name}(const cpp_int & _val, int _idx) { 
    PU_ASSERT(_idx < ${field_array});
    int_var__${field_name}[_idx] = _val.convert_to< ${field_name}_cpp_int_t >();
}

cpp_int ${classname}::${field_name}(int _idx) const {
    PU_ASSERT(_idx < ${field_array});
    return int_var__${field_name}[_idx].convert_to<cpp_int>();
}
""")

    c_ccfile_unpack_head = Template("""
void ${classname}::unpack_${field_name}(uint8_t *bytes, uint32_t start)
{""")
    c_ccfile_unpack_body = Template("""
    ${field_name} = hal::utils::pack_bytes_unpack(bytes, start + ${field_start}, ${field_size});""")
    c_ccfile_unpack_tail = Template("""
}""")
    c_ccfile_unpack_array_head = Template("""
void ${classname}::unpack_${field_name}(uint8_t *bytes, int _idx)
{
    PU_ASSERT(_idx < ${field_array});""")
    c_ccfile_unpack_array_body = Template("""
    ${field_name}[_idx] = hal::utils::pack_bytes_unpack(bytes, ${field_start}, ${field_size});""")
    c_ccfile_unpack_array_tail = Template("""
}""")

    c_ccfile_pack_head = Template("""
void ${classname}::pack_${field_name}(uint8_t *bytes, uint32_t start)
{""")
    c_ccfile_pack_body = Template("""
    hal::utils::pack_bytes_pack(bytes, start + ${field_start}, ${field_size}, ${field_name});""")
    c_ccfile_pack_tail = Template("""
}""")
    c_ccfile_pack_array_head = Template("""
void ${classname}::pack_${field_name}(uint8_t *bytes, int _idx)
{
    PU_ASSERT(_idx < ${field_array});""")
    c_ccfile_pack_array_body = Template("""
    hal::utils::pack_bytes_pack(bytes, ${field_start}, ${field_size}, ${field_name}[_idx]);""")
    c_ccfile_pack_array_tail = Template("""
}""")

    def set_tot_size(self, indent_level=0):

        if self.total_size != 0:
            return self.total_size

        if len(self.fields) > 0:
            # if self.name != 'root':
            #     self.total_size=0

            for field in self.fields:
                if field.decoder != "":
                    component_info.Registry[field.decoder].set_tot_size(indent_level + 1)
                    if field.array > 1:
                        field.total_size = (component_info.Registry[field.decoder].total_size * field.array)
                    else:
                        field.total_size = component_info.Registry[field.decoder].total_size
                else:
                    field.set_tot_size(indent_level + 1)
                self.total_size = self.total_size + field.total_size
                # if self.name != 'root':
                #    print " "*indent_level, ('===> {0}_sv:{1}'.format(self.name ,self.total_size))
        else:
            if self.size != 0:
                if self.array > 1:
                    self.total_size = self.total_size + (self.size * self.array)
                else:
                    self.total_size = self.total_size + self.size
                    # print "{2} : self.size={0} self.total_size={1}".format(self.size,self.total_size,self.name)
            else:
                raise Exception("field size is 0 %s" % (self.name))

    def gen_h_file(self, l_cur_str, indent_level=0):

        if self.size != 0:
            wide_str = ''
            array_str = ''

            if self.size > 64:
                wide_str = '[{0}]'.format(int((self.size + 63) / 64))

            if self.array > 1:
                array_str = '[{0}]'.format(self.array)

            l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_field_info.substitute(
                field_typename='uint64_t', field_name='{0}'.format(self.name),
                field_wide= wide_str, field_array=array_str))

        if len(self.fields) > 0:
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_class_declare.substitute(
                    classname='{0}_t'.format(self.name),instancetype='{0}_inst_id_t'.format(block_name),
                    base=self.subtype))
            for field in self.fields:
                if field.decoder != "":
                    if field.array > 1:
                        l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_decoder_array_info.substitute(
                            field_typename='{0}_t'.format(field.decoder), field_name='{0}'.format(field.name),
                            field_array=field.array, field_size=field.size))
                    else:
                        l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_decoder_info.substitute(
                            field_typename='{0}_t'.format(field.decoder), field_name='{0}'.format(field.name)))
                else:
                    if field.instance_done == 0:
                        l_cur_str = field.gen_h_file(l_cur_str, indent_level + 1)
                        field.instance_done = 1
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_class_tail.substitute(
                    classname='{0}_t'.format(self.name)))
        return l_cur_str
    def gen_c_file(self, cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str,
                       init_str, set_field_str, get_field_str, indent_level=0, p_classname=""):
        if self.size != 0:
            if self.array > 1:
                width_str = '{0}{1}'.format(width_str,
                                            self.c_ccfile_width_array_impl.substitute(field_typename="cpp_int",
                                                                                      field_name='{0}'.format(
                                                                                          self.name),
                                                                                      field_array=self.array,
                                                                                      field_size=self.size))
                s_width_str = '{0}{1}'.format(s_width_str,
                                              self.c_ccfile_s_width_array_impl.substitute(field_typename="cpp_int",
                                                                                          field_name='{0}'.format(
                                                                                              self.name),
                                                                                          field_array=self.array,
                                                                                          field_size=self.size))
                set_all_str = '{0}{1}'.format(set_all_str,
                                              self.c_ccfile_unpack_array_impl.substitute(field_typename="cpp_int",
                                                                                          field_name='{0}'.format(
                                                                                              self.name),
                                                                                          field_array=self.array,
                                                                                          field_size=self.size))
                get_all_str = '{0}{1}'.format(get_all_str,
                                              self.c_ccfile_pack_array_impl.substitute(field_typename="cpp_int",
                                                                                          field_name='{0}'.format(
                                                                                              self.name),
                                                                                          field_array=self.array,
                                                                                          field_size=self.size))
                init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_array_impl.substitute(field_typename="cpp_int",
                                                                                              field_name='{0}'.format(
                                                                                                  self.name),
                                                                                              field_array=self.array,
                                                                                              field_size=self.size))
                set_field_str = '{0}{1}'.format(set_field_str,
                                                self.c_ccfile_pack_array_head.substitute(field_typename="cpp_int",
                                                                                              field_name='{0}'.format(
                                                                                              self.name),
                                                                                              field_array=self.array,
                                                                                              field_size=self.size,
                                                                                              classname=p_classname))
                size = self.size
                start = self.start
                wide = 0
                index = 0
                name = '{0}'.format(self.name)
                while size > 0:
                    chunk = size
                    if chunk > 64:
                        wide = 1
                        chunk = 64

                    if wide > 0:
                        name = '{0}[{1}]'.format(self.name, str(index))
                    set_field_str = '{0}{1}'.format(set_field_str,
                                                    self.c_ccfile_pack_array_body.substitute(field_name='{0}'.format(name),
                                                                                       field_start=start,
                                                                                       field_size=chunk))
                    size -= chunk
                    start += chunk
                    index += 1
                set_field_str = '{0}{1}'.format(set_field_str, self.c_ccfile_pack_array_tail.substitute())

                set_field_str = '{0}{1}'.format(set_field_str,
                                                self.c_ccfile_unpack_array_head.substitute(field_name='{0}'.format(self.name),
                                                                                     classname=p_classname))
                size = self.size
                start = self.start
                wide = 0
                index = 0
                name = '{0}'.format(self.name)
                while size > 0:
                    chunk = size
                    if chunk > 64:
                        wide = 1
                        chunk = 64

                    if wide > 0:
                        name = '{0}[{1}]'.format(self.name, str(index))
                    set_field_str = '{0}{1}'.format(set_field_str,
                                                    self.c_ccfile_unpack_array_body.substitute(field_name='{0}'.format(name),
                                                                                         field_start=start,
                                                                                         field_size=chunk))
                    size -= chunk
                    start += chunk
                    index += 1

                set_field_str = '{0}{1}'.format(set_field_str, self.c_ccfile_unpack_array_tail.substitute())

                init_str = '{0}{1}'.format(init_str,
                                           self.c_ccfile_init_register_set_get_array.substitute(classname=p_classname,
                                                                                                field_name='{0}'.format(
                                                                                                self.name),
                                                                                                field_array=self.array))
            else:
                width_str = '{0}{1}'.format(width_str, self.c_ccfile_width_impl.substitute(field_typename="cpp_int",
                                                                                           field_name='{0}'.format(self.name),
                                                                                           field_array='',
                                                                                           field_size=self.size))
                s_width_str = '{0}{1}'.format(s_width_str,
                                              self.c_ccfile_s_width_impl.substitute(field_typename="cpp_int",
                                                                                    field_name='{0}'.format(self.name),
                                                                                    field_array='',
                                                                                    field_size=self.size))
                set_all_str = '{0}{1}'.format(set_all_str,
                                              self.c_ccfile_unpack_impl.substitute(field_typename="cpp_int",
                                                                                    field_name='{0}'.format(self.name),
                                                                                    field_array='',
                                                                                    field_size=self.size))
                get_all_str = '{0}{1}'.format(get_all_str,
                                              self.c_ccfile_pack_impl.substitute(field_typename="cpp_int",
                                                                                    field_name='{0}'.format(self.name),
                                                                                    field_array='',
                                                                                    field_size=self.size))
#                init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_impl.substitute(
#                                                                        offsetname='{0}_base_addr'.format(block_name),
#                                                                        fieldoffset='{0}'.format(offset)))

                set_field_str = '{0}{1}'.format(set_field_str,
                                                self.c_ccfile_pack_head.substitute(field_name='{0}'.format(self.name),
                                                                                   classname=p_classname))
                size = self.size
                start = self.start
                wide = 0
                index = 0
                name = '{0}'.format(self.name)
                while size > 0:
                    chunk = size
                    if chunk > 64:
                        wide = 1
                        chunk = 64

                    if wide > 0:
                        name = '{0}[{1}]'.format(self.name, str(index))
                    set_field_str = '{0}{1}'.format(set_field_str,
                                                    self.c_ccfile_pack_body.substitute(field_name='{0}'.format(name),
                                                                                       field_start=start,
                                                                                       field_size=chunk))
                    size -= chunk
                    start += chunk
                    index += 1
                set_field_str = '{0}{1}'.format(set_field_str, self.c_ccfile_pack_tail.substitute())

                set_field_str = '{0}{1}'.format(set_field_str,
                                                self.c_ccfile_unpack_head.substitute(field_name='{0}'.format(self.name),
                                                                                     classname=p_classname))
                size = self.size
                start = self.start
                wide = 0
                index = 0
                name = '{0}'.format(self.name)
                while size > 0:
                    chunk = size
                    if chunk > 64:
                        wide = 1
                        chunk = 64

                    if wide > 0:
                        name = '{0}[{1}]'.format(self.name, str(index))
                    set_field_str = '{0}{1}'.format(set_field_str,
                                                    self.c_ccfile_unpack_body.substitute(field_name='{0}'.format(name),
                                                                                         field_start=start,
                                                                                         field_size=chunk))
                    size -= chunk
                    start += chunk
                    index += 1

                set_field_str = '{0}{1}'.format(set_field_str, self.c_ccfile_unpack_tail.substitute())

                init_str = '{0}{1}'.format(init_str,
                                           self.c_ccfile_init_register_set_get.substitute(classname=p_classname,
                                                                                          field_name='{0}'.format(
                                                                                          self.name)))

        if len(self.fields) > 0:
            if self.name != 'root':

                if self.byte_size != 0:
                    if self.subtype == 'block':
                        head_str = '{0}{1}'.format(head_str, self.c_ccfile_class_declare_add_byte_size.substitute(
                            classname='{0}_t'.format(self.name), field_size=self.size, base=self.subtype,
                            field_byte_size=self.byte_size, field_optional_init=''))
                    else:
                        head_str = '{0}{1}'.format(head_str, self.c_ccfile_class_declare_add_byte_size.substitute(
                            classname='{0}_t'.format(self.name), field_size=self.size, base=self.subtype,
                            field_byte_size=self.byte_size, field_optional_init=''))
                else:
                    head_str = '{0}{1}'.format(head_str, self.c_ccfile_class_declare.substitute(
                        classname='{0}_t'.format(self.name), field_size=self.size, base=self.subtype))

                width_str = '{0}{1}'.format(width_str,
                                            self.c_ccfile_width_declare.substitute(classname='{0}_t'.format(self.name),
                                                                                   field_size=self.size))
                s_width_str = '{0}{1}'.format(s_width_str, self.c_ccfile_s_width_declare.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
                set_all_str = '{0}{1}'.format(set_all_str, self.c_ccfile_unpack_declare.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
                get_all_str = '{0}{1}'.format(get_all_str, self.c_ccfile_pack_declare.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))

                init_str = '{0}{1}'.format(init_str,self.c_ccfile_init_declare.substitute(classname='{0}_t'.format(self.name),
                                 instancetype='{0}_inst_id_t'.format(block_name),
                                 instancemax='{0}_inst_max'.format(block_name).upper(),
                                 offsetname='{0}_base_addr'.format(block_name),
                                 fieldoffset='{0}'.format(self.offset)))

            bit_offset = 0
            for field in self.fields:
                field.start = bit_offset
                bit_offset = bit_offset + field.size
                if field.decoder != "":
                    if field.array > 1:
                        width_str = '{0}{1}'.format(width_str, self.c_ccfile_width_decoder_array_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array=field.array),
                                                    field_size=field.size)
                        s_width_str = '{0}{1}'.format(s_width_str, self.c_ccfile_s_width_decoder_array_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array=field.array),
                                                      field_size=field.size)
                        set_all_str = '{0}{1}'.format(set_all_str, self.c_ccfile_unpack_decoder_array_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array=field.array),
                                                      field_size=field.size)
                        get_all_str = '{0}{1}'.format(get_all_str, self.c_ccfile_pack_decoder_array_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array=field.array),
                                                      field_size=field.size)
                        init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_decoder_array_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array=field.array,
                            field_offset=format(field.offset, 'x')), field_size=field.size)
                    else:
                        width_str = '{0}{1}'.format(width_str, self.c_ccfile_width_decoder_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array='',
                            field_size=field.size))
                        s_width_str = '{0}{1}'.format(s_width_str, self.c_ccfile_s_width_decoder_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array='',
                            field_size=field.size))
                        set_all_str = '{0}{1}'.format(set_all_str, self.c_ccfile_unpack_decoder_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array='',
                            field_size=field.size))
                        get_all_str = '{0}{1}'.format(get_all_str, self.c_ccfile_pack_decoder_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array='',
                            field_size=field.size))
                        init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_decoder_impl.substitute(
                            field_typename=field.decoder, field_name='{0}'.format(field.name), field_array='',
                            field_size=field.size, field_offset=format(field.offset, 'x')))
                else:
                    if field.instance_done == 0:
                        cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str, set_field_str, get_field_str = field.gen_c_file(
                        cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str, set_field_str, get_field_str, indent_level + 1,
                        p_classname="{0}_t".format(self.name))
                        field.instance_done = 1

            if self.name != 'root':
                width_str = '{0}{1}'.format(width_str, self.c_ccfile_width_declare_end.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
                s_width_str = '{0}{1}'.format(s_width_str, self.c_ccfile_s_width_declare_end.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
                set_all_str = '{0}{1}'.format(set_all_str, self.c_ccfile_unpack_declare_end.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
                get_all_str = '{0}{1}'.format(get_all_str, self.c_ccfile_pack_declare_end.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
                if (self.reset_value != "0") and (self.reset_value != "0x0"):
                    init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_reset_val.substitute(
                        classname='{0}_t'.format(self.name), field_size=self.size, reset_value=self.reset_value))
                init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_declare_end.substitute(
                    classname='{0}_t'.format(self.name), field_size=self.size))
        return cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str, set_field_str, get_field_str
    def gen_helper_h_file(self, l_cur_str, indent_level=0):

        if len(self.fields) > 0:
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_headerfile_class_declare.substitute(
                    hlprclassname='{0}_helper_t'.format(self.name),
                    csrclassname='{0}_t'.format(self.name)))
            for field in self.fields:
                if field.decoder != "":
                    if field.array > 1:
                        l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_headerfile_decoder_array_info.substitute(
                            field_typename='{0}_helper_t'.format(field.decoder), field_name='{0}'.format(field.name),
                            field_array=field.array, field_size=field.size))
                    else:
                        l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_headerfile_decoder_info.substitute(
                            field_typename='{0}_helper_t'.format(field.decoder),
                            field_name='{0}'.format(field.name)))
                else:
                    if field.instance_done == 0:
                        l_cur_str = field.gen_helper_h_file(l_cur_str, indent_level + 1)
                        field.instance_done = 1
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_headerfile_class_tail.substitute(
                    hlprclassname='{0}_helper_t'.format(self.name)))
        return l_cur_str
    def gen_helper_c_file(self, l_cur_str, indent_level=0):

        if len(self.fields) > 0:
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_ccfile_init_declare.substitute(
                    classname='{0}_helper_t'.format(self.name)))
            for field in self.fields:
                if field.decoder != "":
                    if field.array > 1:
                        l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_ccfile_decoder_array_info.substitute(
                            field_name='{0}'.format(field.name), field_offset=format(field.offset, 'x')))
                    else:
                        l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_ccfile_decoder_info.substitute(
                            field_name='{0}'.format(field.name), field_offset=format(field.offset, 'x')))
                else:
                    if field.instance_done == 0:
                        l_cur_str = field.gen_helper_c_file(l_cur_str, indent_level + 1)
                        field.instance_done = 1
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_helper_ccfile_init_tail.substitute(
                    hlprclassname='{0}_helper_t'.format(self.name),
                    csrclassname='{0}_t'.format(self.name)))
        return l_cur_str

    def gen_c_header(self, include_map):
        cur_str = """
#ifndef {0}_HPP
#define {0}_HPP

#include "sdk/asic/capri/csrlite/cap_csr_base.hpp"
""".format(block_name.upper())
        for i in include_map:
            cur_str = cur_str + """#include "sdk/asic/capri/csrlite/{0}.hpp"
""".format(i)
        cur_str = cur_str + """
using namespace std;""".format(block_name.upper())
        cur_str = self.gen_h_file(cur_str, 0)
        cur_str = cur_str + """
#endif // {0}_HPP
        """.format(block_name.upper())
        return cur_str
    def gen_c_source(self):
        cur_str = """
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "sdk/asic/capri/csrlite/{0}.hpp"

using namespace std;
        """.format(block_name)

        head_str = ""
        tail_str = ""
        show_str = ""
        width_str = ""
        s_width_str = ""
        set_all_str = ""
        get_all_str = ""
        init_str = ""
        set_field_str = ""
        get_field_str = ""

        cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str, set_field_str, get_field_str = self.gen_c_file(
            cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str,
            set_field_str, get_field_str, 0, "")

        return cur_str + head_str + tail_str + show_str + width_str + s_width_str + set_all_str + get_all_str + init_str + get_field_str + set_field_str
    def gen_c_helper_header(self, include_map):
        cur_str = """
#ifndef {0}_HELPER_HPP
#define {0}_HELPER_HPP

#include "sdk/asic/capri/csrlite/{1}.hpp"
""".format(block_name.upper(), block_name)
        for i in include_map:
            cur_str = cur_str + """#include "sdk/asic/capri/csrlite/{0}_helper.hpp" 
""".format(i)
        cur_str = cur_str + """
using namespace std;
""".format(block_name.upper())
        cur_str = self.gen_helper_h_file(cur_str, 0)
        cur_str = cur_str + """
#endif // {0}_HELPER_HPP
""".format(block_name.upper())
        return cur_str
    def gen_c_helper_source(self):
        cur_str = """
#include "sdk/asic/capri/csrlite/{0}_helper.hpp"
""".format(block_name)
        cur_str = cur_str + """
using namespace std;
"""
        cur_str = self.gen_helper_c_file(cur_str, 0)
        return cur_str

def filter_unused(include_map, cur_map, init_map, target, target_block, depth=0, field_or_decoder=1):
    # print "checking ", target

    for k in init_map.fields:
        if (k.name != target_block) and (k.subtype == "block"):
            if k.name not in include_map:
                include_map.append(k.name)
                # print depth, init_map.name, k.name
        elif k.name == target:
            # print "pushing ", k.name, " decoder: " , k.decoder
            cur_map.fields.insert(0, k)
            for inner in k.fields:
                if inner.decoder != "":
                    include_map, cur_map = filter_unused(include_map, cur_map, init_map, inner.decoder, target_block,
                                                         depth + 1, 1)

    return include_map, cur_map

def recurse(e, parent, depth=0):
    if isinstance(e, list):
        for i in e:
            recurse(i, parent, depth + 1)

    elif isinstance(e, dict):
        for k in e:
            if k == 'size':
                parent.size = int(e[k])
                continue
            if k == 'decoder':
                parent.decoder = e[k]
                continue
            elif k == 'array':
                parent.array = int(e[k])
                continue
            elif k == 'fields':
                if parent.name in component_info.Registry:
                    raise Exception('duplicate component %s' % (parent.name))
                component_info.Registry[parent.name] = parent
                # print "adding %s"%parent.name
                recurse(e[k], parent, depth + 1)
                continue
            elif k == 'offset':
                parent.offset = int(e[k])
            elif k == 'block_byte_size':
                parent.byte_size = int(e[k])
            elif k == 'reset_value':
                parent.reset_value = e[k]
            elif k == 'type':
                parent.subtype = e[k]
            else:
                p = component_info()
                p.name = k.rstrip()
                parent.fields.append(p)
                recurse(e[k], parent.fields[-1], depth + 1)

    else:
        print ('ERROR:', parent.name, "  " * depth, e)

def gen_decoders(yaml_src_file, _block_name, cc_gen_dir, hdr_gen_dir):
    global block_name
    f = open(yaml_src_file)
    data_map = yaml.load(f, OrderedDictYAMLLoader)
    p = component_info()
    p.name = 'root'
    block_name = _block_name
    # cleanup before calling recurse on root
    component_info.Registry = {}
    recurse(data_map, p)
    include_map = []

    match_obj = re.match(r"(.*)_csr$", block_name)
    if match_obj:
        filter_map = component_info()
        filter_map.name = "root"
        include_map, filter_map = filter_unused(include_map, filter_map, p, _block_name, _block_name)
    else:
        filter_map = p

    filter_map.set_tot_size()

    temp = copy.deepcopy(filter_map)
    out_f = open(hdr_gen_dir + '/' + block + '.hpp', 'w')
    out_f.write(temp.gen_c_header(include_map))
    out_f.close()

    temp = copy.deepcopy(filter_map)
    out_f = open(cc_gen_dir + block + '.cc', 'w')
    out_f.write(temp.gen_c_source())
    out_f.close()

    temp = copy.deepcopy(filter_map)
    out_f = open(hdr_gen_dir +'/' + block + '_helper.hpp', 'w')
    out_f.write(temp.gen_c_helper_header(include_map))
    out_f.close()

    temp = copy.deepcopy(filter_map)
    out_f = open(cc_gen_dir + block + '_helper.cc', 'w')
    out_f.write(temp.gen_c_helper_source())
    out_f.close()


def gen_block_name(filename):
    block = filename.split(".")[0]
    exceptional_obj = re.match(r'cap_apb|cap_emm|cap_mc|cap_ms|ns_soc|cap_inval_filter|cap_inval_master', block)
    if exceptional_obj is not None:
        return None
    elif block == 'emmc':
        return 'emmc_SDHOST_Memory_Map'
    elif filename.startswith('cap_') == False:
        return "cap_" + block + "_csr"
    else:
        return block

if __name__ == '__main__':

    input_dir   = '../asic/capri/verif/common/csr_gen/'
    cc_gen_dir  = '../gen/csrlite/'
    hdr_gen_dir = '../sdk/gen/include/sdk/asic/capri/csrlite/'

    # create cc_gen_dir if not exists
    if not os.path.exists(cc_gen_dir):
        os.makedirs(cc_gen_dir)

    # create hdr_gen_dir if not exists
    if not os.path.exists(hdr_gen_dir):
        os.makedirs(hdr_gen_dir)

    files = [f for f in os.listdir(input_dir) if os.path.isfile(os.path.join(input_dir, f)) and f.endswith(".yaml")]
    for file in files:
        block = gen_block_name(file)
        if block is not None:
            print ("Processing " +input_dir+file+ " ==> " + cc_gen_dir + block + ".cc " + "[" + hdr_gen_dir + block + ".hpp]")
            gen_decoders(input_dir+file, block, cc_gen_dir, hdr_gen_dir)
        else:
            print ("Skipped " + input_dir+file)
