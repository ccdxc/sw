from trex.astf.api import *

http_req = b'GET /3384 HTTP/1.1\r\nHost: 22.0.0.3\r\nConnection: Keep-Alive\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727)\r\nAccept: */*\r\nAccept-Language: en-us\r\nAccept-Encoding: gzip, deflate, compress\r\n\r\n'
http_response = b'HTTP/1.1 200 OK\r\nServer: Microsoft-IIS/6.0\r\nContent-Type: text/html\r\nContent-Length: 32000\r\n\r\n<html><pre>{0}</pre></html>'

udp_req = b'GET'
udp_response = b'ACK'


class Prof1():
    def __init__(self):
        self.loop =  10
        self.no_temp_types = 2
        self.udp_port = 201
        self.http_port = 80

    def crunch_number(self, max_active_flow, no_pairs, cps):
        no_temp = self.no_temp_types * no_pairs
        flow_time = (max_active_flow / (cps)) * 1000000
        self.delay = int(flow_time / self.loop)
        self.cps = int(cps / no_temp)

    def get_global_info(self):
        c_glob_info = ASTFGlobalInfo()
        # keep alive is much longer in sec time 128sec
        c_glob_info.tcp.keepinit = 253
        c_glob_info.tcp.keepidle = 253
        c_glob_info.tcp.keepintvl = 253
        c_glob_info.tcp.rexmtthresh = 10
        #c_glob_info.tcp.delay_ack_msec = 20
        #c_glob_info.tcp.no_delay = 1
        return c_glob_info

    def get_udp_port(self):
        self.udp_port += 1
        return self.udp_port

    def get_http_port(self):
        self.http_port += 1
        return self.http_port

    def create_udp_template(self, ip_gen, delay, loop, cps):
        keepalive_time = int((loop * delay) / 1000) #UDP keepalive time in msec
        port = self.get_udp_port()

        # client commands
        prog_c = ASTFProgram(stream=False)
        prog_c.fields['commands'].append(ASTFCmdKeepaliveMsg(keepalive_time))
        prog_c.set_var("var1",loop)
        prog_c.set_label("a:")
        prog_c.send_msg(udp_req)
        prog_c.delay(delay)
        prog_c.recv_msg(1,True)
        prog_c.jmp_nz("var1","a:")

        # server commands
        prog_s = ASTFProgram(stream=False)
        prog_s.fields['commands'].append(ASTFCmdKeepaliveMsg(keepalive_time))
        prog_s.set_var("var2",loop)
        prog_s.set_label("b:")
        prog_s.recv_msg(1,True)
        prog_s.delay(delay)
        prog_s.send_msg(udp_response)
        prog_s.jmp_nz("var2","b:")

        # template
        temp_c1 = ASTFTCPClientTemplate(program=prog_c, ip_gen=ip_gen, cps=cps, port=port)
        temp_s1 = ASTFTCPServerTemplate(program=prog_s, assoc=ASTFAssociationRule(port=port))
        return ASTFTemplate(client_template=temp_c1, server_template=temp_s1)

    def create_tcp_template(self, ip_gen, delay, loop, cps):
        port = self.get_http_port()

        # client commands
        prog_c = ASTFProgram()
        prog_c.set_var("var1",loop)
        prog_c.set_label("a:")
        prog_c.send(http_req)
        prog_c.delay(delay)
        prog_c.recv((len(http_response)),True)
        prog_c.jmp_nz("var1","a:")

        # server commands
        prog_s = ASTFProgram()
        prog_s.set_var("var2",loop)
        prog_s.set_label("b:")
        prog_s.recv((len(http_req)),True)
        prog_s.delay(delay)
        prog_s.send(http_response)
        prog_s.jmp_nz("var2","b:")

        # template
        temp_c1 = ASTFTCPClientTemplate(program=prog_c,  ip_gen=ip_gen, cps=cps, port=port)
        temp_s1 = ASTFTCPServerTemplate(program=prog_s, assoc=ASTFAssociationRule(port=port))
        return ASTFTemplate(client_template=temp_c1, server_template=temp_s1)

    def create_profile(self, client_server_pair, max_active_flow, cps):
        self.crunch_number(max_active_flow, len(client_server_pair), cps)
        templates = []

        for client_ip, server_ip in client_server_pair:
            # ip generator
            ip_gen_s1 = ASTFIPGenDist(ip_range=[client_ip, client_ip], distribution="seq")
            ip_gen_c1 = ASTFIPGenDist(ip_range=[server_ip, server_ip], distribution="seq")
            ip_gen1 = ASTFIPGen(glob=ASTFIPGenGlobal(ip_offset="1.0.0.0"),
                                dist_client=ip_gen_c1,
                                dist_server=ip_gen_s1)

            # templates
            templates.append(self.create_tcp_template(ip_gen1, self.delay, self.loop, self.cps))
            templates.append(self.create_udp_template(ip_gen1, self.delay, self.loop, self.cps))

        # profile
        profile = ASTFProfile(default_c_glob_info = self.get_global_info(),
                              default_s_glob_info = self.get_global_info(),
                              default_ip_gen = ip_gen1,
                              templates = templates)
        return profile

    def parse_client_server_pair(self, client_server_pair):
        out = []
        pairs = client_server_pair.split(":")
        for p in pairs:
            client, server = p.split(",")
            out.append([client.strip(), server.strip()])
        return out

    def get_profile(self, **kwargs):
        client_server_pair = kwargs.get("client_server_pair", None)
        max_active_flow    = int(kwargs.get("max_active_flow", None))
        cps                = int(kwargs.get("cps", None))
        client_server_pair = self.parse_client_server_pair(client_server_pair)
        return self.create_profile(client_server_pair, max_active_flow, cps)

def register():
    return Prof1()
