package cfgen

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/workload"
)

// DefaultCfgenParams contains default config gen params
var DefaultCfgenParams = scaleCfgenParams

var scaleCfgenParams = &Cfgen{
	UserParams: UserParams{},
	RoleParams: RoleParams{},
	NetworkParams: NetworkParams{
		NumNetworks: 4,
		NetworkTemplate: &network.Network{
			TypeMeta: api.TypeMeta{Kind: "Network"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "Network-Vlan-{{iter-net:1-10000}}",
			},
			Spec: network.NetworkSpec{
				Type:        network.NetworkType_Bridged.String(),
				IPv4Subnet:  "{{ipv4-subnet:22.x.0.0/16}}",
				IPv4Gateway: "{{ipv4-gateway:22.x.0.254}}",
				VlanID:      998,
			},
		},
	},
	HostParams: HostParams{
		HostTemplate: &cluster.Host{
			TypeMeta: api.TypeMeta{Kind: "Host"},
			ObjectMeta: api.ObjectMeta{
				Name: "host-{{iter:hostid}}",
			},
			Spec: cluster.HostSpec{
				DSCs: []cluster.DistributedServiceCardID{
					{
						MACAddress: "{{mac}}",
					},
				},
			},
		},
	},
	WorkloadParams: WorkloadParams{
		WorkloadsPerHost: 20,
		WorkloadTemplate: &workload.Workload{
			TypeMeta: api.TypeMeta{Kind: "Workload"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "workload-host{{iter-hostid}}-w{{iter-worloadid}}",
			},
			Spec: workload.WorkloadSpec{
				HostName: "host-{{iter:hostid}}",
				Interfaces: []workload.WorkloadIntfSpec{
					workload.WorkloadIntfSpec{
						ExternalVlan: 998,
						MicroSegVlan: 998,
						MACAddress:   "{{mac}}",
						IpAddresses:  []string{"{{ipv4:22.x.x.0}}"},
					},
				},
			},
		},
	},
	NetworkSecurityPolicyParams: NetworkSecurityPolicyParams{
		NumPolicies:       1,
		NumRulesPerPolicy: 10000,
		NetworkSecurityPolicyTemplate: &security.NetworkSecurityPolicy{
			TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "sgpolicy-{{iter}}",
			},
			Spec: security.NetworkSecurityPolicySpec{
				AttachTenant: true,
			},
		},
		SGRuleTemplate: &security.SGRule{
			Action:          "{{rand:PERMIT|DENY|REJECT}}",
			FromIPAddresses: []string{},
			ToIPAddresses:   []string{},
			ProtoPorts:      []security.ProtoPort{{Protocol: "{{rand:tcp|udp}}", Ports: "{{iter-portnum:5000-50000}}"}},
			//Apps:            []string{"app-{{iter-appid:1-20000}}"},
		},
	},
	FirewallProfileParams: FirewallProfileParams{
		FirewallProfileTemplate: &security.FirewallProfile{
			TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
			ObjectMeta: api.ObjectMeta{
				Name:      "default",
				Namespace: "default",
				Tenant:    "default",
			},
			// FIXME: set short timeout value till HAL fixes the drop->allow transition issue
			Spec: security.FirewallProfileSpec{
				SessionIdleTimeout:        "10s",
				TCPConnectionSetupTimeout: "10s",
				TCPCloseTimeout:           "10s",
				TCPHalfClosedTimeout:      "10s",
				TCPDropTimeout:            "10s",
				UDPDropTimeout:            "10s",
				DropTimeout:               "10s",
				TcpTimeout:                "10s",
				UdpTimeout:                "10s",
				ICMPDropTimeout:           "10s",
				IcmpTimeout:               "10s",
			},
		},
	},
	AppParams: AppParams{
		NumApps: 1000,
		AppTemplate: &security.App{
			TypeMeta: api.TypeMeta{Kind: "App"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "app-{{iter-appid:1-20000}}",
			},
			Spec: security.AppSpec{
				ProtoPorts: []security.ProtoPort{{Protocol: "{{rand:tcp|udp}}", Ports: "{{rand:10000-50000}}"}},
			},
		},
		NumDNSAlgs: 100,
		DNSAlgTemplate: &security.App{
			TypeMeta: api.TypeMeta{Kind: "App"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "app-{{iter}}",
			},
			Spec: security.AppSpec{
				ProtoPorts: []security.ProtoPort{{Protocol: "udp", Ports: "53"}},
				ALG: &security.ALG{
					Type: "DNS",
					Dns: &security.Dns{
						DropMultiQuestionPackets: true,
						MaxMessageLength:         9999,
						QueryResponseTimeout:     "{{rand:10-50}}ms",
					},
				},
			},
		},
	},
	SecurityGroupsParams:   SecurityGroupsParams{},
	MirrorSessionParams:    MirrorSessionParams{},
	FwLogPolicyParams:      FwLogPolicyParams{},
	FlowExportPolicyParams: FlowExportPolicyParams{},
}

var smartNicTemplate = &cluster.DistributedServiceCard{
	TypeMeta: api.TypeMeta{
		Kind: "DistributedServiceCard",
	},
	ObjectMeta: api.ObjectMeta{
		Name: "{{mac:00.ac.dd}}",
	},
	Spec: cluster.DistributedServiceCardSpec{
		ID: "host-{{iter}}",
		IPConfig: &cluster.IPConfig{
			IPAddress: "0.0.0.0/0",
		},
		MgmtMode:    "NETWORK",
		NetworkMode: "OOB",
	},
	Status: cluster.DistributedServiceCardStatus{
		AdmissionPhase: "ADMITTED",
		PrimaryMAC:     "{{mac:00.ac.dd}}",
	},
}
