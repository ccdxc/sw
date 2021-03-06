// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package netprotoApiServer is a auto generated package.
Input file: mirror.proto
*/
package netproto

import (
	"reflect"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapMirror = map[string]*api.Struct{

	"netproto.InterfaceMirrorSession": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(InterfaceMirrorSession{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "TypeMeta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.InterfaceMirrorSessionSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.InterfaceMirrorSessionStatus"},

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
			"api-version":      api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"destination":      api.CLIInfo{Path: "Spec.Collectors[].ExportCfg.Destination", Skip: false, Insert: "", Help: ""},
			"gateway":          api.CLIInfo{Path: "Spec.Collectors[].ExportCfg.Gateway", Skip: false, Insert: "", Help: ""},
			"generation-id":    api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"id":               api.CLIInfo{Path: "Status.MirrorSessionID", Skip: false, Insert: "", Help: ""},
			"kind":             api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":           api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"mirror-direction": api.CLIInfo{Path: "Spec.MirrorDirection", Skip: false, Insert: "", Help: ""},
			"name":             api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":        api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"packet-size":      api.CLIInfo{Path: "Spec.PacketSize", Skip: false, Insert: "", Help: ""},
			"pcap-dir-name":    api.CLIInfo{Path: "Spec.Collectors[].PcapDirName", Skip: false, Insert: "", Help: ""},
			"resource-version": api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":        api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"span-id":          api.CLIInfo{Path: "Spec.SpanID", Skip: false, Insert: "", Help: ""},
			"strip-vlan-hdr":   api.CLIInfo{Path: "Spec.Collectors[].StripVlanHdr", Skip: false, Insert: "", Help: ""},
			"tenant":           api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"type":             api.CLIInfo{Path: "Spec.Collectors[].Type", Skip: false, Insert: "", Help: ""},
			"uuid":             api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
			"vrf-name":         api.CLIInfo{Path: "Spec.VrfName", Skip: false, Insert: "", Help: ""},
		},
	},
	"netproto.InterfaceMirrorSessionEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(InterfaceMirrorSessionEvent{}) },
		Fields: map[string]api.Field{
			"EventType": api.Field{Name: "EventType", CLITag: api.CLIInfo{ID: "event-type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "event-type", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_ENUM"},

			"InterfaceMirrorSession": api.Field{Name: "InterfaceMirrorSession", CLITag: api.CLIInfo{ID: "interface-mirror-session", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "interface-mirror-session", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.InterfaceMirrorSession"},
		},
	},
	"netproto.InterfaceMirrorSessionEventList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(InterfaceMirrorSessionEventList{}) },
		Fields: map[string]api.Field{
			"InterfaceMirrorSessionEvents": api.Field{Name: "InterfaceMirrorSessionEvents", CLITag: api.CLIInfo{ID: "InterfaceMirrorSessionEvents", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.InterfaceMirrorSessionEvent"},
		},
	},
	"netproto.InterfaceMirrorSessionList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(InterfaceMirrorSessionList{}) },
		Fields: map[string]api.Field{
			"InterfaceMirrorSessions": api.Field{Name: "InterfaceMirrorSessions", CLITag: api.CLIInfo{ID: "InterfaceMirrorSessions", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.InterfaceMirrorSession"},
		},
	},
	"netproto.InterfaceMirrorSessionSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(InterfaceMirrorSessionSpec{}) },
		Fields: map[string]api.Field{
			"VrfName": api.Field{Name: "VrfName", CLITag: api.CLIInfo{ID: "vrf-name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "vrf-name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Collectors": api.Field{Name: "Collectors", CLITag: api.CLIInfo{ID: "collectors", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "collectors", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "netproto.MirrorCollector"},

			"SpanID": api.Field{Name: "SpanID", CLITag: api.CLIInfo{ID: "span-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "span-id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"PacketSize": api.Field{Name: "PacketSize", CLITag: api.CLIInfo{ID: "packet-size", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "packet-size", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"MirrorDirection": api.Field{Name: "MirrorDirection", CLITag: api.CLIInfo{ID: "mirror-direction", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mirror-direction", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_ENUM"},

			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"StripVlanHdr": api.Field{Name: "StripVlanHdr", CLITag: api.CLIInfo{ID: "strip-vlan-hdr", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "strip-vlan-hdr", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_BOOL"},
		},
	},
	"netproto.InterfaceMirrorSessionStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(InterfaceMirrorSessionStatus{}) },
		Fields: map[string]api.Field{
			"MirrorSessionID": api.Field{Name: "MirrorSessionID", CLITag: api.CLIInfo{ID: "id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT64"},
		},
	},
	"netproto.MirrorCollector": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorCollector{}) },
		Fields: map[string]api.Field{
			"ExportCfg": api.Field{Name: "ExportCfg", CLITag: api.CLIInfo{ID: "export-config", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "export-config", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.MirrorExportConfig"},

			"PcapDirName": api.Field{Name: "PcapDirName", CLITag: api.CLIInfo{ID: "pcap-dir-name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "pcap-dir-name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Type": api.Field{Name: "Type", CLITag: api.CLIInfo{ID: "type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "type", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"StripVlanHdr": api.Field{Name: "StripVlanHdr", CLITag: api.CLIInfo{ID: "strip-vlan-hdr", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "strip-vlan-hdr", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_BOOL"},
		},
	},
	"netproto.MirrorExportConfig": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorExportConfig{}) },
		Fields: map[string]api.Field{
			"Destination": api.Field{Name: "Destination", CLITag: api.CLIInfo{ID: "destination", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "destination", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Gateway": api.Field{Name: "Gateway", CLITag: api.CLIInfo{ID: "gateway", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "gateway", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"netproto.MirrorSession": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorSession{}) },
		Fields: map[string]api.Field{
			"TypeMeta": api.Field{Name: "TypeMeta", CLITag: api.CLIInfo{ID: "TypeMeta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "api.TypeMeta"},

			"ObjectMeta": api.Field{Name: "ObjectMeta", CLITag: api.CLIInfo{ID: "meta", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": api.Field{Name: "Spec", CLITag: api.CLIInfo{ID: "spec", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.MirrorSessionSpec"},

			"Status": api.Field{Name: "Status", CLITag: api.CLIInfo{ID: "status", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.MirrorSessionStatus"},

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
			"api-version":      api.CLIInfo{Path: "APIVersion", Skip: false, Insert: "", Help: ""},
			"destination":      api.CLIInfo{Path: "Spec.Collectors[].ExportCfg.Destination", Skip: false, Insert: "", Help: ""},
			"gateway":          api.CLIInfo{Path: "Spec.Collectors[].ExportCfg.Gateway", Skip: false, Insert: "", Help: ""},
			"generation-id":    api.CLIInfo{Path: "GenerationID", Skip: false, Insert: "", Help: ""},
			"id":               api.CLIInfo{Path: "Status.MirrorSessionID", Skip: false, Insert: "", Help: ""},
			"kind":             api.CLIInfo{Path: "Kind", Skip: false, Insert: "", Help: ""},
			"labels":           api.CLIInfo{Path: "Labels", Skip: false, Insert: "", Help: ""},
			"mirror-direction": api.CLIInfo{Path: "Spec.MirrorDirection", Skip: false, Insert: "", Help: ""},
			"name":             api.CLIInfo{Path: "Name", Skip: false, Insert: "", Help: ""},
			"namespace":        api.CLIInfo{Path: "Namespace", Skip: false, Insert: "", Help: ""},
			"packet-size":      api.CLIInfo{Path: "Spec.PacketSize", Skip: false, Insert: "", Help: ""},
			"pcap-dir-name":    api.CLIInfo{Path: "Spec.Collectors[].PcapDirName", Skip: false, Insert: "", Help: ""},
			"resource-version": api.CLIInfo{Path: "ResourceVersion", Skip: false, Insert: "", Help: ""},
			"self-link":        api.CLIInfo{Path: "SelfLink", Skip: false, Insert: "", Help: ""},
			"span-id":          api.CLIInfo{Path: "Spec.SpanID", Skip: false, Insert: "", Help: ""},
			"strip-vlan-hdr":   api.CLIInfo{Path: "Spec.Collectors[].StripVlanHdr", Skip: false, Insert: "", Help: ""},
			"tenant":           api.CLIInfo{Path: "Tenant", Skip: false, Insert: "", Help: ""},
			"type":             api.CLIInfo{Path: "Spec.Collectors[].Type", Skip: false, Insert: "", Help: ""},
			"uuid":             api.CLIInfo{Path: "UUID", Skip: false, Insert: "", Help: ""},
			"vrf-name":         api.CLIInfo{Path: "Spec.VrfName", Skip: false, Insert: "", Help: ""},
		},
	},
	"netproto.MirrorSessionEvent": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorSessionEvent{}) },
		Fields: map[string]api.Field{
			"EventType": api.Field{Name: "EventType", CLITag: api.CLIInfo{ID: "event-type", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "event-type", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_ENUM"},

			"MirrorSession": api.Field{Name: "MirrorSession", CLITag: api.CLIInfo{ID: "mirror-session", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mirror-session", Pointer: true, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.MirrorSession"},
		},
	},
	"netproto.MirrorSessionEventList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorSessionEventList{}) },
		Fields: map[string]api.Field{
			"MirrorSessionEvents": api.Field{Name: "MirrorSessionEvents", CLITag: api.CLIInfo{ID: "MirrorSessionEvents", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.MirrorSessionEvent"},
		},
	},
	"netproto.MirrorSessionList": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorSessionList{}) },
		Fields: map[string]api.Field{
			"MirrorSessions": api.Field{Name: "MirrorSessions", CLITag: api.CLIInfo{ID: "MirrorSessions", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "netproto.MirrorSession"},
		},
	},
	"netproto.MirrorSessionSpec": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorSessionSpec{}) },
		Fields: map[string]api.Field{
			"VrfName": api.Field{Name: "VrfName", CLITag: api.CLIInfo{ID: "vrf-name", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "vrf-name", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Collectors": api.Field{Name: "Collectors", CLITag: api.CLIInfo{ID: "collectors", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "collectors", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "netproto.MirrorCollector"},

			"MatchRules": api.Field{Name: "MatchRules", CLITag: api.CLIInfo{ID: "match-rules", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "match-rules", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: true, FromInline: false, KeyType: "", Type: "netproto.MatchRule"},

			"PacketSize": api.Field{Name: "PacketSize", CLITag: api.CLIInfo{ID: "packet-size", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "packet-size", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},

			"MirrorDirection": api.Field{Name: "MirrorDirection", CLITag: api.CLIInfo{ID: "mirror-direction", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "mirror-direction", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_ENUM"},

			"SpanID": api.Field{Name: "SpanID", CLITag: api.CLIInfo{ID: "span-id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "span-id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT32"},
		},
	},
	"netproto.MirrorSessionStatus": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(MirrorSessionStatus{}) },
		Fields: map[string]api.Field{
			"MirrorSessionID": api.Field{Name: "MirrorSessionID", CLITag: api.CLIInfo{ID: "id", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "id", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_UINT64"},
		},
	},
}

var keyMapMirror = map[string][]api.PathsMap{}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapMirror)
	schema.AddPaths(keyMapMirror)
}
