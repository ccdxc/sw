from trex.astf.api import *
import os


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

        # ip generator
        ip_gen_s1 = ASTFIPGenDist(ip_range=[client_ip, client_ip], distribution="seq")
        ip_gen_c1 = ASTFIPGenDist(ip_range=[server_ip, server_ip], distribution="seq")
        ip_gen1 = ASTFIPGen(glob=ASTFIPGenGlobal(ip_offset="1.0.0.0"),
                           dist_client=ip_gen_c1,
                           dist_server=ip_gen_s1)

        # template
        temp_c1 = ASTFTCPClientTemplate(program=my_client_prog1,
                                        ip_gen=ip_gen1, cps=cps)
        temp_s1 = ASTFTCPServerTemplate(program=my_server_prog1)  # using default association
        template1 = ASTFTemplate(client_template=temp_c1,
                                 server_template=temp_s1)

        # profile
        profile = ASTFProfile(default_ip_gen=ip_gen1,templates=[template1])
        return profile

    def get_profile(self, **kwargs):
        client_ip = kwargs.get("client_ip", "128.0.0.1")
        server_ip = kwargs.get("server_ip", "128.0.0.1")
        cps =       kwargs.get("cps", 100)
        pcap_file = kwargs.get("pcap_file", "http_get.pcap")

        return self.create_profile(str(client_ip), str(server_ip), cps, pcap_file)

def register():
    return Prof1()
