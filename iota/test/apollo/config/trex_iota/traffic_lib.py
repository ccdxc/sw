import sys
sys.path.append('/opt/trex/v2.53/automation/trex_control_plane/interactive/trex/examples/stl/')
sys.path.append('/opt/trex/v2.53/automation/regression/stateless_test')
sys.path.append('/opt/trex/v2.53/automation/regression')
import stl_path
from trex_stl_lib.api import *
import ipaddr
from scapy.layers.vxlan import *
import time
import json
import sys 
import yamlordereddictloader

trafficfile = sys.argv[1]
trafficstreams = sys.argv[2].split(",")
port0rate = float(sys.argv[3])
port1rate = float(sys.argv[4])
duration  = float(sys.argv[5])
tolerance = float(sys.argv[6])

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
    fieldoffset['UDP']['sport']   =  0
    fieldoffset['UDP']['dport']   =  2
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
       

def simple_burst ():

    # create client
    c = STLClient()
    passed = True

    try:
        # turn this on for some information
        #c.set_verbose("high")

        # create two streams
        p0_streams = []
        p1_streams = []

        fp = open(trafficfile)
        trafficdict = yaml.load(fp,Loader=yamlordereddictloader.Loader)
        fp.close()

        for stream in trafficstreams:
            streamdict = trafficdict['streams'][stream]
            vm = []
            layers = streamdict['layers'].keys()
            pgid_start = 0
            if 'inner_loop' in streamdict.keys():
                pgid_increment = 1
                for layer in streamdict['inner_loop']['layers'].keys():
                    layertype = streamdict['layers'][layer]['type']
                    prev_layers = []
                    for prev_layer in layers:
                        if prev_layer == layer:
                            break
                        prev_layers.append(streamdict['layers'][prev_layer]['type'])
                    udfs = streamdict['inner_loop']['layers'][layer]['fields'].keys()
                    for udf in udfs:
                        udfdict  = streamdict['inner_loop']['layers'][layer]['fields'][udf]
                        udfstart = getudfval(streamdict['layers'][layer]['fields'][udf],streamdict['inner_loop']['layers'],layer,layertype,udf,0)
                        udfend   = getudfval(streamdict['layers'][layer]['fields'][udf],streamdict['inner_loop']['layers'],layer,layertype,udf,udfdict['count']-1)
                        udfsize  = getfieldlength(layertype, udf)
                        udfoffset = getoffset(prev_layers,layertype,udf)
                        if 'next_var' not in udfdict.keys():
                            vm.append(STLVmFlowVar(name=udfdict['name'],
                                min_value=udfstart,
                                max_value=udfend,
                                size=udfsize,op=udfdict['op']))
                        else:
                            vm.append(STLVmFlowVar(name=udfdict['name'],
                                min_value=udfstart,
                                max_value=udfend,
                                size=udfsize,op=udfdict['op'],next_var=udfdict['next_var']))
                        vm.append(STLVmWrFlowVar(fv_name=udfdict['name'],pkt_offset= udfoffset))
                        pgid_increment *= udfdict['count']
                    if layertype == "IP" and ('src' in udfs or 'dst' in udfs):
                        vm.append(STLVmFixIpv4(offset = getoffset(prev_layers)))

            per_stream_percent = 100.0 / streamdict['outer_loop']['count']
            for index in range(streamdict['outer_loop']['count']):
                 base = None
                 for layer in layers:
                     kwdict = {}
                     layertype = streamdict['layers'][layer]['type']
                     for field in streamdict['layers'][layer]['fields'].keys():
                         kwdict[field] = getudfval(\
                                 streamdict['layers'][layer]['fields'][field],\
                                 streamdict['outer_loop']['layers'],layer,layertype,field,index)
                     if base is None:
                         base = getlayer(layertype,kwdict)
                     else:
                         base /= getlayer(layertype,kwdict)
                 base_len = len(base)
                 padlen = streamdict['pkt_len'] - base_len
                 if padlen < 8:
                     padlen = 8
                 pad = '0' * (padlen - 8)
                 pad += 'PGID'
                 pad += '0' * 4
                 base = base / Raw(pad)
                 pgidvm = [STLVmFlowVar(name='pgid', min_value=pgid_start+(index*pgid_increment), \
                         max_value=pgid_start+(index+1)*pgid_increment, size=4,op='inc'),\
                     STLVmWrFlowVar(fv_name='pgid',pkt_offset=base_len+padlen-4)]
                 pkt= STLPktBuilder(pkt = base, vm = vm + pgidvm)
                 if streamdict['src_interface'] == 0:
                     p0_streams.append(STLStream(packet = pkt, mode = STLTXCont(percentage = per_stream_percent)))
                 else:
                     p1_streams.append(STLStream(packet = pkt, mode = STLTXCont(percentage = per_stream_percent)))
                 #base.show() 
                 #print(vm)

        # connect to server
        c.connect()                                                                

        # prepare our ports (my machine has 0 <--> 1 with static route)
        c.reset(ports = [0, 1]) #  Acquire port 0,1 for $USER                      

        # add both streams to ports
        for s1 in p0_streams:
            c.add_streams(s1, ports = [0])
        for s1 in p1_streams:
            c.add_streams(s1, ports = [1])

        # clear the stats before injecting
        c.clear_stats()
        if p0_streams and port0rate:
            c.start(ports = [0], mult = "%smpps"%port0rate, duration = duration )                     
        if p1_streams and port1rate:
            c.start(ports = [1], mult = "%smpps"%port1rate, duration = duration )                     


        # block until done
        c.wait_on_traffic(ports = [0,1])                                          

        # read the stats after the test
        stats = c.get_stats()                                                      

        print json.dumps(stats[0], indent = 4, separators=(',', ': '), \
                sort_keys = True)
        print json.dumps(stats[1], indent = 4, separators=(',', ': '), \
                sort_keys = True)

        lost_a = stats[0]["opackets"] - stats[1]["ipackets"]
        lost_b = stats[1]["opackets"] - stats[0]["ipackets"]

        lost_a_percent = 0.0
        lost_b_percent = 0.0
        if stats[1]['opackets'] > 0:
            lost_a_percent = lost_a /stats[1]['opackets']
        if stats[0]['opackets'] > 0:
            lost_b_percent = lost_b /stats[0]['opackets']

        #print "\npackets lost from 0 --> 1:   {0} pkts".format(lost_a)
        #print "packets lost from 1 --> 0:   {0} pkts".format(lost_b)

        if (lost_a_percent <= tolerance) and (lost_b <= tolerance):
            passed = True
        else:
            passed = False


    except STLError as e:
        passed = False
        print e

    finally:
        c.disconnect()                                                            

    if passed:
        print "\nTest has passed :-)\n"
    else:
        print "\nTest has failed :-(\n"


# run the tests
simple_burst()
