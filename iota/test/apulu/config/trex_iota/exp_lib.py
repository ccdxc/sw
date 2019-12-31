import stl_path
from trex_stl_lib.api import *
import ipaddr
from scapy.layers.vxlan import *
import time
import json
import sys 
import yamlordereddictloader
import sqlite3
from copy import deepcopy
import pdb

trafficfile = sys.argv[1]
trafficstreams = sys.argv[2].split(",")
port0rate = float(sys.argv[3])
port1rate = float(sys.argv[4])
duration  = float(sys.argv[5])
pcapfile  = sys.argv[6]

def convert_to_colon_separate(inputstr):
    returnstr = ''
    for index in range(len(inputstr)/2-1):
        returnstr +=  inputstr[index * 2 : index * 2 + 2] + ':'
    index += 1
    returnstr +=  inputstr[index * 2 : index * 2 + 2]
    return returnstr

def convert_v4_to_v6(inputstr):
    inputlist = inputstr.split(".")
    returnstr = '%02x'%int(inputlist[0]) + '%02x'%int(inputlist[1]) + ":" + '%02x'%int(inputlist[2]) + '%02x'%int(inputlist[3])
    return returnstr

def getlayer(layertype, kwdict):
    if layertype == 'Ether':
        return Ether(**kwdict)
    elif layertype == 'IP':
        return IP(**kwdict)
    elif layertype == 'IPv6':
        return IPv6(**kwdict)
    elif layertype == 'Dot1Q':
        return Dot1Q(**kwdict)
    elif layertype == 'UDP':
        return UDP(**kwdict)
    elif layertype == 'TCP':
        return TCP(**kwdict)
    elif layertype == 'VxLAN':
        return VxLAN(**kwdict)

def getlayerlength(layertype):

    lengths = {}
    lengths['Ether'] = 14
    lengths['Dot1Q'] =  4
    lengths['IP']    = 20
    lengths['IPv6']  = 40
    lengths['UDP']   =  8
    lengths['TCP']   = 20
    lengths['VxLAN'] =  8
    return lengths[layertype]

def getfieldlength(layertype, field):

    lengths = {}
    lengths['Ether'] = {}
    lengths['IP']    = {}
    lengths['IPv6']    = {}
    lengths['TCP']   = {}
    lengths['UDP']   = {}
    lengths['Dot1Q'] = {}
    lengths['VxLAN'] = {}

    lengths['Ether']['src'] = 6
    lengths['Ether']['dst'] = 6
    lengths['IP']['src']    = 4
    lengths['IP']['dst']    = 4
    lengths['UDP']['sport'] = 2
    lengths['UDP']['dport'] = 2
    lengths['TCP']['sport'] = 2
    lengths['TCP']['dport'] = 2
    lengths['IPv6']['src']  = 16
    lengths['IPv6']['dst']  = 16
    lengths['VxLAN']['vni']  = 3

    return lengths[layertype][field]

def getfieldoffset(layertype, field):

    fieldoffset = {}
    fieldoffset['Ether'] = {}
    fieldoffset['IP']    = {}
    fieldoffset['IPv6']  = {}
    fieldoffset['TCP']   = {}
    fieldoffset['UDP']   = {}
    fieldoffset['Dot1Q'] = {}
    fieldoffset['VxLAN'] = {}

    fieldoffset['Ether']['src'] =  6
    fieldoffset['Ether']['dst'] =  0
    fieldoffset['IP']['src']    = 12
    fieldoffset['IP']['dst']    = 16
    fieldoffset['IP']['checksum'] = 10
    fieldoffset['IPv6']['src']  =  8
    fieldoffset['IPv6']['dst']  = 24
    fieldoffset['UDP']['src']   =  0
    fieldoffset['UDP']['dst']   =  2
    fieldoffset['VxLAN']['vni'] =  4

    return fieldoffset[layertype][field]

def getoffset(prev_layers, cur_layer = None, field = None):

    returnval = 0
    for layer in prev_layers:
        returnval += getlayerlength(layer)
    if cur_layer and field:
        returnval += getfieldoffset(cur_layer, field)
    return returnval

