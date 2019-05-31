package cfgen

import (
	"fmt"
	"math/rand"
	"strings"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
)

type UserParams struct {
	NumUsers        int // number of users configured
	NumRolesPerUser int //number of roles per user
	UserTemplate    *auth.User
}
type RoleParams struct {
	NumRoles            int // number of roles configured
	RoleTemplate        auth.Role
	RoleBindingTemplate *auth.RoleBinding // for now, we create one role-binding per role
}
type NetworkParams struct {
	NumNetworks     int // specifies number of networks to be created
	NetworkTemplate *network.Network
}
type HostParams struct {
	HostTemplate *cluster.Host
}
type WorkloadParams struct {
	WorkloadsPerHost int // specifies number of workloads to be created per host
	WorkloadTemplate *workload.Workload
}
type WPair struct {
	From *workload.Workload
	To   *workload.Workload
}
type SGPolicyParams struct {
	NumPolicies        int    // specifies number of policies
	NumRulesPerPolicy  int    // specifies number of rules per policy
	DefaultAllowPolicy string // creates default policy to allow all traffic
	SGPolicyTemplate   *security.SGPolicy
	SGRuleTemplate     *security.SGRule
}
type FirewallProfileParams struct {
	FirewallProfileTemplate *security.FirewallProfile
}
type AppParams struct {
	NumApps        int // specifies number of app objects; app objects are sequentially added to sgpolicies
	NumDnsAlgs     int // specifies number of dns alg app objects; app objects are sequentially added to sgpolicies
	AppTemplate    *security.App
	DnsAlgTemplate *security.App
}
type SecurityGroupsParams struct {
	NumSGs                int // specifies number of security-groups
	SecurityGroupTemplate security.SecurityGroup
}
type MirrorSessionParams struct {
	NumSessionMirrors     int                      // number of mirror sessions to be configured within the system
	MirrorSessionTemplate monitoring.MirrorSession // will match random set of workloads (those can be queried)
}
type FwLogPolicyParams struct {
	NumFwLogPolicies    int
	FwLogPolicyTemplate monitoring.FwlogPolicy
}
type FlowExportPolicyParams struct {
	NumFlowExportPolicies    int
	FlowExportPolicyTemplate monitoring.FlowExportPolicy
}

type CfgItems struct {
	Networks   []*network.Network   `json:"Networks"`
	Hosts      []*cluster.Host      `json:"Hosts"`
	Workloads  []*workload.Workload `json:"Workloads"`
	SGPolicies []*security.SGPolicy `json:"SGPolicies"`
	Apps       []*security.App      `json:"Apps"`
}
type Cfgen struct {
	UserParams
	RoleParams
	NetworkParams
	HostParams
	WorkloadParams
	SGPolicyParams
	FirewallProfileParams
	AppParams
	SecurityGroupsParams
	MirrorSessionParams
	FwLogPolicyParams
	FlowExportPolicyParams

	// user also inputs the SmartNICs in the cluster
	Smartnics []*cluster.SmartNIC

	// generated objects
	WPairs      []*WPair
	ConfigItems CfgItems

	Fwprofile *security.FirewallProfile
}

func (cfgen *Cfgen) Do() {
	// make sure we only have as many hosts are SmartNICs provided to us
	if len(cfgen.Smartnics) == 0 {
		panic("user must provide smartnic objects to allow associating smartNIC macs with other objects")
	}

	// order the generation in specific way
	cfgen.ConfigItems.Networks = cfgen.genNetworks()
	cfgen.ConfigItems.Hosts = cfgen.genHosts()
	cfgen.ConfigItems.Workloads = cfgen.genWorkloads()
	cfgen.ConfigItems.Apps = cfgen.genApps()
	cfgen.ConfigItems.SGPolicies = cfgen.genSGPolicies()
}

func (cfgen *Cfgen) genNetworks() []*network.Network {
	networks := []*network.Network{}

	n := cfgen.NetworkParams.NetworkTemplate
	netCtx := NewIterContext()
	for ii := 0; ii < cfgen.NetworkParams.NumNetworks; ii++ {
		tNetwork := netCtx.transform(n).(*network.Network)

		networks = append(networks, tNetwork)
	}
	return networks
}

func (cfgen *Cfgen) genHosts() []*cluster.Host {
	hosts := []*cluster.Host{}

	h := cfgen.HostParams.HostTemplate
	h.ObjectMeta.Name = fmt.Sprintf("host-{{iter:1-%d}}", len(cfgen.Smartnics))
	hostCtx := NewIterContext()
	for ii := 0; ii < len(cfgen.Smartnics); ii++ {
		tHost := hostCtx.transform(h).(*cluster.Host)
		tHost.Spec.SmartNICs[0].MACAddress = cfgen.Smartnics[ii].Status.PrimaryMAC

		hosts = append(hosts, tHost)
	}
	return hosts
}

