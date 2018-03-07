#!/usr/bin/python

import os
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
            except TypeError, exc:
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

    c_headerfile_class_declare = Template("""
class ${classname} : public cap_sw_${base}_base {
    public:
        ${classname}(cap_sw_csr_base *parent = 0);
        virtual ~${classname}();
        virtual void init();
        virtual uint32_t get_width() const;
        static uint32_t s_get_width();
        virtual void pack(uint8_t *bytes, uint32_t start=0);
        virtual void unpack(uint8_t *bytes, uint32_t start=0);
""")
    c_headerfile_class_tail = Template("""
}; // ${classname}
""")
    c_headerfile_decoder_array_info = Template("""
        $field_typename $field_name[$field_array];
        int get_depth_${field_name}() { return $field_array; }
""")
    c_headerfile_decoder_info = Template("""
        ${field_typename} $field_name;
""")
    c_headerfile_field_info = Template("""
        ${field_typename} ${field_name}${field_array}${field_wide};
        void pack_${field_name}(uint8_t *bytes, uint32_t start=0);
        void unpack_${field_name}(uint8_t *bytes, uint32_t start=0);
""")

    c_ccfile_class_declare = Template("""
${classname}::${classname}(cap_sw_csr_base * parent):cap_sw_${base}_base(parent)
{ 
    init();
}

${classname}::~${classname}()
{
}
""")
    c_ccfile_class_declare_add_byte_size = Template("""
${classname}::${classname}(cap_sw_csr_base * parent): 
    cap_sw_${base}_base(parent)  { 
        set_byte_size(${field_byte_size});
        ${field_optional_init}
        }
${classname}::~${classname}() { }
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
    return ${classname}::s_get_width();
""")
    c_ccfile_width_declare_end = Template("""
}
""")
    c_ccfile_width_decoder_array_impl = Template("")
    c_ccfile_width_decoder_impl = Template("")
    c_ccfile_width_array_impl = Template("")
    c_ccfile_width_impl = Template("")

    c_ccfile_init_declare = Template("""
void ${classname}::init() {
""")
    c_ccfile_init_reset_val = Template("""""")
    c_ccfile_init_register_set_get = Template("""""")
    c_ccfile_init_register_set_get_array = Template("""""")
    c_ccfile_init_declare_end = Template("""
}
""")
    c_ccfile_init_decoder_array_impl = Template("""""")
    c_ccfile_init_decoder_impl = Template("""""")
    c_ccfile_init_array_impl = Template("""
""")
    c_ccfile_init_impl = Template("""""")

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
    }
""")
    c_ccfile_unpack_array_head = Template("""
    void ${classname}::unpack_${field_name}(uint8_t *bytes, int _idx)
    {
        PU_ASSERT(_idx < ${field_array});""")
    c_ccfile_unpack_array_body = Template("""
        ${field_name}[_idx] = hal::utils::pack_bytes_unpack(bytes, ${field_start}, ${field_size});""")
    c_ccfile_unpack_array_tail = Template("""
    }
""")

    c_ccfile_pack_head = Template("""
    void ${classname}::pack_${field_name}(uint8_t *bytes, uint32_t start)
    {""")
    c_ccfile_pack_body = Template("""
        hal::utils::pack_bytes_pack(bytes, start + ${field_start}, ${field_size}, ${field_name});""")
    c_ccfile_pack_tail = Template("""
    }
""")
    c_ccfile_pack_array_head = Template("""
    void ${classname}::pack_${field_name}(uint8_t *bytes, int _idx)
    {
        PU_ASSERT(_idx < ${field_array});""")
    c_ccfile_pack_array_body = Template("""
        hal::utils::pack_bytes_pack(bytes, ${field_start}, ${field_size}, ${field_name}[_idx]);""")
    c_ccfile_pack_array_tail = Template("""
    }
