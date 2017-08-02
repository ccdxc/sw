#!/usr/bin/env python

import json
import pprint
import yaml
import yaml.constructor
import copy
import re
from collections import OrderedDict

import sys, getopt
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
                    node.start_mark, 'found unacceptable key (%s)' % exc, key_node.start_mark)
            value = self.construct_object(value_node, deep=deep)
            mapping[key] = value
        return mapping


def recurse(e, parent, depth=0):
    if isinstance(e,list):
        for i in e:
            recurse(i, parent, depth+1)

    elif isinstance(e,dict):
        for k in e:
            if k == 'size':
                parent.size = e[k]
                continue
            if k == 'decoder':
                parent.decoder = e[k]
                continue
            elif k == 'array':
                parent.array = e[k]
                continue
            elif k == 'fields':
                recurse(e[k], parent, depth+1)
                continue
            elif k == 'offset':
                parent.offset = e[k]
            elif k == 'block_byte_size':
                parent.byte_size = e[k]
            elif k == 'reset_value':
                parent.reset_value = e[k]
                parent.value = parent.reset_value
            elif k == 'type':
                parent.subtype = e[k]
            else :
                p = component_info()
                p.name = k.rstrip()
                parent.fields.append(p)
                recurse(e[k],  parent.fields[-1], depth+1)
    else:
        print 'ERROR:',parent.name,"  "*depth, e

lvl = 0

class component_info:
    def __init__(self):
        self.name = "foo"
        self.size = 0;
        self.array = 0
        self.decoder = ""
        self.total_size = 0
        self.byte_size = 0;
        self.reset_value = "0";
        self.subtype = "decoder";
        self.offset = 0
        self.value = 0
        self.fields = []

    def prn(self):
        global lvl
        if self.name == "root":
            lvl = 0
        print " "*lvl, self.name, self.decoder,
        if hasattr(self,'dec_inst'):
            print self.dec_inst.name
        else:
            print
        lvl += 1
        for f in self.fields:
            f.prn()
        lvl -= 1

    def find(self, name):
        if self.name == name:
            return self
        else:
            for f in self.fields:
                ret = f.find(name)
                if ret:
                    return ret
        return None

    def rep(self):
        l = [ self ]
        if self.array != 0:
            count = self.array
            self.array = 1
            l = [ self ]
            for i in range(0,count-1):
                #print self.name," ",
                l.append(copy.deepcopy(self))
            #print
        return l

    def link_dec(self, root):
        self.fields = [ f.rep() for f in self.fields ]
        self.fields = [f for sl in self.fields for f in sl ]
        for f in self.fields:
            if f.decoder != "":
                f.dec_inst = copy.deepcopy(root.find(f.decoder))
            f.link_dec(root)

    def set_value(self, val):
        #print '>>>>', self.name
        #pprint.pprint(val)

        if self.decoder == "" and len(self.fields) == 0:
            self.value = int(val['value'],0) & ((1<<int(self.size)) - 1)
            #print "Setting", self.name, self.value
        elif self.decoder != "":
            self.dec_inst.set_value(val)
        else:
            f_inst = 0
            f_old = ""
            if (isinstance(val, dict)):
                for f in self.fields:
                    if f.name in val:
                        if isinstance(val[f.name], dict):
                            f.set_value(val[f.name])
                            f_inst = 0
                        else:
                            if f_old != f.name:
                                f_inst = 0
                                f_old = f.name
                            #print f.name, f_inst
                            #pprint.pprint(val[f.name])
                            f.set_value(val[f.name][f_inst])
                            f_inst = f_inst + 1

    def get_value(self):
        curr_size = 0
        val = 0
        if len(self.fields):
            for f in self.fields:
                #print "GET", f.name, f.value
                if f.decoder == "":
                    val = val | f.value << curr_size
                    curr_size += f.size
                else:
                    (sz,v) = f.dec_inst.get_value()
                    val = val | v << curr_size
                    curr_size += sz
                #print "GET", curr_size, "0x%x"%val
            return (curr_size,val)
        else:
            return (size, int(value,0))

