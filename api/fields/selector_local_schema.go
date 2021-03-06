// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package fieldsApiServer is a auto generated package.
Input file: selector.proto
*/
package fields

import (
	"reflect"

	"github.com/pensando/sw/api"
)

var typesMapSelector = map[string]*api.Struct{

	"fields.Requirement": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Requirement{}) },
		Fields: map[string]api.Field{
			"Key": api.Field{Name: "Key", CLITag: api.CLIInfo{ID: "key", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "key", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Operator": api.Field{Name: "Operator", CLITag: api.CLIInfo{ID: "operator", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "operator", Pointer: false, Slice: false, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},

			"Values": api.Field{Name: "Values", CLITag: api.CLIInfo{ID: "values", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "values", Pointer: false, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"fields.Selector": &api.Struct{
		Kind: "", APIGroup: "", Scopes: []string{}, GetTypeFn: func() reflect.Type { return reflect.TypeOf(Selector{}) },
		Fields: map[string]api.Field{
			"Requirements": api.Field{Name: "Requirements", CLITag: api.CLIInfo{ID: "requirements", Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "requirements", Pointer: true, Slice: true, Mutable: true, Map: false, Inline: false, FromInline: false, KeyType: "", Type: "fields.Requirement"},
		},
	},
}

func init() {
	schema_init_once.Do(init_schema_map)
	for k, v := range typesMapSelector {
		local_schema[k] = v
	}
}