""")

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

    def gen_cpp_h_file(self, l_cur_str, indent_level=0):

        if self.size != 0:
            wide_str = ''
            array_str = ''

            if self.size > 64:
                wide_str = '[{0}]'.format((self.size + 63) / 64)

            if self.array > 1:
                array_str = '[{0}]'.format(self.array)

            l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_field_info.substitute(
                field_typename='uint64_t', field_name='{0}'.format(self.name),
                field_wide= wide_str, field_array=array_str))

        if len(self.fields) > 0:
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_class_declare.substitute(
                    classname='{0}_t'.format(self.name), base=self.subtype))
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
                        l_cur_str = field.gen_cpp_h_file(l_cur_str, indent_level + 1)
                        field.instance_done = 1
            if self.name != 'root':
                l_cur_str = '{0}{1}'.format(l_cur_str, self.c_headerfile_class_tail.substitute(
                    classname='{0}_t'.format(self.name)))
        return l_cur_str

    def gen_cpp_c_file(self, cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str,
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
                                                                                           field_name='{0}'.format(
                                                                                               self.name),
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
                # init_str = '{0}{1}'.format(init_str, self.c_ccfile_init_impl.substitute( field_typename="cpp_int", field_name='{0}'.format(self.name), field_array='', field_size=self.size))
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
                init_str = '{0}{1}'.format(init_str,
                                           self.c_ccfile_init_declare.substitute(classname='{0}_t'.format(self.name),
                                                                                 field_size=self.size))

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
                        cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str, set_field_str, get_field_str = field.gen_cpp_c_file(
                            cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str,
                            init_str, set_field_str, get_field_str, indent_level + 1,
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

    def gen_c_header(self, include_map):
        cur_str = """
#ifndef {0}_H
#define {0}_H

#include "nic/include/base.h"
#include "nic/gen/cap_csr_lite/include/cap_sw_csr_base.hpp"
""".format(block_name.upper())

        for i in include_map:
            cur_str = cur_str + """#include "nic/gen/cap_csr_lite/include/{0}.hpp" 
""".format(i)

        cur_str = cur_str + """
using namespace std;""".format(block_name.upper())

        cur_str = self.gen_cpp_h_file(cur_str, 0)
        cur_str = cur_str + """
#endif // {0}_H
        """.format(block_name.upper())
        return cur_str

    def gen_c_cc(self):
        cur_str = """
#include "nic/utils/pack_bytes/pack_bytes.hpp"
#include "nic/gen/cap_csr_lite/include/{0}.hpp"

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

        cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str, set_field_str, get_field_str = self.gen_cpp_c_file(
            cur_str, head_str, tail_str, show_str, width_str, s_width_str, set_all_str, get_all_str, init_str,
            set_field_str, get_field_str, 0, "")

        # cc_file = open('ppa_decoders_csr.cc', 'w')

        return cur_str + head_str + tail_str + show_str + width_str + s_width_str + set_all_str + get_all_str + init_str + get_field_str + set_field_str
        # cc_file.write(cur_str)
        # cc_file.write(head_str)
        # cc_file.write(tail_str)
        # cc_file.write(show_str)
        # cc_file.write(width_str)
        # cc_file.write(get_all_str)
        # cc_file.write(set_all_str)
        # cc_file.write(init_str)
        # cc_file.write(get_field_str)
        # cc_file.write(set_field_str)
        # cc_file.close()

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
        print 'ERROR:', parent.name, "  " * depth, e

def gen_decoders(yaml_src_file, _block_name, outfile_name, options):
    global block_name
    if type(options) != list: options = [options]
    f = open(yaml_src_file)
    out_f = open(outfile_name, 'w')
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
    for option in options:
        if option == 'c_header':
            cur_str = filter_map.gen_c_header(include_map)
            out_f.write(cur_str)
            # cur_str= p.gen_c_header([])
            # out_f.write(cur_str)
        if option == 'c_cc':
            cur_str = filter_map.gen_c_cc()
            out_f.write(cur_str)
            # cur_str= p.gen_c_cc()
            # out_f.write(cur_str)

    out_f.close()

