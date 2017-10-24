package main

import (
	"testing"
)

func TestPercentParsing(t *testing.T) {
	rec := findPassFail([]byte(verboseRun), &CoverRec{})
	if rec.Passed != 51 || rec.Failed != 1 {
		t.Fatalf("unable to parse and tests pass/fail run: %+v, expecting Passed 3 Failed 1%", rec)
	}
}

func TestNumTestsParsing(t *testing.T) {
	rec := findPercent([]byte(coverageRun), &CoverRec{})

	if rec.Percent != 75 {
		t.Fatalf("unable to parse and average percentage: got %+v, expecting 75%", rec)
	}
}

var coverageRun = `
ok  	github.com/pensando/sw/venice/cli	0.885s	coverage: 76.0% of statements
?   	github.com/pensando/sw/venice/cli/api	0.680s  coverage: 74.0% of statements
?   	github.com/pensando/sw/venice/cli/gen	[no test files]
?   	github.com/pensando/sw/venice/cli/gen/pregen	[no test files]
?   	github.com/pensando/sw/venice/cli/testserver	[no test files]
?   	github.com/pensando/sw/venice/cli/testserver/tserver	[no test files]
`

var verboseRun = `=== RUN   TestStartServer
time="2017-10-20T14:27:09-07:00" level=info msg="Starting http server at :30748" 
[martini] listening on :30748 (development)
[martini] Started POST /v1/cmd/cluster for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/cluster/9e3aad04-daec-45e0-96f8-5210de706049 object {TypeMeta:{Kind:cluster APIVersion:} ObjectMeta:{Name:dc-az-cluster1 Tenant: Namespace: ResourceVersion: UUID:9e3aad04-daec-45e0-96f8-5210de706049 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{QuorumNodes:[] VirtualIP: NTPServers:[] DNSSubDomain: AutoAdmitNICs:false} Status:{Leader:}}" 
[martini] Completed 200 OK in 210.492µs
--- PASS: TestStartServer (0.00s)
=== RUN   TestClusterCreate
[martini] Started GET /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
[martini] Completed 200 OK in 1.286525ms
[martini] Started PUT /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/cluster/9e3aad04-daec-45e0-96f8-5210de706049 object {TypeMeta:{Kind:cluster APIVersion:} ObjectMeta:{Name:dc-az-cluster1 Tenant:default Namespace: ResourceVersion:1 UUID:9e3aad04-daec-45e0-96f8-5210de706049 Labels:map[allowed:qa dc:sjc-lab22 type:vcenter] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{QuorumNodes:[mgmt1 mgmt2 mgmt3] VirtualIP:158.21.34.44 NTPServers:[ntp-svr1 ntp-svr2] DNSSubDomain:internal.pensando.io AutoAdmitNICs:false} Status:{Leader:}}" 
[martini] Completed 200 OK in 124.167µs
[martini] Started GET /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
[martini] Completed 200 OK in 974.814µs
[martini] Started GET /v1/cmd/cluster for [::1]:52738
[martini] Completed 200 OK in 902.134µs
--- PASS: TestClusterCreate (0.01s)
=== RUN   TestClusterUpdate
[martini] Started GET /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
[martini] Completed 200 OK in 983.993µs
[martini] Started PUT /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/cluster/9e3aad04-daec-45e0-96f8-5210de706049 object {TypeMeta:{Kind:cluster APIVersion:} ObjectMeta:{Name:dc-az-cluster1 Tenant:default Namespace: ResourceVersion:2 UUID:9e3aad04-daec-45e0-96f8-5210de706049 Labels:map[allowed:qa dc:sjc-lab22 type:vcenter] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{QuorumNodes:[mgmt1 mgmt2 mgmt3] VirtualIP:151.21.43.44 NTPServers:[ntp-svr1 ntp-svr2] DNSSubDomain:internal.pensando.io AutoAdmitNICs:false} Status:{Leader:}}" 
[martini] Completed 200 OK in 117.384µs
[martini] Started GET /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
[martini] Completed 200 OK in 716.9µs
--- PASS: TestClusterUpdate (0.00s)
=== RUN   TestSmartNICCreate
[martini] Started POST /v1/cmd/smartnics for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/smartnics/11c31c11-3f5a-454a-8a25-cb2c5c6d3d94 object {TypeMeta:{Kind:smartnic APIVersion:} ObjectMeta:{Name:naples1 Tenant: Namespace: ResourceVersion: UUID:11c31c11-3f5a-454a-8a25-cb2c5c6d3d94 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Phase: Ports:[]} Status:{Conditions:[] SerialNum: PrimaryMacAddress:0029.ef38.8a01 NodeName:vm221 Ports:[]}}" 
[martini] Completed 200 OK in 293.274µs
[martini] Started POST /v1/cmd/smartnics for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/smartnics/e6eb68df-88da-4de8-9a76-b904aa7e79df object {TypeMeta:{Kind:smartnic APIVersion:} ObjectMeta:{Name:naples2 Tenant: Namespace: ResourceVersion: UUID:e6eb68df-88da-4de8-9a76-b904aa7e79df Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Phase: Ports:[]} Status:{Conditions:[] SerialNum: PrimaryMacAddress:0029.ef38.8a02 NodeName:vm222 Ports:[]}}" 
[martini] Completed 200 OK in 137.534µs
[martini] Started POST /v1/cmd/smartnics for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/smartnics/2116787c-5ee3-4cbf-a7fa-84e923c64a53 object {TypeMeta:{Kind:smartnic APIVersion:} ObjectMeta:{Name:naples3 Tenant: Namespace: ResourceVersion: UUID:2116787c-5ee3-4cbf-a7fa-84e923c64a53 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Phase: Ports:[]} Status:{Conditions:[] SerialNum: PrimaryMacAddress:0029.ef38.8a03 NodeName:vm223 Ports:[]}}" 
[martini] Completed 200 OK in 87.019µs
--- PASS: TestSmartNICCreate (0.00s)
=== RUN   TestNodeCreate
[martini] Started POST /v1/cmd/nodes for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/nodes/b4768cd8-1d46-4886-9ff0-e4c9f1f0458c object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:vm233 Tenant:default Namespace: ResourceVersion: UUID:b4768cd8-1d46-4886-9ff0-e4c9f1f0458c Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[QUORUM WORKLOAD]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 109.873µs
[martini] Started GET /v1/cmd/nodes/vm233 for [::1]:52738
[martini] Completed 200 OK in 759.808µs
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 733.948µs
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 415.68µs
--- PASS: TestNodeCreate (0.01s)
=== RUN   TestNodeUpdate
[martini] Started GET /v1/cmd/nodes/vm233 for [::1]:52738
[martini] Completed 200 OK in 556.574µs
[martini] Started PUT /v1/cmd/nodes/vm233 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/nodes/b4768cd8-1d46-4886-9ff0-e4c9f1f0458c object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:vm233 Tenant:default Namespace: ResourceVersion:1 UUID:b4768cd8-1d46-4886-9ff0-e4c9f1f0458c Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[CONTROLLER]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 143.704µs
[martini] Started GET /v1/cmd/nodes/vm233 for [::1]:52738
[martini] Completed 200 OK in 564.085µs
--- PASS: TestNodeUpdate (0.00s)
=== RUN   TestSmartNICUpdate
--- PASS: TestSmartNICUpdate (0.00s)
=== RUN   TestTenantCreate
[martini] Started POST /v1/tenants/tenants for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/tenants/tenants/368c3933-ba18-4f81-987d-13d58a9368e8 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:newco Tenant:default Namespace: ResourceVersion: UUID:368c3933-ba18-4f81-987d-13d58a9368e8 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:newco-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 198.253µs
[martini] Started GET /v1/tenants/tenants/newco for [::1]:52738
[martini] Completed 200 OK in 849.858µs
[martini] Started GET /v1/tenants/tenants for [::1]:52738
[martini] Completed 200 OK in 752.86µs
--- PASS: TestTenantCreate (0.00s)
=== RUN   TestTenantUpdate
[martini] Started GET /v1/tenants/tenants/newco for [::1]:52738
[martini] Completed 200 OK in 528.884µs
[martini] Started PUT /v1/tenants/tenants/newco for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/tenants/tenants/368c3933-ba18-4f81-987d-13d58a9368e8 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:newco Tenant:default Namespace: ResourceVersion:1 UUID:368c3933-ba18-4f81-987d-13d58a9368e8 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:newco-adm} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 976.665µs
[martini] Started GET /v1/tenants/tenants/newco for [::1]:52738
[martini] Completed 200 OK in 531.681µs
--- PASS: TestTenantUpdate (0.00s)
=== RUN   TestNetworkCreate
[martini] Started POST /v1/networks/default/networks for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/fb276e0c-3989-4802-9b82-8cc14dc82db9 object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:lab22-net145 Tenant:default Namespace: ResourceVersion: UUID:fb276e0c-3989-4802-9b82-8cc14dc82db9 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:145.1.1.0/24 IPv4Gateway:145.1.1.254 IPv6Subnet:2001:db8::0/64 IPv6Gateway:2001:db8::1 VlanID:345 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 138.354µs
[martini] Started GET /v1/networks/default/networks/lab22-net145 for [::1]:52738
[martini] Completed 200 OK in 652.256µs
[martini] Started GET /v1/networks/default/networks for [::1]:52738
[martini] Completed 200 OK in 561.819µs
--- PASS: TestNetworkCreate (0.00s)
=== RUN   TestNetworkUpdate
[martini] Started GET /v1/networks/default/networks/lab22-net145 for [::1]:52738
[martini] Completed 200 OK in 540.62µs
[martini] Started PUT /v1/networks/default/networks/lab22-net145 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/fb276e0c-3989-4802-9b82-8cc14dc82db9 object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:lab22-net145 Tenant:default Namespace: ResourceVersion:1 UUID:fb276e0c-3989-4802-9b82-8cc14dc82db9 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vxlan IPv4Subnet:145.1.1.0/24 IPv4Gateway:145.1.1.254 IPv6Subnet:2001:db8::0/64 IPv6Gateway:2001:db8::1 VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 91.314µs
[martini] Started GET /v1/networks/default/networks/lab22-net145 for [::1]:52738
[martini] Completed 200 OK in 527.591µs
--- PASS: TestNetworkUpdate (0.00s)
=== RUN   TestSecurityGroupCreate
[martini] Started POST /v1/security-groups/default/security-groups for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/a8e6f087-b186-48c4-8026-b1898b83ebc4 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:sg10 Tenant:default Namespace: ResourceVersion: UUID:a8e6f087-b186-48c4-8026-b1898b83ebc4 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[key1:val1 key2:val2] ServiceSelector:[] MatchPrefixes:[12.1.1.0/22]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 112.298µs
[martini] Started GET /v1/security-groups/default/security-groups/sg10 for [::1]:52738
[martini] Completed 200 OK in 552.282µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 466.303µs
--- PASS: TestSecurityGroupCreate (0.00s)
=== RUN   TestSecurityGroupUpdate
[martini] Started GET /v1/security-groups/default/security-groups/sg10 for [::1]:52738
[martini] Completed 200 OK in 471.171µs
[martini] Started PUT /v1/security-groups/default/security-groups/sg10 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/a8e6f087-b186-48c4-8026-b1898b83ebc4 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:sg10 Tenant:default Namespace: ResourceVersion:1 UUID:a8e6f087-b186-48c4-8026-b1898b83ebc4 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[key3:val3] ServiceSelector:[] MatchPrefixes:[12.1.1.0/22]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 178.201µs
[martini] Started GET /v1/security-groups/default/security-groups/sg10 for [::1]:52738
[martini] Completed 200 OK in 371.969µs
--- PASS: TestSecurityGroupUpdate (0.00s)
=== RUN   TestSecurityGroupPolicyCreate
[martini] Started POST /v1/security-groups/default/security-groups for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/f5ab17d1-192d-410d-ad19-4467b855b4f5 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:sg20 Tenant:default Namespace: ResourceVersion: UUID:f5ab17d1-192d-410d-ad19-4467b855b4f5 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[key5:val5] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 158.093µs
[martini] Started POST /v1/sgpolicy/default/sgpolicy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/8a95a622-a433-4e4b-9ac2-136d0e50f8ee object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:sg10-ingress Tenant:default Namespace: ResourceVersion: UUID:8a95a622-a433-4e4b-9ac2-136d0e50f8ee Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg10] InRules:[{Ports:tcp/8440 Action:permit PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 194.481µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 200 OK in 371.593µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy for [::1]:52738
[martini] Completed 200 OK in 509.268µs
--- PASS: TestSecurityGroupPolicyCreate (0.00s)
=== RUN   TestSecurityGroupPolicyUpdate
[martini] Started GET /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 200 OK in 774.934µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/8a95a622-a433-4e4b-9ac2-136d0e50f8ee object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:sg10-ingress Tenant:default Namespace: ResourceVersion:1 UUID:8a95a622-a433-4e4b-9ac2-136d0e50f8ee Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg30] InRules:[{Ports:tcp/8440 Action:permit PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 132.557µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 200 OK in 628.338µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 200 OK in 566.339µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/8a95a622-a433-4e4b-9ac2-136d0e50f8ee object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:sg10-ingress Tenant:default Namespace: ResourceVersion:2 UUID:8a95a622-a433-4e4b-9ac2-136d0e50f8ee Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg30] InRules:[{Ports:tcp/4550 Action:permit PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:} {Ports:tcp/8440 Action:permit PeerGroup:sg10 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 115.903µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 200 OK in 528.702µs
--- PASS: TestSecurityGroupPolicyUpdate (0.00s)
=== RUN   TestServiceCreate
[martini] Started POST /v1/services/default/services for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/04edd0be-db6d-4fc8-ae48-ba7b99a2de70 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:uService122 Tenant:default Namespace: ResourceVersion: UUID:04edd0be-db6d-4fc8-ae48-ba7b99a2de70 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] VirtualIp:12.1.1.122 Ports:8080 LBPolicy:dev-lb-policy} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 144.69µs
[martini] Started GET /v1/services/default/services/uService122 for [::1]:52738
[martini] Completed 200 OK in 566.049µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 540.314µs
--- PASS: TestServiceCreate (0.01s)
=== RUN   TestServiceUpdate
[martini] Started GET /v1/services/default/services/uService122 for [::1]:52738
[martini] Completed 200 OK in 580.802µs
[martini] Started PUT /v1/services/default/services/uService122 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/04edd0be-db6d-4fc8-ae48-ba7b99a2de70 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:uService122 Tenant:default Namespace: ResourceVersion:1 UUID:04edd0be-db6d-4fc8-ae48-ba7b99a2de70 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] VirtualIp:12.1.1.123 Ports:8080 LBPolicy:prod-lb-policy} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 156.713µs
[martini] Started GET /v1/services/default/services/uService122 for [::1]:52738
[martini] Completed 200 OK in 494.957µs
--- PASS: TestServiceUpdate (0.00s)
=== RUN   TestLbPolicyCreate
[martini] Started POST /v1/lb-policy/default/lb-policy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/5091f77b-68c2-4a4a-8309-db514d765974 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:dev-lb-policy Tenant:default Namespace: ResourceVersion: UUID:5091f77b-68c2-4a4a-8309-db514d765974 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:l4 Algorithm:round-robin SessionAffinity:yes HealthCheck:Interval:5 ProbePortOrUrl:\"/healthStatus\" MaxTimeouts:25 } Status:{Services:[]}}" 
[martini] Completed 200 OK in 172.053µs
[martini] Started GET /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
[martini] Completed 200 OK in 660.119µs
[martini] Started GET /v1/lb-policy/default/lb-policy for [::1]:52738
[martini] Completed 200 OK in 615.204µs
--- PASS: TestLbPolicyCreate (0.00s)
=== RUN   TestLbPolicyUpdate
[martini] Started GET /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
[martini] Completed 200 OK in 557.007µs
[martini] Started PUT /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/5091f77b-68c2-4a4a-8309-db514d765974 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:dev-lb-policy Tenant:default Namespace: ResourceVersion:1 UUID:5091f77b-68c2-4a4a-8309-db514d765974 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:l4 Algorithm:least-latency SessionAffinity:yes HealthCheck:Interval:5 ProbePortOrUrl:\"/healthStatus\" MaxTimeouts:25 } Status:{Services:[]}}" 
[martini] Completed 200 OK in 162.798µs
[martini] Started GET /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
[martini] Completed 200 OK in 608.165µs
--- PASS: TestLbPolicyUpdate (0.00s)
=== RUN   TestEndpointCreate
[martini] Started POST /v1/endpoints/default/endpoints for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/e04b212b-8b63-4125-b770-ff6bdba0436a object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:vm23 Tenant:default Namespace: ResourceVersion: UUID:e04b212b-8b63-4125-b770-ff6bdba0436a Labels:map[dc:lab-22 dmz:yes tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 179.736µs
[martini] Started GET /v1/endpoints/default/endpoints/vm23 for [::1]:52738
[martini] Completed 200 OK in 630.123µs
[martini] Started GET /v1/endpoints/default/endpoints/vm23 for [::1]:52738
[martini] Completed 200 OK in 428.459µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 411.575µs
--- PASS: TestEndpointCreate (0.00s)
=== RUN   TestEndpointUpdate
[martini] Started GET /v1/endpoints/default/endpoints/vm23 for [::1]:52738
[martini] Completed 200 OK in 331.516µs
[martini] Started PUT /v1/endpoints/default/endpoints/vm23 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/e04b212b-8b63-4125-b770-ff6bdba0436a object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:vm23 Tenant:default Namespace: ResourceVersion:1 UUID:e04b212b-8b63-4125-b770-ff6bdba0436a Labels:map[dc:lab-22 dmz:no tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 125.076µs
[martini] Started GET /v1/endpoints/default/endpoints/vm23 for [::1]:52738
[martini] Completed 200 OK in 324.166µs
--- PASS: TestEndpointUpdate (0.00s)
=== RUN   TestUserCreate
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /user/1f3b58eb-26ea-4da3-99ae-33973415868d object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:bot-bot Tenant:default Namespace: ResourceVersion: UUID:1f3b58eb-26ea-4da3-99ae-33973415868d Labels:map[bot:yes dept:qa] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[network-admin security-admin]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 161.03µs
[martini] Started GET /user/bot-bot for [::1]:52738
[martini] Completed 200 OK in 423.224µs
[martini] Started GET /user for [::1]:52738
[martini] Completed 200 OK in 438.133µs
--- PASS: TestUserCreate (0.00s)
=== RUN   TestUserUpdate
[martini] Started GET /user/bot-bot for [::1]:52738
[martini] Completed 200 OK in 342.238µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /user/1f3b58eb-26ea-4da3-99ae-33973415868d object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:bot-bot Tenant:default Namespace: ResourceVersion:1 UUID:1f3b58eb-26ea-4da3-99ae-33973415868d Labels:map[bot:yes dept:qa] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[full-access]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 116.311µs
[martini] Started GET /user/bot-bot for [::1]:52738
[martini] Completed 200 OK in 364.579µs
--- PASS: TestUserUpdate (0.00s)
=== RUN   TestRoleCreate
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /role/f4c4b2c0-a104-4946-b72f-d83f504603ec object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:network-admin Tenant:default Namespace: ResourceVersion: UUID:f4c4b2c0-a104-4946-b72f-d83f504603ec Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[network-objs-rw policy-objs-rw]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 103.185µs
[martini] Started GET /role/network-admin for [::1]:52738
[martini] Completed 200 OK in 317.012µs
[martini] Started GET /role for [::1]:52738
[martini] Completed 200 OK in 394.439µs
--- PASS: TestRoleCreate (0.00s)
=== RUN   TestRoleUpdate
[martini] Started GET /role/network-admin for [::1]:52738
[martini] Completed 200 OK in 946.151µs
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /role/f4c4b2c0-a104-4946-b72f-d83f504603ec object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:network-admin Tenant:default Namespace: ResourceVersion:1 UUID:f4c4b2c0-a104-4946-b72f-d83f504603ec Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[security-objs-rw]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 117.859µs
[martini] Started GET /role/network-admin for [::1]:52738
[martini] Completed 200 OK in 650.863µs
--- PASS: TestRoleUpdate (0.01s)
=== RUN   TestPermissionCreate
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /permission/9a742012-5598-486e-8678-dda4ca40f9cd object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:network-objs-rw Tenant:default Namespace: ResourceVersion: UUID:9a742012-5598-486e-8678-dda4ca40f9cd Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action:rw ObjectSelector:map[sgpolicy:*] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 121.586µs
[martini] Started GET /permission/network-objs-rw for [::1]:52738
[martini] Completed 200 OK in 710.936µs
[martini] Started GET /permission for [::1]:52738
[martini] Completed 200 OK in 808.083µs
--- PASS: TestPermissionCreate (0.00s)
=== RUN   TestPermissionUpdate
[martini] Started GET /permission/network-objs-rw for [::1]:52738
[martini] Completed 200 OK in 580.592µs
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /permission/9a742012-5598-486e-8678-dda4ca40f9cd object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:network-objs-rw Tenant:default Namespace: ResourceVersion:1 UUID:9a742012-5598-486e-8678-dda4ca40f9cd Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action:rw ObjectSelector:map[network:*] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 169.818µs
[martini] Started GET /permission/network-objs-rw for [::1]:52738
[martini] Completed 200 OK in 375.039µs
--- PASS: TestPermissionUpdate (0.00s)
=== RUN   TestCommandCompletion
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 636.407µs
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 572.83µs
[martini] Started GET /v1/cmd/smartnics for [::1]:52738
[martini] Completed 200 OK in 506.904µs
[martini] Started GET /v1/cmd/smartnics for [::1]:52738
[martini] Completed 200 OK in 365.09µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 336.685µs
[martini] Started GET /v1/tenants/tenants for [::1]:52738
[martini] Completed 200 OK in 330.722µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 356.583µs
[martini] Started GET /user for [::1]:52738
[martini] Completed 200 OK in 426.37µs
[martini] Started GET /user for [::1]:52738
[martini] Completed 200 OK in 621.733µs
[martini] Started GET /role for [::1]:52738
[martini] Completed 200 OK in 558.296µs
[martini] Started GET /role for [::1]:52738
[martini] Completed 200 OK in 376.939µs
[martini] Started GET /permission for [::1]:52738
[martini] Completed 200 OK in 516.981µs
[martini] Started GET /permission for [::1]:52738
[martini] Completed 200 OK in 358.269µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 361.156µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 344.235µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy for [::1]:52738
[martini] Completed 200 OK in 287.238µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy for [::1]:52738
[martini] Completed 200 OK in 359.708µs
[martini] Started GET /v1/lb-policy/default/lb-policy for [::1]:52738
[martini] Completed 200 OK in 356.239µs
[martini] Started GET /v1/lb-policy/default/lb-policy for [::1]:52738
[martini] Completed 200 OK in 334.364µs
--- PASS: TestCommandCompletion (0.02s)
=== RUN   TestCreateFromFile
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/e94e913a-4e20-4d3d-888b-7b91fca39442 object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:test-sgpolicy Tenant:default Namespace: ResourceVersion: UUID:e94e913a-4e20-4d3d-888b-7b91fca39442 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg10] InRules:[{Ports:tcp/8440 Action:permit PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 204.681µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
[martini] Completed 200 OK in 543.362µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/e94e913a-4e20-4d3d-888b-7b91fca39442 object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:test-sgpolicy Tenant:default Namespace: ResourceVersion: UUID:e94e913a-4e20-4d3d-888b-7b91fca39442 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg10] InRules:[{Ports:tcp/8440 Action:permit,log PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 103.943µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
[martini] Completed 200 OK in 767.189µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/e94e913a-4e20-4d3d-888b-7b91fca39442 object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:test-sgpolicy Tenant:default Namespace: ResourceVersion: UUID:e94e913a-4e20-4d3d-888b-7b91fca39442 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg30] InRules:[{Ports:tcp/8440 Action:permit,log PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 233.13µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
[martini] Completed 200 OK in 598.17µs
[martini] Started POST /v1/networks/default/networks for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/85f5ecf4-c0af-4fe2-971e-70f22426662d object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:uploaded Tenant:default Namespace: ResourceVersion: UUID:85f5ecf4-c0af-4fe2-971e-70f22426662d Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: IPv4Subnet: IPv4Gateway: IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 127.701µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 742.245µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 532.425µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 574.852µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 381.446µs
[martini] Started PUT /v1/networks/default/networks/uploaded for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/85f5ecf4-c0af-4fe2-971e-70f22426662d object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:uploaded Tenant:default Namespace: ResourceVersion:1 UUID:85f5ecf4-c0af-4fe2-971e-70f22426662d Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: IPv4Subnet: IPv4Gateway: IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 93.207µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 333.995µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 257.138µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 350.413µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 318.221µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 262.035µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 327.83µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 252.013µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 315.728µs
[martini] Started GET /v1/networks/:tenant/networks/uploaded for [::1]:52738
[martini] Completed 200 OK in 330.279µs
--- PASS: TestCreateFromFile (0.02s)
=== RUN   TestEditCommand
[martini] Started POST /v1/security-groups/default/security-groups for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/7c61fd40-d51f-4b28-a2f2-99600ad37830 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:editsg Tenant:default Namespace: ResourceVersion: UUID:7c61fd40-d51f-4b28-a2f2-99600ad37830 Labels:map[one:two three:four] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 151.026µs
[martini] Started GET /v1/security-groups/default/security-groups/editsg for [::1]:52738
[martini] Completed 200 OK in 351.33µs
[martini] Started PUT /v1/security-groups/default/security-groups/editsg for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/7c61fd40-d51f-4b28-a2f2-99600ad37830 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:editsg Tenant:default Namespace: ResourceVersion:1 UUID:7c61fd40-d51f-4b28-a2f2-99600ad37830 Labels:map[one:two three:four] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 118.299µs
[martini] Started GET /v1/security-groups/default/security-groups/editsg for [::1]:52738
[martini] Completed 200 OK in 356.087µs
[martini] Started PUT /v1/security-groups/default/security-groups/editsg for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/7c61fd40-d51f-4b28-a2f2-99600ad37830 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:editsg Tenant:default Namespace: ResourceVersion:2 UUID:7c61fd40-d51f-4b28-a2f2-99600ad37830 Labels:map[five:six one:two three:four] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 120.005µs
[martini] Started GET /v1/security-groups/default/security-groups/editsg for [::1]:52738
[martini] Completed 200 OK in 386.17µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /user/f2a76970-5d93-4744-b25a-76985dcf1609 object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:joe Tenant:default Namespace: ResourceVersion: UUID:f2a76970-5d93-4744-b25a-76985dcf1609 Labels:map[dept:eng level:mts] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 106.386µs
[martini] Started GET /user/joe for [::1]:52738
[martini] Completed 200 OK in 449.09µs
--- PASS: TestEditCommand (0.01s)
=== RUN   TestTree
[martini] Started GET /v1/cmd/cluster for [::1]:52738
[martini] Completed 200 OK in 375.962µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 346.478µs
[martini] Started GET /v1/lb-policy/default/lb-policy for [::1]:52738
[martini] Completed 200 OK in 340.94µs
[martini] Started GET /v1/networks/default/networks for [::1]:52738
[martini] Completed 200 OK in 337.974µs
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 505.919µs
[martini] Started GET /permission for [::1]:52738
[martini] Completed 200 OK in 389.117µs
[martini] Started GET /role for [::1]:52738
[martini] Completed 200 OK in 348.074µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 396.57µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 387.784µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy for [::1]:52738
[martini] Completed 200 OK in 353.876µs
[martini] Started GET /v1/cmd/smartnics for [::1]:52738
[martini] Completed 200 OK in 370.158µs
[martini] Started GET /v1/tenants/tenants for [::1]:52738
[martini] Completed 200 OK in 539.676µs
[martini] Started GET /user for [::1]:52738
[martini] Completed 200 OK in 695.078µs
--- PASS: TestTree (0.01s)
=== RUN   TestExample
--- PASS: TestExample (0.00s)
=== RUN   TestList
[martini] Started POST /v1/cmd/nodes for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/nodes/4e2eee49-6eb0-4e4f-af1b-d13fdcaba3e0 object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:4e2eee49-6eb0-4e4f-af1b-d13fdcaba3e0 Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[QUORUM]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 121.696µs
[martini] Started POST /v1/cmd/nodes for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/nodes/e92342b6-c521-4889-a0e5-48f92b0e226d object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:e92342b6-c521-4889-a0e5-48f92b0e226d Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[WORKLOAD]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 94.497µs
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 630.928µs
[martini] Started POST /v1/tenants/tenants for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/tenants/tenants/9f87436e-6067-4b6a-a857-c1b8830db7bb object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:9f87436e-6067-4b6a-a857-c1b8830db7bb Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:test1-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 127.129µs
[martini] Started POST /v1/tenants/tenants for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/tenants/tenants/372514a7-f2cf-4e8a-ae1f-2fb1b3d59c11 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:372514a7-f2cf-4e8a-ae1f-2fb1b3d59c11 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:test2-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 140.322µs
[martini] Started GET /v1/tenants/tenants for [::1]:52738
[martini] Completed 200 OK in 628.279µs
[martini] Started POST /v1/networks/default/networks for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/f2631ca4-d60c-4150-86cc-f0cd7d5ecd1f object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:f2631ca4-d60c-4150-86cc-f0cd7d5ecd1f Labels:map[dhcp:yes] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:13.1.1.0/24 IPv4Gateway:13.1.1.254 IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 85.949µs
[martini] Started POST /v1/networks/default/networks for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/22f0b3f1-c729-4d52-8263-e11d64f7f7ef object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:22f0b3f1-c729-4d52-8263-e11d64f7f7ef Labels:map[dhcp:no level:prod] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:13.1.2.0/24 IPv4Gateway:13.1.2.254 IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 184.827µs
[martini] Started POST /v1/networks/default/networks for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/85d1c7bf-b366-4423-802e-0d00bcdfe977 object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:test3 Tenant:default Namespace: ResourceVersion: UUID:85d1c7bf-b366-4423-802e-0d00bcdfe977 Labels:map[dhcp:no] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:13.1.3.0/24 IPv4Gateway:13.1.3.254 IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 85.614µs
[martini] Started GET /v1/networks/default/networks for [::1]:52738
[martini] Completed 200 OK in 479.542µs
--- PASS: TestList (0.01s)
=== RUN   TestDeleteList
[martini] Started POST /v1/security-groups/default/security-groups for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/e95acef5-c9cb-4666-b512-239bf61dd568 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:e95acef5-c9cb-4666-b512-239bf61dd568 Labels:map[key1:label1 key2:label2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 164.781µs
[martini] Started POST /v1/security-groups/default/security-groups for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/9d385258-994c-4749-a1d5-6da0a7556fc3 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:9d385258-994c-4749-a1d5-6da0a7556fc3 Labels:map[key3:label3 key2:label2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:backend] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 164.469µs
[martini] Started POST /v1/security-groups/default/security-groups for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/d42481d6-f37a-4e4f-b7ee-53d4dd1ad8e8 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:test3 Tenant:default Namespace: ResourceVersion: UUID:d42481d6-f37a-4e4f-b7ee-53d4dd1ad8e8 Labels:map[key1:label1 key4:label4] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:app] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 101.381µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 712.553µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 886.641µs
[martini] Started DELETE /v1/security-groups/default/security-groups/test2 for [::1]:52738
[martini] Completed 200 OK in 202.608µs
[martini] Started DELETE /v1/security-groups/default/security-groups/test1 for [::1]:52738
[martini] Completed 200 OK in 81.959µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 818.389µs
--- PASS: TestDeleteList (0.01s)
=== RUN   TestMultiRead
[martini] Started GET /v1/cmd/smartnics for [::1]:52738
[martini] Completed 200 OK in 1.229465ms
[martini] Started POST /v1/endpoints/default/endpoints for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/a1b4c07c-2193-4bd4-a813-3971e3c562a7 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:testep-vm23 Tenant:default Namespace: ResourceVersion: UUID:a1b4c07c-2193-4bd4-a813-3971e3c562a7 Labels:map[dc:lab-22 dmz:yes tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 160.199µs
[martini] Started POST /v1/endpoints/default/endpoints for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/093478b4-4a51-4099-b8f2-61bba1ed7879 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:testep-vm24 Tenant:default Namespace: ResourceVersion: UUID:093478b4-4a51-4099-b8f2-61bba1ed7879 Labels:map[dc:lab-22 dmz:no tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 92.549µs
[martini] Started POST /v1/endpoints/default/endpoints for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/c5684fcf-1189-4332-88ce-a7ef6b239bc3 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:blah-vm25 Tenant:default Namespace: ResourceVersion: UUID:c5684fcf-1189-4332-88ce-a7ef6b239bc3 Labels:map[dmz:no tier:frontend dc:lab-22] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 92.492µs
[martini] Started POST /v1/endpoints/default/endpoints for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/5ac8a906-10c5-4780-87bd-5eb63356ed53 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:blah-vm26 Tenant:default Namespace: ResourceVersion: UUID:5ac8a906-10c5-4780-87bd-5eb63356ed53 Labels:map[dc:lab-22 dmz:yes tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 91.598µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 661.676µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 817.147µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 662.081µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 627.859µs
[martini] Started POST /v1/lb-policy/default/lb-policy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/8295d7a2-3269-4044-ab00-68b5b2e7a8ec object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:mr-lbp1 Tenant:default Namespace: ResourceVersion: UUID:8295d7a2-3269-4044-ab00-68b5b2e7a8ec Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: Algorithm: SessionAffinity: HealthCheck:} Status:{Services:[]}}" 
[martini] Completed 200 OK in 150.175µs
[martini] Started POST /v1/lb-policy/default/lb-policy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/bf6617d4-233c-4787-917c-35dbf9e53300 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:mr-lbp2 Tenant:default Namespace: ResourceVersion: UUID:bf6617d4-233c-4787-917c-35dbf9e53300 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: Algorithm: SessionAffinity: HealthCheck:} Status:{Services:[]}}" 
[martini] Completed 200 OK in 88.623µs
[martini] Started GET /v1/lb-policy/default/lb-policy for [::1]:52738
[martini] Completed 200 OK in 602.195µs
[martini] Started POST /v1/sgpolicy/default/sgpolicy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/73abf785-d5f3-4703-8035-dfc66bafbc5b object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:mr-sgp1 Tenant:default Namespace: ResourceVersion: UUID:73abf785-d5f3-4703-8035-dfc66bafbc5b Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[] InRules:[] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 125.406µs
[martini] Started POST /v1/sgpolicy/default/sgpolicy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/74785ae1-9fdf-46e5-b511-8dce0cc35f44 object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:mr-sgp2 Tenant:default Namespace: ResourceVersion: UUID:74785ae1-9fdf-46e5-b511-8dce0cc35f44 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[] InRules:[] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 70.771µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy for [::1]:52738
[martini] Completed 200 OK in 456.157µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /user/dc09473c-bce5-47ae-a983-c8bfed60e3f7 object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:mr-user1 Tenant:default Namespace: ResourceVersion: UUID:dc09473c-bce5-47ae-a983-c8bfed60e3f7 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[mr-role1]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 105.838µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /user/37436a61-791b-4429-9b9a-a4a091b2e108 object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:mr-user2 Tenant:default Namespace: ResourceVersion: UUID:37436a61-791b-4429-9b9a-a4a091b2e108 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[mr-role2]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 91.453µs
[martini] Started GET /user for [::1]:52738
[martini] Completed 200 OK in 632.04µs
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /role/85dc1d4f-d97a-433a-b237-72a8f5b5e8fb object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:mr-role1 Tenant:default Namespace: ResourceVersion: UUID:85dc1d4f-d97a-433a-b237-72a8f5b5e8fb Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[mr-perm1]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 104.766µs
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /role/f0868585-d43e-4e69-888c-c91cc7613f96 object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:mr-role2 Tenant:default Namespace: ResourceVersion: UUID:f0868585-d43e-4e69-888c-c91cc7613f96 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[mr-perm2]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 70.288µs
[martini] Started GET /role for [::1]:52738
[martini] Completed 200 OK in 709.522µs
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /permission/fc16302a-006c-40dc-b7f9-2e6dfe235207 object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:mr-perm1 Tenant:default Namespace: ResourceVersion: UUID:fc16302a-006c-40dc-b7f9-2e6dfe235207 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action: ObjectSelector:map[] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 76.012µs
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /permission/d7b9a3fc-b24a-4f34-9ece-85a0fa09b9d7 object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:mr-perm2 Tenant:default Namespace: ResourceVersion: UUID:d7b9a3fc-b24a-4f34-9ece-85a0fa09b9d7 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action: ObjectSelector:map[] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 82.471µs
[martini] Started GET /permission for [::1]:52738
[martini] Completed 200 OK in 692.786µs
--- PASS: TestMultiRead (0.02s)
=== RUN   TestLabelCommand
[martini] Started POST /v1/services/default/services for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/8b7925ac-4327-4e2f-b142-9ce55dd82879 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc1 Tenant:default Namespace: ResourceVersion: UUID:8b7925ac-4327-4e2f-b142-9ce55dd82879 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] VirtualIp:155.12.12.1 Ports:8001 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 93.834µs
[martini] Started POST /v1/services/default/services for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/d5d8c91d-301b-46f0-87fc-7eec603e3a6c object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc2 Tenant:default Namespace: ResourceVersion: UUID:d5d8c91d-301b-46f0-87fc-7eec603e3a6c Labels:map[auth:ldap] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:app] VirtualIp:155.12.12.2 Ports:8002 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 102.171µs
[martini] Started POST /v1/services/default/services for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/b6c97f18-b868-433e-acf1-5fd9bd774931 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc3 Tenant:default Namespace: ResourceVersion: UUID:b6c97f18-b868-433e-acf1-5fd9bd774931 Labels:map[auth:ldap] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:db] VirtualIp:155.12.12.3 Ports:8003 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 130.366µs
[martini] Started POST /v1/services/default/services for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/fd76c4a6-dec3-48e8-b866-34e69a6eb853 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc4 Tenant:default Namespace: ResourceVersion: UUID:fd76c4a6-dec3-48e8-b866-34e69a6eb853 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:db] VirtualIp:155.12.12.4 Ports:8004 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 104.909µs
[martini] Started GET /v1/services/default/services/testsvc1 for [::1]:52738
[martini] Completed 200 OK in 687.879µs
[martini] Started PUT /v1/services/default/services/testsvc1 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/8b7925ac-4327-4e2f-b142-9ce55dd82879 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc1 Tenant:default Namespace: ResourceVersion:1 UUID:8b7925ac-4327-4e2f-b142-9ce55dd82879 Labels:map[dmz:yes] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] VirtualIp:155.12.12.1 Ports:8001 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 191.387µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 759.535µs
[martini] Started GET /v1/services/default/services/testsvc2 for [::1]:52738
[martini] Completed 200 OK in 804.898µs
[martini] Started PUT /v1/services/default/services/testsvc2 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/d5d8c91d-301b-46f0-87fc-7eec603e3a6c object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc2 Tenant:default Namespace: ResourceVersion:1 UUID:d5d8c91d-301b-46f0-87fc-7eec603e3a6c Labels:map[-auth:ldap auth:ldap] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:app] VirtualIp:155.12.12.2 Ports:8002 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 122.586µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 584.401µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 567.62µs
[martini] Started PUT /v1/services/default/services/testsvc2 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/d5d8c91d-301b-46f0-87fc-7eec603e3a6c object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc2 Tenant:default Namespace: ResourceVersion:2 UUID:d5d8c91d-301b-46f0-87fc-7eec603e3a6c Labels:map[-auth:ldap auth:passwd] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:app] VirtualIp:155.12.12.2 Ports:8002 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 113.585µs
[martini] Started PUT /v1/services/default/services/testsvc3 for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/services/:tenant/services/b6c97f18-b868-433e-acf1-5fd9bd774931 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc3 Tenant:default Namespace: ResourceVersion:1 UUID:b6c97f18-b868-433e-acf1-5fd9bd774931 Labels:map[auth:passwd] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:db] VirtualIp:155.12.12.3 Ports:8003 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 139.807µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 517.915µs
--- PASS: TestLabelCommand (0.01s)
=== RUN   TestShowVersion
--- PASS: TestShowVersion (0.00s)
=== RUN   TestSnapshot
[martini] Started POST /v1/tenants/tenants for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/tenants/tenants/99ca88d0-6070-4b69-949e-422ca0b23b10 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:snap-tenant Tenant:default Namespace: ResourceVersion: UUID:99ca88d0-6070-4b69-949e-422ca0b23b10 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:snap-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 117.346µs
[martini] Started POST /v1/lb-policy/default/lb-policy for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/d14c68e4-a450-4697-8396-cdc822dbfaa4 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:snap-lbpolicy Tenant:default Namespace: ResourceVersion: UUID:d14c68e4-a450-4697-8396-cdc822dbfaa4 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:l4 Algorithm:llatency SessionAffinity:yes HealthCheck:Interval:5 ProbePortOrUrl:\"/healthStatus\" MaxTimeouts:25 } Status:{Services:[]}}" 
[martini] Completed 200 OK in 116.455µs
[martini] Started POST /v1/cmd/nodes for [::1]:52738
time="2017-10-20T14:27:09-07:00" level=info msg="Create key: /v1/cmd/nodes/d1e1c532-34b9-40ee-9b69-3f6301850b00 object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:snap-node Tenant:default Namespace: ResourceVersion: UUID:d1e1c532-34b9-40ee-9b69-3f6301850b00 Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[WORKLOAD]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 96.555µs
[martini] Started GET /v1/cmd/cluster for [::1]:52738
[martini] Completed 200 OK in 701.132µs
[martini] Started GET /v1/endpoints/default/endpoints for [::1]:52738
[martini] Completed 200 OK in 683.868µs
[martini] Started GET /v1/lb-policy/default/lb-policy for [::1]:52738
[martini] Completed 200 OK in 377.269µs
[martini] Started GET /v1/networks/default/networks for [::1]:52738
[martini] Completed 200 OK in 452.335µs
[martini] Started GET /v1/cmd/nodes for [::1]:52738
[martini] Completed 200 OK in 404.132µs
[martini] Started GET /permission for [::1]:52738
[martini] Completed 200 OK in 404.643µs
[martini] Started GET /role for [::1]:52738
[martini] Completed 200 OK in 516.584µs
[martini] Started GET /v1/security-groups/default/security-groups for [::1]:52738
[martini] Completed 200 OK in 458.612µs
[martini] Started GET /v1/services/default/services for [::1]:52738
[martini] Completed 200 OK in 497.566µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy for [::1]:52738
[martini] Completed 200 OK in 705.389µs
[martini] Started GET /v1/cmd/smartnics for [::1]:52738
[martini] Completed 200 OK in 545.781µs
[martini] Started GET /v1/tenants/tenants for [::1]:52738
[martini] Completed 200 OK in 454.87µs
[martini] Started GET /user for [::1]:52738
[martini] Completed 200 OK in 894.278µs
[martini] Started PUT /v1/cmd/cluster/dc-az-cluster1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/cmd/cluster/9e3aad04-daec-45e0-96f8-5210de706049 object {TypeMeta:{Kind:cluster APIVersion:} ObjectMeta:{Name:dc-az-cluster1 Tenant:default Namespace: ResourceVersion: UUID:9e3aad04-daec-45e0-96f8-5210de706049 Labels:map[allowed:qa dc:sjc-lab22 type:vcenter] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{QuorumNodes:[mgmt1 mgmt2 mgmt3] VirtualIP:151.21.43.44 NTPServers:[ntp-svr1 ntp-svr2] DNSSubDomain:internal.pensando.io AutoAdmitNICs:false} Status:{Leader:}}" 
[martini] Completed 200 OK in 151.667µs
[martini] Started PUT /v1/endpoints/default/endpoints/blah-vm25 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/c5684fcf-1189-4332-88ce-a7ef6b239bc3 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:blah-vm25 Tenant:default Namespace: ResourceVersion: UUID:c5684fcf-1189-4332-88ce-a7ef6b239bc3 Labels:map[dc:lab-22 dmz:no tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 549.573µs
[martini] Started PUT /v1/endpoints/default/endpoints/blah-vm26 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/5ac8a906-10c5-4780-87bd-5eb63356ed53 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:blah-vm26 Tenant:default Namespace: ResourceVersion: UUID:5ac8a906-10c5-4780-87bd-5eb63356ed53 Labels:map[dc:lab-22 dmz:yes tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 251.323µs
[martini] Started PUT /v1/endpoints/default/endpoints/testep-vm23 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/a1b4c07c-2193-4bd4-a813-3971e3c562a7 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:testep-vm23 Tenant:default Namespace: ResourceVersion: UUID:a1b4c07c-2193-4bd4-a813-3971e3c562a7 Labels:map[dc:lab-22 dmz:yes tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 248.902µs
[martini] Started PUT /v1/endpoints/default/endpoints/testep-vm24 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/093478b4-4a51-4099-b8f2-61bba1ed7879 object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:testep-vm24 Tenant:default Namespace: ResourceVersion: UUID:093478b4-4a51-4099-b8f2-61bba1ed7879 Labels:map[dc:lab-22 dmz:no tier:frontend] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 274.185µs
[martini] Started PUT /v1/endpoints/default/endpoints/vm23 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/endpoints/:tenant/endpoints/e04b212b-8b63-4125-b770-ff6bdba0436a object {TypeMeta:{Kind:endpoint APIVersion:} ObjectMeta:{Name:vm23 Tenant:default Namespace: ResourceVersion: UUID:e04b212b-8b63-4125-b770-ff6bdba0436a Labels:map[dmz:no tier:frontend dc:lab-22] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{} Status:{EndpointUUID: WorkloadUUID: WorkloadName: Network: HomingHostAddr: HomingHostName: IPv4Address: IPv4Gateway: IPv6Address: IPv6Gateway: MacAddress: NodeUUID: EndpointState: SecurityGroups:[] MicroSegmentVlan:0 WorkloadAttributes:[]}}" 
[martini] Completed 200 OK in 272.699µs
[martini] Started PUT /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/5091f77b-68c2-4a4a-8309-db514d765974 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:dev-lb-policy Tenant:default Namespace: ResourceVersion: UUID:5091f77b-68c2-4a4a-8309-db514d765974 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:l4 Algorithm:least-latency SessionAffinity:yes HealthCheck:Interval:5 ProbePortOrUrl:\"/healthStatus\" MaxTimeouts:25 } Status:{Services:[]}}" 
[martini] Completed 200 OK in 167.66µs
[martini] Started PUT /v1/lb-policy/default/lb-policy/mr-lbp1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/8295d7a2-3269-4044-ab00-68b5b2e7a8ec object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:mr-lbp1 Tenant:default Namespace: ResourceVersion: UUID:8295d7a2-3269-4044-ab00-68b5b2e7a8ec Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: Algorithm: SessionAffinity: HealthCheck:} Status:{Services:[]}}" 
[martini] Completed 200 OK in 108.512µs
[martini] Started PUT /v1/lb-policy/default/lb-policy/mr-lbp2 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/bf6617d4-233c-4787-917c-35dbf9e53300 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:mr-lbp2 Tenant:default Namespace: ResourceVersion: UUID:bf6617d4-233c-4787-917c-35dbf9e53300 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: Algorithm: SessionAffinity: HealthCheck:} Status:{Services:[]}}" 
[martini] Completed 200 OK in 109.392µs
[martini] Started PUT /v1/lb-policy/default/lb-policy/snap-lbpolicy for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/lb-policy/:tenant/lb-policy/d14c68e4-a450-4697-8396-cdc822dbfaa4 object {TypeMeta:{Kind:lbPolicy APIVersion:} ObjectMeta:{Name:snap-lbpolicy Tenant:default Namespace: ResourceVersion: UUID:d14c68e4-a450-4697-8396-cdc822dbfaa4 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:l4 Algorithm:llatency SessionAffinity:yes HealthCheck:Interval:5 ProbePortOrUrl:\"/healthStatus\" MaxTimeouts:25 } Status:{Services:[]}}" 
[martini] Completed 200 OK in 115.829µs
[martini] Started PUT /v1/networks/default/networks/lab22-net145 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/fb276e0c-3989-4802-9b82-8cc14dc82db9 object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:lab22-net145 Tenant:default Namespace: ResourceVersion: UUID:fb276e0c-3989-4802-9b82-8cc14dc82db9 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vxlan IPv4Subnet:145.1.1.0/24 IPv4Gateway:145.1.1.254 IPv6Subnet:2001:db8::0/64 IPv6Gateway:2001:db8::1 VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 150.022µs
[martini] Started PUT /v1/networks/default/networks/test1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/f2631ca4-d60c-4150-86cc-f0cd7d5ecd1f object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:f2631ca4-d60c-4150-86cc-f0cd7d5ecd1f Labels:map[dhcp:yes] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:13.1.1.0/24 IPv4Gateway:13.1.1.254 IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 185.942µs
[martini] Started PUT /v1/networks/default/networks/test2 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/22f0b3f1-c729-4d52-8263-e11d64f7f7ef object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:22f0b3f1-c729-4d52-8263-e11d64f7f7ef Labels:map[level:prod dhcp:no] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:13.1.2.0/24 IPv4Gateway:13.1.2.254 IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 113.457µs
[martini] Started PUT /v1/networks/default/networks/test3 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/85d1c7bf-b366-4423-802e-0d00bcdfe977 object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:test3 Tenant:default Namespace: ResourceVersion: UUID:85d1c7bf-b366-4423-802e-0d00bcdfe977 Labels:map[dhcp:no] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type:vlan IPv4Subnet:13.1.3.0/24 IPv4Gateway:13.1.3.254 IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 108.121µs
[martini] Started PUT /v1/networks/default/networks/uploaded for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/networks/:tenant/networks/85f5ecf4-c0af-4fe2-971e-70f22426662d object {TypeMeta:{Kind:network APIVersion:} ObjectMeta:{Name:uploaded Tenant:default Namespace: ResourceVersion: UUID:85f5ecf4-c0af-4fe2-971e-70f22426662d Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Type: IPv4Subnet: IPv4Gateway: IPv6Subnet: IPv6Gateway: VlanID:0 VxlanVNI:0} Status:{Workloads:[] AllocatedIPv4Addrs:[]}}" 
[martini] Completed 200 OK in 102.734µs
[martini] Started PUT /v1/cmd/nodes/snap-node for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/cmd/nodes/d1e1c532-34b9-40ee-9b69-3f6301850b00 object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:snap-node Tenant:default Namespace: ResourceVersion: UUID:d1e1c532-34b9-40ee-9b69-3f6301850b00 Labels:map[vCenter:modified-vc1] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[WORKLOAD]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 167.509µs
[martini] Started PUT /v1/cmd/nodes/test1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/cmd/nodes/4e2eee49-6eb0-4e4f-af1b-d13fdcaba3e0 object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:4e2eee49-6eb0-4e4f-af1b-d13fdcaba3e0 Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[QUORUM]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 107.66µs
[martini] Started PUT /v1/cmd/nodes/test2 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/cmd/nodes/e92342b6-c521-4889-a0e5-48f92b0e226d object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:e92342b6-c521-4889-a0e5-48f92b0e226d Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[WORKLOAD]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 110.216µs
[martini] Started PUT /v1/cmd/nodes/vm233 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/cmd/nodes/b4768cd8-1d46-4886-9ff0-e4c9f1f0458c object {TypeMeta:{Kind:node APIVersion:} ObjectMeta:{Name:vm233 Tenant:default Namespace: ResourceVersion: UUID:b4768cd8-1d46-4886-9ff0-e4c9f1f0458c Labels:map[vCenter:vc2] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[CONTROLLER]} Status:{Phase: Conditions:[] Nics:[]}}" 
[martini] Completed 200 OK in 133.206µs
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /permission/fc16302a-006c-40dc-b7f9-2e6dfe235207 object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:mr-perm1 Tenant:default Namespace: ResourceVersion:1 UUID:fc16302a-006c-40dc-b7f9-2e6dfe235207 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action: ObjectSelector:map[] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 170.913µs
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /permission/d7b9a3fc-b24a-4f34-9ece-85a0fa09b9d7 object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:mr-perm2 Tenant:default Namespace: ResourceVersion:1 UUID:d7b9a3fc-b24a-4f34-9ece-85a0fa09b9d7 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action: ObjectSelector:map[] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 128.357µs
[martini] Started POST /permission for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /permission/9a742012-5598-486e-8678-dda4ca40f9cd object {TypeMeta:{Kind:permission APIVersion:} ObjectMeta:{Name:network-objs-rw Tenant:default Namespace: ResourceVersion:2 UUID:9a742012-5598-486e-8678-dda4ca40f9cd Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Action:rw ObjectSelector:map[network:*] ValidUntil:} Status:{CreationTime: Roles:[] Users:[]}}" 
[martini] Completed 200 OK in 107.103µs
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /role/85dc1d4f-d97a-433a-b237-72a8f5b5e8fb object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:mr-role1 Tenant:default Namespace: ResourceVersion:1 UUID:85dc1d4f-d97a-433a-b237-72a8f5b5e8fb Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[mr-perm1]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 126.866µs
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /role/f0868585-d43e-4e69-888c-c91cc7613f96 object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:mr-role2 Tenant:default Namespace: ResourceVersion:1 UUID:f0868585-d43e-4e69-888c-c91cc7613f96 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[mr-perm2]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 100.004µs
[martini] Started POST /role for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /role/f4c4b2c0-a104-4946-b72f-d83f504603ec object {TypeMeta:{Kind:role APIVersion:} ObjectMeta:{Name:network-admin Tenant:default Namespace: ResourceVersion:2 UUID:f4c4b2c0-a104-4946-b72f-d83f504603ec Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Permissions:[security-objs-rw]} Status:{Users:[]}}" 
[martini] Completed 200 OK in 106.576µs
[martini] Started PUT /v1/security-groups/default/security-groups/editsg for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/7c61fd40-d51f-4b28-a2f2-99600ad37830 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:editsg Tenant:default Namespace: ResourceVersion: UUID:7c61fd40-d51f-4b28-a2f2-99600ad37830 Labels:map[five:six one:two three:four] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 140.79µs
[martini] Started PUT /v1/security-groups/default/security-groups/sg10 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/a8e6f087-b186-48c4-8026-b1898b83ebc4 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:sg10 Tenant:default Namespace: ResourceVersion: UUID:a8e6f087-b186-48c4-8026-b1898b83ebc4 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[key3:val3] ServiceSelector:[] MatchPrefixes:[12.1.1.0/22]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 113.481µs
[martini] Started PUT /v1/security-groups/default/security-groups/sg20 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/f5ab17d1-192d-410d-ad19-4467b855b4f5 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:sg20 Tenant:default Namespace: ResourceVersion: UUID:f5ab17d1-192d-410d-ad19-4467b855b4f5 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[key5:val5] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 111.179µs
[martini] Started PUT /v1/security-groups/default/security-groups/test3 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/security-groups/:tenant/security-groups/d42481d6-f37a-4e4f-b7ee-53d4dd1ad8e8 object {TypeMeta:{Kind:securityGroup APIVersion:} ObjectMeta:{Name:test3 Tenant:default Namespace: ResourceVersion: UUID:d42481d6-f37a-4e4f-b7ee-53d4dd1ad8e8 Labels:map[key4:label4 key1:label1] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:app] ServiceSelector:[] MatchPrefixes:[]} Status:{Workloads:[] Policies:[]}}" 
[martini] Completed 200 OK in 118.6µs
[martini] Started PUT /v1/services/default/services/testsvc1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/services/:tenant/services/8b7925ac-4327-4e2f-b142-9ce55dd82879 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc1 Tenant:default Namespace: ResourceVersion: UUID:8b7925ac-4327-4e2f-b142-9ce55dd82879 Labels:map[dmz:yes] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] VirtualIp:155.12.12.1 Ports:8001 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 151.513µs
[martini] Started PUT /v1/services/default/services/testsvc2 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/services/:tenant/services/d5d8c91d-301b-46f0-87fc-7eec603e3a6c object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc2 Tenant:default Namespace: ResourceVersion: UUID:d5d8c91d-301b-46f0-87fc-7eec603e3a6c Labels:map[-auth:ldap auth:passwd] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:app] VirtualIp:155.12.12.2 Ports:8002 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 137.004µs
[martini] Started PUT /v1/services/default/services/testsvc3 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/services/:tenant/services/b6c97f18-b868-433e-acf1-5fd9bd774931 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc3 Tenant:default Namespace: ResourceVersion: UUID:b6c97f18-b868-433e-acf1-5fd9bd774931 Labels:map[auth:passwd] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:db] VirtualIp:155.12.12.3 Ports:8003 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 115.533µs
[martini] Started PUT /v1/services/default/services/testsvc4 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/services/:tenant/services/fd76c4a6-dec3-48e8-b866-34e69a6eb853 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:testsvc4 Tenant:default Namespace: ResourceVersion: UUID:fd76c4a6-dec3-48e8-b866-34e69a6eb853 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:db] VirtualIp:155.12.12.4 Ports:8004 LBPolicy:} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 102.64µs
[martini] Started PUT /v1/services/default/services/uService122 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/services/:tenant/services/04edd0be-db6d-4fc8-ae48-ba7b99a2de70 object {TypeMeta:{Kind:service APIVersion:} ObjectMeta:{Name:uService122 Tenant:default Namespace: ResourceVersion: UUID:04edd0be-db6d-4fc8-ae48-ba7b99a2de70 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{WorkloadSelector:[tier:frontend] VirtualIp:12.1.1.123 Ports:8080 LBPolicy:prod-lb-policy} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 114.542µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/mr-sgp1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/73abf785-d5f3-4703-8035-dfc66bafbc5b object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:mr-sgp1 Tenant:default Namespace: ResourceVersion: UUID:73abf785-d5f3-4703-8035-dfc66bafbc5b Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[] InRules:[] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 149.248µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/mr-sgp2 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/74785ae1-9fdf-46e5-b511-8dce0cc35f44 object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:mr-sgp2 Tenant:default Namespace: ResourceVersion: UUID:74785ae1-9fdf-46e5-b511-8dce0cc35f44 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[] InRules:[] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 93.611µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/8a95a622-a433-4e4b-9ac2-136d0e50f8ee object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:sg10-ingress Tenant:default Namespace: ResourceVersion: UUID:8a95a622-a433-4e4b-9ac2-136d0e50f8ee Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg30] InRules:[{Ports:tcp/4550 Action:permit PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:} {Ports:tcp/8440 Action:permit PeerGroup:sg10 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 112.068µs
[martini] Started PUT /v1/sgpolicy/default/sgpolicy/test-sgpolicy for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/sgpolicy/:tenant/sgpolicy/e94e913a-4e20-4d3d-888b-7b91fca39442 object {TypeMeta:{Kind:sgpolicy APIVersion:} ObjectMeta:{Name:test-sgpolicy Tenant:default Namespace: ResourceVersion: UUID:e94e913a-4e20-4d3d-888b-7b91fca39442 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AttachGroups:[sg30] InRules:[{Ports:tcp/8440 Action:permit,log PeerGroup:sg20 Apps:[] AppUser: AppUserGrp:}] OutRules:[]} Status:{Workloads:[]}}" 
[martini] Completed 200 OK in 110.25µs
[martini] Started PUT /v1/tenants/tenants/newco for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/tenants/tenants/368c3933-ba18-4f81-987d-13d58a9368e8 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:newco Tenant:default Namespace: ResourceVersion: UUID:368c3933-ba18-4f81-987d-13d58a9368e8 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:newco-adm} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 155.304µs
[martini] Started PUT /v1/tenants/tenants/snap-tenant for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/tenants/tenants/99ca88d0-6070-4b69-949e-422ca0b23b10 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:snap-tenant Tenant:default Namespace: ResourceVersion: UUID:99ca88d0-6070-4b69-949e-422ca0b23b10 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:snap-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 110.823µs
[martini] Started PUT /v1/tenants/tenants/test1 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/tenants/tenants/9f87436e-6067-4b6a-a857-c1b8830db7bb object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:test1 Tenant:default Namespace: ResourceVersion: UUID:9f87436e-6067-4b6a-a857-c1b8830db7bb Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:test1-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 91.371µs
[martini] Started PUT /v1/tenants/tenants/test2 for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /v1/tenants/tenants/372514a7-f2cf-4e8a-ae1f-2fb1b3d59c11 object {TypeMeta:{Kind:tenant APIVersion:} ObjectMeta:{Name:test2 Tenant:default Namespace: ResourceVersion: UUID:372514a7-f2cf-4e8a-ae1f-2fb1b3d59c11 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{AdminUser:test2-admin} Status:{Users:[] Networks:[] SGs:[] Services:[]}}" 
[martini] Completed 200 OK in 105.968µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /user/1f3b58eb-26ea-4da3-99ae-33973415868d object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:bot-bot Tenant:default Namespace: ResourceVersion:2 UUID:1f3b58eb-26ea-4da3-99ae-33973415868d Labels:map[bot:yes dept:qa] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[full-access]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 161.477µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /user/f2a76970-5d93-4744-b25a-76985dcf1609 object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:joe Tenant:default Namespace: ResourceVersion:1 UUID:f2a76970-5d93-4744-b25a-76985dcf1609 Labels:map[dept:eng level:mts] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 127.247µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /user/dc09473c-bce5-47ae-a983-c8bfed60e3f7 object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:mr-user1 Tenant:default Namespace: ResourceVersion:1 UUID:dc09473c-bce5-47ae-a983-c8bfed60e3f7 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[mr-role1]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 102.798µs
[martini] Started POST /user for [::1]:52738
time="2017-10-20T14:27:10-07:00" level=info msg="Create key: /user/37436a61-791b-4429-9b9a-a4a091b2e108 object {TypeMeta:{Kind:user APIVersion:} ObjectMeta:{Name:mr-user2 Tenant:default Namespace: ResourceVersion:1 UUID:37436a61-791b-4429-9b9a-a4a091b2e108 Labels:map[] CreationTime:{Timestamp:{Seconds:0 Nanos:0}} ModTime:{Timestamp:{Seconds:0 Nanos:0}}} Spec:{Roles:[mr-role2]} Status:{AuditTrail:[]}}" 
[martini] Completed 200 OK in 99.404µs
[martini] Started GET /v1/cmd/nodes/snap-node for [::1]:52738
[martini] Completed 200 OK in 386.658µs
--- PASS: TestSnapshot (0.75s)
=== RUN   TestDefinition
--- PASS: TestDefinition (0.00s)
=== RUN   TestSecurityGroupDelete
[martini] Started DELETE /v1/security-groups/default/security-groups/sg10 for [::1]:52738
[martini] Completed 200 OK in 118.553µs
--- PASS: TestSecurityGroupDelete (0.00s)
=== RUN   TestSecurityGroupPolicyDelete
[martini] Started DELETE /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 200 OK in 145.105µs
[martini] Started GET /v1/sgpolicy/default/sgpolicy/sg10-ingress for [::1]:52738
[martini] Completed 404 Not Found in 383.101µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting sgpolicy 'sg10-ingress': EOF" 
--- PASS: TestSecurityGroupPolicyDelete (0.00s)
=== RUN   TestServiceDelete
[martini] Started DELETE /v1/services/default/services/uService122 for [::1]:52738
[martini] Completed 200 OK in 74.972µs
[martini] Started GET /v1/services/default/services/uService122 for [::1]:52738
[martini] Completed 404 Not Found in 431.245µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting service 'uService122': EOF" 
--- PASS: TestServiceDelete (0.00s)
=== RUN   TestLbPolicyDelete
[martini] Started DELETE /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
[martini] Completed 200 OK in 70.117µs
[martini] Started GET /v1/lb-policy/default/lb-policy/dev-lb-policy for [::1]:52738
[martini] Completed 404 Not Found in 352.677µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting lbPolicy 'dev-lb-policy': EOF" 
--- PASS: TestLbPolicyDelete (0.00s)
=== RUN   TestEndpointDelete
[martini] Started DELETE /v1/endpoints/default/endpoints/vm23 for [::1]:52738
[martini] Completed 200 OK in 66.151µs
[martini] Started DELETE /v1/endpoints/default/endpoints/non-existent for [::1]:52738
[martini] Completed 404 Not Found in 28.933µs
--- PASS: TestEndpointDelete (0.00s)
=== RUN   TestPermissionDelete
[martini] Started DELETE /permission/network-objs-rw for [::1]:52738
[martini] Completed 200 OK in 74.829µs
[martini] Started GET /permission/network-objs-rw for [::1]:52744
[martini] Completed 404 Not Found in 386.163µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting permission 'network-objs-rw': 404 Not Found" 
--- PASS: TestPermissionDelete (0.00s)
=== RUN   TestRoleDelete
[martini] Started DELETE /role/network-admin for [::1]:52744
[martini] Completed 200 OK in 68.136µs
[martini] Started GET /role/network-admin for [::1]:52746
[martini] Completed 404 Not Found in 518.9µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting role 'network-admin': 404 Not Found" 
--- PASS: TestRoleDelete (0.00s)
=== RUN   TestUserDelete
[martini] Started DELETE /user/bot-bot for [::1]:52746
[martini] Completed 200 OK in 115.049µs
[martini] Started GET /user/bot-bot for [::1]:52748
[martini] Completed 404 Not Found in 552.318µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting user 'bot-bot': 404 Not Found" 
--- PASS: TestUserDelete (0.00s)
=== RUN   TestNetworkDelete
[martini] Started DELETE /v1/networks/default/networks/lab22-net145 for [::1]:52748
[martini] Completed 200 OK in 70.926µs
[martini] Started GET /v1/networks/default/networks/lab22-net145 for [::1]:52748
[martini] Completed 404 Not Found in 564.997µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting network 'lab22-net145': EOF" 
--- PASS: TestNetworkDelete (0.00s)
=== RUN   TestTenantDelete
[martini] Started DELETE /v1/tenants/tenants/newco for [::1]:52748
[martini] Completed 200 OK in 96.328µs
[martini] Started GET /v1/tenants/tenants/newco for [::1]:52748
[martini] Completed 404 Not Found in 642.949µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting tenant 'newco': EOF" 
--- PASS: TestTenantDelete (0.00s)
=== RUN   TestNodeDelete
[martini] Started DELETE /v1/cmd/nodes/vm233 for [::1]:52748
[martini] Completed 200 OK in 78.02µs
[martini] Started GET /v1/cmd/nodes/vm233 for [::1]:52748
[martini] Completed 404 Not Found in 581.304µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting node 'vm233': EOF" 
--- PASS: TestNodeDelete (0.00s)
=== RUN   TestClusterDelete
[martini] Started DELETE /v1/cmd/cluster/dc-az-cluster1 for [::1]:52748
[martini] Completed 200 OK in 72.839µs
[martini] Started GET /v1/cmd/cluster/dc-az-cluster1 for [::1]:52748
[martini] Completed 404 Not Found in 465.439µs
time="2017-10-20T14:27:10-07:00" level=info msg="Error getting cluster 'dc-az-cluster1': EOF" 
--- PASS: TestClusterDelete (0.00s)
=== RUN   TestGetOpenConnections
--- FAIL: TestGetOpenConnections (0.01s)
	cli_test.go:1604: Active Internet connections (only servers)
		Proto Recv-Q Send-Q Local Address           Foreign Address         State      
		tcp        0      0 0.0.0.0:ssh             0.0.0.0:*               LISTEN     
		tcp6       0      0 [::]:ssh                [::]:*                  LISTEN     
		tcp6       0      0 [::]:19001              [::]:*                  LISTEN     
		tcp6       0      0 [::]:30748              [::]:*                  LISTEN     
		udp        0      0 0.0.0.0:23804           0.0.0.0:*                          
		udp        0      0 0.0.0.0:bootpc          0.0.0.0:*                          
		udp6       0      0 [::]:14968              [::]:*                             
		Active UNIX domain sockets (only servers)
		Proto RefCnt Flags       Type       State         I-Node   Path
		unix  2      [ ACC ]     STREAM     LISTENING     7941     /run/systemd/private
		unix  2      [ ACC ]     STREAM     LISTENING     7974     /run/lvm/lvmetad.socket
		unix  2      [ ACC ]     SEQPACKET  LISTENING     7993     /run/udev/control
		unix  2      [ ACC ]     STREAM     LISTENING     7996     /run/lvm/lvmpolld.socket
		unix  2      [ ACC ]     STREAM     LISTENING     11881    /var/run/dbus/system_bus_socket
		unix  2      [ ACC ]     STREAM     LISTENING     20083    /var/run/openvswitch/db.sock
		unix  2      [ ACC ]     STREAM     LISTENING     20085    /var/run/openvswitch/ovsdb-server.2948.ctl
		unix  2      [ ACC ]     STREAM     LISTENING     25987    /var/run/docker.sock
		unix  2      [ ACC ]     STREAM     LISTENING     26002    /var/run/docker/libcontainerd/docker-containerd.sock
		unix  2      [ ACC ]     STREAM     LISTENING     20118    /var/run/openvswitch/ovs-vswitchd.2964.ctl
		unix  2      [ ACC ]     STREAM     LISTENING     26042    /run/docker/libnetwork/8038870e4c47a13da18f249640047b36222b9c53837b1cd1a6090d7ae60a458f.sock
		unix  2      [ ACC ]     STREAM     LISTENING     20708    /var/run/docker/metrics.sock
		unix  2      [ ACC ]     STREAM     LISTENING     6896     /run/systemd/journal/stdout

?   	github.com/pensando/sw/venice/cli/api	[no test files]
?   	github.com/pensando/sw/venice/cli/gen	[no test files]
?   	github.com/pensando/sw/venice/cli/gen/pregen	[no test files]
?   	github.com/pensando/sw/venice/cli/testserver	[no test files]
?   	github.com/pensando/sw/venice/cli/testserver/tserver	[no test files]`
