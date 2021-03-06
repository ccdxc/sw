// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package orchestrationApiServer is a auto generated package.
Input file: orchestration.proto
*/
package orchestration

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapOrchestration = map[string]*api.Struct{

	"orchestration.Orchestrator": &api.Struct{
		Kind: "Orchestrator", APIGroup: "orchestration", Scopes: []string{"Cluster"}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Orchestrator{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "T", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "orchestration.OrchestratorSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "orchestration.OrchestratorStatus"},

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
			"connection-status":     api.CLIInfo{Path: "Status.Status", Skip: false, Insert: "", Help: ""},
			"discovered-namespaces": api.CLIInfo{Path: "Status.DiscoveredNamespaces", Skip: false, Insert: "", Help: ""},
			"generation-id":         api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"incompatible-dscs":     api.CLIInfo{Path: "Status.IncompatibleDSCs", Skip: false, Insert: "", Help: ""},
			"kind":                  api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":                api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"manage-namespaces":     api.CLIInfo{Path: "Spec.ManageNamespaces", Skip: false, Insert: "", Help: ""},
			"message":               api.CLIInfo{Path: "Status.Message", Skip: false, Insert: "", Help: ""},
			"name":                  api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":             api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"orch-id":               api.CLIInfo{Path: "Status.OrchID", Skip: false, Insert: "", Help: ""},
			"resource-version":      api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":             api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"tenant":                api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"type":                  api.CLIInfo{Path: "Spec.Type", Skip: false, Insert: "", Help: ""},
			"uri":                   api.CLIInfo{Path: "Spec.URI", Skip: false, Insert: "", Help: ""},
			"uuid":                  api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
		},
	},
	"orchestration.OrchestratorSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(OrchestratorSpec{}) },
		Fields: map[string]api.Field{
			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Mutable: false, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"URI": api.Field{Name: "URI", CLITag: api.CLIInfo{ID: "uri", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "uri", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Credentials": api.Field{Name: "Credentials", CLITag: api.CLIInfo{ID: "credentials", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "credentials", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "monitoring.ExternalCred"},

			"ManageNamespaces": api.Field{Name: "ManageNamespaces", CLITag: api.CLIInfo{ID: "manage-namespaces", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "manage-namespaces", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"orchestration.OrchestratorStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(OrchestratorStatus{}) },
		Fields: map[string]api.Field{
			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "connection-status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "connection-status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"LastTransitionTime": api.Field{Name: "LastTransitionTime", CLITag: api.CLIInfo{ID: "last-transition-time", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "last-transition-time", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.Timestamp"},

			"Message": api.Field{Name: "Message", CLITag: api.CLIInfo{ID: "message", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "message", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"OrchID": api.Field{Name: "OrchID", CLITag: api.CLIInfo{ID: "orch-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "orch-id", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_INT32"},

			"DiscoveredNamespaces": api.Field{Name: "DiscoveredNamespaces", CLITag: api.CLIInfo{ID: "discovered-namespaces", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "discovered-namespaces", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"IncompatibleDSCs": api.Field{Name: "IncompatibleDSCs", CLITag: api.CLIInfo{ID: "incompatible-dscs", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "incompatible-dscs", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
}

var keyMapOrchestration = map[string][]api.PathsMap{}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapOrchestration)
	schema.AddPaths(keyMapOrchestration)
}