class sknob_dumper:
    reg_file = ""
    mem_file = ""
    csr_file = ""
    decoder_file = ""
    reg_dict = {}
    mem_dict = {}
    csr_dict = {}
    decoder_dic = {}
    def __init__(self, reg_json = "", mem_json = "", csr_json = "", decoder_json = ""):
        self.reg_file = reg_json
        self.mem_file = mem_json
        self.csr_file = reg_json
        self.decoder_file = decoder_json
        pass

    def read_reg_json(self):
        if self.reg_file != "":
            fp = open(self.reg_file, "r")
            self.reg_dict = json.load(fp)
            for b in self.reg_dict.keys():
                if 'registers' in self.reg_dict[b].keys():
                    self.reg_dict = self.reg_dict[b]['registers']
            self.del_reg_defaults()
            #pprint.pprint(self.reg_dict)



    def link_dec(self):
        self.p.link_dec(self.p)

    def del_reg_defaults(self):
        for reg, reginfo in self.reg_dict.items():
            default_value = True
            #print reg
            #pprint.pprint(reginfo)
            for field,info in reginfo.iteritems():
                if field == 'is_array': continue
                if 'value' in info and info['value'] != "-1" and info['value'] != "Default":
                    default_value = False
            if default_value:
                #print "deleting", reg
                del self.reg_dict[reg]


    def print_reg_sknobs(self, prefix):
        for reg, reginfo in self.reg_dict.items():
            regval = 0
            for field,info in reginfo.iteritems():
                if field == 'is_array': continue
                if 'value' in info and info['value'] != "-1" and info['value'] != "Default":
                    regval = regval | ((int(info['value'],0) << int(info['field_lsb'])))
            reg = reg.replace('[','/')
            reg = reg.replace(']','')
            tmp_prefix = re.sub('/[0-9]+/*', '', prefix);
            if 'inst_name' in reginfo:
                iname = reginfo['inst_name']
                iname = re.sub('^[^\.]*\.','',iname)
                iname = iname.replace(".","/")
                m = re.search('(/[0-9]+)$',reg)
                if m:
                    iname = iname + m.group(1)
                reg = iname
            else:
                reg = reg.replace(tmp_prefix+"_","")

            m = re.search('^dhs.*_entry/([0-9]+)$', reg)
            if m:
                reg = re.sub('_entry/[0-9]+','/entry/'+m.group(1), reg)
            print '+'+prefix + reg + "=0x" + "%x"%regval


    def load_decoder(self):
        if self.decoder_file == "":
            return
        f = open( self.decoder_file)
        data_map = yaml.load(f, OrderedDictYAMLLoader)
        self.p = component_info()
        self.p.name = 'root'
        recurse(data_map, self.p)
        self.link_dec()
        #self.p.prn()

    def merge_mem_json(self, prefix):
        if self.mem_file == "":
            return
        f = open(self.mem_file)
        if self.mem_file != "":
            f = open(self.mem_file, "r")
            self.mem_dict = json.load(f)
        else:
            return None
        for mem,mem_entry in self.mem_dict.iteritems():
            if 'decoder' in mem_entry.keys():
                iname = ""
                tmp_prefix = re.sub('/[0-9]+/*', '', prefix);
                if "addr_offset" in mem_entry:
                    iname = mem_entry['addr_offset']
                if 'inst_name' in mem_entry:
                    iname = mem_entry['inst_name']
                if iname != "":
                    iname = re.sub('^[^\.]*\.','',iname)
                    iname = iname.replace(".","/")
                    mem = iname
                else:
                    mem = mem.replace(tmp_prefix+"_","")
                dec = self.p.find(mem_entry['decoder'])
                for e in mem_entry['entries']:
                    dec_inst = copy.deepcopy(dec)
                    dec_inst.set_value(e)
                    (sz,val) = dec_inst.get_value()
                    print '+'+ prefix +  mem + "/entry/" + e['entry_idx'] + "=" + "0x%x"%val

def main(argv):
    try:
      opts, args = getopt.getopt(argv[1:],"hr:m:c:d:o:p:",["reg_file=","mem_file=","csr_file=","dec_file=","ofile=","prefix="])
    except getopt.GetoptError:
      print argv[0], '-r <reg_file> -m <mem_file> -c <csr_file> -d <decoder_file> -o <outfile> -p <prefix>'
      sys.exit(2)
    (regfile,memfile,csrfile,decoderfile,outputfile,opt_prefix) = ("","","","","","")
    for opt, arg in opts:
      if opt == '-h':
         print argv[0], '-r <reg_file> -m <mem_file> -c <csr_file> -d <decoder_file> -o <outfile>'
         sys.exit()
      elif opt in ("-r", "--reg_file"):
         regfile = arg
      elif opt in ("-m", "--mem_file"):
         memfile = arg
      elif opt in ("-c", "--csr_file"):
         csrfile = arg
      elif opt in ("-d", "--dec_file"):
         decoderfile = arg
      elif opt in ("-o", "--ofile"):
         outputfile = arg
      elif opt in ("-p", "--prefix"):
         opt_prefix = arg

    my_dumper = sknob_dumper(regfile,memfile,csrfile,decoderfile)
    my_dumper.load_decoder()
    my_dumper.merge_mem_json(opt_prefix)
    my_dumper.read_reg_json()
    my_dumper.print_reg_sknobs(opt_prefix)


if __name__ == "__main__":
    main(sys.argv)
