# Running venice a pure L3 network

Some customers can be running their network in a pure L3 environment. This doc examnises tradeoffs and changes in venice to run in such an environment.
This approach also enables environments where customers dont have all the venice nodes on the same L2 subnet.

## Background - Virtual IP

When a customer creates a cluster he can provide few IP address as VirtualIP (VIP) for the cluster.
Venice runs election and sets one of the nodes with the VIP. Currenty the winner of the master service within CMD sets the VIP on its node.
If the venice nodes are running on different subnets, then VIP may not be compatible with the winner of the election. Thats the problem we are trying to solve.

We use VIP for following customer-visible cases
1. Customer can use VIP to access venice cluster without any load balancer still achieving HA. The APIGW running on all the nodes listens on the physical address (on the election winner it listens on VIP as well). Hence customer APIs are always serviced - whether they are addressed to VIP or any node IP.

VIP is used internally within venice for following cases. These need to be changed
1. Kubernetes API Server currently runs on the master venice node (and all the kubelets connect to k8s API server at the VIP). If the master goes down, another node becomes master and claims VIP and all kubelets continue to talk to k8s API Server at VIP. 
1. Filebeat on naples and Venice nodes use VIP to talk to ElasticClient
1. Naples agent currently use VIP to talk to resolver
1. NMD on Naples joins the cluster by registering with the CMD at the VIP

we examine each of the case below

## Pure L3 network - customer visibility

In a pure L3 network, if the customer needs HA for talking to APIGW, then he can use external load balancer to connect to any/all venice nodes.
One of the main work item is to make sure that all the internal usecases dont need a VIP to operate correctly.

### Kubernetes on Venice nodes

Kubelet needs to know the address of the API Server. Currently CMD passes the Cluster VIP in /etc/kubernetes/kubelet.conf  as a command line parameter to kubelet.
When we dont have VIP, kubelet needs physical IP of the k8s API Server.  Note that kubelet talks to only one address of the k8s API server. It does not have any dynamic way to changing the API Server after the process is started. i.e it does expose any api endpoint using which this can be configured. Nor does it provide a way to re-read the configuration file like a SIGHUP.

In a HA setup, when a node fails, CMD on all nodes know of this - either through leader election (or through resolver updates).  We need a way to inform kubelet of the new k8s API Server.

There are few possible ways to solve this
a. Restart Kubelet with the new IP address
    CMD can write the new kubelet.conf with the new K8s APIServer address and restarts kubelet. In theory, this should have no disruption since kubelet will continue to talk to the new api server. However there seem to be bugs in kubelet and many cases observed ( https://github.com/kubernetes/kubernetes/issues/43516 and comments in https://github.com/kubernetes/kubernetes/issues/48638 ) where the restart of kubelet marks the node as NotHealthy temporarily and causes all the services running on the node to be scheduled out - this happens on all the healthy venice nodes and hence not acceptable.
a. Using Dynamic Proxy DNS Service
    Either CMD (or) another process can serve a DNS service. Kubelet will try to resolve this name to connect to API Server. And CMD can give the updated IP address on the next DNS query. However the success of this approach depends on the DNS cache timeout. If the kubelet caches DNS lookup then it might take a while for the updated DNS to be reflected in kubelet and we might hit node timeout before then.
a. Using Dynamic TCP proxy
    CMD (or another process if there are objections) can act as a proxy connection to the k8s API server. kubelet always talks to localhost,port say 127.0.0.1:8000 to reach the API server. CMD listens on this port and proxies to the correct API Server. When APIServer changes, CMD proxies to the new one. This can be pure TCP level proxy so that CMD does not have to know much about the protocol between kubelet and k8s API Server
a. Using Dynamic TCP proxy in kernel space
    In this case, instead of running the proxy in user-space (and hence CMD/some process possibly looking at lots of traffic), the same can be achieved by placing IPtables rules in kernel space such that traffic to localhost:specialport gets masqueraded to actual location). The mechanism to know the
    current API server is same as above. But on a change in the API Server locations, the iptables rule is updated to the new location.

Dynamic TCP Proxy is preferred because of the issues mentioned above.

### Filebeat
Filebeat has live-reload support for some types of config changes but does not for elasticsearch output config.
In the simplest case, the config file is updated with the new elastic address and filebeat can be restarted for the new config to be effective.
NMD has to implement these on naples.

An alternative approach is for NMD to program IPtables on ARM such that filebeat always talks to a localhost:specialport which gets masqueraded to the API server.
Currently iptables is considered more complex than simple restarting of filebeat.

### Naples agent connection to resolver
Naples agent can be changed to take a list of physical address where the resolver can be reached. Agents can pick one of the servers randomly and connect to them.
These addresses can be obtained as part of NMD Registration response with CMD.

### NMD on naples joining the cluster
While the exact details of discovery is beyond the scope, the NMD, instead of taking one IP address and take a list of IP addresses to contact the CMD on the venice.

## Summary
With all the changes above, we dont need to mandate that customer specify a VIP while creation of cluster and hence customer will be able run venice in a pure L3 network.

Please note that these changes will work if the customer continue to run the cluster in L2 environment also. These make one less assumption about the customer environment.