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
		{Type: "NamedRef", To: "auth/User", Field: "Users"},
		{Type: "NamedRef", To: "auth/Role", Field: "Role"},
	},
	"bookstore.BookSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "bookstore/Publisher", Field: "Publisher"},
		{Type: "NamedRef", To: "bookstore/Publisher", Field: "Publisher"},
	},
	"bookstore.CustomerStatus": []apiintf.ObjRelation{
		{Type: "SelectorRef", To: "bookstore/Book", Field: "Interests"},
		{Type: "SelectorRef", To: "bookstore/Book", Field: "Interests"},
	},
	"bookstore.OrderItem": []apiintf.ObjRelation{
		{Type: "WeakRef", To: "bookstore/Book", Field: "Book"},
		{Type: "WeakRef", To: "bookstore/Book", Field: "Book"},
	},
	"monitoring.AlertPolicySpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "monitoring/AlertDestination", Field: "Destinations"},
		{Type: "NamedRef", To: "monitoring/AlertDestination", Field: "Destinations"},
	},
	"network.DHCPServer": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/VirtualRouter", Field: "VirtualRouter"},
		{Type: "NamedRef", To: "network/VirtualRouter", Field: "VirtualRouter"},
	},
	"network.NetworkInterfaceSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "cluster/Tenant", Field: "AttachTenant"},
		{Type: "NamedRef", To: "cluster/Tenant", Field: "AttachTenant"},
	},
	"network.NetworkSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/VirtualRouter", Field: "VirtualRouter"},
		{Type: "NamedRef", To: "network/IPAMPolicy", Field: "IPAMPolicy"},
		{Type: "NamedRef", To: "network/VirtualRouter", Field: "VirtualRouter"},
		{Type: "NamedRef", To: "network/IPAMPolicy", Field: "IPAMPolicy"},
	},
	"network.Route": []apiintf.ObjRelation{
		{Type: "WeakRef", To: "network/VirtualRouter", Field: "TargetVirtualRouter"},
		{Type: "WeakRef", To: "network/VirtualRouter", Field: "TargetVirtualRouter"},
	},
	"network.VirtualRouterStatus": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/RouteTable", Field: "RouteTable"},
		{Type: "NamedRef", To: "network/RouteTable", Field: "RouteTable"},
	},
	"security.NetworkSecurityPolicySpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "AttachGroups"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "AttachGroups"},
	},
	"security.SGRule": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "security/App", Field: "Apps"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "FromSecurityGroups"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "ToSecurityGroups"},
		{Type: "NamedRef", To: "security/App", Field: "Apps"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "FromSecurityGroups"},
		{Type: "NamedRef", To: "security/SecurityGroup", Field: "ToSecurityGroups"},
	},
	"workload.EndpointStatus": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "network/Network", Field: "Network"},
		{Type: "NamedRef", To: "network/Network", Field: "Network"},
	},
	"workload.WorkloadSpec": []apiintf.ObjRelation{
		{Type: "NamedRef", To: "cluster/Host", Field: "HostName"},
		{Type: "NamedRef", To: "cluster/Host", Field: "HostName"},
	},
}
