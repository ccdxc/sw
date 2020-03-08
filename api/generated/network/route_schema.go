// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkApiServer is a auto generated package.
Input file: route.proto
*/
package network

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapRoute = map[string]*api.Struct{

	"network.BGPConfig": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(BGPConfig{}) },
		Fields: map[string]api.Field{
			"RouterId": api.Field{Name: "RouterId", CLITag: api.CLIInfo{ID: "router-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "router-id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"ASNumber": api.Field{Name: "ASNumber", CLITag: api.CLIInfo{ID: "as-number", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "as-number", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"KeepaliveInterval": api.Field{Name: "KeepaliveInterval", CLITag: api.CLIInfo{ID: "keepalive-interval", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "keepalive-interval", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"Holdtime": api.Field{Name: "Holdtime", CLITag: api.CLIInfo{ID: "holdtime", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "holdtime", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"Neighbors": api.Field{Name: "Neighbors", CLITag: api.CLIInfo{ID: "neighbors", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "neighbors", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.BGPNeighbor"},
		},
	},
	"network.BGPNeighbor": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(BGPNeighbor{}) },
		Fields: map[string]api.Field{
			"Shutdown": api.Field{Name: "Shutdown", CLITag: api.CLIInfo{ID: "shutdown", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "shutdown", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_BOOL"},

			"IPAddress": api.Field{Name: "IPAddress", CLITag: api.CLIInfo{ID: "ip-address", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ip-address", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"RemoteAS": api.Field{Name: "RemoteAS", CLITag: api.CLIInfo{ID: "remote-as", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "remote-as", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"MultiHop": api.Field{Name: "MultiHop", CLITag: api.CLIInfo{ID: "multi-hop", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "multi-hop", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"EnableAddressFamilies": api.Field{Name: "EnableAddressFamilies", CLITag: api.CLIInfo{ID: "enable-address-families", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "enable-address-families", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Password": api.Field{Name: "Password", CLITag: api.CLIInfo{ID: "password", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "password", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"SourceFromLoopback": api.Field{Name: "SourceFromLoopback", CLITag: api.CLIInfo{ID: "source-from-loopback", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "source-from-loopback", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_BOOL"},
		},
	},
	"network.EVPNConfig": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(EVPNConfig{}) },
		Fields: map[string]api.Field{
			"Shutdown": api.Field{Name: "Shutdown", CLITag: api.CLIInfo{ID: "shutdown", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "shutdown", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_BOOL"},
		},
	},
	"network.RDSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RDSpec{}) },
		Fields: map[string]api.Field{
			"AddressFamily": api.Field{Name: "AddressFamily", CLITag: api.CLIInfo{ID: "address-family", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "address-family", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"RDAuto": api.Field{Name: "RDAuto", CLITag: api.CLIInfo{ID: "rd-auto", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "rd-auto", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_BOOL"},

			"RD": api.Field{Name: "RD", CLITag: api.CLIInfo{ID: "rd", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "rd", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RouteDistinguisher"},

			"ExportRTs": api.Field{Name: "ExportRTs", CLITag: api.CLIInfo{ID: "rt-export", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "rt-export", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RouteDistinguisher"},

			"ImportRTs": api.Field{Name: "ImportRTs", CLITag: api.CLIInfo{ID: "rt-import", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "rt-import", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RouteDistinguisher"},
		},
	},
	"network.Route": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Route{}) },
		Fields: map[string]api.Field{
			"Prefix": api.Field{Name: "Prefix", CLITag: api.CLIInfo{ID: "prefix", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "prefix", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"NexhHop": api.Field{Name: "NexhHop", CLITag: api.CLIInfo{ID: "next-hop", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "next-hop", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"TargetVirtualRouter": api.Field{Name: "TargetVirtualRouter", CLITag: api.CLIInfo{ID: "target-virtual-router", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "target-virtual-router", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.RouteDistinguisher": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RouteDistinguisher{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"AdminValue": api.Field{Name: "AdminValue", CLITag: api.CLIInfo{ID: "admin-value", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "admin-value", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"AssignedValue": api.Field{Name: "AssignedValue", CLITag: api.CLIInfo{ID: "assigned-value", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "assigned-value", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},
		},
	},
	"network.RouteTable": &api.Struct{
		Kind: "RouteTable", APIGroup: "network", Scopes: []string{"Tenant"}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RouteTable{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RouteTableSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RouteTableStatus"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Name": api.Field{Name: "Name", CLITag: api.CLIInfo{ID: "name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Tenant": api.Field{Name: "Tenant", CLITag: api.CLIInfo{ID: "tenant", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tenant", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Namespace": api.Field{Name: "Namespace", CLITag: api.CLIInfo{ID: "namespace", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "namespace", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"GenerationID": api.Field{Name: "GenerationID", CLITag: api.CLIInfo{ID: "generation-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "generation-id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"UUID": api.Field{Name: "UUID", CLITag: api.CLIInfo{ID: "uuid", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "uuid", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Labels": api.Field{Name: "Labels", CLITag: api.CLIInfo{ID: "labels", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "labels", Pointer: true, Slice: false, Mutable: true, Map: true, Inline: false, FromInline: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},

			"CreationTime": api.Field{Name: "CreationTime", CLITag: api.CLIInfo{ID: "creation-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "creation-time", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "api.Timestamp"},

			"ModTime": api.Field{Name: "ModTime", CLITag: api.CLIInfo{ID: "mod-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mod-time", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "api.Timestamp"},

			"SelfLink": api.Field{Name: "SelfLink", CLITag: api.CLIInfo{ID: "self-link", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "self-link", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},

		CLITags: map[string]api.CLIInfo{
			"api-version":           api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"generation-id":         api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"kind":                  api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":                api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"name":                  api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":             api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"next-hop":              api.CLIInfo{Path: "Status.Routes[].NexhHop", Skip: false, Insert: "", Help: ""},
			"prefix":                api.CLIInfo{Path: "Status.Routes[].Prefix", Skip: false, Insert: "", Help: ""},
			"resource-version":      api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":             api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"target-virtual-router": api.CLIInfo{Path: "Status.Routes[].TargetVirtualRouter", Skip: false, Insert: "", Help: ""},
			"tenant":                api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"uuid":                  api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
		},
	},
	"network.RouteTableSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RouteTableSpec{}) },
		Fields: map[string]api.Field{},
	},
	"network.RouteTableStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RouteTableStatus{}) },
		Fields: map[string]api.Field{
			"Routes": api.Field{Name: "Routes", CLITag: api.CLIInfo{ID: "routes", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "routes", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.Route"},
		},
	},
	"network.RoutingConfig": &api.Struct{
		Kind: "RoutingConfig", APIGroup: "network", Scopes: []string{"Cluster"}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RoutingConfig{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RoutingConfigSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.RoutingConfigStatus"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Name": api.Field{Name: "Name", CLITag: api.CLIInfo{ID: "name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Tenant": api.Field{Name: "Tenant", CLITag: api.CLIInfo{ID: "tenant", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tenant", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Namespace": api.Field{Name: "Namespace", CLITag: api.CLIInfo{ID: "namespace", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "namespace", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"GenerationID": api.Field{Name: "GenerationID", CLITag: api.CLIInfo{ID: "generation-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "generation-id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"UUID": api.Field{Name: "UUID", CLITag: api.CLIInfo{ID: "uuid", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "uuid", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Labels": api.Field{Name: "Labels", CLITag: api.CLIInfo{ID: "labels", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "labels", Pointer: true, Slice: false, Mutable: true, Map: true, Inline: false, FromInline: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},

			"CreationTime": api.Field{Name: "CreationTime", CLITag: api.CLIInfo{ID: "creation-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "creation-time", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "api.Timestamp"},

			"ModTime": api.Field{Name: "ModTime", CLITag: api.CLIInfo{ID: "mod-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mod-time", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "api.Timestamp"},

			"SelfLink": api.Field{Name: "SelfLink", CLITag: api.CLIInfo{ID: "self-link", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "self-link", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},

		CLITags: map[string]api.CLIInfo{
			"api-version":             api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"as-number":               api.CLIInfo{Path: "Spec.BGPConfig.ASNumber", Skip: false, Insert: "", Help: ""},
			"enable-address-families": api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].EnableAddressFamilies", Skip: false, Insert: "", Help: ""},
			"generation-id":           api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"holdtime":                api.CLIInfo{Path: "Spec.BGPConfig.Holdtime", Skip: false, Insert: "", Help: ""},
			"ip-address":              api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].IPAddress", Skip: false, Insert: "", Help: ""},
			"keepalive-interval":      api.CLIInfo{Path: "Spec.BGPConfig.KeepaliveInterval", Skip: false, Insert: "", Help: ""},
			"kind":                    api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":                  api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"multi-hop":               api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].MultiHop", Skip: false, Insert: "", Help: ""},
			"name":                    api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":               api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"password":                api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].Password", Skip: false, Insert: "", Help: ""},
			"remote-as":               api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].RemoteAS", Skip: false, Insert: "", Help: ""},
			"resource-version":        api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"router-id":               api.CLIInfo{Path: "Spec.BGPConfig.RouterId", Skip: false, Insert: "", Help: ""},
			"self-link":               api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"shutdown":                api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].Shutdown", Skip: false, Insert: "", Help: ""},
			"source-from-loopback":    api.CLIInfo{Path: "Spec.BGPConfig.Neighbors[].SourceFromLoopback", Skip: false, Insert: "", Help: ""},
			"tenant":                  api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"uuid":                    api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
		},
	},
	"network.RoutingConfigSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RoutingConfigSpec{}) },
		Fields: map[string]api.Field{
			"BGPConfig": api.Field{Name: "BGPConfig", CLITag: api.CLIInfo{ID: "bgp-config", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "bgp-config", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.BGPConfig"},

			"EVPNConfig": api.Field{Name: "EVPNConfig", CLITag: api.CLIInfo{ID: "evpn-config", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "evpn-config", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.EVPNConfig"},
		},
	},
	"network.RoutingConfigStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(RoutingConfigStatus{}) },
		Fields: map[string]api.Field{},
	},
}

var keyMapRoute = map[string][]api.PathsMap{}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapRoute)
	schema.AddPaths(keyMapRoute)
}
