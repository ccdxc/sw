// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package clusterApiServer is a auto generated package.
Input file: svc_cluster.proto
*/
package cluster

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapSvc_cluster = map[string]*api.Struct{

	"cluster.AutoMsgClusterWatchHelper": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgClusterWatchHelper{}) },
		Fields: map[string]api.Field{
			"Events": api.Field{Name: "Events", CLITag: api.CLIInfo{ID: "events", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "events", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.AutoMsgClusterWatchHelper.WatchEvent"},
		},
	},
	"cluster.AutoMsgClusterWatchHelper.WatchEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgClusterWatchHelper_WatchEvent{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": api.Field{Name: "Object", CLITag: api.CLIInfo{ID: "object", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "object", Pointer: true, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Cluster"},
		},
	},
	"cluster.AutoMsgHostWatchHelper": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgHostWatchHelper{}) },
		Fields: map[string]api.Field{
			"Events": api.Field{Name: "Events", CLITag: api.CLIInfo{ID: "events", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "events", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.AutoMsgHostWatchHelper.WatchEvent"},
		},
	},
	"cluster.AutoMsgHostWatchHelper.WatchEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgHostWatchHelper_WatchEvent{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": api.Field{Name: "Object", CLITag: api.CLIInfo{ID: "object", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "object", Pointer: true, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Host"},
		},
	},
	"cluster.AutoMsgNodeWatchHelper": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgNodeWatchHelper{}) },
		Fields: map[string]api.Field{
			"Events": api.Field{Name: "Events", CLITag: api.CLIInfo{ID: "events", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "events", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.AutoMsgNodeWatchHelper.WatchEvent"},
		},
	},
	"cluster.AutoMsgNodeWatchHelper.WatchEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgNodeWatchHelper_WatchEvent{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": api.Field{Name: "Object", CLITag: api.CLIInfo{ID: "object", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "object", Pointer: true, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Node"},
		},
	},
	"cluster.AutoMsgSmartNICWatchHelper": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgSmartNICWatchHelper{}) },
		Fields: map[string]api.Field{
			"Events": api.Field{Name: "Events", CLITag: api.CLIInfo{ID: "events", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "events", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.AutoMsgSmartNICWatchHelper.WatchEvent"},
		},
	},
	"cluster.AutoMsgSmartNICWatchHelper.WatchEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgSmartNICWatchHelper_WatchEvent{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": api.Field{Name: "Object", CLITag: api.CLIInfo{ID: "object", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "object", Pointer: true, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.SmartNIC"},
		},
	},
	"cluster.AutoMsgTenantWatchHelper": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgTenantWatchHelper{}) },
		Fields: map[string]api.Field{
			"Events": api.Field{Name: "Events", CLITag: api.CLIInfo{ID: "events", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "events", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.AutoMsgTenantWatchHelper.WatchEvent"},
		},
	},
	"cluster.AutoMsgTenantWatchHelper.WatchEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(AutoMsgTenantWatchHelper_WatchEvent{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Object": api.Field{Name: "Object", CLITag: api.CLIInfo{ID: "object", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "object", Pointer: true, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Tenant"},
		},
	},
	"cluster.ClusterList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(ClusterList{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": api.Field{Name: "ListMeta", CLITag: api.CLIInfo{ID: "list-meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "list-meta", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.ListMeta"},

			"Items": api.Field{Name: "Items", CLITag: api.CLIInfo{ID: "items", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "items", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Cluster"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"cluster.HostList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(HostList{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": api.Field{Name: "ListMeta", CLITag: api.CLIInfo{ID: "list-meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "list-meta", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.ListMeta"},

			"Items": api.Field{Name: "Items", CLITag: api.CLIInfo{ID: "items", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "items", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Host"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"cluster.NodeList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(NodeList{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": api.Field{Name: "ListMeta", CLITag: api.CLIInfo{ID: "list-meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "list-meta", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.ListMeta"},

			"Items": api.Field{Name: "Items", CLITag: api.CLIInfo{ID: "items", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "items", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Node"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"cluster.SmartNICList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(SmartNICList{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": api.Field{Name: "ListMeta", CLITag: api.CLIInfo{ID: "list-meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "list-meta", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.ListMeta"},

			"Items": api.Field{Name: "Items", CLITag: api.CLIInfo{ID: "items", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "items", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.SmartNIC"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"cluster.TenantList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(TenantList{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ListMeta": api.Field{Name: "ListMeta", CLITag: api.CLIInfo{ID: "list-meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "list-meta", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.ListMeta"},

			"Items": api.Field{Name: "Items", CLITag: api.CLIInfo{ID: "items", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "items", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "cluster.Tenant"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},
	},
}

var keyMapSvc_cluster = map[string][]api.PathsMap{

	"cluster.Cluster": []api.PathsMap{
		{URI: "/configs/cluster/v1/cluster", Key: "/venice/config/cluster/cluster/Singleton"}},
	"cluster.Host": []api.PathsMap{
		{URI: "/configs/cluster/v1/hosts/{Name}", Key: "/venice/config/cluster/hosts/{Name}"}},
	"cluster.Node": []api.PathsMap{
		{URI: "/configs/cluster/v1/nodes/{Name}", Key: "/venice/config/cluster/nodes/{Name}"}},
	"cluster.SmartNIC": []api.PathsMap{
		{URI: "/configs/cluster/v1/smartnics/{Name}", Key: "/venice/config/cluster/smartnics/{Name}"}},
	"cluster.Tenant": []api.PathsMap{
		{URI: "/configs/cluster/v1/tenants/{Name}", Key: "/venice/config/cluster/tenants/{Name}"}},
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapSvc_cluster)
	schema.AddPaths(keyMapSvc_cluster)
}