def getudfval(initial_value, udfs, layer, layertype, field, instance_count):

    if layer not in udfs.keys():
        return initial_value
    if field not in udfs[layer]['fields'].keys():
        return initial_value
    if 'offset' not in udfs[layer]['fields'][field]:
        # TODO Only increment operation is handled. 
        if udfs[layer]['fields'][field]['op'] == 'inc':
            if layertype == 'IP' and field in ['src','dst']:
                start = int(ipaddr.IPAddress(initial_value))
                return int(ipaddr.IPAddress(start)+(instance_count * udfs[layer]['fields'][field]['step']))
            else:
                return initial_value + (udfs[layer]['fields'][field]['step'] * instance_count)
    if layertype == 'Ether' and field in ['src','dst']:
       macbytes =  initial_value.split(":")
       offset = udfs[layer]['fields'][field]['offset']
       udflen = udfs[layer]['fields'][field]['len']
       udfbytes = macbytes[offset:udflen]
       macstr = ":".join(udfbytes)
       macint = int(macstr.translate(None, ":.- "), 16)
       macint += instance_count * udfs[layer]['fields'][field]['step']
       udfnibbles = udflen * 2
       pattern = "{:0%sx}"%udfnibbles
       machex = pattern.format(macint)
       macstr = ":".join(machex[i:i+2] for i in range(0, len(machex), 2))
       returnstr = ""
       if offset > 0:
           returnstr = macbytes[0]
           for index in range(1,offset):
               retrunstr = returnstr+macbytes[index]+":"
           if offset == 1:
               retrunstr = returnstr+":"
       returnstr += macstr
       if (offset + udflen) < 6:
           initial_index = offset + udflen
           for index in range(initial_index,5):
               returnstr = returnstr + ":" + macbytes[index] + ":"
           returnstr = returnstr + macbytes[5]
       return returnstr
      

def get_max_count_update_repeat_count(udfdict):

    udfs = udfdict.keys()
    max_count = 0
    root_nodes = []
    for udf in udfdict.keys():
        if 'prev_var' not in udfdict[udf].keys(): 
            root_nodes.append(udf)
            if udfdict[udf]['count'] > max_count:
                max_count = udfdict[udf]['count']
    for root in root_nodes:
        total_count = 1
        udfdict[root]['repeat_count'] = 1
        cur_node = root
        while True:
            if 'next_var' in udfdict[cur_node].keys():
               udfdict[cur_node]['repeat_count'] = total_count
               total_count *= udfdict[cur_node]['count']
               cur_node =  udfdict[cur_node]['next_var']
            else:
               if cur_node != root:
                   udfdict[cur_node]['repeat_count'] = total_count
                   total_count *= udfdict[cur_node]['count']
               break
        udfdict[root]['total_count'] = total_count
        if total_count > max_count:
            max_count = total_count

    return (max_count,udfdict)

def get_insert_str(table,newkwdict):

    columns  = ''
    values   = ''
    for key in newkwdict.keys():
        if columns:
            columns =  columns + "," + key
            values  =  values + ",'" + str(newkwdict[key]) + "'"
        else:
            columns = key
            values  = "'"+newkwdict[key]+"'"
    return_str = 'INSERT INTO %s(%s) VALUES(%s)'%(table,columns,values)
    return return_str

def get_query_str(table,newkwdict):

    columns  = ''
    values   = ''
    for key in newkwdict.keys():
        if values:
            columns =  columns + "," + key
            values  =  values + " and %s == '"%key + str(newkwdict[key]) + "'"
        else:
            columns = key
            values  =  "%s == '"%key + str(newkwdict[key]) + "'"
    return_str = 'SELECT %s FROM  %s where %s'%(columns,table,values)
    return return_str

