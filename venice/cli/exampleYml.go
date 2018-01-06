package vcli

var exampleYmls = map[string]string{
	`cluster`: `kind: cluster                         # kind of this object
meta:
  name: lab22-test-cluster            # unique name of this object
  labels:                             # arbitrary list of user defined tags 'key: value' 
    allowed: qa,prod,dev
    dc: sjc-lab22
    type: vcenter
spec:                                 # user configuration (desired intent)
  quorum-nodes:                       # quorum nodes implement leadership election and state store
  - mgmt1
  - mgmt2
  - mgmt3
  virtual-ip: 10.1.1.1                # virtual IP on which cluster API is accessible
  ntp-servers:                        # list of NTP servers IP address for clock synchronization
  - 12.1.1.3
  - 12.1.1.54
  dns-subdomain: internal.myco.com    # dns addressable subdomain for the cluster
  auto-admit-nics: true               # automatically admin NICs as they are discovered

`,
	`endpoint`: `# please edit and/or delete the lines in this sample
kind: endpoint                                  # kind of this object
meta:
  name: dev-app-1                               # name of this object
  # tenant: default		                # override tenant value (applicable only for super user)
  labels:			                # arbitrary user defined tags 'key: value' 
    io.pensando.network.prio: best-effort       # arbitrary label to identify traffic priority
    io.pensando.network.sg: dev-sg              # arbitrary label to allow security group selection for the workload
    io.pensando.network.svc: dev-app            # arbitrary label to allow 'service' to select the endpoint
`,
	`lbpolicy`: `kind: lbPolicy 			# kind of this object
meta:
  name: basic-lb-policy 	# name of this object
  # tenant: default		# override tenant value (applicable only for super admin user)
spec:				# configuration specification intent
  type: l4			# type of load balancing 'l4', or 'l7'
  algorithm: round-robin	# load balancing algorithm, can be 'round-robin', 'random', or 'latency-based'
  session-affinity: true	# 'true' or 'false' indicating if affinity to the backend must be maintained
  health-check:		        # health check parameters for a given backend
    interval: 30                # time interval within which to send a probe
    max-timeouts: 3             # maximum number of timeouts before declaing a backend dead
    probes-per-interval: 1      # how many probes are sent within healthCheck interval
    probe-port-or-url: /ping    # REST endpoint or protocol/port to check backend health
    declare-healthy-count: 3    # minimum number of healthy probes before backend is declared back to be healthy

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
    match-labels: io.pensando.network.sg:dmz-web-sg     # match applications tagged as labels defined here
    match-names: dev-app*                               # match names of the workloads
    match-dest-ports: udp/4404                          # match only traffic going to udp/4404
    match-dest-prefix: 172.15.0.0/16                    # match a set of destination prefixes
    match-src-ports: tcp/4090                           # match flows on to specific ports in addition
    match-src-prefix: 10.33.2.0/24                      # match a set of source IP address prefixes
  destinationIp: 192.131.4.54                           # destination IP address to which mirroed packets are to be sent to
  encap: erspan                                         # erspan encapsulate frames, other options are netflow, tsdb

`,
	`monitoringpolicy`: `kind: monitoringpolicy                                  # kind of this object
meta:
  name: flow-monitoring-policy                          # name of this object
  # tenant: default		                        # override tenant value (applicable only for super user)
spec:
  area: 
  object-selector:                                      # selection (ORed list) of workloads to which this policy is applied
  - match-labels: io.pensando.network.sg:dmz-web-sg     # match applications tagged as labels defined here
  - match-names: prod*                                  # match names of the workloads
    match-dest-port: udp/4404                           # match only traffic going to udp/4404
  - match-dest-prefix: 172.15.0.0/16                    # match a set of destination prefixes
    match-src-port: tcp/4090                            # match flows on to specific ports in addition
  - match-src-prefix: 10.33.2.0/24                      # match a set of source IP address prefixes
  export-policy: external-flow-exporter                 # export parameters for selected metrics
  retention-policy: local-retention-policy              # retention parameters describes for how long to keep the metrics

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
  match-fields:                 # field names (e.g. kind of an object) to match 
    kind: network               # apply to objects with kind = network

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
	`securityGroup`: `# please edit and/or delete the lines in this sample
kind: securityGroup                                     # kind of this object
meta:
  name: dev-sg                                          # name of this object
  # tenant: default		                        # override tenant value (applicable only for super user)
  # labels:			                        # arbitrary user defined tags 'key: value' 
  #  tier: db
spec:
  workload-labels:                                      # workloads selection that are part of this group
  - "io.pensando.network.sg: dev-sg"                    # match applications tagged as labels
  service-labels:                                       # match all services with specified labels
  - "io.pensando.network.sg: prod"                      # match services tagged with specific label(s)
  match-prefixes:                                       # match specified IP prefixes
  - 123.99.8.0/24                                       # match all workloads with ip address in the specified prefix

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
  lb-policy: basic-lb-policy                    # load balancing policy used by the service
  ports: tcp/80                                 # port(s) this service is served on
  virtual-ip: 141.34.55.12                      # VIP of the service
  workload-labels:                              # workloads that this service represents the backend
  - "io.pensando.network.svc: prod-web"         # match all workloads that match a specific label

`,
	`sgpolicy`: `# please edit and/or delete the lines in this sample
kind: sgpolicy                          # kind of this object
meta:
  name: dev-sgp                         # name of this object
  # labels:			        # arbitrary user defined tags 'key: value' 
  #  area: network
spec:
  attach-group: dev-sg                  # apply security policy to this group
  in-rules:                             # apply a list of inbound rules
  - action: permit,log                  #  permit and log new incoming connections
    peer-group: dev-sg                  #   only permit from the specified peer group
    ports: '*/*'                        #   allow all ports, yml requires '*' be specified in quotes
  - action: permit,log                  #  another rule that permits traffic
    peer-group: '*'                     #   from any group
    ports: tcp/22,icmp,tcp/23           #   but ports '22' and '23'
  out-rules:                            # apply a list of outbound rules
  - action: permit,log                  #  permit and log new incoming connections
    peer-group: '*'                     #   only permit from the specified peer group
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
  export-interval: 3s
  format-scheme: statsd
  target-ip-address: 192.54.23.21
  target-port: 66590

`,
	`tenant`: `# please edit and/or delete the lines in this sample
kind: tenant            # kind of this object
meta:
  name: default         # name of this object
  labels:	        # arbitrary user defined tags 'key: value' 
    tier: bronze
spec:
  admin-user: joe        # administrator (user name ) of this tenant

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
}
