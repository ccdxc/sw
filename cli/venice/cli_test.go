package main

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/cli/api"
	"github.com/pensando/sw/cli/venice/testserver/tserver"
)

const (
	veniceCmd      = "venice "
	testServerPort = "30748"
	testServerOpt  = "--server http://localhost:" + testServerPort + " "
	fmtOutput      = false
)

func veniceCLI(cmdStr string) string {
	cmdStr = veniceCmd + testServerOpt + cmdStr
	cmdArgs := strings.Split(cmdStr, " ")

	stdOut := InvokeCLI(cmdArgs, true)
	return stdOut
}

func TestStartServer(t *testing.T) {
	tserver.Start(":" + testServerPort)

	// initiaize cluster with default cluster
	cluster := &cmd.Cluster{}
	cluster.Kind = "cluster"
	cluster.Name = "dc-az-cluster1"
	url := "http://localhost:" + testServerPort + "/v1/cmd/cluster"
	if err := httpPost(url, cluster); err != nil {
		t.Fatalf("error creating default cluster: %s", err)
	}
}

func TestClusterCreate(t *testing.T) {
	out := veniceCLI("update cluster --label type:vcenter --label allowed:qa --label dc:sjc-lab22 --virtualIP 158.21.34.44 --quorumNodes mgmt1 --quorumNodes mgmt2 --quorumNodes mgmt3 --nTPServers ntp-svr1 --nTPServers ntp-svr2 --dNSSubDomain internal.pensando.io dc-az-cluster1")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update: garbled output '%s'", out)
	}
	out = veniceCLI("read cluster dc-az-cluster1")
	cluster := &cmd.Cluster{}
	if err := json.Unmarshal([]byte(out), cluster); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if cluster.Labels["type"] != "vcenter" ||
		len(cluster.Spec.NTPServers) != 2 || len(cluster.Spec.QuorumNodes) != 3 ||
		cluster.Spec.NTPServers[0] != "ntp-svr1" || cluster.Spec.NTPServers[1] != "ntp-svr2" ||
		cluster.Spec.QuorumNodes[0] != "mgmt1" || cluster.Spec.QuorumNodes[1] != "mgmt2" || cluster.Spec.QuorumNodes[2] != "mgmt3" {
		t.Fatalf("Create operation failed: %+v \n", cluster)
	}

	out = veniceCLI("read cluster dc-az --gbc")
	if !strings.Contains(out, "dc-az-cluster1") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update cluster dc-az --gbc")
}

func TestClusterUpdate(t *testing.T) {
	out := veniceCLI("update cluster --virtualIP 151.21.43.44 dc-az-cluster1")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update: garbled output '%s'", out)
	}

	out = veniceCLI("read cluster dc-az-cluster1")
	cluster := &cmd.Cluster{}
	if err := json.Unmarshal([]byte(out), cluster); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if len(cluster.Spec.NTPServers) != 2 ||
		cluster.Spec.NTPServers[0] != "ntp-svr1" || cluster.Spec.NTPServers[1] != "ntp-svr2" ||
		cluster.Labels["type"] != "vcenter" || cluster.Spec.VirtualIP != "151.21.43.44" {
		t.Fatalf("Update operation failed: %+v\n", cluster)
	}
}

func TestNodeCreate(t *testing.T) {
	out := veniceCLI("create node --label vCenter:vc2 --roles 2 --roles 1 vm233")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update: garbled output '%s'", out)
	}

	out = veniceCLI("read node vm233")
	node := &cmd.Node{}
	if err := json.Unmarshal([]byte(out), node); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if node.Labels["vCenter"] != "vc2" || node.Spec.Roles[0] != 2 || node.Spec.Roles[1] != 1 {
		t.Fatalf("Create operation failed: %+v \n", node)
	}

	out = veniceCLI("read node")
	node = &cmd.Node{}
	if err := json.Unmarshal([]byte(out), node); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if node.Labels["vCenter"] != "vc2" || node.Spec.Roles[0] != 2 || node.Spec.Roles[1] != 1 {
		t.Fatalf("Single read read failed: %+v \n", node)
	}

	out = veniceCLI("read node vm2 --gbc")
	if !strings.Contains(out, "vm233") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update node vm2 --gbc")
}

func TestNodeUpdate(t *testing.T) {
	out := veniceCLI("update node --roles 3 vm233")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("Update: garbled output '%s'", out)
	}
	out = veniceCLI("read node vm233")
	node := &cmd.Node{}
	if err := json.Unmarshal([]byte(out), node); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if node.Labels["vCenter"] != "vc2" || len(node.Spec.Roles) != 1 || node.Spec.Roles[0] != 3 {
		t.Fatalf("Update operation failed: %+v \n", node)
	}
}

func TestTenantCreate(t *testing.T) {
	out := veniceCLI("create tenant newco --adminUser newco-admin")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read tenant newco")
	tenant := &network.Tenant{}
	if err := json.Unmarshal([]byte(out), tenant); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if tenant.ResourceVersion != "1" || tenant.Spec.AdminUser != "newco-admin" {
		t.Fatalf("Create operation failed: %+v \n", tenant)
	}

	out = veniceCLI("read tenant new --gbc")
	if !strings.Contains(out, "newco") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update tenant new --gbc")
}

func TestTenantUpdate(t *testing.T) {
	out := veniceCLI("update tenant newco --adminUser newco-adm")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read tenant newco")
	tenant := &network.Tenant{}
	if err := json.Unmarshal([]byte(out), tenant); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if tenant.ResourceVersion != "2" || tenant.Spec.AdminUser != "newco-adm" {
		t.Fatalf("Create operation failed: %+v \n", tenant)
	}
}

func TestNetworkCreate(t *testing.T) {
	out := veniceCLI("create network lab22-net145 --iPv4Subnet 145.1.1.0/24 --iPv4Gateway 145.1.1.254 --type vlan --iPv6Subnet 2001:db8::0/64 --iPv6Gateway 2001:db8::1 --vlanID 345")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read network lab22-net145")
	net := &network.Network{}
	if err := json.Unmarshal([]byte(out), net); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if net.ResourceVersion != "1" || net.Spec.IPv4Subnet != "145.1.1.0/24" || net.Spec.IPv4Gateway != "145.1.1.254" ||
		net.Spec.Type != "vlan" || net.Spec.IPv6Subnet != "2001:db8::0/64" || net.Spec.IPv6Gateway != "2001:db8::1" ||
		net.Spec.VlanID != 345 {
		t.Fatalf("Create operation failed: %+v \n", net)
	}

	out = veniceCLI("read network lab2 --gbc")
	if !strings.Contains(out, "lab22-net145") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update network lab2 --gbc")
}

