#! /usr/bin/python3
import os
import time
import iota.harness.api as api

LARGE_DOMAIN_NAME = 'Isthisabigenoughdomainnameforareallllllyyyyybigdomainserver.howtoconstructadomainname.thatislargerthanthemaximumdomainnamesize.doesntseemlikeaneasydeal.isthisbigenoughforareallllllyyyybigggggdomainname.thenamedoesntcrossthemaximumsize.atalllookslikeaa.com'
LONG_LABEL = 'Isthisabigenoughdomainnameforareallllllyyyyybigdomainserverahowtoconstructadomainname'

dir_path = os.path.dirname(os.path.realpath(__file__))

def GetTcpdumpData(node):
    resp = api.CopyFromWorkload(node.node_name, node.workload_name, ['out.txt'], dir_path)
    if resp is None:
       return None

def SetupDNSClient(client, server, qtype="NONE"):
    node = client.node_name
    workload = client.workload_name

    dnsscapy = dir_path + '/' + "dnsscapy.py"
    f = open(dnsscapy, "w")
    f.write("#! /usr/bin/python3\n")
    f.write("from scapy.all import *\n")
    f.write("import codecs\n")
    f.write("ip=IP(src=\"%s\", dst=\"%s\")\n"%(client.ip_address,server.ip_address))
    if qtype == "LARGE_DOMAIN_NAME":
        f.write("dnspkt=UDP(sport=53333,dport=53)/DNS(rd=1, qd=DNSQR(qname=\'%s\'))\n"%(LARGE_DOMAIN_NAME))
    elif qtype == "LONG_LABEL":
        f.write("data=\'0000010000010000000000003b49737468697361626967656e6f756768646f6d61696e6e616d65666f72617265616c6c6c6c6c6c7979797979626967646f6d61696e736572766572446f77746f636f6e73747275637461646f6d61696e6e616d65197468617469736c61726765727468616e7468656d6178696d756d646f6d61696e6e616d6573697a6518646f0e736e74736503636f6d0000010001\'\n")
        f.write("dnspkt=UDP(sport=53333,dport=53)/codecs.decode(data, 'hex')\n")
    elif qtype == "MULTI_QUESTION":
        f.write("dnspkt=UDP(sport=53333,dport=53)/DNS(rd=2,qd=DNSQR(qname='www.thepacketgeek.com')/DNSQR(qname='test3.example.com'))\n")
    elif qtype == "LABEL_PARSING_ERROR":
        f.write("data=\'0000010000010000000000003b49737468697361626967656e6f756768646f6d61696e6e616d65666f72617265616c6c6c6c6c6c7979797979626967646f6d61696e736572766572686f77746f636f6e73747275637461646f6d61696e6e616d65197468617469736c61726765727468616e7468656d6178696d756d646f6d61696e6e616d6573697a6518646f0e736e74736503636f6d0000010001\'\n")
    f.write("sendp(Ether(src=\"%s\", dst=\"%s\")/ip/dnspkt, iface=\"%s\")\n"%(client.mac_address, server.mac_address, client.interface))
    f.close()

    resp = api.CopyToWorkload(node, workload, [dnsscapy], 'dnsdir')
    if resp is None:
       return None

    os.remove(dnsscapy)

def SetupDNSServer(server, stop=False):
    node = server.node_name
    workload = server.workload_name

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if (stop == True):
        api.Trigger_AddCommand(req, node, workload,
                           "sudo systemctl stop named")
        api.Trigger_AddCommand(req, node, workload,
                           "sudo systemctl disable named")
    else:
        zonefile = dir_path + '/' + "example.com.zone"
        api.Logger.info("fullpath %s" % (zonefile))
        resp = api.CopyToWorkload(node, workload, [zonefile], 'dnsdir')
        if resp is None:
            return None

        named_conf = dir_path + '/' + "named.conf"
        resp = api.CopyToWorkload(node, workload, [named_conf], 'dnsdir')
        if resp is None:
            return None

        api.Trigger_AddCommand(req, node, workload,
                           "yes | sudo cp dnsdir/named.conf /etc/")
        api.Trigger_AddCommand(req, node, workload,
                           "ex -s -c \'%s/192.168.100.102/%s/g|x\' /etc/named.conf"%("%s", server.ip_address))
        api.Trigger_AddCommand(req, node, workload,
                           "yes | sudo cp dnsdir/example.com.zone /var/named/")
        api.Trigger_AddCommand(req, node, workload,
                           "sudo systemctl start named")
        api.Trigger_AddCommand(req, node, workload,
                           "sudo systemctl enable named")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

    return api.types.status.SUCCESS
