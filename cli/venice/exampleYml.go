package main

var exampleYmls = map[string]string{
	`cluster`: `kind: cluster			# kind of this object
meta:
  name: vmware-lab22		# unique name of this object
  labels:			# arbitrary user defined tags 'key: value' 
    allowed: qa,prod,dev
    dc: sjc-lab22
    type: vcenter
spec:				# configuration specification intent
  nodes:			# set of nodes for management purposes
  - mgmt1
  - mgmt2
  - mgmt3
  - mgmt4
  - mgmt5
  quorumNodes:			# nodes implementing highly available distributed state system
  - mgmt1
  - mgmt2
  - mgmt3
  workerNodes:			# service nodes where workloads run
  - node1
  - node2
  - node3
  - node4
  - node5
  - node6
  - node7
  - node8
  - node9
  - node10
  - node21
  - node22
  - node23
  - node24
  - node25
`,
	`lbpolicy`: `kind: lbpolicy 			# kind of this object
meta:
  name: basic-lb-policy 	# name of this object
  # tenant: default		# override tenant value (applicable only for super user)
spec:				# configuration specification intent
  type: L4			# type of load balancing 'l4', or 'l7'
  algorithm: round-robin	# load balancing algorithm, can be 'round-robin', 'random', or 'latency-based'
  sessionAffinity: "true"	# 'true' or 'false' indicating if affinity to the backend must be maintained
  healthCheck:			# health check parameters for a given backend
    interval: 30                # time interval within which to send a probe
    maxtimeouts: 3              # maximum number of timeouts before declaing a backend dead
    probesPerInterval: 1        # how many probes are sent within healthCheck interval

`,
	`mirror`: `# please edit and/or delete the lines in this sample
kind: mirror                                            # kind of this object
meta:
  name: mirror-session-2                                # name of this object
  # tenant: default		                        # override tenant value (applicable only for super user)
  # labels:			                        # arbitrary user defined tags 'key: value' 
  #  area: network
spec:                                                   # configuration specification intent
  applyToWorkloads:                                     # selection (ORed list) of workloads to which this policy is applied
  - matchlabels: io.pensando.network.sg:dmz-web-sg      # match applications tagged as labels defined here
  - matchNames: dev-app*                                # match names of the workloads
    matchDestPorts: udp/4404                            # match only traffic going to udp/4404
  - matchDestPrefix: 172.15.0.0/16                      # match a set of destination prefixes
    matchDestPorts: tcp/4090                            # match flows on to specific ports in addition
  - matchSrcPrefix: 10.33.2.0/24                        # match a set of source IP address prefixes
  destinationIp: 192.131.4.54                           # destination IP address to which mirroed packets are to be sent to
  encap: erspan                                         # erspan encapsulate frames, other options are netflow, tsdb

`,
	`monitoringpolicy`: `kind: monitoringpolicy                                  # kind of this object
meta:
  name: flow-monitoring-policy                          # name of this object
  # tenant: default		                        # override tenant value (applicable only for super user)
spec:
  applyToWorkloads:                                     # selection (ORed list) of workloads to which this policy is applied
  - matchlabels: io.pensando.network.sg:dmz-web-sg      # match applications tagged as labels defined here
  - matchNames: prod*                                   # match names of the workloads
    matchDestPorts: udp/4404                            # match only traffic going to udp/4404
  - matchDestPrefix: 172.15.0.0/16                      # match a set of destination prefixes
    matchDestPorts: tcp/4090                            # match flows on to specific ports in addition
  - matchSrcPrefix: 10.33.2.0/24                        # match a set of source IP address prefixes
  collectionpolicy: flow-collector
  exportpolicy: tsdb-slow-exporter

`,
	`network`: `# please edit and/or delete the lines in this sample
kind: network                   # kind of this object
meta:
  name: dev-network             # unique name of this object
  labels:			# arbitrary user defined tags 'key: value' 
    dmz: internal
    dns: auto
    ipam: dhcp
  # tenant: default		# override tenant value (applicable only for super user)
spec:                           # configuration specification intent
  type: bridged                 # bridged network, other options are 'routed', 'overlay'
  gateway: 10.13.1.254          # default gateway (aka router IP address) for the network
  subnet: 10.13.0.0/16          # subnet and mask for the network

`,
	`permission`: `# please edit and/or delete the lines in this sample
kind: permission                # kind of this object
meta:
  name: network-rw              # unique name of this object
  # tenant: default		# override tenant value (applicable only for super user)
  labels:			# arbitrary user defined tags 'key: value' 
    area: network
spec:                           # configuration specification intent
  action: read,write            # type of permission to a specified object
  matchFields:                  # field names (e.g. kind of an object) to match 
    kind: network               # apply to objects with kind = network

`,
	`qos`: `# please edit and/or delete the lines in this sample
kind: qos                                               # kind of this object
meta:
  name: qos-critical-priority                           # unique name of this object
  # tenant: default		                        # override tenant value (applicable only for super user)
  labels:			                        # arbitrary user defined tags 'key: value' 
    class: elite
spec:                                                   # configuration specification intent
  applyToWorkloads:
  - matchlabels: io.pensando.network.prio:critical      # apply policy to workloads tagged with this label
  - matchNames: dev-app*                                # apply policy to workloads with this name/regex
    matchDestPorts: udp/4404                            # and match only traffic going to udp/4404 
  - matchDestPrefix: 172.15.0.0/16                      # match a set of destination prefixes
    matchDestPorts: tcp/4090                            # match flows on to specific ports in addition
  - matchSrcPrefix: 10.33.2.0/24                        # match a set of source IP address prefixes
  priority: critical                                    # define queuing/buffering priority: critical, high or best-effort
  setDscp: 7                                            # also set DSCP in IP header to '7' (value from 0-7)
`,
	`role`: `# please edit and/or delete the lines in this sample
kind: role                      # kind of this object
meta:
  name: dev-admin               # unique name of this object
  labels:			# arbitrary user defined tags 'key: value' 
    area: network
spec:                           # configuration specification intent
  permissions:                  # permissions this role is entitled to
  - network-rw
`,
	`service`: `# please edit and/or delete the lines in this sample
kind: service
meta:
  name: prod-web-svc                            # name of this object
  # tenant: default		                # override tenant value (applicable only for super user)
  labels:			                # arbitrary user defined tags 'key: value' 
    zone: dmz
    monitor: "yes"                              # yml convention requires bools to be quoted if value (i.e. string)
    policy: auto-compute
spec:                                           # configuration specification intent
  lbPolicy: basic-lb-policy                     # load balancing policy used by the service
  ports: tcp/80                                 # port(s) this service is served on
  virtualIp: 141.34.55.12                       # VIP of the service
  workloadLabels:                               # workloads that this service represents the backend
    io.pensando.network.svc: prod-web           # match all workloads that match a specific label

`,
	`sg`: `# please edit and/or delete the lines in this sample
kind: sg                                                # kind of this object
meta:
  name: dev-sg                                          # name of this object
  # tenant: default		                        # override tenant value (applicable only for super user)
  # labels:			                        # arbitrary user defined tags 'key: value' 
  #  area: network
spec:
  workloadLabels:                                       # workloads selection that are part of this group
    io.pensando.network.sg: dev-sg                      # match applications tagged as labels
  serviceLabels:                                        # match all services with specified labels
    io.pensando.network.sg: prod                        # match services tagged with specific label(s)
  matchPrefixs:                                         # match specified IP prefixes
  - 123.99.8.0/24                                       # match all workloads with ip address in the specified prefix

`,
	`sgpolicy`: `# please edit and/or delete the lines in this sample
kind: sgpolicy                          # kind of this object
meta:
  name: dev-sgp                         # name of this object
  # labels:			        # arbitrary user defined tags 'key: value' 
  #  area: network
spec:
  attachGroup: dev-sg                   # apply security policy to this group
  inRules:                              # apply a list of inbound rules
  - action: permit,log                  #  permit and log new incoming connections
    peerGroup: dev-sg                   #   only permit from the specified peer group
    ports: '*/*'                        #   allow all ports, yml requires '*' be specified in quotes
  - action: permit,log                  #  another rule that permits traffic
    peerGroup: '*'                      #   from any group
    ports: tcp/22,icmp,tcp/23           #   but ports '22' and '23'
  outRules:                             # apply a list of outbound rules
  - action: permit,log                  #  permit and log new incoming connections
    peerGroup: '*'                      #   only permit from the specified peer group
    ports: tcp/80,tcp/8080              #   allow all ports

`,
	`statscollectionpolicy`: `# please edit and/or delete the lines in this sample
kind: statscollectionpolicy     # kind of this object
meta:
  name: flow-collector          # name of this object
  labels:			# arbitrary user defined tags 'key: value' 
    source: bonzo-bot
spec:
  ActiveConns: false            # set of fields we need to collect
  NumLBConns: false
  NumSGRules: false
  RxBytes: false
  RxPackets: false
  TxBytes: false
  TxPackets: false

`,
	`statsexportpolicy`: `kind: statsexportpolicy
meta:
  name: tsdb-exporter
  tenant: default
spec:
  encap: udp
  exportInterval: 3s
  formatScheme: statsd
  targetIpAddress: 192.54.23.21
  targetPort: "66590"
Status: 

`,
	`tenant`: `# please edit and/or delete the lines in this sample
kind: tenant            # kind of this object
meta:
  name: default         # name of this object
  labels:	        # arbitrary user defined tags 'key: value' 
    tier: bronze
spec:
  adminUser: joe        # administrator (user name ) of this tenant

`,
	`user`: `# please edit and/or delete the lines in this sample
kind: user              # kind of this object
meta:
  name: joe             # unique name of this object
  # tenant: default	# override tenant value (applicable only for super user)
  labels:		# arbitrary user defined tags 'key: value' 
    dept: eng
    level: mts
spec:                   # configuration specification intent
  roles:                # roles this user has
  - dev-admin
`,
	`workload`: `# please edit and/or delete the lines in this sample
kind: workload                                  # kind of this object
meta:
  name: dev-app-1                               # name of this object
  # tenant: default		                # override tenant value (applicable only for super user)
  labels:			                # arbitrary user defined tags 'key: value' 
    io.pensando.network.prio: best-effort       # indicates the priority to be best-effort
    io.pensando.network.sg: dev-sg              # indicates security group for the network is dev-sg
    io.pensando.network.svc: dev-app            # this workload is backend of service 'dev-app'
spec:                                           # configuration specification intent
  dns: auto                                     # automatically publish dns information against the app
  network: dev-network                          # workload's network association
  node: node4                                   # node on which workload is scheduled

`,
}
