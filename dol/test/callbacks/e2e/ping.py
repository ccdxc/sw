#! /usr/bin/python3
import pdb
import infra.api.api as infra_api

def GetPingCmd(testcase, args):
    src = testcase.config.root.src_ep_hdl
    dst = testcase.config.root.dst_ep_hdl
    if args.to == "src":
        return "ping -c 5  -d -i 3 " + src.GetIp()
    elif args.to == "dst":
        return "ping -c 5  -d -i 3 " + dst.GetIp()
    else:
        assert(0)
        
    
