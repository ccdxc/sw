## VPP cheatsheet

To login to VPP:
telnet 0 5002

################################
         VPP Commands
################################
Admin up DPDK interface:
By default DPDK interface is down in VPP. This will be fixed later, but for now we need to bring it up by:
set interface state VirtualFunctionEthernet0 up

Check interface state:
show interface --> shows admin state
show hardware-interface --> shows link state

Enable packet trace:
This will trace next 50 packets received from DPDK.
trace add dpdk-input 50
To see packet trace:
show trace
To clear trace:
clear trace

Node counters:
show error/show node counter
Sample output for flow programming:
vpp# show errors 
   Count                    Node                  Reason
    464430          pen-p4cpu-hdr-lookup          IPv4 flow packets
         1          pen-p4cpu-hdr-lookup          IPv6 flow packets
    928814          pen-ip4-flow-program          Flow programming success
        46          pen-ip4-flow-program          Flow programming failed
         2          pen-ip6-flow-program          Flow programming success

To clear:
clear error