def gen_cap_csr_lite_base_include():
    return """#ifndef CAP_SW_CSR_BASE_H
#define CAP_SW_CSR_BASE_H
    
#include "nic/include/base.h"
#include "nic/asic/ip/verif/pcpp/cpu.h"

class cap_sw_csr_base {
    public:
        enum csr_type_t {
            CSR_TYPE_NONE = 0,
            CSR_TYPE_REGISTER,
            CSR_TYPE_MEM_ENTRY,
            CSR_TYPE_MEMORY,
            CSR_TYPE_DECODER,
            CSR_TYPE_BLOCK
        };

    protected:
        cap_sw_csr_base * base__parent;
        uint64_t base__offset;
        csr_type_t base__csr_type;
        unsigned base__csr_id;
        uint64_t base__csr_end_addr;

    public:
        cap_sw_csr_base(cap_sw_csr_base * _parent = 0);
        virtual ~cap_sw_csr_base();
    
        virtual cap_sw_csr_base * get_parent() const;
        virtual uint32_t get_width() const;
        virtual csr_type_t get_csr_type() const;
        virtual void set_csr_type(csr_type_t _type);
        virtual void init();
        virtual void pack(uint8_t *bytes, uint32_t start=0);
        virtual void unpack(uint8_t *bytes, uint32_t start=0);

        virtual void set_offset(uint64_t _offset);
        virtual uint64_t get_offset() const;
        virtual uint32_t get_chip_id() const;
        virtual uint32_t get_byte_size() const;
        virtual void write();
        virtual void read();
        virtual void write_hw(uint8_t *bytes, int block_write=0);
        virtual void read_hw(uint8_t *bytes, int block_read=0);
        virtual void read_compare(int block_read=0);
};

class cap_sw_register_base : public cap_sw_csr_base {
    public:
        cap_sw_register_base(cap_sw_csr_base * _parent = 0);
        virtual ~cap_sw_register_base();
        virtual void write();
        virtual void read();
        virtual void write_hw(uint8_t *write_bytes, int block_write=0);
        virtual void read_hw(uint8_t *write_bytes, int block_read=0);
        virtual void read_compare(int block_read=0);
};

class cap_sw_memory_base : public cap_sw_csr_base {
    public:
        cap_sw_memory_base(cap_sw_csr_base * _parent = 0);
        virtual ~cap_sw_memory_base();
        virtual void write();
        virtual void read();
};

class cap_sw_decoder_base : public cap_sw_csr_base {
    public:
        cap_sw_decoder_base(cap_sw_csr_base * _parent = 0);
        virtual ~cap_sw_decoder_base();
        virtual void write();
        virtual void read();
};

class cap_sw_block_base : public cap_sw_csr_base {

    protected:
    uint32_t block__chip_id;
    uint32_t block__byte_size;

    public:
        cap_sw_block_base(cap_sw_csr_base * _parent = 0);
        virtual ~cap_sw_block_base();
        virtual uint32_t get_chip_id() const;
        virtual void set_chip_id(uint32_t _chip_id);
        virtual void write();
        virtual void read();
        virtual void set_byte_size(uint32_t _byte_size);
        virtual uint32_t  get_byte_size() const;
};


#endif // CAP_SW_CSR_BASE_H
"""

