// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package netprotoApiServer is a auto generated package.
Input file: endpoint.proto
*/
package netproto

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapEndpoint = map[string]*api.Struct{

	"netproto.Endpoint": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Endpoint{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "TypeMeta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.EndpointSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.EndpointStatus"},

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
			"api-version":         api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"enic-id":             api.CLIInfo{Path: "Status.EnicID", Skip: false, Insert: "", Help: ""},
			"generation-id":       api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"homing-host-address": api.CLIInfo{Path: "Spec.HomingHostAddr", Skip: false, Insert: "", Help: ""},
			"ipv4-addresses":      api.CLIInfo{Path: "Spec.IPv4Addresses", Skip: false, Insert: "", Help: ""},
			"ipv6-address":        api.CLIInfo{Path: "Spec.IPv6Addresses", Skip: false, Insert: "", Help: ""},
			"kind":                api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":              api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"mac-address":         api.CLIInfo{Path: "Spec.MacAddress", Skip: false, Insert: "", Help: ""},
			"migration":           api.CLIInfo{Path: "Spec.Migration", Skip: false, Insert: "", Help: ""},
			"name":                api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":           api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"network-name":        api.CLIInfo{Path: "Spec.NetworkName", Skip: false, Insert: "", Help: ""},
			"node-uuid":           api.CLIInfo{Path: "Spec.NodeUUID", Skip: false, Insert: "", Help: ""},
			"resource-version":    api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":           api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"tenant":              api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"useg-vlan":           api.CLIInfo{Path: "Spec.UsegVlan", Skip: false, Insert: "", Help: ""},
			"uuid":                api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
			"vrf-name":            api.CLIInfo{Path: "Spec.VrfName", Skip: false, Insert: "", Help: ""},
		},
	},
	"netproto.EndpointEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(EndpointEvent{}) },
		Fields: map[string]api.Field{
			"EventType": api.Field{Name: "EventType", CLITag: api.CLIInfo{ID: "event-type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "event-type", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_ENUM"},

			"Endpoint": api.Field{Name: "Endpoint", CLITag: api.CLIInfo{ID: "endpoint", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "endpoint", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.Endpoint"},
		},
	},
	"netproto.EndpointEventList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(EndpointEventList{}) },
		Fields: map[string]api.Field{
			"EndpointEvents": api.Field{Name: "EndpointEvents", CLITag: api.CLIInfo{ID: "EndpointEvents", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.EndpointEvent"},
		},
	},
	"netproto.EndpointList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(EndpointList{}) },
		Fields: map[string]api.Field{
			"endpoints": api.Field{Name: "endpoints", CLITag: api.CLIInfo{ID: "endpoints", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.Endpoint"},
		},
	},
	"netproto.EndpointSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(EndpointSpec{}) },
		Fields: map[string]api.Field{
			"VrfName": api.Field{Name: "VrfName", CLITag: api.CLIInfo{ID: "vrf-name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "vrf-name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"NetworkName": api.Field{Name: "NetworkName", CLITag: api.CLIInfo{ID: "network-name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "network-name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"IPv4Addresses": api.Field{Name: "IPv4Addresses", CLITag: api.CLIInfo{ID: "ipv4-addresses", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ipv4-addresses", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"IPv6Addresses": api.Field{Name: "IPv6Addresses", CLITag: api.CLIInfo{ID: "ipv6-address", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ipv6-address", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"MacAddress": api.Field{Name: "MacAddress", CLITag: api.CLIInfo{ID: "mac-address", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mac-address", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"NodeUUID": api.Field{Name: "NodeUUID", CLITag: api.CLIInfo{ID: "node-uuid", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "node-uuid", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"HomingHostAddr": api.Field{Name: "HomingHostAddr", CLITag: api.CLIInfo{ID: "homing-host-address", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "homing-host-address", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"UsegVlan": api.Field{Name: "UsegVlan", CLITag: api.CLIInfo{ID: "useg-vlan", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "useg-vlan", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"Migration": api.Field{Name: "Migration", CLITag: api.CLIInfo{ID: "migration", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "migration", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"netproto.EndpointStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(EndpointStatus{}) },
		Fields: map[string]api.Field{
			"EnicID": api.Field{Name: "EnicID", CLITag: api.CLIInfo{ID: "enic-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "enic-id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT64"},

			"NodeUUID": api.Field{Name: "NodeUUID", CLITag: api.CLIInfo{ID: "node-uuid", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "node-uuid", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Migration": api.Field{Name: "Migration", CLITag: api.CLIInfo{ID: "migration", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "migration", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
}

var keyMapEndpoint = map[string][]api.PathsMap{}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapEndpoint)
	schema.AddPaths(keyMapEndpoint)
}
