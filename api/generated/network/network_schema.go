// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkApiServer is a auto generated package.
Input file: network.proto
*/
package network

import (
	"reflect"

	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapNetwork = map[string]*runtime.Struct{

	"network.Network": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(Network{}) },
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"O": runtime.Field{Name: "O", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": runtime.Field{Name: "Spec", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "network.NetworkSpec"},

			"Status": runtime.Field{Name: "Status", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "network.NetworkStatus"},
		},

		CLITags: map[string]runtime.CLIInfo{
			"allocated-ipv4-addrs": runtime.CLIInfo{Path: "Status.AllocatedIPv4Addrs", Skip: false, Insert: "", Help: ""},
			"ipv4-gateway":         runtime.CLIInfo{Path: "Spec.IPv4Gateway", Skip: false, Insert: "", Help: ""},
			"ipv4-subnet":          runtime.CLIInfo{Path: "Spec.IPv4Subnet", Skip: false, Insert: "", Help: ""},
			"ipv6-gateway":         runtime.CLIInfo{Path: "Spec.IPv6Gateway", Skip: false, Insert: "", Help: ""},
			"ipv6-subnet":          runtime.CLIInfo{Path: "Spec.IPv6Subnet", Skip: false, Insert: "", Help: ""},
			"type":                 runtime.CLIInfo{Path: "Spec.Type", Skip: false, Insert: "", Help: ""},
			"vlan-id":              runtime.CLIInfo{Path: "Spec.VlanID", Skip: false, Insert: "", Help: ""},
			"vxlan-vni":            runtime.CLIInfo{Path: "Spec.VxlanVNI", Skip: false, Insert: "", Help: ""},
			"workloads":            runtime.CLIInfo{Path: "Status.Workloads", Skip: false, Insert: "", Help: ""},
		},
	},
	"network.NetworkSpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(NetworkSpec{}) },
		Fields: map[string]runtime.Field{
			"Type": runtime.Field{Name: "Type", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"IPv4Subnet": runtime.Field{Name: "IPv4Subnet", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ipv4-subnet", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"IPv4Gateway": runtime.Field{Name: "IPv4Gateway", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ipv4-gateway", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"IPv6Subnet": runtime.Field{Name: "IPv6Subnet", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ipv6-subnet", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"IPv6Gateway": runtime.Field{Name: "IPv6Gateway", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ipv6-gateway", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"VlanID": runtime.Field{Name: "VlanID", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "vlan-id", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_UINT32"},

			"VxlanVNI": runtime.Field{Name: "VxlanVNI", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "vxlan-vni", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_UINT32"},
		},
	},
	"network.NetworkStatus": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(NetworkStatus{}) },
		Fields: map[string]runtime.Field{
			"Workloads": runtime.Field{Name: "Workloads", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workloads", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"AllocatedIPv4Addrs": runtime.Field{Name: "AllocatedIPv4Addrs", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "allocated-ipv4-addrs", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_BYTES"},
		},
	},
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapNetwork)
}