func (cfgen *Cfgen) genWorkloads() []*workload.Workload {
	workloads := []*workload.Workload{}

	subnets := make([]string, len(cfgen.ConfigItems.Networks))
	nIters := make([]*iterContext, len(cfgen.ConfigItems.Networks))
	for netIdx, n := range cfgen.ConfigItems.Networks {
		subnet := strings.Split(n.Spec.IPv4Subnet, "/")[0]
		subnets[netIdx] = "ipv4:" + strings.Replace(subnet, ".0", ".x", -1)
		nIters[netIdx] = NewIterContext()
	}

	w := cfgen.WorkloadParams.WorkloadTemplate
	wCtx := NewIterContext()
	for ii := 0; ii < len(cfgen.Smartnics); ii++ {
		h := cfgen.ConfigItems.Hosts[ii]
		w.Spec.HostName = h.ObjectMeta.Name
		for jj := 0; jj < cfgen.WorkloadParams.WorkloadsPerHost; jj++ {
			tWorkload := wCtx.transform(w).(*workload.Workload)
			tWorkload.ObjectMeta.Name = fmt.Sprintf("workload-%s-w%d", w.Spec.HostName, jj)

			// fix up the workload IP with that of a network it belongs to
			netIdx := rand.Intn(len(cfgen.ConfigItems.Networks))
			tWorkload.Spec.Interfaces[0].IpAddresses[0] = nIters[netIdx].ipSub(subnets[netIdx])
			tWorkload.Spec.Interfaces[0].MicroSegVlan = (uint32)(jj + 1)
			tWorkload.Spec.Interfaces[0].ExternalVlan = cfgen.ConfigItems.Networks[netIdx].Spec.VlanID

			workloads = append(workloads, tWorkload)
		}
	}
	return workloads
}

func (cfgen *Cfgen) genSGPolicies() []*security.SGPolicy {
	sgpolicies := []*security.SGPolicy{}
	//Ignore apps for now
	//cfgen.SGPolicyParams.SGRuleTemplate.Apps = []string{fmt.Sprintf("app-{{iter-appid:1-%d}}", cfgen.AppParams.NumApps)}

	sgp := cfgen.SGPolicyParams.SGPolicyTemplate

	sgpCtx := NewIterContext()
	for ii := 0; ii < cfgen.SGPolicyParams.NumPolicies; ii++ {
		rule := cfgen.SGPolicyParams.SGRuleTemplate
		tSgp := sgpCtx.transform(sgp).(*security.SGPolicy)

		rules := []security.SGRule{}
		ruleCtx := NewIterContext()
		for jj := 0; jj < cfgen.SGPolicyParams.NumRulesPerPolicy; jj++ {
			tRule := ruleCtx.transform(rule).(*security.SGRule)

			fromIpIdx := rand.Intn(len(cfgen.ConfigItems.Workloads))
			toIpIdx := rand.Intn(len(cfgen.ConfigItems.Workloads))
			fromW := cfgen.ConfigItems.Workloads[fromIpIdx]
			toW := cfgen.ConfigItems.Workloads[toIpIdx]
			tRule.FromIPAddresses = []string{fromW.Spec.Interfaces[0].IpAddresses[0]}
			tRule.ToIPAddresses = []string{toW.Spec.Interfaces[0].IpAddresses[0]}
			tRule.ProtoPorts = []security.ProtoPort{security.ProtoPort{Protocol: "tcp", Ports: "1000-65000"}}
			cfgen.WPairs = append(cfgen.WPairs, &WPair{From: fromW, To: toW})
			rules = append(rules, *tRule)
		}
		tSgp.Spec.Rules = rules

		sgpolicies = append(sgpolicies, tSgp)
	}

	return sgpolicies
}

func (cfgen *Cfgen) genApps() []*security.App {
	apps := []*security.App{}
	ctx := NewIterContext()

	for ii := 0; ii < cfgen.AppParams.NumDnsAlgs; ii++ {
		app := cfgen.AppParams.DnsAlgTemplate
		tApp := ctx.transform(app).(*security.App)
		apps = append(apps, tApp)
	}

	cfgen.AppParams.AppTemplate.ObjectMeta.Name = fmt.Sprintf("app-{{iter-appid:1-%d}}", cfgen.AppParams.NumApps)
	for ii := 0; ii < cfgen.AppParams.NumApps; ii++ {
		app := cfgen.AppParams.AppTemplate
		tApp := ctx.transform(app).(*security.App)
		apps = append(apps, tApp)
	}

	return apps
}

func (cfgen *Cfgen) genFirewallProfile() *security.FirewallProfile {
	if cfgen.FirewallProfileParams.FirewallProfileTemplate == nil {
		return nil
	}

	fw := cfgen.FirewallProfileParams.FirewallProfileTemplate
	iter := NewIterContext()
	fw = iter.transform(fw).(*security.FirewallProfile)
	return fw
}
