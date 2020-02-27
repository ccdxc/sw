from trex.astf.api import *
import os
import sys
import random
import ipaddress

class Prof1():
    def __init__(self):
        pass  # tunables

    def create_profile(self, client_ip, server_ip, cps, pcap_file):
        pcap_file = os.path.join(os.path.dirname(__file__),
                                 "pcap", pcap_file)
        # Client program taken from client side of given file
        my_client_prog1 = ASTFProgram(file=pcap_file, side="c")
        # Server program taken from server side of given file
        my_server_prog1 = ASTFProgram(file=pcap_file, side="s")
        port_count = 1
        ip_count = 15 #number of secondary ips on the vnic
        seed_val = random.randrange(sys.maxsize)
        random.seed(seed_val)
        template_list = []


        # ip generator
        print(client_ip)
        print(server_ip)
        print(str(ipaddress.IPv4Address(client_ip)+ip_count))
        print(str(ipaddress.IPv4Address(server_ip)+ip_count))
        ip_gen_s1 = ASTFIPGenDist(ip_range=[client_ip, str(ipaddress.IPv4Address(client_ip)+ip_count)], distribution="seq")
        ip_gen_c1 = ASTFIPGenDist(ip_range=[server_ip, str(ipaddress.IPv4Address(server_ip)+ip_count)], distribution="seq")
        ip_gen1 = ASTFIPGen(glob=ASTFIPGenGlobal(ip_offset="1.0.0.0"),
                           dist_client=ip_gen_c1,
                           dist_server=ip_gen_s1)
        
        # template
        temp_c1 = ASTFTCPClientTemplate(program=my_client_prog1,ip_gen=ip_gen1,cps=(cps/port_count))
        temp_s1 = ASTFTCPServerTemplate(program=my_server_prog1)  # using default association
        template1 = ASTFTemplate(client_template=temp_c1,
                                 server_template=temp_s1)

        template_list.append(template1)

        i = 1
        while i < port_count :
            #port_gen = random.randint(81,1000)
            port_gen = 80 + i #trex doesnt like duplicate server ports
            temp_c2 = ASTFTCPClientTemplate(program=my_client_prog1,ip_gen=ip_gen1,cps=(cps/port_count),port=port_gen)
            temp_s2 = ASTFTCPServerTemplate(program=my_server_prog1,assoc=ASTFAssociationRule(port=port_gen))
            template_port = ASTFTemplate(client_template=temp_c2,
                                 server_template=temp_s2)
            template_list.append(template_port)
            i = i + 1
            
       # profile
        profile = ASTFProfile(default_ip_gen=ip_gen1,templates = template_list)
        return profile

    def get_profile(self, **kwargs):
        client_ip = kwargs.get("client_ip", "128.0.0.1")
        server_ip = kwargs.get("server_ip", "128.0.0.1")
        cps =       kwargs.get("cps", 100)
        pcap_file = kwargs.get("pcap_file", "http_get.pcap")

        return self.create_profile(str(client_ip), str(server_ip), cps, pcap_file)

def register():
    return Prof1()
