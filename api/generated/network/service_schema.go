// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkApiServer is a auto generated package.
Input file: service.proto
*/
package network

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapService = map[string]*api.Struct{

	"network.Service": &api.Struct{
		Kind: "Service", APIGroup: "network", Scopes: []string{"Tenant"}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Service{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.ServiceSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.ServiceStatus"},

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
			"api-version":                      api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"client-authentication":            api.CLIInfo{Path: "Spec.TLSServerPolicy.ClientAuthentication", Skip: false, Insert: "", Help: ""},
			"generation-id":                    api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"kind":                             api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":                           api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"lb-policy":                        api.CLIInfo{Path: "Spec.LBPolicy", Skip: false, Insert: "", Help: ""},
			"name":                             api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":                        api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"ports":                            api.CLIInfo{Path: "Spec.Ports", Skip: false, Insert: "", Help: ""},
			"resource-version":                 api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":                        api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"tenant":                           api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"tls-client-allowed-peer-id":       api.CLIInfo{Path: "Spec.TLSClientPolicy.AllowedPeerId", Skip: false, Insert: "", Help: ""},
			"tls-client-certificates-selector": api.CLIInfo{Path: "Spec.TLSClientPolicy.CertificatesSelector", Skip: false, Insert: "", Help: ""},
			"tls-client-trust-roots":           api.CLIInfo{Path: "Spec.TLSClientPolicy.TrustRoots", Skip: false, Insert: "", Help: ""},
			"tls-server-allowed-peer-id":       api.CLIInfo{Path: "Spec.TLSServerPolicy.AllowedPeerId", Skip: false, Insert: "", Help: ""},
			"tls-server-certificates":          api.CLIInfo{Path: "Spec.TLSServerPolicy.Certificates", Skip: false, Insert: "", Help: ""},
			"tls-server-trust-roots":           api.CLIInfo{Path: "Spec.TLSServerPolicy.TrustRoots", Skip: false, Insert: "", Help: ""},
			"uuid":                             api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
			"virtual-ip":                       api.CLIInfo{Path: "Spec.VirtualIp", Skip: false, Insert: "", Help: ""},
			"workload-labels":                  api.CLIInfo{Path: "Spec.WorkloadSelector", Skip: false, Insert: "", Help: ""},
			"workloads":                        api.CLIInfo{Path: "Status.Workloads", Skip: false, Insert: "", Help: ""},
		},
	},
	"network.ServiceSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(ServiceSpec{}) },
		Fields: map[string]api.Field{
			"WorkloadSelector": api.Field{Name: "WorkloadSelector", CLITag: api.CLIInfo{ID: "workload-labels", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workload-labels", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"VirtualIp": api.Field{Name: "VirtualIp", CLITag: api.CLIInfo{ID: "virtual-ip", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "virtual-ip", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Ports": api.Field{Name: "Ports", CLITag: api.CLIInfo{ID: "ports", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "ports", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"LBPolicy": api.Field{Name: "LBPolicy", CLITag: api.CLIInfo{ID: "lb-policy", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "lb-policy", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"TLSServerPolicy": api.Field{Name: "TLSServerPolicy", CLITag: api.CLIInfo{ID: "tls-server-policy", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-policy", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.TLSServerPolicySpec"},

			"TLSClientPolicy": api.Field{Name: "TLSClientPolicy", CLITag: api.CLIInfo{ID: "tls-client-policy", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-policy", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "network.TLSClientPolicySpec"},
		},
	},
	"network.ServiceStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(ServiceStatus{}) },
		Fields: map[string]api.Field{
			"Workloads": api.Field{Name: "Workloads", CLITag: api.CLIInfo{ID: "workloads", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workloads", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.TLSClientPolicySpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(TLSClientPolicySpec{}) },
		Fields: map[string]api.Field{
			"CertificatesSelector": api.Field{Name: "CertificatesSelector", CLITag: api.CLIInfo{ID: "tls-client-certificates-selector", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-certificates-selector", Pointer: true, Slice: false, Mutable: true, Map: true, Inline: false, FromInline: false, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},

			"TrustRoots": api.Field{Name: "TrustRoots", CLITag: api.CLIInfo{ID: "tls-client-trust-roots", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-trust-roots", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"AllowedPeerId": api.Field{Name: "AllowedPeerId", CLITag: api.CLIInfo{ID: "tls-client-allowed-peer-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-client-allowed-peer-id", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.TLSClientPolicySpec.CertificatesSelectorEntry": &api.Struct{
		Fields: map[string]api.Field{
			"key": api.Field{Name: "key", CLITag: api.CLIInfo{ID: "key", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"value": api.Field{Name: "value", CLITag: api.CLIInfo{ID: "value", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"network.TLSServerPolicySpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(TLSServerPolicySpec{}) },
		Fields: map[string]api.Field{
			"Certificates": api.Field{Name: "Certificates", CLITag: api.CLIInfo{ID: "tls-server-certificates", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-certificates", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"ClientAuthentication": api.Field{Name: "ClientAuthentication", CLITag: api.CLIInfo{ID: "client-authentication", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "client-authentication", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"TrustRoots": api.Field{Name: "TrustRoots", CLITag: api.CLIInfo{ID: "tls-server-trust-roots", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-trust-roots", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"AllowedPeerId": api.Field{Name: "AllowedPeerId", CLITag: api.CLIInfo{ID: "tls-server-allowed-peer-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tls-server-allowed-peer-id", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
}

var keyMapService = map[string][]api.PathsMap{}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapService)
	schema.AddPaths(keyMapService)
}