func TestNetworkUpdate(t *testing.T) {
	out := veniceCLI("update network lab22-net145 --type vxlan")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read network lab22-net145")
	net := &network.Network{}
	if err := json.Unmarshal([]byte(out), net); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if net.ResourceVersion != "2" || net.Spec.IPv4Subnet != "145.1.1.0/24" || net.Spec.IPv4Gateway != "145.1.1.254" ||
		net.Spec.Type != "vxlan" || net.Spec.IPv6Subnet != "2001:db8::0/64" || net.Spec.IPv6Gateway != "2001:db8::1" {
		t.Fatalf("Create operation failed: %+v \n", net)
	}
}

func TestSecurityGroupCreate(t *testing.T) {
	out := veniceCLI("create securityGroup sg10 --workloadSelector key1:val1 --workloadSelector key2:val2 --matchPrefixes 12.1.1.0/22")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read securityGroup sg10")
	sg := &network.SecurityGroup{}
	if err := json.Unmarshal([]byte(out), sg); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sg.ResourceVersion != "1" || len(sg.Spec.WorkloadSelector) != 2 || len(sg.Spec.MatchPrefixes) != 1 ||
		sg.Spec.WorkloadSelector[0] != "key1:val1" || sg.Spec.WorkloadSelector[1] != "key2:val2" ||
		sg.Spec.MatchPrefixes[0] != "12.1.1.0/22" {
		t.Fatalf("Create operation failed: %+v \n", sg)
	}

	out = veniceCLI("read securityGroup sg --gbc")
	if !strings.Contains(out, "sg10") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update securityGroup sg --gbc")
}

func TestSecurityGroupUpdate(t *testing.T) {
	out := veniceCLI("update securityGroup sg10 --workloadSelector key3:val3")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read securityGroup sg10")
	sg := &network.SecurityGroup{}
	if err := json.Unmarshal([]byte(out), sg); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sg.ResourceVersion != "2" || len(sg.Spec.WorkloadSelector) != 1 || len(sg.Spec.MatchPrefixes) != 1 ||
		sg.Spec.WorkloadSelector[0] != "key3:val3" || sg.Spec.MatchPrefixes[0] != "12.1.1.0/22" {
		t.Fatalf("Create operation failed: %+v \n", sg)
	}
}

func TestSecurityGroupPolicyCreate(t *testing.T) {
	out := veniceCLI("create securityGroup sg20 --workloadSelector key5:val5")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create sg for sgpolicy: garbled output '%s'", out)
	}

	out = veniceCLI("create sgpolicy sg10-ingress --ports tcp/8440 --peerGroup sg20 --action permit --attachGroups sg10")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create sgpolicy: garbled output '%s'", out)
	}

	out = veniceCLI("read sgpolicy sg10-ingress")
	sgp := &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "1" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 1 ||
		sgp.Spec.AttachGroups[0] != "sg10" ||
		sgp.Spec.InRules[0].Ports != "tcp/8440" || sgp.Spec.InRules[0].Action != "permit" || sgp.Spec.InRules[0].PeerGroup != "sg20" {
		t.Fatalf("Create operation failed: %+v \n", sgp)
	}

	out = veniceCLI("read sgpolicy sg10 --gbc")
	if !strings.Contains(out, "sg10-ingress") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update sgpolicy sg10 --gbc")
}

func TestSecurityGroupPolicyUpdate(t *testing.T) {
	out := veniceCLI("update sgpolicy sg10-ingress --attachGroups sg30")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update sgpolicy: garbled output '%s'", out)
	}

	out = veniceCLI("read sgpolicy sg10-ingress")
	sgp := &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "2" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 1 ||
		sgp.Spec.AttachGroups[0] != "sg30" ||
		sgp.Spec.InRules[0].Ports != "tcp/8440" || sgp.Spec.InRules[0].Action != "permit" || sgp.Spec.InRules[0].PeerGroup != "sg20" {
		t.Fatalf("Create operation failed: %+v \n", sgp)
	}

	out = veniceCLI("update sgpolicy sg10-ingress --ports tcp/4550 --peerGroup sg20 --action permit --ports tcp/8440 --peerGroup sg10 --action permit")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update sgpolicy: garbled output '%s'", out)
	}

	out = veniceCLI("read sgpolicy sg10-ingress")
	sgp = &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "3" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 2 ||
		sgp.Spec.AttachGroups[0] != "sg30" ||
		sgp.Spec.InRules[0].Ports != "tcp/4550" || sgp.Spec.InRules[0].Action != "permit" || sgp.Spec.InRules[0].PeerGroup != "sg20" ||
		sgp.Spec.InRules[1].Ports != "tcp/8440" || sgp.Spec.InRules[1].Action != "permit" || sgp.Spec.InRules[1].PeerGroup != "sg10" {
		t.Fatalf("Update operation failed: %+v \n", sgp)
	}
}

func TestServiceCreate(t *testing.T) {
	out := veniceCLI("create service uService122 --virtualIp 12.1.1.122 --ports 8080 --workloadSelector tier:frontend --lBPolicy dev-lb-policy")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read service uService122")
	svc := &network.Service{}
	if err := json.Unmarshal([]byte(out), svc); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if svc.ResourceVersion != "1" || svc.Spec.VirtualIp != "12.1.1.122" || len(svc.Spec.WorkloadSelector) != 1 ||
		svc.Spec.WorkloadSelector[0] != "tier:frontend" || svc.Spec.Ports != "8080" || svc.Spec.LBPolicy != "dev-lb-policy" {
		t.Fatalf("Create operation failed: %+v \n", svc)
	}

	out = veniceCLI("read service uS --gbc")
	if !strings.Contains(out, "uService122") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update service uS --gbc")
}

func TestServiceUpdate(t *testing.T) {
	out := veniceCLI("update service uService122 --virtualIp 12.1.1.123 --lBPolicy prod-lb-policy")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("Update: garbled output '%s'", out)
	}

	out = veniceCLI("read service uService122")
	svc := &network.Service{}
	if err := json.Unmarshal([]byte(out), svc); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if svc.ResourceVersion != "2" || svc.Spec.VirtualIp != "12.1.1.123" || len(svc.Spec.WorkloadSelector) != 1 ||
		svc.Spec.WorkloadSelector[0] != "tier:frontend" || svc.Spec.Ports != "8080" || svc.Spec.LBPolicy != "prod-lb-policy" {
		t.Fatalf("Update operation failed: %+v \n", svc)
	}
}

