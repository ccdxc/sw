// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package securityApiServer is a auto generated package.
Input file: securitygroup.proto
*/
package security

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapSecuritygroup = map[string]*api.Struct{

	"security.SecurityGroup": &api.Struct{
		Kind: "SecurityGroup", APIGroup: "security", Scopes: []string{"Tenant"}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(SecurityGroup{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "security.SecurityGroupSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "security.SecurityGroupStatus"},

			"Kind": api.Field{Name: "Kind", CLITag: api.CLIInfo{ID: "kind", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "kind", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"APIVersion": api.Field{Name: "APIVersion", CLITag: api.CLIInfo{ID: "api-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "api-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Name": api.Field{Name: "Name", CLITag: api.CLIInfo{ID: "name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "name", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Tenant": api.Field{Name: "Tenant", CLITag: api.CLIInfo{ID: "tenant", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "tenant", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Namespace": api.Field{Name: "Namespace", CLITag: api.CLIInfo{ID: "namespace", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "namespace", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"GenerationID": api.Field{Name: "GenerationID", CLITag: api.CLIInfo{ID: "generation-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "generation-id", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"ResourceVersion": api.Field{Name: "ResourceVersion", CLITag: api.CLIInfo{ID: "resource-version", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "resource-version", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"UUID": api.Field{Name: "UUID", CLITag: api.CLIInfo{ID: "uuid", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "uuid", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},

			"Labels": api.Field{Name: "Labels", CLITag: api.CLIInfo{ID: "labels", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "labels", Pointer: true, Slice: false, Map: true, Inline: false, FromInline: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},

			"CreationTime": api.Field{Name: "CreationTime", CLITag: api.CLIInfo{ID: "creation-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "creation-time", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "api.Timestamp"},

			"ModTime": api.Field{Name: "ModTime", CLITag: api.CLIInfo{ID: "mod-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mod-time", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "api.Timestamp"},

			"SelfLink": api.Field{Name: "SelfLink", CLITag: api.CLIInfo{ID: "self-link", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "self-link", Pointer: false, Slice: false, Map: false, Inline: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
		},

		CLITags: map[string]api.CLIInfo{
			"Policies":         api.CLIInfo{Path: "Status.Policies", Skip: false, Insert: "", Help: ""},
			"api-version":      api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"generation-id":    api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"kind":             api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":           api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"match-prefixes":   api.CLIInfo{Path: "Spec.MatchPrefixes", Skip: false, Insert: "", Help: ""},
			"name":             api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":        api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"resource-version": api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":        api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"service-labels":   api.CLIInfo{Path: "Spec.ServiceSelector", Skip: false, Insert: "", Help: ""},
			"tenant":           api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"uuid":             api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
			"workloads":        api.CLIInfo{Path: "Status.Workloads", Skip: false, Insert: "", Help: ""},
		},
	},
	"security.SecurityGroupSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(SecurityGroupSpec{}) },
		Fields: map[string]api.Field{
			"WorkloadSelector": api.Field{Name: "WorkloadSelector", CLITag: api.CLIInfo{ID: "workload-selector", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workload-selector", Pointer: true, Slice: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "labels.Selector"},

			"ServiceSelector": api.Field{Name: "ServiceSelector", CLITag: api.CLIInfo{ID: "service-labels", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "service-labels", Pointer: false, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"MatchPrefixes": api.Field{Name: "MatchPrefixes", CLITag: api.CLIInfo{ID: "match-prefixes", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "match-prefixes", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"security.SecurityGroupStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(SecurityGroupStatus{}) },
		Fields: map[string]api.Field{
			"Workloads": api.Field{Name: "Workloads", CLITag: api.CLIInfo{ID: "workloads", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "workloads", Pointer: true, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Policies": api.Field{Name: "Policies", CLITag: api.CLIInfo{ID: "Policies", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
}

var keyMapSecuritygroup = map[string][]api.PathsMap{}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapSecuritygroup)
	schema.AddPaths(keyMapSecuritygroup)
}