def simple_burst ():

    # create client
    #c = STLClient()
    #passed = True

    #try:
        # turn this on for some information
        #c.set_verbose("high")

        # create two streams
        p0_streams = []
        p1_streams = []

        fp = open(trafficfile)
        trafficdict = yaml.load(fp,Loader=yamlordereddictloader.Loader)
        fp.close()

        db = sqlite3.connect(':memory:')
        cur = db.cursor()

        for stream in trafficstreams:
            streamdict = trafficdict['streams'][stream]['expected_pkt']
            vm = []
            layers = streamdict['layers'].keys()
            query_str = ''
            for layer in layers:
                layertype = streamdict['layers'][layer]['type']    
                if layertype == 'Ether':
                    field_names = [field.name for field in Ether.fields_desc]
                elif layertype == 'IP':
                    field_names = [field.name for field in IP.fields_desc]
                elif layertype == 'IPv6':
                    field_names = [field.name for field in IPv6.fields_desc]
                elif layertype == 'TCP':
                    field_names = [field.name for field in TCP.fields_desc]
                elif layertype == 'UDP':
                    field_names = [field.name for field in UDP.fields_desc]
                elif layertype == 'VXLAN':
                    field_names = [field.name for field in VXLAN.fields_desc]
                for field_name in field_names:
                    if query_str:
                        query_str += ', %s_%s TEXT'%(layer,field_name)
                    else:
                        query_str = '%s_%s TEXT'%(layer,field_name)
            create_str = 'CREATE TABLE %s (%s)' % (stream,query_str)
            cur.execute(create_str)
            maxudf_count=0
            udfdict = collections.OrderedDict()
            inner_udfs = collections.OrderedDict()
            if 'inner_loop' in streamdict.keys():
                for layer in streamdict['inner_loop']['layers'].keys():
                    layertype = streamdict['layers'][layer]['type']
                    udfdict = streamdict['inner_loop']['layers'][layer]['fields']
                    for field in streamdict['inner_loop']['layers'][layer]['fields'].keys():
                        field_name = '%s_%s'%(layer,field)
                        if field_name not in inner_udfs:
                            inner_udfs[field_name]  = {}
                        inner_udfs[field_name]['layer_name']  = layer
                        inner_udfs[field_name]['count']       = streamdict['inner_loop']['layers'][layer]['fields'][field]['count']
                        inner_udfs[field_name]['start']       = getudfval(streamdict['layers'][layer]['fields'][field],streamdict['inner_loop']['layers'],layer,layertype,field,0)
                        inner_udfs[field_name]['end']         = getudfval(streamdict['layers'][layer]['fields'][field],streamdict['inner_loop']['layers'],layer,layertype,field,udfdict[field]['count'])
                        if 'next_var' in streamdict['inner_loop']['layers'][layer]['fields'][field].keys():
                            next_var = streamdict['inner_loop']['layers'][layer]['fields'][field]['next_var']
                            inner_udfs[field_name]['next_var']    = streamdict['inner_loop']['layers'][layer]['fields'][field]['next_var']
                            if next_var not in inner_udfs:
                                inner_udfs[next_var] = {}
                            inner_udfs[next_var]['prev_var'] = field_name
                maxudf_count,udfdict = get_max_count_update_repeat_count(inner_udfs)
            for index in range(streamdict['outer_loop']['count']):
                 kwdict = collections.OrderedDict()
                 for layer in layers:
                     layertype = streamdict['layers'][layer]['type']
                     for field in streamdict['layers'][layer]['fields'].keys():
                         kwdict['%s_%s'%(layer,field)] = getudfval(\
                                 streamdict['layers'][layer]['fields'][field],\
                                 streamdict['outer_loop']['layers'],layer,layertype,field,index)
                 insert_str = get_insert_str(stream,kwdict)
                 if maxudf_count != 0:
                     for index in range(maxudf_count):
                         newkwdict = deepcopy(kwdict)
                         for udf in udfdict.keys():
                             layer = udfdict[udf]['layer_name']
                             layertype = streamdict['layers'][layer]['type']
                             udfindex = index
                             if udfdict[udf]['repeat_count'] != 1:
                                 udfindex=((index+udfdict[udf]['repeat_count'])/udfdict[udf]['repeat_count'])-1
                             udfindex = udfindex % udfdict[udf]['count']
                             field_name = udf.split(layer)[1].lstrip("_")
                             if udf in newkwdict.keys():
                                 newkwdict[udf] = getudfval(newkwdict[udf],streamdict['inner_loop']['layers'],layer,layertype,field_name,udfindex)
                             else:
                                 newkwdict[udf] = getudfval(streamdict['layers'][layer]['fields'][field_name],streamdict['inner_loop']['layers'],layer,layertype,field_name,udfindex)
                             if layertype == "IP" and field_name in ['src','dst']:
                                 newkwdict[udf] = str(ipaddr.IPAddress(newkwdict[udf]))
                         insert_str = get_insert_str(stream,newkwdict)
                         cur.execute(insert_str)
                 else:
                     cur.execute(insert_str)


                 #base.show() 


    #except:
    #    import pdb
        #pdb.set_trace()
        pkts = rdpcap(pcapfile)
        for pkt in pkts:
            cur_pkt = pkt
            kwdict = collections.OrderedDict()
            totallayers = len(layers)
            for layer in layers:
                for field in streamdict['layers'][layer]['fields'].keys():
                    try:
                        kwdict['%s_%s'%(layer,field)] = cur_pkt.fields[field]
                    except:
                        pdb.set_trace()
                layerindex = layers.index(layer)
                nextlayer  = layerindex + 1
                if nextlayer == totallayers:
                    continue
                layertype = streamdict['layers'][layers[nextlayer]]['type']
                if layertype == 'Ether':
                    cur_pkt = cur_pkt[Ether]
                elif layertype == 'IP':
                    cur_pkt = cur_pkt[IP]
                elif layertype == 'UDP':
                    cur_pkt = cur_pkt[UDP]
                elif layertype == 'TCP':
                    cur_pkt = cur_pkt[TCP]
                elif layertype == 'VXLAN':
                    cur_pkt = cur_pkt[VXLAN]
                elif layertype == 'IPv6':
                    cur_pkt = cur_pkt[IPv6]
            query_str = get_query_str(stream,kwdict)
            query_result = cur.execute(query_str)
            rows = query_result.fetchall()
            if len(rows) == 0:
                print('%s not found'%query_str)
                #pdb.set_trace()
            else:
                pass

# run the tests
simple_burst()
