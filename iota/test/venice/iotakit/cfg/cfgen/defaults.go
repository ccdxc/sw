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
		NumNetworks: 8,
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
				VlanID:      98,
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
				DSCs: []cluster.DistributedServiceCardID{},
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
				SessionIdleTimeout:        "30s",
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

	RoutingConfigParams: RoutingConfigParams{
		NumRoutingConfigs: 1,
		RoutingConfigTemplate: &network.RoutingConfig{
			TypeMeta: api.TypeMeta{Kind: "RoutingConfig"},
			ObjectMeta: api.ObjectMeta{
				Name: "RRConfig-node-{{iter}}",
			},
			Spec: network.RoutingConfigSpec{
				BGPConfig: &network.BGPConfig{
					ASNumber:  100,
					RouterId:  "{{ipv4:10.1.1.0}}",
					Neighbors: []*network.BGPNeighbor{},
				},
			},
		},
		BgpNeihbourTemplate: &network.BGPNeighbor{
			EnableAddressFamilies: []string{"l2vpn-evpn"},
			RemoteAS:              100,
			MultiHop:              10,
			IPAddress:             "{{ipv4:22.x.1.1}}",
		},
	},

	UnderlayRoutingConfigParams: UnderlayRoutingConfigParams{
		NumUnderlayRoutingConfigs: 1,
		UnderlayRoutingConfigTemplate: &network.RoutingConfig{
			TypeMeta: api.TypeMeta{Kind: "RoutingConfig"},
			ObjectMeta: api.ObjectMeta{
				Name: "NaplesRRConfig-node-{{iter}}",
			},
			Spec: network.RoutingConfigSpec{
				BGPConfig: &network.BGPConfig{
					ASNumber:      100, //Static testbeds had this AS number
					DSCAutoConfig: true,
					Neighbors:     []*network.BGPNeighbor{},
				},
			},
		},
		UnderlayBgpNeihbourTemplate: &network.BGPNeighbor{
			RemoteAS:              600, //Static testbed has this AS number
			MultiHop:              10,
			DSCAutoConfig:         true,
			EnableAddressFamilies: []string{"ipv4-unicast"},
		},
		OverlayBgpNeihbourTemplate: &network.BGPNeighbor{
			RemoteAS:              100, //Static testbed has this AS number
			MultiHop:              10,
			DSCAutoConfig:         true,
			EnableAddressFamilies: []string{"l2vpn-evpn"},
		},
	},

	TenantConfigParams: TenantConfigParams{
		NumOfTenants: 1,
		TenantTemplate: &cluster.Tenant{
			TypeMeta: api.TypeMeta{Kind: "Tenant", APIVersion: "v1"},
			ObjectMeta: api.ObjectMeta{
				Name: "customer{{iter}}",
			},
			Spec: cluster.TenantSpec{},
		},
	},

	VRFConfigParams: VRFConfigParams{
		NumOfVRFs: 1,
		VirtualRouterTemplate: &network.VirtualRouter{
			TypeMeta: api.TypeMeta{Kind: "VirtualRouter"},
			ObjectMeta: api.ObjectMeta{
				Name: "vpc{{iter}}",
			},
			Spec: network.VirtualRouterSpec{
				Type:              "tenant",
				RouterMACAddress:  "{{mac}}",
				VxLanVNI:          998,
				DefaultIPAMPolicy: "vpc1DHCP0",
				RouteImportExport: &network.RDSpec{
					AddressFamily: "l2vpn-evpn",
					RDAuto:        true,
					ImportRTs: []*network.RouteDistinguisher{
						&network.RouteDistinguisher{
							Type:          "type2",
							AdminValue:    100,
							AssignedValue: 100,
						},
					},
					ExportRTs: []*network.RouteDistinguisher{
						&network.RouteDistinguisher{
							Type:          "type2",
							AdminValue:    200,
							AssignedValue: 100,
						},
						&network.RouteDistinguisher{
							Type:          "type2",
							AdminValue:    200,
							AssignedValue: 101,
						},
					},
				},
			},
		},
	},

	IPAMPConfigParams: IPAMPConfigParams{
		NumOfIPAMPs: 1,
		IPAMPPolicyTemplate: &network.IPAMPolicy{
			TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
			ObjectMeta: api.ObjectMeta{
				Name: "vpc1DHCP{{iter}}",
			},
			Spec: network.IPAMPolicySpec{
				Type: "dhcp-relay",
				DHCPRelay: &network.DHCPRelayPolicy{
					Servers: []*network.DHCPServer{
						&network.DHCPServer{
							VirtualRouter: "",
							IPAddress:     "20.20.{testbed}.1",
						},
					},
				},
			},
		},
	},

	SubnetConfigParams: SubnetConfigParams{
		NumOfSubnets: 1,
		SubnetTemplate: &network.Network{
			TypeMeta: api.TypeMeta{Kind: "Network"},
			ObjectMeta: api.ObjectMeta{
				Name: "network{{iter}}",
			},
			Spec: network.NetworkSpec{
				Type:        "routed",
				IPv4Subnet:  "{{ipv4-subnet:10.x.0.0/24}}",
				IPv4Gateway: "{{ipv4-gateway:10.x.0.1}}",
				VxlanVNI:    998,
				RouteImportExport: &network.RDSpec{
					AddressFamily: "l2vpn-evpn",
					RDAuto:        true,
					ExportRTs: []*network.RouteDistinguisher{
						&network.RouteDistinguisher{
							AdminValue:    998,
							Type:          "type2",
							AssignedValue: 998,
						},
					},
					ImportRTs: []*network.RouteDistinguisher{
						&network.RouteDistinguisher{
							AdminValue:    998,
							Type:          "type2",
							AssignedValue: 998,
						},
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
