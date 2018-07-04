// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkApiServer is a auto generated package.
Input file: service.proto
*/
package network

import (
	"reflect"

	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapService = map[string]*runtime.Struct{

	"network.Service": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(Service{}) },
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"O": runtime.Field{Name: "O", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": runtime.Field{Name: "Spec", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "network.ServiceSpec"},

			"Status": runtime.Field{Name: "Status", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "network.ServiceStatus"},
		},

		CLITags: map[string]runtime.CLIInfo{
			"client-authentication": runtime.CLIInfo{Path: "Spec.TLSServerPolicy.ClientAuthentication", Skip: false, Insert: "", Help: ""},
			"lb-policy":             runtime.CLIInfo{Path: "Spec.LBPolicy", Skip: false, Insert: "", Help: ""},
			"ports":                 runtime.CLIInfo{Path: "Spec.Ports", Skip: false, Insert: "", Help: ""},
			"tls-client-allowed-peer-id":       runtime.CLIInfo{Path: "Spec.TLSClientPolicy.AllowedPeerId", Skip: false, Insert: "", Help: ""},
			"tls-client-certificates-selector": runtime.CLIInfo{Path: "Spec.TLSClientPolicy.CertificatesSelector", Skip: false, Insert: "", Help: ""},
			"tls-client-trust-roots":           runtime.CLIInfo{Path: "Spec.TLSClientPolicy.TrustRoots", Skip: false, Insert: "", Help: ""},
			"tls-server-allowed-peer-id":       runtime.CLIInfo{Path: "Spec.TLSServerPolicy.AllowedPeerId", Skip: false, Insert: "", Help: ""},
			"tls-server-certificates":          runtime.CLIInfo{Path: "Spec.TLSServerPolicy.Certificates", Skip: false, Insert: "", Help: ""},
			"tls-server-trust-roots":           runtime.CLIInfo{Path: "Spec.TLSServerPolicy.TrustRoots", Skip: false, Insert: "", Help: ""},
			"virtual-ip":                       runtime.CLIInfo{Path: "Spec.VirtualIp", Skip: false, Insert: "", Help: ""},
			"workload-labels":                  runtime.CLIInfo{Path: "Spec.WorkloadSelector", Skip: false, Insert: "", Help: ""},
			"workloads":                        runtime.CLIInfo{Path: "Status.Workloads", Skip: false, Insert: "", Help: ""},
		},
	},
	"network.ServiceSpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(ServiceSpec{}) },
		Fields: map[string]runtime.Field{
			"WorkloadSelector": runtime.Field{Name: "WorkloadSelector", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workload-labels", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"VirtualIp": runtime.Field{Name: "VirtualIp", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "virtual-ip", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Ports": runtime.Field{Name: "Ports", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ports", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"LBPolicy": runtime.Field{Name: "LBPolicy", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "lb-policy", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"TLSServerPolicy": runtime.Field{Name: "TLSServerPolicy", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-policy", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "network.TLSServerPolicySpec"},

			"TLSClientPolicy": runtime.Field{Name: "TLSClientPolicy", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-policy", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "network.TLSClientPolicySpec"},
		},
	},
	"network.ServiceStatus": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(ServiceStatus{}) },
		Fields: map[string]runtime.Field{
			"Workloads": runtime.Field{Name: "Workloads", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workloads", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.TLSClientPolicySpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(TLSClientPolicySpec{}) },
		Fields: map[string]runtime.Field{
			"CertificatesSelector": runtime.Field{Name: "CertificatesSelector", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-certificates-selector", Pointer: false, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},

			"TrustRoots": runtime.Field{Name: "TrustRoots", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-trust-roots", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"AllowedPeerId": runtime.Field{Name: "AllowedPeerId", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-allowed-peer-id", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.TLSClientPolicySpec.CertificatesSelectorEntry": &runtime.Struct{
		Fields: map[string]runtime.Field{
			"key": runtime.Field{Name: "key", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"value": runtime.Field{Name: "value", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.TLSServerPolicySpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(TLSServerPolicySpec{}) },
		Fields: map[string]runtime.Field{
			"Certificates": runtime.Field{Name: "Certificates", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-certificates", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"ClientAuthentication": runtime.Field{Name: "ClientAuthentication", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "client-authentication", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"TrustRoots": runtime.Field{Name: "TrustRoots", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-trust-roots", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"AllowedPeerId": runtime.Field{Name: "AllowedPeerId", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-allowed-peer-id", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapService)
}