func TestLbPolicyCreate(t *testing.T) {
	out := veniceCLI("create lbPolicy --algorithm round-robin --probePortOrUrl /healthStatus --sessionAffinity yes --type l4 --interval 5 --maxTimeouts 25 dev-lb-policy")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create: garbled output '%s'", out)
	}

	out = veniceCLI("read lbPolicy dev-lb-policy")
	lbp := &network.LbPolicy{}
	if err := json.Unmarshal([]byte(out), lbp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if lbp.ResourceVersion != "1" || lbp.Spec.HealthCheck.ProbePortOrUrl != "/healthStatus" ||
		lbp.Spec.SessionAffinity != "yes" || lbp.Spec.Type != "l4" || lbp.Spec.Algorithm != "round-robin" ||
		lbp.Spec.HealthCheck.Interval != 5 || lbp.Spec.HealthCheck.MaxTimeouts != 25 {
		t.Fatalf("Create operation failed: %+v \n", lbp)
	}

	out = veniceCLI("read lbPolicy dev- --gbc")
	if !strings.Contains(out, "dev-lb-policy") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update lbPolicy dev- --gbc")
}

func TestLbPolicyUpdate(t *testing.T) {
	out := veniceCLI("update lbPolicy --algorithm least-latency dev-lb-policy")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("Update: garbled output '%s'", out)
	}

	out = veniceCLI("read lbPolicy dev-lb-policy")
	lbp := &network.LbPolicy{}
	if err := json.Unmarshal([]byte(out), lbp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if lbp.ResourceVersion != "2" || lbp.Spec.HealthCheck.ProbePortOrUrl != "/healthStatus" ||
		lbp.Spec.SessionAffinity != "yes" || lbp.Spec.Type != "l4" || lbp.Spec.Algorithm != "least-latency" {
		t.Fatalf("Update operation failed: %+v \n", lbp)
	}
}

func TestEndpointCreate(t *testing.T) {
	out := veniceCLI("create endpoint --label dmz=yes --label dc=lab-22 --label tier:frontend vm23")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read endpoint vm23")
	ep := &network.Endpoint{}
	if err := json.Unmarshal([]byte(out), ep); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := ep.Labels["dmz"]
	lv2, ok2 := ep.Labels["dc"]
	lv3, ok3 := ep.Labels["tier"]
	if ep.ResourceVersion != "1" || len(ep.Labels) != 3 ||
		!ok1 || lv1 != "yes" || !ok2 || lv2 != "lab-22" || !ok3 || lv3 != "frontend" {
		t.Fatalf("Create endpoint failed: %+v \n", ep)
	}

	out = veniceCLI("read endpoint -y vm23")
	if !strings.Contains(out, "dc: lab-22") || !strings.Contains(out, "tier: frontend") {
		t.Fatalf("Yaml read failed: %s \n", out)
	}

	out = veniceCLI("read endpoint vm --gbc")
	if !strings.Contains(out, "vm23") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update endpoint vm --gbc")
}

func TestEndpointUpdate(t *testing.T) {
	out := veniceCLI("update endpoint vm23 --label dmz:no --label dc:lab-22 --label tier:frontend")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read endpoint vm23")
	ep := &network.Endpoint{}
	if err := json.Unmarshal([]byte(out), ep); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := ep.Labels["dmz"]
	lv2, ok2 := ep.Labels["dc"]
	lv3, ok3 := ep.Labels["tier"]
	if ep.ResourceVersion != "2" || len(ep.Labels) != 3 ||
		!ok1 || lv1 != "no" || !ok2 || lv2 != "lab-22" || !ok3 || lv3 != "frontend" {
		t.Fatalf("Update endpoint failed: %+v \n", ep)
	}
}

func TestUserCreate(t *testing.T) {
	out := veniceCLI("create user bot-bot --label dept:qa --label bot:yes --roles network-admin --roles security-admin")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read user bot-bot")
	user := &api.User{}
	if err := json.Unmarshal([]byte(out), user); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := user.Labels["dept"]
	lv2, ok2 := user.Labels["bot"]
	if user.ResourceVersion != "1" || len(user.Labels) != 2 ||
		!ok1 || lv1 != "qa" || !ok2 || lv2 != "yes" ||
		len(user.Spec.Roles) != 2 || user.Spec.Roles[0] != "network-admin" || user.Spec.Roles[1] != "security-admin" {
		t.Fatalf("Create user failed: %+v \n", user)
	}

	out = veniceCLI("read user bot --gbc")
	if !strings.Contains(out, "bot-bot") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update user bot --gbc")
}

func TestUserUpdate(t *testing.T) {
	out := veniceCLI("update user bot-bot --roles full-access")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read user bot-bot")
	user := &api.User{}
	if err := json.Unmarshal([]byte(out), user); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := user.Labels["dept"]
	lv2, ok2 := user.Labels["bot"]
	if user.ResourceVersion != "2" || len(user.Labels) != 2 ||
		!ok1 || lv1 != "qa" || !ok2 || lv2 != "yes" ||
		len(user.Spec.Roles) != 1 || user.Spec.Roles[0] != "full-access" {
		t.Fatalf("Update user failed: %+v \n", user)
	}
}

func TestRoleCreate(t *testing.T) {
	out := veniceCLI("create role network-admin --permissions network-objs-rw --permissions policy-objs-rw")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("Create endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read role network-admin")
	role := &api.Role{}
	if err := json.Unmarshal([]byte(out), role); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	if role.ResourceVersion != "1" || len(role.Spec.Permissions) != 2 ||
		role.Spec.Permissions[0] != "network-objs-rw" || role.Spec.Permissions[1] != "policy-objs-rw" {
		t.Fatalf("Create role failed: %+v \n", role)
	}

	out = veniceCLI("read role net --gbc")
	if !strings.Contains(out, "network-admin") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update role net --gbc")
}

func TestRoleUpdate(t *testing.T) {
	out := veniceCLI("update role network-admin --permissions security-objs-rw")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("Create endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read role network-admin")
	role := &api.Role{}
	if err := json.Unmarshal([]byte(out), role); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	if role.ResourceVersion != "2" || len(role.Spec.Permissions) != 1 ||
		role.Spec.Permissions[0] != "security-objs-rw" {
		t.Fatalf("Update role failed: %+v \n", role)
	}
}

func TestPermissionCreate(t *testing.T) {
	out := veniceCLI("create permission --objectSelector sgpolicy:* --action rw network-objs-rw")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("Create endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read permission network-objs-rw")
	permission := &api.Permission{}
	if err := json.Unmarshal([]byte(out), permission); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := permission.Spec.ObjectSelector["sgpolicy"]
	if permission.ResourceVersion != "1" || len(permission.Spec.ObjectSelector) != 1 ||
		!ok1 || lv1 != "*" || permission.Spec.Action != "rw" {
		t.Fatalf("Create permission failed: %+v \n", permission)
	}

	out = veniceCLI("read permission net --gbc")
	if !strings.Contains(out, "network-objs-rw") {
		t.Fatalf("Command completion for specific object failed: %s \n", out)
	}
	veniceCLI("update permission net --gbc")
}