def gen_cap_csr_lite_base_source():
    return """#include "nic/gen/cap_csr_lite/include/cap_sw_csr_base.hpp"
    
cap_sw_csr_base::cap_sw_csr_base(cap_sw_csr_base * _parent) {
    base__parent = _parent;
    base__offset = 0;
    base__csr_id = 0;
    base__csr_end_addr = 0;
}

cap_sw_csr_base::~cap_sw_csr_base() { }

cap_sw_csr_base * cap_sw_csr_base::get_parent() const {
    return base__parent;
}

uint32_t cap_sw_csr_base::get_width() const {
    return 0;
}

void cap_sw_csr_base::init() {
}

void cap_sw_csr_base::pack(uint8_t *bytes, uint32_t start) {
}

void cap_sw_csr_base::unpack(uint8_t *bytes, uint32_t start) {
}

uint32_t cap_sw_csr_base::get_byte_size() const {
    int x;
    uint32_t my_width = get_width();
    if(my_width < 32) my_width = 32;
    x  = (int) my_width;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x  = (x & ~(x >> 1));    
    if ((uint32_t)x != my_width) {
        x <<= 1;
    }
    return (x/8);
}

void cap_sw_csr_base::set_offset(uint64_t _offset) {
    base__offset = _offset;
}

uint64_t cap_sw_csr_base::get_offset() const {
    uint64_t ret_val = 0;
    if(get_parent() != 0) {
        ret_val = get_parent()->get_offset() + base__offset;
    } else {
        ret_val = base__offset;
    }
    return ret_val;
}

uint32_t cap_sw_csr_base::get_chip_id() const {
    uint32_t ret_val = 0;
    if(get_parent() != 0) {
        ret_val = get_parent()->get_chip_id();
    } else {
        HAL_TRACE_ERR("cap_sw_csr_base:get_chip_id() parent is null and its not"
                      " block or system\\n");
        return 0;
    }
    return ret_val;
}

void cap_sw_csr_base::write() {
    HAL_TRACE_ERR("cap_sw_csr_base::write() should not be used\\n");
}

void cap_sw_csr_base::read() {
    HAL_TRACE_ERR("cap_sw_csr_base::read() should not be used\\n");
}

void cap_sw_csr_base::write_hw(uint8_t *bytes, int block_write) {
    HAL_TRACE_ERR("cap_sw_csr_base::write_hw() should not be used\\n");
}

void cap_sw_csr_base::read_hw(uint8_t *bytes, int block_read) {
    HAL_TRACE_ERR("cap_sw_csr_base::read_hw() should not be used\\n");
}

void cap_sw_csr_base::read_compare(int block_read) {
    HAL_TRACE_ERR("cap_sw_csr_base::read_compare() should not be used\\n");
}

cap_sw_csr_base::csr_type_t cap_sw_csr_base::get_csr_type() const {
    return base__csr_type;
}

void cap_sw_csr_base::set_csr_type(csr_type_t _type) {
    base__csr_type = _type;
}


cap_sw_register_base::cap_sw_register_base(cap_sw_csr_base * _parent):
    cap_sw_csr_base(_parent) {
        set_csr_type(CSR_TYPE_REGISTER);
}

cap_sw_register_base::~cap_sw_register_base() { }

void cap_sw_register_base::write() {
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;
    uint32_t num_bytes = words*4;
    auto *bytes = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));
    pack(bytes);
    write_hw(bytes);
    free(bytes);
}
void cap_sw_register_base::read() {
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;
    uint32_t num_bytes = words*4;
    auto *bytes = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));
    read_hw(bytes);
    unpack(bytes);
    free(bytes);
}

void cap_sw_register_base::write_hw(uint8_t *write_bytes, int block_write) {
    uint32_t words = (get_width()+31)/32;
    uint64_t offset = get_offset();
    
    HAL_ASSERT(block_write == 0);

    for(uint32_t ii = 0; ii < words; ii++) {
        uint32_t data = *((uint32_t *)&(write_bytes[ii*4]));
        HAL_TRACE_DEBUG("cap_sw_register_base::write_hw(): Addr: {}; Data: {}\\n",
                         offset + (ii*4), data);
        cpu::access()->write(get_chip_id(), offset + (ii*4), data, false,
                       secure_acc_e);
    }
}

void cap_sw_register_base::read_hw(uint8_t *read_bytes, int block_read) {
    uint32_t chip_id = get_chip_id();
    uint64_t offset = get_offset();
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;

    HAL_ASSERT(block_read == 0);

    for(uint32_t ii = 0; ii < words; ii++) {
        uint32_t data = cpu::access()->read(chip_id, offset + (ii*4), false, secure_acc_e);
        *((uint32_t *)&(read_bytes[ii*4])) = data;
        HAL_TRACE_DEBUG("cap_sw_register_base::read_hw(): Addr: {}; Data: {}\\n",
                         offset + (ii*4), data);
    }
}

void cap_sw_register_base::read_compare(int block_read) {
    uint8_t *bytes1, *bytes2;
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;
    uint32_t num_bytes = words*4;

    bytes1 = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));
    bytes2 = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));

    pack(bytes1);
    read_hw(bytes2, block_read);

    for (uint32_t ii=0; ii<num_bytes; ii++) {
        if (bytes1[ii] != bytes2[ii]) {
            HAL_TRACE_ERR("cap_sw_register_base::read_compare(): Actual: {}, Read: {}\\n", bytes1[ii], bytes2[ii]);
        }
    }

    free(bytes1);
    free(bytes2);
}

cap_sw_memory_base::cap_sw_memory_base(cap_sw_csr_base * _parent):
    cap_sw_csr_base(_parent) {
        set_csr_type(CSR_TYPE_MEMORY);
}

cap_sw_memory_base::~cap_sw_memory_base() { }

void cap_sw_memory_base::write() {
    HAL_TRACE_ERR("cap_sw_memory_base::write() should not be used\\n");
}

void cap_sw_memory_base::read() {
    HAL_TRACE_ERR("cap_sw_memory_base::read() should not be used\\n");
}


cap_sw_decoder_base::cap_sw_decoder_base(cap_sw_csr_base * _parent):
    cap_sw_csr_base(_parent) {
        set_csr_type(CSR_TYPE_DECODER);
}

cap_sw_decoder_base::~cap_sw_decoder_base() { }

void cap_sw_decoder_base::write() {
    HAL_TRACE_ERR("cap_sw_decoder_base::write() should not be used\\n");
}

void cap_sw_decoder_base::read() {
    HAL_TRACE_ERR("cap_sw_decoder_base::read() should not be used\\n");
}


cap_sw_block_base::cap_sw_block_base(cap_sw_csr_base * _parent):
    cap_sw_csr_base(_parent) {

        block__chip_id = 0;
        set_csr_type(CSR_TYPE_BLOCK);
    }

cap_sw_block_base::~cap_sw_block_base() { }

void cap_sw_block_base::write() {
    HAL_TRACE_ERR("cap_sw_block_base::write() should not be used\\n");
}

void cap_sw_block_base::read() {
    HAL_TRACE_ERR("cap_sw_block_base::read() should not be used\\n");
}

uint32_t cap_sw_block_base::get_chip_id() const {
    return block__chip_id;
}

void cap_sw_block_base::set_chip_id(uint32_t _chip_id) {
    block__chip_id = _chip_id;
}

void cap_sw_block_base::set_byte_size(uint32_t _byte_size) {
    block__byte_size = _byte_size;
}

uint32_t cap_sw_block_base::get_byte_size() const {
    return block__byte_size;
}

"""

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

