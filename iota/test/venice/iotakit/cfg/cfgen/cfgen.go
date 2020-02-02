package cfgen

import (
	"fmt"
	"math/rand"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
)

// UserParams contains user params
type UserParams struct {
	NumUsers        int // number of users configured
	NumRolesPerUser int //number of roles per user
	UserTemplate    *auth.User
}

// RoleParams contains role params
type RoleParams struct {
	NumRoles            int // number of roles configured
	RoleTemplate        auth.Role
	RoleBindingTemplate *auth.RoleBinding // for now, we create one role-binding per role
}

// NetworkParams network attributes
type NetworkParams struct {
	NumNetworks     int // specifies number of networks to be created
	NetworkTemplate *network.Network
}

// HostParams host attributes
type HostParams struct {
	HostTemplate *cluster.Host
}

// WorkloadParams workload attributes
type WorkloadParams struct {
	WorkloadsPerHost int // specifies number of workloads to be created per host
	WorkloadTemplate *workload.Workload
}

// WPair workload pairs
type WPair struct {
	From *workload.Workload
	To   *workload.Workload
}

// NetworkSecurityPolicyParams sg policy params
type NetworkSecurityPolicyParams struct {
	NumPolicies        int    // specifies number of policies
	NumRulesPerPolicy  int    // specifies number of rules per policy
	NumIPPairsPerRule  int    // specfifies number of IP pairs per rule
	NumAppsPerRules    int    // specfifies number of Apps per rule
	DefaultAllowPolicy string // creates default policy to allow all traffic
	NetworkSecurityPolicyTemplate   *security.NetworkSecurityPolicy
	SGRuleTemplate     *security.SGRule
}

// FirewallProfileParams firewall profile params
type FirewallProfileParams struct {
	FirewallProfileTemplate *security.FirewallProfile
}

// AppParams app object params
type AppParams struct {
	NumApps        int // specifies number of app objects; app objects are sequentially added to sgpolicies
	NumDNSAlgs     int // specifies number of dns alg app objects; app objects are sequentially added to sgpolicies
	AppTemplate    *security.App
	DNSAlgTemplate *security.App
}

// SecurityGroupsParams security group params
type SecurityGroupsParams struct {
	NumSGs                int // specifies number of security-groups
	SecurityGroupTemplate security.SecurityGroup
}

// MirrorSessionParams mirror session params
type MirrorSessionParams struct {
	NumSessionMirrors     int                      // number of mirror sessions to be configured within the system
	MirrorSessionTemplate monitoring.MirrorSession // will match random set of workloads (those can be queried)
}

// FwLogPolicyParams firewall log params
type FwLogPolicyParams struct {
	NumFwLogPolicies    int
	FwLogPolicyTemplate monitoring.FwlogPolicy
}

// FlowExportPolicyParams flow export params
type FlowExportPolicyParams struct {
	NumFlowExportPolicies    int
	FlowExportPolicyTemplate monitoring.FlowExportPolicy
}

// CfgItems items to configure
type CfgItems struct {
	Networks   []*network.Network   `json:"Networks"`
	Hosts      []*cluster.Host      `json:"Hosts"`
	Workloads  []*workload.Workload `json:"Workloads"`
	SGPolicies []*security.NetworkSecurityPolicy `json:"SGPolicies"`
	Apps       []*security.App      `json:"Apps"`
}

// Cfgen config gen params
type Cfgen struct {
	UserParams
	RoleParams
	NetworkParams
	HostParams
	WorkloadParams
	NetworkSecurityPolicyParams
	FirewallProfileParams
	AppParams
	SecurityGroupsParams
	MirrorSessionParams
	FwLogPolicyParams
	FlowExportPolicyParams

	// user also inputs the SmartNICs in the cluster
	Smartnics []*cluster.DistributedServiceCard

	// generated objects
	WPairs      []*WPair
	ConfigItems CfgItems

	Fwprofile *security.FirewallProfile
}

// Do does
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
	netCtx := newIterContext()
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
	hostCtx := newIterContext()
	for ii := 0; ii < len(cfgen.Smartnics); ii++ {
		tHost := hostCtx.transform(h).(*cluster.Host)
		tHost.Spec.DSCs[0].MACAddress = cfgen.Smartnics[ii].Status.PrimaryMAC

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
		nIters[netIdx] = newIterContext()
	}

	w := cfgen.WorkloadParams.WorkloadTemplate
	wCtx := newIterContext()
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

func (cfgen *Cfgen) genSGPolicies() []*security.NetworkSecurityPolicy {
	sgpolicies := []*security.NetworkSecurityPolicy{}
	//Ignore apps for now
	//cfgen.NetworkSecurityPolicyParams.SGRuleTemplate.Apps = []string{fmt.Sprintf("app-{{iter-appid:1-%d}}", cfgen.AppParams.NumApps)}

	sgp := cfgen.NetworkSecurityPolicyParams.NetworkSecurityPolicyTemplate

	sgpCtx := newIterContext()
	for ii := 0; ii < cfgen.NetworkSecurityPolicyParams.NumPolicies; ii++ {
		rule := cfgen.NetworkSecurityPolicyParams.SGRuleTemplate
		tSgp := sgpCtx.transform(sgp).(*security.NetworkSecurityPolicy)

		rules := []security.SGRule{}
		ruleCtx := newIterContext()
		for jj := 0; jj < cfgen.NetworkSecurityPolicyParams.NumRulesPerPolicy; jj++ {
			tRule := ruleCtx.transform(rule).(*security.SGRule)

			for kk := 0; kk < cfgen.NetworkSecurityPolicyParams.NumIPPairsPerRule; kk++ {
				fromIPIdx := rand.Intn(len(cfgen.ConfigItems.Workloads))
				toIPIdx := rand.Intn(len(cfgen.ConfigItems.Workloads))
				fromW := cfgen.ConfigItems.Workloads[fromIPIdx]
				toW := cfgen.ConfigItems.Workloads[toIPIdx]
				tRule.FromIPAddresses = append(tRule.FromIPAddresses, fromW.Spec.Interfaces[0].IpAddresses[0])
				tRule.ToIPAddresses = append(tRule.ToIPAddresses, toW.Spec.Interfaces[0].IpAddresses[0])
				cfgen.WPairs = append(cfgen.WPairs, &WPair{From: fromW, To: toW})
			}
			for ll := 0; ll < cfgen.NetworkSecurityPolicyParams.NumAppsPerRules; ll++ {
				port := strconv.Itoa(10000 + ll)
				tRule.ProtoPorts = append(tRule.ProtoPorts,
					security.ProtoPort{Protocol: "tcp", Ports: port})
			}
			rules = append(rules, *tRule)
		}
		tSgp.Spec.Rules = rules

		sgpolicies = append(sgpolicies, tSgp)
	}

	return sgpolicies
}

func (cfgen *Cfgen) genApps() []*security.App {
	apps := []*security.App{}
	ctx := newIterContext()

	for ii := 0; ii < cfgen.AppParams.NumDNSAlgs; ii++ {
		app := cfgen.AppParams.DNSAlgTemplate
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
	iter := newIterContext()
	fw = iter.transform(fw).(*security.FirewallProfile)
	return fw
}
