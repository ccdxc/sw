// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package apiApiServer is a auto generated package.
Input file: common.proto
*/
package api

import "reflect"

var typesMapCommon = map[string]*Struct{

	"api.Empty": &Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Empty{}) },
		Fields: map[string]Field{},
	},
	"api.Filter": &Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Filter{}) },
		Fields: map[string]Field{
			"Specs": Field{Name: "Specs", CLITag: CLIInfo{ID: "Specs", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.Spec"},
		},
	},
	"api.Spec": &Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Spec{}) },
		Fields: map[string]Field{
			"Key": Field{Name: "Key", CLITag: CLIInfo{ID: "Key", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Operator": Field{Name: "Operator", CLITag: CLIInfo{ID: "Operator", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Values": Field{Name: "Values", CLITag: CLIInfo{ID: "Values", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"api.WatchSpec": &Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(WatchSpec{}) },
		Fields: map[string]Field{
			"RefVersion": Field{Name: "RefVersion", CLITag: CLIInfo{ID: "RefVersion", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Specs": Field{Name: "Specs", CLITag: CLIInfo{ID: "Specs", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "api.Spec"},
		},
	},
}

func init() {
	schema_init_once.Do(init_schema_map)
	for k, v := range typesMapCommon {
		local_schema[k] = v
	}
}