def gen_cap_csr_lite_build(files):
    sources = """\"cap_sw_csr_base.cc\","""
    includes = """\"include/cap_sw_csr_base.hpp\","""
    depends  = """\"//nic:capricsr_int\",
            \"//nic/utils/pack_bytes\","""

    for file in files:
        block = gen_block_name(file)
        if block is not None:
            sources += """\n            \"{0}.cc\",""".format(block)
            includes += """\n            \"include/{0}.hpp\",""".format(block)

    cur_str = """package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "cap_csr_lite",
    srcs = [{0}
           ],
    hdrs = [{1}
           ],
    deps = [{2}
           ],
)
""".format(sources, includes, depends)

    return cur_str

def gen_cap_csr_lite_base(gen_dir, files):

    inc_f = open(gen_dir+'include/cap_sw_csr_base.hpp', 'w')
    cur_str = gen_cap_csr_lite_base_include()
    inc_f.write(cur_str)

    src_f = open(gen_dir+'cap_sw_csr_base.cc', 'w')
    cur_str = gen_cap_csr_lite_base_source()
    src_f.write(cur_str)

#    bld_f = open(gen_dir+'BUILD', 'w')
#    cur_str = gen_cap_csr_lite_build(files)
#    bld_f.write(cur_str)

if __name__ == '__main__':

    input_dir = '../asic/capri/verif/common/csr_gen/'
    gen_dir = '../gen/cap_csr_lite/'
    if not os.path.exists(gen_dir + 'include'):
        os.makedirs(gen_dir + 'include')

    files = [f for f in os.listdir(input_dir) if os.path.isfile(os.path.join(input_dir, f)) and f.endswith(".yaml")]
    gen_cap_csr_lite_base(gen_dir, files)
    for file in files:
        block = gen_block_name(file)
        if block is not None:
            print "Processing " +input_dir+file+ " ==> " +gen_dir+block+".cc [include/"+block+".hpp]"
            gen_decoders(input_dir+file, block, gen_dir+'include/'+block+'.hpp', 'c_header')
            gen_decoders(input_dir+file, block, gen_dir+block+'.cc', 'c_cc')
        else:
            print "Skipped " + input_dir+file

