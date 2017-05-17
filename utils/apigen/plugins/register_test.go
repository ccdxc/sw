package plugin

import (
	"reflect"
	"testing"

	"github.com/gogo/protobuf/proto"
	descriptor "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	reg "github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	_ "github.com/pensando/sw/utils/apigen/annotations"
)

func TestDdbPathGet(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest2_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
			field <
				name: 'leaf_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
			options:<[venice.objectPrefix]:"{nest2_field}/{leaf_field}">
		>
		message_type <
			name: 'testmsg'
			field <
				name: 'embedded_msg'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				options:<[gogoproto.nullable]:false [gogoproto.embed]:true [gogoproto.jsontag]:"metadata,inline">
				number: 1
			>
			field <
				name: 'real_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
			field <
				name: 'leaf_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 3
			>
			options:<[venice.objectPrefix]:"prefix-{leaf_field}/qual{real_field.leaf_field}">
		>
		syntax: "proto3"
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'Nest2'
			field <
				name: 'embedded_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			options:<[venice.objectPrefix]:"prefix-{embedded_field">
		>
		syntax: "proto2"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		fds = append(fds, &fd)
	}
	t.Logf("Setting up registry")
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}

	t.Logf("Test dbPath with [prefix-{leaf_field}/qual{real_field.leaf_field}]")
	testmsg, err := r.LookupMsg("", ".example.testmsg")
	result := []KeyComponent{
		{Type: "prefix", Val: "prefix-"},
		{Type: "field", Val: "leaf_field"},
		{Type: "prefix", Val: "/qual"},
		{Type: "field", Val: "real_field.leaf_field"},
		{Type: "prefix", Val: "/"},
		{Type: "field", Val: "Name"},
	}
	paths, err := getDbKey(testmsg)
	if err != nil {
		t.Errorf("getDbKey failed")
	}
	if !reflect.DeepEqual(result, paths) {
		t.Errorf("result does not match")
	}

	t.Logf("Test dbPath with [{nest2_field}/{leaf_field}]")
	testmsg, err = r.LookupMsg("", ".example.Nest1")
	result = []KeyComponent{
		{Type: "field", Val: "nest2_field"},
		{Type: "prefix", Val: "/"},
		{Type: "field", Val: "leaf_field"},
		{Type: "prefix", Val: "/"},
		{Type: "field", Val: "Name"},
	}
	paths, err = getDbKey(testmsg)
	if err != nil {
		t.Errorf("getDbKey failed")
	}
	if !reflect.DeepEqual(result, paths) {
		t.Errorf("result does not match %+v", paths)
	}

	t.Logf("Test faulty dbPath with [prefix-{embedded_field]")
	testmsg, err = r.LookupMsg("", ".example.Nest2")
	paths, err = getDbKey(testmsg)
	if err == nil {
		t.Errorf("getDbKey succedded on faulty specification")
	}
}
