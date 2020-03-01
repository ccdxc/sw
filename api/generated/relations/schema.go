package relations

import apiintf "github.com/pensando/sw/api/interfaces"

type ObjRelation struct {
	Type  string
	To    string
	Field string
}

var ObjRelations = map[string][]apiintf.ObjRelation{
	"auth.RoleBindingSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "auth/User", Field: "Users"},
		{Type: "NamedRef", To: "auth/Role", Field: "Role"},
	},
	"bookstore.BookSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "bookstore/Publisher", Field: "Publisher"},
	},
	"bookstore.CustomerStatus": []apiintf.ObjRelation{
		{Type: "SelectorRef", To: "bookstore/Book", Field: "Interests"},
	},
	"bookstore.OrderItem": []apiintf.ObjRelation{
		{Type: "WeakRef", To: "bookstore/Book", Field: "Book"},
	},
	"cluster.DistributedServiceCardSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "cluster/DSCProfile", Field: "DSCProfile"},
	},
	"monitoring.AlertPolicySpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "monitoring/AlertDestination", Field: "Destinations"},
	},
	"network.DHCPServer": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/VirtualRouter", Field: "VirtualRouter"},
	},
	"network.NetworkInterfaceSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/Network", Field: "AttachNetwork"},
	},
	"network.NetworkSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/VirtualRouter", Field: "VirtualRouter"},
		{Type: "NamedRef", To: "network/IPAMPolicy", Field: "IPAMPolicy"},
		{Type: "NamedRef", To: "security/NetworkSecurityPolicy", Field: "IngressSecurityPolicy"},
		{Type: "NamedRef", To: "security/NetworkSecurityPolicy", Field: "EgressSecurityPolicy"},
	},
	"network.OrchestratorInfo": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "orchestration/Orchestrator", Field: "Name"},
	},
	"network.Route": []apiintf.ObjRelation{
		{Type: "WeakRef", To: "network/VirtualRouter", Field: "TargetVirtualRouter"},
	},
	"network.VirtualRouterSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/IPAMPolicy", Field: "DefaultIPAMPolicy"},
	},
	"network.VirtualRouterStatus": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/RouteTable", Field: "RouteTable"},
	},
	"security.NetworkSecurityPolicySpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "AttachGroups"},
	},
	"security.SGRule": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "security/App", Field: "Apps"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "FromSecurityGroups"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "ToSecurityGroups"},
	},
	"workload.EndpointStatus": []apiintf.ObjRelation{
		{Type: "WeakRef", To: "network/Network", Field: "Network"},
	},
	"workload.WorkloadIntfSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/Network", Field: "Network"},
	},
	"workload.WorkloadSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "cluster/Host", Field: "HostName"},
	},
}
