#! /usr/bin/python3
import os
import time
import iota.harness.api as api

dir_path = os.path.dirname(os.path.realpath(__file__))

def SetupDNSClient(client, server, qtype="NONE"):
    node = client.node_name
    workload = client.workload_name

    dnsscapy = dir_path + '/' + "dnsscapy.py"
    f = open(dnsscapy, "w")
    f.write("#! /usr/bin/python3\n")
    f.write("from scapy.all import *\n")
    f.write("ip=IP(src=\"%s\", dst=\"%s\")\n"%(client.ip_address,server.ip_address))
    if qtype == "LARGE_DOMAIN_NAME":
        f.write("dnspkt=UDP(sport=53333,dport=53)/DNS(rd=1, qd=DNSQR(qname='Isthisabigenoughdomainnameforareallllllyyyyybigdomainserver.howtoconstructadomainname.thatislargerthanthemaximumdomainnamesize.doesntseemlikeaneasydeal.isthisbigenoughforareallllllyyyybigggggdomainname.thenamedoesntcrossthemaximumsize.atalllookslikeaa.com'))\n")
    elif qtype == "LONG_LABEL":
        f.write("dnspkt=UDP(sport=53333,dport=53)/DNS(rd=1, qd=DNSQR(qname='Isthisabigenoughdomainnameforareallllllyyyyybigdomainservera.howtoconstructadomainname.thatislargerthanthemaximumdomainnamesize.doesntseemlikeaneasydeal.isthisbigenoughforareallllllyyyybigggggdomainname.thenamedoesntcrossthemaximumsize.atalllookslike.com'))\n")
    elif qtype == "MULTI_QUESTION":
        f.write("dnspkt=UDP(sport=53333,dport=53)/DNS(rd=2,qd=DNSQR(qname='www.thepacketgeek.com')/DNSQR(qname='test3.example.com'))\n")
    f.write("send(ip/dnspkt)\n")
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
                           "systemctl stop named")
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl disable named")
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
                           "yes | cp dnsdir/named.conf /etc/")
        api.Trigger_AddCommand(req, node, workload,
                           "ex -s -c \'%s/192.168.100.102/%s/g|x\' /etc/named.conf"%("%s", server.ip_address))
        api.Trigger_AddCommand(req, node, workload,
                           "yes | cp dnsdir/example.com.zone /var/named/")
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl start named")
        api.Trigger_AddCommand(req, node, workload,
                           "systemctl enable named")

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    for cmd in trig_resp.commands:
        api.PrintCommandResults(cmd)

    return api.types.status.SUCCESS