func TestPermissionUpdate(t *testing.T) {
	out := veniceCLI("update permission --objectSelector network:* network-objs-rw")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read permission network-objs-rw")
	permission := &api.Permission{}
	if err := json.Unmarshal([]byte(out), permission); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := permission.Spec.ObjectSelector["network"]
	if permission.ResourceVersion != "2" || len(permission.Spec.ObjectSelector) != 1 ||
		!ok1 || lv1 != "*" || permission.Spec.Action != "rw" {
		t.Fatalf("Update permission failed: %+v \n", permission)
	}
}

func TestCommandCompletion(t *testing.T) {
	out := veniceCLI("--help")
	if !strings.Contains(out, "Pensando Venice CLI") {
		t.Fatalf("help not working: invalid output '%s'", out)
	}
	out = veniceCLI("--version")
	if !strings.Contains(out, "version v0.1-alpha") {
		t.Fatalf("version not working: invalid output '%s'", out)
	}
	out = veniceCLI("read --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}

	out = veniceCLI("create cluster --virtualIP --gbc")
	if !strings.Contains(out, "{value}") {
		t.Fatalf("command completion for string value failed: invalid output '%s'", out)
	}
	out = veniceCLI("create cluster --label --gbc")
	if !strings.Contains(out, "{key:value}") {
		t.Fatalf("command completion for string-value map: invalid output '%s'", out)
	}
	out = veniceCLI("create cluster --gbc")
	if !strings.Contains(out, "--label --dry-run --file --dNSSubDomain --nTPServers --quorumNodes --virtualIP {cluster}") {
		t.Fatalf("cluster command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create node --gbc")
	if !strings.Contains(out, "--label --dry-run --file --roles {node}") {
		t.Fatalf("node command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update node --gbc")
	if !strings.Contains(out, "--label --dry-run --file --roles") {
		t.Fatalf("node command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete node --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("node command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create endpoint --gbc")
	if !strings.Contains(out, "--label --dry-run --file {endpoint}") {
		t.Fatalf("endpoint command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update endpoint --gbc")
	if !strings.Contains(out, "--label --dry-run --file") {
		t.Fatalf("endpoint command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create tenant --gbc")
	if !strings.Contains(out, "--label --dry-run --file --adminUser {tenant}") {
		t.Fatalf("tenant command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update tenant --gbc")
	if !strings.Contains(out, "--label --dry-run --file --adminUser") {
		t.Fatalf("tenant command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create securityGroup --gbc")
	if !strings.Contains(out, "--label --dry-run --file --matchPrefixes --serviceSelector --workloadSelector {securityGroup}") {
		t.Fatalf("security-group command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update securityGroup blah-sg --gbc")
	if !strings.Contains(out, "--label --dry-run --file --matchPrefixes --serviceSelector --workloadSelector") {
		t.Fatalf("security-group command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update securityGroup blah-sg --matchPrefixes 1.1.1.3/32 --gbc")
	if !strings.Contains(out, "--label --dry-run --file --matchPrefixes --serviceSelector --workloadSelector") {
		t.Fatalf("security-group command completion: invalid output '%s'", out)
	}

	out = veniceCLI("delete securityGroup blah-sg --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("security-group command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create user --gbc")
	if !strings.Contains(out, "--label --dry-run --file --roles {user}") {
		t.Fatalf("user command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update user --gbc")
	if !strings.Contains(out, "--label --dry-run --file --roles") {
		t.Fatalf("user command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete user --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("user command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create role --gbc")
	if !strings.Contains(out, "--label --dry-run --file --permissions {role}") {
		t.Fatalf("role command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update role --gbc")
	if !strings.Contains(out, "--label --dry-run --file --permissions") {
		t.Fatalf("role command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete role --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("role command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create permission --gbc")
	if !strings.Contains(out, "--label --dry-run --file --action --objectSelector --validUntil {permission}") {
		t.Fatalf("permission command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update permission --gbc")
	if !strings.Contains(out, "--label --dry-run --file --action --objectSelector --validUntil") {
		t.Fatalf("permission command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete permission --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("permission command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create service --gbc")
	if !strings.Contains(out, "--label --dry-run --file --lBPolicy --ports --virtualIp --workloadSelector {service}") {
		t.Fatalf("service command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update service --gbc")
	if !strings.Contains(out, "--label --dry-run --file --lBPolicy --ports --virtualIp --workloadSelector uService122") {
		t.Fatalf("service command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete service --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression uService122") {
		t.Fatalf("service command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create sgpolicy --gbc")
	if !strings.Contains(out, "--label --dry-run --file --action --attachGroups --peerGroup --ports {sgpolicy}") {
		t.Fatalf("lbPolicy command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update sgpolicy --gbc")
	if !strings.Contains(out, "--label --dry-run --file --action --attachGroups --peerGroup --ports") {
		t.Fatalf("lbPolicy command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete sgpolicy --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("lbPolicy command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create lbPolicy --gbc")
	if !strings.Contains(out, "--algorithm") {
		t.Fatalf("lbPolicy command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update lbPolicy --gbc")
	if !strings.Contains(out, "--algorithm") {
		t.Fatalf("lbPolicy command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete lbPolicy --gbc")
	if !strings.Contains(out, "--label --dry-run --regular-expression") {
		t.Fatalf("lbPolicy command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create network lab22-net145 --iPv4Subnet 145.1.1.0/24 --iPv4Gateway 145.1.1.254 --type vlan --iPv6Subnet 2001:db8::0/64 --iPv6Gateway 2001:db8::1 --vlanID 345 --gbc")
	if !strings.Contains(out, "--label --dry-run --file --vxlanVNI") {
		t.Fatalf("command completion with filled params failed: %s", out)
	}

	out = veniceCLI("create lbPolicy --algorithm round-robin --probePortOrUrl /healthStatus --sessionAffinity yes --type l4 --interval 5 --maxTimeouts 25 dev-lb-policy --gbc")
	if !strings.Contains(out, "--label --dry-run --file --declareHealthyCount --probesPerInterval") {
		t.Fatalf("command completion with partially filled params failed: %s", out)
	}

	veniceCLI("create upload --gbc")
	out = veniceCLI("create -h")
	if !strings.Contains(out, "create command [command options] [arguments...]") {
		t.Fatalf("create help command completion: invalid output '%s'", out)
	}
	out = veniceCLI("update -h")
	if !strings.Contains(out, "update command [command options] [arguments...]") {
		t.Fatalf("update help command completion: invalid output '%s'", out)
	}
	out = veniceCLI("delete -h")
	if !strings.Contains(out, "delete command [command options] [arguments...]") {
		t.Fatalf("delete help command completion: invalid output '%s'", out)
	}

	out = veniceCLI("create --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("update --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("delete --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("edit --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("label --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("example --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("definition --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("tree --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
	out = veniceCLI("snapshot --gbc")
	if !strings.Contains(out, "cluster endpoint") {
		t.Fatalf("command completion for command names failed: invalid output '%s'", out)
	}
}

func TestCreateFromFile(t *testing.T) {
	ymlBytes := `#
kind: sgpolicy
meta:
  Name: test-sgpolicy
spec:
  attach-groups:
  - sg10
  in-rules:
  - acion: permit
    peer-group: sg20
    ports: tcp/8440

`
	fileName := "/tmp/tmp3343.yml"
	ioutil.WriteFile(fileName, []byte(ymlBytes), 0644)
	defer os.Remove(fileName)

	out := veniceCLI("create upload " + fileName)
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create sgpolicy from file: garbled output '%s'", out)
	}

	out = veniceCLI("read sgpolicy test-sgpolicy")
	sgp := &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "1" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 1 ||
		sgp.Spec.AttachGroups[0] != "sg10" ||
		sgp.Spec.InRules[0].Ports != "tcp/8440" || sgp.Spec.InRules[0].Action != "permit" || sgp.Spec.InRules[0].PeerGroup != "sg20" {
		t.Fatalf("Create operation failed: %+v \n", sgp)
	}

	jsonBytes := `
	{
	  "Kind": "sgpolicy",
	  "meta": {
	    "Name": "test-sgpolicy"
	  },
	  "spec": {
	    "attach-groups": [
	        "sg10"
	      ],
	    "in-rules": [
	      {
	        "acion": "permit,log",
	        "peer-group": "sg20",
	        "ports" : "tcp/8440"
	      }
	    ]
	  }
	}
	`

	fileName = "/tmp/tmp3343.json"
	if err := ioutil.WriteFile(fileName, []byte(jsonBytes), 0644); err != nil {
		t.Fatalf("error writing to file %s: %s", fileName, err)
	}

	defer os.Remove(fileName)

	out = veniceCLI("create upload " + fileName)
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create sgpolicy from file: garbled output '%s'", out)
	}

	out = veniceCLI("read sgpolicy test-sgpolicy")
	sgp = &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "2" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 1 ||
		sgp.Spec.AttachGroups[0] != "sg10" ||
		sgp.Spec.InRules[0].Ports != "tcp/8440" || sgp.Spec.InRules[0].Action != "permit,log" ||
		sgp.Spec.InRules[0].PeerGroup != "sg20" {
		t.Fatalf("Create operation failed: %+v \n", sgp)
	}

	dir, err := ioutil.TempDir("", "test")
	if err != nil {
		t.Fatalf("create dir failed: %s", err)
	}
	defer os.RemoveAll(dir)

	jsonBytes = `
	{
	  "Kind": "sgpolicy",
	  "meta": {
	    "Name": "test-sgpolicy"
	  },
	  "spec": {
	    "attach-groups": [
	        "sg30"
	      ],
	    "in-rules": [
	      {
	        "acion": "permit,log",
	        "peer-group": "sg20",
	        "ports" : "tcp/8440"
	      }
	    ]
	  }
	}
	`
	fileName = filepath.Join(dir, "tmp3343.json")
	if err := ioutil.WriteFile(fileName, []byte(jsonBytes), 0644); err != nil {
		t.Fatalf("error writing to file %s: %s", fileName, err)
	}

	out = veniceCLI("create upload " + dir)
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create sgpolicy from file: garbled output '%s'", out)
	}

	out = veniceCLI("read sgpolicy test-sgpolicy")
	sgp = &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "3" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 1 ||
		sgp.Spec.AttachGroups[0] != "sg30" ||
		sgp.Spec.InRules[0].Ports != "tcp/8440" || sgp.Spec.InRules[0].Action != "permit,log" ||
		sgp.Spec.InRules[0].PeerGroup != "sg20" {
		t.Fatalf("Create operation failed: %+v \n", sgp)
	}

	out = veniceCLI("create upload nonexistentfile")
	if !fmtOutput && !strings.Contains(out, "File does not exist") {
		t.Fatalf("unable to see error from upload on non existent file: %s", out)
	}

	ymlBytes = `#
{kind: sgpolicy
meta:
  Name: test-sgpolicy
spec:
  attach-groups:
  - sg10
  in-rules:
  - acion: permit
    peer-group: sg20
    ports: tcp/8440

`
	fileName = "/tmp/tmp3343.yml"
	ioutil.WriteFile(fileName, []byte(ymlBytes), 0644)
	defer os.Remove(fileName)

	out = veniceCLI("create upload " + fileName)
	if !fmtOutput && !strings.Contains(out, "Error converting yaml to json") {
		t.Fatalf("Failed to error on invalid yaml to json conversion: %s", out)
	}

	out = veniceCLI("create network uploaded")
	url := "http://localhost:" + testServerPort + api.Objs["network"].URL + "/uploaded"
	out = veniceCLI("create upload " + url)
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("create objects from URL failed '%s'", out)
	}

	out = veniceCLI("create upload http://localhost:/invalid-url")
	if !fmtOutput && !strings.Contains(out, "Error fetching URL") {
		t.Fatalf("unable to verify invalid URL '%s'", out)
	}
}

func TestEditCommand(t *testing.T) {
	veniceCLI("create securityGroup editsg --label one:two --label three:four --workloadSelector tier:frontend")

	// change the editor to cat command
	oldEditor := os.Getenv("VENICE_EDITOR")
	os.Setenv("VENICE_EDITOR", "cat")
	veniceCLI("edit securityGroup editsg")

	// make sure that the resource version has been updated
	out := veniceCLI("read securityGroup editsg")
	sg := &network.SecurityGroup{}
	if err := json.Unmarshal([]byte(out), sg); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sg.ResourceVersion != "2" || len(sg.Spec.WorkloadSelector) != 1 || sg.Spec.WorkloadSelector[0] != "tier:frontend" ||
		len(sg.Labels) != 2 || sg.Labels["one"] != "two" || sg.Labels["three"] != "four" {
		t.Fatalf("Edit operation failed: %+v \n", sg)
	}

	sg.Labels["five"] = "six"
	b, err := json.Marshal(sg)
	if err != nil {
		t.Fatalf("Unable to marshal node object %+v", sg)
	}
	ioutil.WriteFile("tmp-3772.json", b, 0644)
	defer os.RemoveAll("tmp-3772.json")
	veniceCLI("update securityGroup -f tmp-3772.json")

	out = veniceCLI("read securityGroup editsg")
	sg = &network.SecurityGroup{}
	if err := json.Unmarshal([]byte(out), sg); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sg.ResourceVersion != "3" || len(sg.Spec.WorkloadSelector) != 1 || sg.Spec.WorkloadSelector[0] != "tier:frontend" ||
		len(sg.Labels) != 3 || sg.Labels["one"] != "two" || sg.Labels["three"] != "four" ||
		sg.Labels["five"] != "six" {
		t.Fatalf("Edit operation failed: %+v \n", sg)
	}

	// edit from pre-baked file
	veniceCLI("edit user")
	out = veniceCLI("read user joe")
	user := &api.User{}
	if err := json.Unmarshal([]byte(out), user); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if user.Labels["dept"] != "eng" {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	// resotre the old editor, if any
	os.Setenv("VENICE_EDITOR", oldEditor)
}

func TestTree(t *testing.T) {
	out := veniceCLI("tree")
	if !strings.Contains(out, "+node") {
		t.Fatalf("unable to find node within the tree")
	}
	if !strings.Contains(out, "vm233") {
		t.Fatalf("unable to find specific node within the tree")
	}
}

func TestExample(t *testing.T) {
	out := veniceCLI("example network")
	if !strings.Contains(out, "IPv4Subnet: string") {
		t.Fatalf("example verb output invalid: %s", out)
	}
	out = veniceCLI("read user --example")
	if !strings.Contains(out, "read all users") {
		t.Fatalf("example flag in read command invalid: %s", out)
	}
}

func getEmptyMap(fields []string) map[string]bool {
	fmap := make(map[string]bool)
	for _, field := range fields {
		fmap[field] = false
	}

	return fmap
}

func isMapFull(fmap map[string]bool) bool {
	for _, v := range fmap {
		if v == false {
			return false
		}
	}
	return true
}

func matchLineFields(out string, fields []string) bool {
	lines := strings.Split(out, "\n")
	for _, line := range lines {
		words := strings.Split(line, " ")
		fmap := getEmptyMap(fields)
		for _, word := range words {
			if _, ok := fmap[word]; ok {
				fmap[word] = true
			}
		}
		if isMapFull(fmap) {
			return true
		}
	}

	return false
}

func TestList(t *testing.T) {
	veniceCLI("create node --label vCenter:vc2 --roles 2 test1")
	veniceCLI("create node --label vCenter:vc2 --roles 1 test2")
	out := veniceCLI("read node")
	if !matchLineFields(out, []string{"test1", "vCenter:vc2", "test1", "QUORUM_NODE"}) {
		t.Fatalf("Invalid list:\n%s", out)
	}

	veniceCLI("create tenant test1 --adminUser test1-admin")
	veniceCLI("create tenant test2 --adminUser test2-admin")
	out = veniceCLI("read tenant")
	if !matchLineFields(out, []string{"test1", "test1-admin"}) {
		t.Fatalf("Invalid list:\n%s", out)
	}

	veniceCLI("create network test1 --label dhcp:yes --iPv4Subnet 13.1.1.0/24 --iPv4Gateway 13.1.1.254 --type vlan")
	veniceCLI("create network test2 --label dhcp:no --label level:prod --iPv4Subnet 13.1.2.0/24 --iPv4Gateway 13.1.2.254 --type vlan")
	veniceCLI("create network test3 --label dhcp:no --iPv4Subnet 13.1.3.0/24 --iPv4Gateway 13.1.3.254 --type vlan")
	out = veniceCLI("read network")
	if !matchLineFields(out, []string{"test2", "dhcp:no", "13.1.2.0/24", "13.1.2.254", "vlan"}) {
		t.Fatalf("Invalid list:\n%s", out)
	}
	if !matchLineFields(out, []string{"level:prod"}) {
		t.Fatalf("Invalid list:\n%s", out)
	}
}

func TestDeleteList(t *testing.T) {
	veniceCLI("create securityGroup test1 --label key1:label1 --label key2:label2 --workloadSelector tier:frontend")
	veniceCLI("create securityGroup test2 --label key3:label3 --label key2:label2 --workloadSelector tier:backend")
	veniceCLI("create securityGroup test3 --label key1:label1 --label key4:label4 --workloadSelector tier:app")

	veniceCLI("delete securityGroup --label key2:label2")
	out := veniceCLI("read securityGroup")
	if matchLineFields(out, []string{"key2:label2"}) {
		t.Fatalf("Unable to delete by label:\n%s", out)
	}
}

func TestMultiRead(t *testing.T) {
	veniceCLI("create endpoint --label dmz:yes --label dc:lab-22 --label tier:frontend testep-vm23")
	veniceCLI("create endpoint --label dmz:no --label dc:lab-22 --label tier:frontend testep-vm24")
	veniceCLI("create endpoint --label dmz:no --label dc:lab-22 --label tier:frontend blah-vm25")
	veniceCLI("create endpoint --label dmz:yes --label dc:lab-22 --label tier:frontend blah-vm26")

	out := veniceCLI("read endpoint -q")
	if !matchLineFields(out, []string{"testep-vm23"}) || !matchLineFields(out, []string{"testep-vm24"}) ||
		!matchLineFields(out, []string{"blah-vm25"}) || !matchLineFields(out, []string{"blah-vm26"}) {
		t.Fatalf("Unable to read objects in quiet mode:\n%s", out)
	}

	out = veniceCLI("read endpoint --label dmz:no")
	if matchLineFields(out, []string{"dmz:yes"}) {
		t.Fatalf("Unable to filter reads based on labels:\n%s", out)
	}

	out = veniceCLI("read endpoint --re testep*")
	if matchLineFields(out, []string{"blah-vm25"}) || matchLineFields(out, []string{"blah-vm26"}) {
		t.Fatalf("Unable to match regular expression based reads:\n%s", out)
	}

	out = veniceCLI("read endpoint testep-vm23,blah-vm25")
	if matchLineFields(out, []string{"testep-vm24"}) || matchLineFields(out, []string{"blah-vm26"}) {
		t.Fatalf("Unable to read multiple records based on names:\n%s", out)
	}

	veniceCLI("create lbPolicy mr-lbp1")
	veniceCLI("create lbPolicy mr-lbp2")
	out = veniceCLI("read lbPolicy")
	if !matchLineFields(out, []string{"mr-lbp1"}) {
		t.Fatalf("unable to read record in multi-reads: %s", out)
	}

	veniceCLI("create sgpolicy mr-sgp1")
	veniceCLI("create sgpolicy mr-sgp2")
	out = veniceCLI("read sgpolicy")
	if !matchLineFields(out, []string{"mr-sgp1"}) {
		t.Fatalf("unable to read record in multi-reads: %s", out)
	}

	veniceCLI("create user mr-user1 --roles mr-role1")
	veniceCLI("create user mr-user2 --roles mr-role2")
	out = veniceCLI("read user")
	if !matchLineFields(out, []string{"mr-user1"}) {
		t.Fatalf("unable to read record in multi-reads: %s", out)
	}

	veniceCLI("create role mr-role1 --permissions mr-perm1")
	veniceCLI("create role mr-role2 --permissions mr-perm2")
	out = veniceCLI("read role")
	if !matchLineFields(out, []string{"mr-perm1"}) {
		t.Fatalf("unable to read record in multi-reads: %s", out)
	}

	veniceCLI("create permission mr-perm1")
	veniceCLI("create permission mr-perm2")
	out = veniceCLI("read permission")
	if !matchLineFields(out, []string{"mr-perm2"}) {
		t.Fatalf("unable to read record in multi-reads: %s", out)
	}
}

func TestLabelCommand(t *testing.T) {
	veniceCLI("create service testsvc1 --virtualIp 155.12.12.1 --ports 8001 --workloadSelector tier:frontend")
	veniceCLI("create service testsvc2 --label auth:ldap --virtualIp 155.12.12.2 --ports 8002 --workloadSelector tier:app")
	veniceCLI("create service testsvc3 --label auth:ldap --virtualIp 155.12.12.3 --ports 8003 --workloadSelector tier:db")
	veniceCLI("create service testsvc4 --virtualIp 155.12.12.4 --ports 8004 --workloadSelector tier:db")

	veniceCLI("label service --update-label dmz:yes testsvc1")
	out := veniceCLI("read service")
	if !matchLineFields(out, []string{"testsvc1", "155.12.12.1", "8001", "tier:frontend", "dmz:yes"}) {
		t.Fatalf("Unable to label objects:\n%s", out)
	}
	veniceCLI("label service --update-label -auth:ldap testsvc2")
	out = veniceCLI("read service")
	if matchLineFields(out, []string{"testsvc2", "auth:ldap"}) {
		t.Fatalf("Unable to un-label objects:\n%s", out)
	}

	veniceCLI("label service --label auth:ldap --update-label auth:passwd")
	out = veniceCLI("read service")
	if matchLineFields(out, []string{"testsvc2", "auth:ldap"}) || matchLineFields(out, []string{"testsvc3", "auth:ldap"}) {
		t.Fatalf("Unable to label multiple objects:\n%s", out)
	}
}

func TestShowVersion(t *testing.T) {
	out := veniceCLI("version")
	if !strings.Contains(out, "Client Version") || !strings.Contains(out, "Server Version") {
		t.Fatalf("Version command output invalid: %s", out)
	}
}

func TestSnapshot(t *testing.T) {

	// create some objects
	veniceCLI("create tenant snap-tenant --adminUser snap-admin")
	veniceCLI("create lbPolicy --algorithm llatency --probePortOrUrl /healthStatus --sessionAffinity yes --type l4 --interval 5 --maxTimeouts 25 snap-lbpolicy")
	veniceCLI("create node --label vCenter:vc2 --roles 1 snap-node")

	// take a snapshot
	out := veniceCLI("snapshot -f snap3443")
	defer os.RemoveAll("snap3443")
	if !strings.Contains(out, "Successful - stored snapshot") {
		t.Fatalf("unable to store snapshot: %s", out)
	}

	// read the snapshot objects to confirm they look as per configuration
	b, err := ioutil.ReadFile("./snap3443/tenants/snap-tenant.json")
	if err != nil {
		t.Fatalf("Error reading file contents: %s", err)
	}
	tenant := &network.Tenant{}
	if err := json.Unmarshal([]byte(b), tenant); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if tenant.Spec.AdminUser != "snap-admin" {
		t.Fatalf("Unable to decode object: %+v", tenant)
	}

	b, err = ioutil.ReadFile("./snap3443/lbPolicys/snap-lbpolicy.json")
	if err != nil {
		t.Fatalf("Error reading file contents: %s", err)
	}
	lbp := &network.LbPolicy{}
	if err := json.Unmarshal([]byte(b), lbp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if lbp.Spec.HealthCheck.ProbePortOrUrl != "/healthStatus" || lbp.Spec.HealthCheck.Interval != 5 ||
		lbp.Spec.SessionAffinity != "yes" || lbp.Spec.Type != "l4" || lbp.Spec.Algorithm != "llatency" {
		t.Fatalf("Unable to decode object: %+v", tenant)
	}

	b, err = ioutil.ReadFile("./snap3443/nodes/snap-node.json")
	if err != nil {
		t.Fatalf("Error reading file contents: %s", err)
	}
	node := &cmd.Node{}
	if err := json.Unmarshal([]byte(b), node); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if node.Labels["vCenter"] != "vc2" || node.Spec.Roles[0] != 1 {
		t.Fatalf("Unable to decode object: %+v \n", node)
	}

	// modify some objects and write them back
	node.Labels["vCenter"] = "modified-vc1"
	b, err = json.Marshal(node)
	if err != nil {
		t.Fatalf("Unable to marshal node object %+v", node)
	}
	ioutil.WriteFile("./snap3443/nodes/snap-node.json", b, 0644)

	// restore from snapshot
	out = veniceCLI("snapshot --restore --id snap3443")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("snapshot restore returned some junk: %s", out)
	}

	// now read the objects and confirm if modified objects work as expected
	out = veniceCLI("read node snap-node")
	node = &cmd.Node{}
	if err := json.Unmarshal([]byte(out), node); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if node.Labels["vCenter"] != "modified-vc1" || node.Spec.Roles[0] != 1 {
		t.Fatalf("Restore object different from expectation: %+v \n", node)
	}
}

func TestDefinition(t *testing.T) {
	out := veniceCLI("definition securityGroup")
	if !strings.Contains(out, "WorkloadSelector: []string") {
		t.Fatalf("Unable to read multiple records based on names:\n%s", out)
	}

	out = veniceCLI("read securityGroup --show-definition")
	if !strings.Contains(out, "WorkloadSelector: []string") {
		t.Fatalf("Unable to read multiple records based on names:\n%s", out)
	}
}

func TestSecurityGroupDelete(t *testing.T) {
	out := veniceCLI("delete securityGroup sg10")
	sg := &network.SecurityGroup{}
	if err := json.Unmarshal([]byte(out), sg); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sg.ResourceVersion != "3" || len(sg.Spec.WorkloadSelector) != 1 || len(sg.Spec.MatchPrefixes) != 1 ||
		sg.Spec.WorkloadSelector[0] != "key3:val3" || sg.Spec.MatchPrefixes[0] != "12.1.1.0/22" {
		t.Fatalf("Delete operation failed: %+v \n", sg)
	}
}

func TestSecurityGroupPolicyDelete(t *testing.T) {
	out := veniceCLI("delete sgpolicy sg10-ingress")
	sgp := &network.Sgpolicy{}
	if err := json.Unmarshal([]byte(out), sgp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if sgp.ResourceVersion != "4" || len(sgp.Spec.AttachGroups) != 1 || len(sgp.Spec.InRules) != 2 ||
		sgp.Spec.AttachGroups[0] != "sg30" ||
		sgp.Spec.InRules[0].Ports != "tcp/4550" || sgp.Spec.InRules[0].Action != "permit" || sgp.Spec.InRules[0].PeerGroup != "sg20" ||
		sgp.Spec.InRules[1].Ports != "tcp/8440" || sgp.Spec.InRules[1].Action != "permit" || sgp.Spec.InRules[1].PeerGroup != "sg10" {
		t.Fatalf("Delete operation failed: %+v \n", sgp)
	}

	out = veniceCLI("read sgpolicy sg10-ingress")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}

func TestServiceDelete(t *testing.T) {
	out := veniceCLI("delete service uService122")
	svc := &network.Service{}
	if err := json.Unmarshal([]byte(out), svc); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if svc.ResourceVersion != "3" || svc.Spec.VirtualIp != "12.1.1.123" || len(svc.Spec.WorkloadSelector) != 1 ||
		svc.Spec.WorkloadSelector[0] != "tier:frontend" || svc.Spec.Ports != "8080" || svc.Spec.LBPolicy != "prod-lb-policy" {
		t.Fatalf("Delete operation failed: %+v \n", svc)
	}

	out = veniceCLI("read service uService122")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}

func TestLbPolicyDelete(t *testing.T) {
	out := veniceCLI("delete lbPolicy dev-lb-policy")
	lbp := &network.LbPolicy{}
	if err := json.Unmarshal([]byte(out), lbp); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if lbp.ResourceVersion != "3" || lbp.Spec.HealthCheck.ProbePortOrUrl != "/healthStatus" ||
		lbp.Spec.SessionAffinity != "yes" || lbp.Spec.Type != "l4" || lbp.Spec.Algorithm != "least-latency" {
		t.Fatalf("Update operation failed: %+v \n", lbp)
	}

	out = veniceCLI("read lbPolicy dev-lb-policy")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}

func TestEndpointDelete(t *testing.T) {
	out := veniceCLI("delete endpoint vm23")
	ep := &network.Endpoint{}
	if err := json.Unmarshal([]byte(out), ep); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}

	lv1, ok1 := ep.Labels["dmz"]
	lv2, ok2 := ep.Labels["dc"]
	lv3, ok3 := ep.Labels["tier"]
	if ep.ResourceVersion != "3" || len(ep.Labels) != 3 ||
		!ok1 || lv1 != "no" || !ok2 || lv2 != "lab-22" || !ok3 || lv3 != "frontend" {
		t.Fatalf("Update endpoint failed: %+v \n", ep)
	}

	out = veniceCLI("delete endpoint non-existent")
	if !fmtOutput && !strings.Contains(out, "Error deleting endpoint") {
		t.Fatalf("Unable to get error on non existent object: %s\n", out)
	}
}

func TestPermissionDelete(t *testing.T) {
	out := veniceCLI("delete permission network-objs-rw")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("delete permission: garbled output '%s'", out)
	}

	out = veniceCLI("read permission network-objs-rw")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("delete permission: garbled output '%s'", out)
	}
}

func TestRoleDelete(t *testing.T) {
	out := veniceCLI("delete role network-admin")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read role network-admin")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update endpoint: garbled output '%s'", out)
	}
}

func TestUserDelete(t *testing.T) {
	out := veniceCLI("delete user bot-bot")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update endpoint: garbled output '%s'", out)
	}

	out = veniceCLI("read user bot-bot")
	if !fmtOutput && strings.TrimSpace(out) != "" {
		t.Fatalf("update endpoint: garbled output '%s'", out)
	}
}

func TestNetworkDelete(t *testing.T) {
	out := veniceCLI("delete network lab22-net145")
	net := &network.Network{}
	if err := json.Unmarshal([]byte(out), net); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if net.ResourceVersion != "3" || net.Spec.IPv4Subnet != "145.1.1.0/24" || net.Spec.IPv4Gateway != "145.1.1.254" ||
		net.Spec.Type != "vxlan" || net.Spec.IPv6Subnet != "2001:db8::0/64" || net.Spec.IPv6Gateway != "2001:db8::1" {
		t.Fatalf("Delete operation failed: %+v \n", net)
	}

	out = veniceCLI("read network lab22-net145")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}

func TestTenantDelete(t *testing.T) {
	out := veniceCLI("delete tenant newco")
	net := &network.Tenant{}
	if err := json.Unmarshal([]byte(out), net); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if net.ResourceVersion != "3" || net.Spec.AdminUser != "newco-adm" {
		t.Fatalf("Delete operation failed: %+v \n", net)
	}

	out = veniceCLI("read tenant newco")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}

func TestNodeDelete(t *testing.T) {
	out := veniceCLI("delete node vm233")

	node := &cmd.Node{}
	if err := json.Unmarshal([]byte(out), node); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if node.Labels["vCenter"] != "vc2" || node.Spec.Roles[0] != 3 {
		t.Fatalf("Update operation failed: %+v \n", node)
	}

	out = veniceCLI("read node vm233")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}

func TestClusterDelete(t *testing.T) {
	out := veniceCLI("delete cluster dc-az-cluster1")

	cluster := &cmd.Cluster{}
	if err := json.Unmarshal([]byte(out), cluster); err != nil {
		t.Fatalf("Unmarshling error: %s\nRec: %s\n", err, out)
	}
	if len(cluster.Spec.NTPServers) != 2 ||
		cluster.Spec.NTPServers[0] != "ntp-svr1" || cluster.Spec.NTPServers[1] != "ntp-svr2" ||
		cluster.Labels["type"] != "vcenter" || cluster.Spec.VirtualIP != "151.21.43.44" {
		t.Fatalf("Update operation failed: %+v\n", cluster)
	}

	out = veniceCLI("read cluster dc-az-cluster1")
	if strings.TrimSpace(out) != "" {
		t.Fatalf("read object after delete: %s", out)
	}
}
