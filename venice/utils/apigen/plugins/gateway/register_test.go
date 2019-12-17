package plugin

import (
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"strings"
	"testing"

	govldtr "github.com/asaskevich/govalidator"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoproto "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	_ "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
	"github.com/pensando/sw/venice/utils/testutils"
)

func TestDbPathGet(t *testing.T) {
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
			options:<[venice.objectPrefix]:{Collection: "prefix", Path:"{nest2_field}/{leaf_field}"}>
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
			options:<[venice.objectPrefix]:{Collection:"prefix-{leaf_field}", Path:"/qual{real_field.leaf_field}"}>
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
			options:<[venice.objectPrefix]:{Collection:"prefix", Path:"{embedded_field"}>
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

	t.Logf("Test dbPath with [{collection:'prefix-{leaf_field}', Path:'qual{real_field.leaf_field}'}]")
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
		t.Errorf("result does not match got %+v", paths)
	}

	t.Logf("Test dbPath with [{Collection: 'prefix', Path:'{nest2_field}/{leaf_field}'}]")
	testmsg, err = r.LookupMsg("", ".example.Nest1")
	result = []KeyComponent{
		{Type: "prefix", Val: "prefix/"},
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
		t.Errorf("getDbKey succeeded on faulty specification")
	}
}

func cleanupTmpFiles(t *testing.T, paths []string) {
	for _, path := range paths {
		if _, err := os.Stat(path); os.IsNotExist(err) {
			t.Logf("file not found, no cleanup needed")
		} else {
			err = os.Remove(path)
			if err != nil {
				t.Fatalf("failed to remove existing temp relations file")
			}
		}
	}
}
func TestGenRelMap(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		message_type <
			name: 'RelationObj'
			field <
				name: 'relation_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example/ToObjMsg">>
			>
		>
		message_type <
			name: 'ToObjMsg'
			field <
				name: 'back_ref_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'another_ref__field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example/ExternalRefMsg">>
			>
		>
		syntax: "proto3"
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'ExternalRefMsg'
			field <
				name: 'back_ref_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
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
	path := "_test_relations.json"
	paths := []string{path}
	defer cleanupTmpFiles(t, paths)

	t.Logf("Setting up registry")
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	// Delete any existing files
	t.Logf("Deleting any existing file")

	if _, err := os.Stat(path); os.IsNotExist(err) {
		t.Logf("file not found, no cleanup needed")
	} else {
		err = os.Remove(path)
		if err != nil {
			t.Fatalf("failed to remove existing temp relations file")
		}
	}

	relobj, err := r.LookupMsg("", ".example.RelationObj")
	if err != nil {
		t.Fatalf("Cound not lookup message (%s)", err)
	}
	for _, f := range relobj.Fields {
		err := addRelations(f)
		if err != nil {
			t.Fatalf("error processing field (%s)", err)
		}
	}
	if len(relMap) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(relMap))
	}
	if len(relMap["example.RelationObj"]) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(relMap["example.ToObjMsg"]))
	}
	toobj, err := r.LookupMsg("", ".example.ToObjMsg")
	if err != nil {
		t.Fatalf("failed to get message (%s)", err)
	}
	for _, f := range toobj.Fields {
		err := addRelations(f)
		if err != nil {
			t.Fatalf("error processing field (%s)", err)
		}
	}
	if len(relMap) != 2 {
		t.Fatalf("expected [2] messages, found [%d] [%+v]", len(relMap), relMap)
	}
	if len(relMap["example.ToObjMsg"]) != 1 {
		t.Fatalf("expected [2] relations, found [%d]", len(relMap["example.ToObjMsg"]))
	}

	t.Logf("Writing relations Map")
	str, err := genRelMap(path)
	if err != nil {
		t.Fatalf("failed to generate relations map file (%s)", err)
	}
	tmap := make(map[string][]relationRef)
	err = json.Unmarshal([]byte(str), &tmap)
	if err != nil {
		t.Fatalf("failed to unamrshal generated relations map (%s)", err)
	}
	if len(tmap) != 2 {
		t.Fatalf("expected [2] messages, found [%d] [%+v]", len(tmap), tmap)
	}
	if item, ok := tmap["example.RelationObj"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}
	if item, ok := tmap["example.ToObjMsg"]; ok && len(item) != 1 {
		t.Fatalf("expected [2] relations, found [%d]", len(item))
	}

	// Write the the file
	err = ioutil.WriteFile(path, []byte(str), 0644)
	if _, err := os.Stat(path); os.IsNotExist(err) {
		t.Fatalf("relation map file not found")
	}

	t.Logf("add more relations with existing file")
	relMap = make(map[string][]relationRef)

	anotherobj, err := r.LookupMsg("", ".example.ExternalRefMsg")
	if err != nil {
		t.Fatalf("failed to get message (%s)", err)
	}
	for _, f := range anotherobj.Fields {
		err := addRelations(f)
		if err != nil {
			t.Fatalf("error processing field (%s)", err)
		}
	}
	if len(relMap) != 0 {
		t.Fatalf("expected [0] messages, found [%d] [%+v]", len(relMap), relMap)
	}
	t.Logf("Writing relations with existing file")
	str, err = genRelMap(path)
	tmap = make(map[string][]relationRef)
	err = json.Unmarshal([]byte(str), &tmap)
	if err != nil {
		t.Fatalf("failed to unamarshal generated relations map (%s)", err)
	}
	if len(tmap) != 2 {
		t.Fatalf("expected [2] messages, found [%d] [%+v]", len(tmap), tmap)
	}
	if item, ok := tmap["example.RelationObj"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}
	if item, ok := tmap["example.ToObjMsg"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}
	// reset the in memory relMap
	relMap = make(map[string][]relationRef)
	rmap, err := genRelMapGo(path)
	if err != nil {
		t.Fatalf("failed to generate go relations map (%s)", err)
	}
	t.Logf("Relmap is [%+v]", rmap)
	if len(rmap.Map) != 2 {
		t.Fatalf("expected [2] messages, found [%d] [%+v]", len(tmap), tmap)
	}
	if item, ok := rmap.Map["example.RelationObj"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}
	if item, ok := rmap.Map["example.ToObjMsg"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}
}

func TestGenSwaggerIndex(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		message_type <
			name: 'RelationObj'
			field <
				name: 'relation_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example/ToObjMsg">>
			>
		>
		syntax: "proto3"
		`, `
		name: 'example1.proto'
		package: 'example1'
		message_type <
			name: 'RelationObj'
			field <
				name: 'relation_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example/ToObjMsg">>
			>
		>
		syntax: "proto3"
		`, `
		name: 'example2.proto'
		package: 'bookstore'
		message_type <
			name: 'RelationObj'
			field <
				name: 'relation_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example/ToObjMsg">>
			>
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		fds = append(fds, &fd)
	}
	path := "_test_relations.json"
	paths := []string{path}
	defer cleanupTmpFiles(t, paths)
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}

	file1, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("file not found example.proto (%s)", err)
	}
	file2, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("file not found example.proto (%s)", err)
	}
	file3, err := r.LookupFile("example2.proto")
	if err != nil {
		t.Fatalf("file not found example.proto (%s)", err)
	}

	// cleanup any existing files
	cleanupTmpFiles(t, paths)
	out, err := genSwaggerIndex(file1, path)
	got := make(map[string]swaggerFile)
	err = json.Unmarshal([]byte(out), &got)
	if err != nil {
		t.Fatalf("failed to parse output (%s)", err)
	}
	if len(got) != 1 {
		t.Fatalf("expecting 1 entry  got [%d]", len(got))
	}
	if _, ok := got["example"]; !ok {
		t.Fatalf("expecting entry for example")
	}
	// write the file
	err = ioutil.WriteFile(path, []byte(out), 0644)
	if err != nil {
		t.Fatalf("could not write output file (%s)", err)
	}

	out, err = genSwaggerIndex(file2, path)
	got = make(map[string]swaggerFile)
	err = json.Unmarshal([]byte(out), &got)
	if err != nil {
		t.Fatalf("failed to parse output (%s)", err)
	}
	if len(got) != 2 {
		t.Fatalf("expecting 1 entry  got [%d]", len(got))
	}
	if _, ok := got["example"]; !ok {
		t.Fatalf("expecting entry for example")
	}
	if _, ok := got["example1"]; !ok {
		t.Fatalf("expecting entry for example1")
	}
	// write the file
	err = ioutil.WriteFile(path, []byte(out), 0644)
	if err != nil {
		t.Fatalf("could not write output file (%s)", err)
	}

	// We should not see an entry for the bookstore group
	out, err = genSwaggerIndex(file3, path)
	got = make(map[string]swaggerFile)
	err = json.Unmarshal([]byte(out), &got)
	if err != nil {
		t.Fatalf("failed to parse output (%s)", err)
	}
	if len(got) != 2 {
		t.Fatalf("expecting 1 entry  got [%d]", len(got))
	}
	if _, ok := got["example"]; !ok {
		t.Fatalf("expecting entry for example")
	}
	if _, ok := got["example1"]; !ok {
		t.Fatalf("expecting entry for example1")
	}
	// write the file
	err = ioutil.WriteFile(path, []byte(out), 0644)
	if err != nil {
		t.Fatalf("could not write output file (%s)", err)
	}
}
func TestDetitle(t *testing.T) {
	var tests = []struct {
		input    string
		expected string
		err      error
	}{
		{"DetitleThis", "detitleThis", nil},
		{"dEtitleThis", "dEtitleThis", nil},
		{"detitleThis", "detitleThis", nil},
		{"D", "d", errors.New("someerror")},
		{"d", "d", errors.New("someerror")},
	}
	for _, test := range tests {
		ret, err := detitle(test.input)
		if test.err != nil && err == nil {
			t.Errorf("Expected error but got [%s]", err)
		}
		if err == nil && ret != test.expected {
			t.Errorf("expecting [%s] found [%s]", test.expected, ret)
		}
	}
}

func TestGetTypes(t *testing.T) {
	var fd descriptor.FileDescriptorProto
	src := `
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
			options:<[venice.objectPrefix]:{Collection:"prefix", Path:"{nest2_field}/{leaf_field}"}>
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
			options:<[venice.objectPrefix]:{Collection:"prefix" Path:"{leaf_field}/qual{real_field.leaf_field}"}>
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc'
				input_type: '.example.Nest1'
				output_type: '.example.testmsg'
			>
		>
		syntax: "proto3"
	`
	if err := proto.UnmarshalText(src, &fd); err != nil {
		t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
	}
	var fds []*descriptor.FileDescriptorProto
	fds = append(fds, &fd)
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	method := file.Services[0].Methods[0]
	resp := getInputType("example", method, true)
	if resp != "example.Nest1" {
		t.Errorf("expecting input type [example.Nest1] got [%s]", resp)
	}
	resp = getInputType("example", method, false)
	if resp != "Nest1" {
		t.Errorf("expecting input type [example.Nest1] got [%s]", resp)
	}
	resp = getOutputType("example", method, true)
	if resp != "example.testmsg" {
		t.Errorf("expecting input type [example.testmsg] got [%s]", resp)
	}
	resp = getOutputType("example", method, false)
	if resp != "testmsg" {
		t.Errorf("expecting input type [example.testmsg] got [%s]", resp)
	}
	// Get Json Tags
	msg, err := r.LookupMsg("", ".example.testmsg")
	if err != nil {
		t.Errorf("expecting to find testmsg")
	}
	fld := msg.Fields[0]
	tag := common.GetJSONTag(fld)
	if tag != "metadata" {
		t.Errorf("failed to get JSON tag")
	}
	fld = msg.Fields[1]
	tag = common.GetJSONTag(fld)
	if tag != "" {
		t.Errorf("failed to get JSON tag")
	}

	tag, err = getJSONTagByName(msg, "embedded_msg")
	if err != nil {
		t.Fatalf("expecting to succeed")
	}
	if tag != "metadata" {
		t.Fatalf("unexpected output [%v]", tag)
	}
}

func TestGetPackageCrudObjs(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
		>
		message_type <
			name: 'testmsg'
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
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg">
		>
		service <
			name: 'full_crudservice'
			options:<[venice.apiGrpcCrudService]:"Nest1">
		>
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
		>
		service <
			name: 'crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest2">
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		fds = append(fds, &fd)
	}
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	out, err := getPackageCrudObjects(file)
	if err != nil {
		t.Fatalf("failed to get getPackageCrudObjects()")
	}
	if len(out) != 2 {
		t.Fatalf("failed to get correct getPackageCrudObjects()")
	}
}

func TestGetSvcCrudObjs(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
		>
		message_type <
			name: 'testmsg'
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
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg">
		>
		service <
			name: 'full_crudservice'
			options:<[venice.apiGrpcCrudService]:"Nest1">
			method: <
			name: 'dummy'
			input_type: '.example.Nest1'
			output_type: '.example.Nest1'
		>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		fds = append(fds, &fd)
	}
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	for _, svc := range file.Services {
		out, err := getSvcCrudObjects(svc)
		if err != nil {
			t.Fatalf("failed to get getPackageCrudObjects()")
		}

		if *svc.Name == "hybrid_crudservice" {
			if len(out) != 2 {
				t.Fatalf("failed to get correct getSvcCrudObjects()")
			}
		}
		if *svc.Name == "full_crudservice" {
			if len(out) != 1 {
				t.Fatalf("failed to get correct getSvcCrudObjects()")
			}
		}
	}

}

func TestGetActionEndpoints(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
		>
		message_type <
			name: 'testmsg'
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
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg"
			[venice.apiAction]: {Collection: "Nest1", Action:"nestaction1", Request:"testmsg", Response:"Nest1"}
			[venice.apiAction]: {Object: "Nest1", Action:"nestobjaction1", Request:"testmsg", Response:"Nest1"}
			[venice.apiAction]: {Collection: "testmsg", Action:"testmsgaction1", Request:"testmsg", Response:"Nest1"}
		  [venice.apiAction]: {Collection: "Nest1", Action:"nestaction2", Request:"Nest1", Response:"testmsg"}>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		fds = append(fds, &fd)
	}
	r := reg.NewRegistry()
	var req gogoplugin.CodeGeneratorRequest
	req.FileToGenerate = []string{"example.proto"}
	req.ProtoFile = fds
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	expected := make(map[string][]ActionEndpoints)
	expected["Nest1"] = []ActionEndpoints{
		{Name: "Nestaction1", Request: "testmsg", Response: "Nest1"},
		{Name: "Nestobjaction1", Request: "testmsg", Response: "Nest1"},
		{Name: "Nestaction2", Request: "Nest1", Response: "testmsg"},
	}
	expected["testmsg"] = []ActionEndpoints{
		{Name: "Testmsgaction1", Request: "testmsg", Response: "Nest1"},
	}

	found := make(map[string][]ActionEndpoints)
	msgs := []string{"Nest1", "testmsg"}
	for _, svc := range file.Services {
		for _, m := range msgs {
			a, err := getSvcActionEndpoints(svc, m)
			if err != nil {
				t.Errorf("getActionEndpoint failed for %s/%s", *svc.Name, a)
			}
			for _, v := range a {
				old := found[m]
				found[m] = append(old, v)
			}
		}
	}
	for _, m := range msgs {
		if !reflect.DeepEqual(found[m], expected[m]) {
			t.Errorf("Mismatched [%s]\n  Expected %+v \n  found %+v", m, expected[m], found[m])
		}
	}
}

func TestGetParams(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
			field <
				name: 'str_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
			field <
				name: 'O'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.ObjectMeta'
				number: 3
			>
			options:<[venice.objectPrefix]:{Collection:"prefix-{str_field}", Path:"/qual{nest1_field.embedded_field}"}>
		>
		message_type <
			name: 'testmsg'
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
			field <
				name: 'O'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.ObjectMeta'
				number: 4
			>
			options:<[venice.objectPrefix]:{Collection:"prefix1", Path:"/{leaf_field}"}>
		>
		message_type <
			name: 'Auto_ListNest1'
			field <
				name: 'meta'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
			field <
				name: 'Items'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest1'
				number: 2
			>
		>
		message_type <
		name: 'ObjectMeta'
		field <
			name: 'Name'
			label: LABEL_OPTIONAL
			type: TYPE_STRING
			number: 2
		>
	>
		service <
			name: 'full_crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1">
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc_create'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"create" [venice.methodAutoGen]: true [google.api.http]:<selector:"" post:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_update'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"update" [venice.methodAutoGen]: true [google.api.http]:<selector:"" put:"/prefix/{str_field}">>
			>
			method: <
				name: 'noncrudsvc_get'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"get" [venice.methodAutoGen]: true [google.api.http]:<selector:"" get:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_delete'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"delete" [venice.methodAutoGen]: true [google.api.http]:<selector:"" delete:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_list'
				input_type: '.example.Nest1'
				output_type: '.example.Auto_ListNest1'
				options:<[venice.methodOper]:"list" [venice.methodAutoGen]: true [google.api.http]:<selector:"" get:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_action'
				input_type: '.example.Nest1'
				output_type: '.example.testmsg'
				options:<[venice.methodOper]:"create" [venice.methodAutoGen]: true [venice.methodAutoGen]: true [venice.methodActionObject]: "Nest1" [google.api.http]:<selector:"" get:"/prefix">>
			>
			method: <
				name: 'noncrudsvc_watch'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"watch" [venice.methodAutoGen]: true>
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg" [venice.apiRestService]: {Object: "Nest1", Method: [ "put", "post" ], Pattern: "/testpattern"}>
		>
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
		>
		service <
			name: 'crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest2">
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	for _, svc := range file.Services {
		if *svc.Name != "hybrid_crudservice" {
			continue
		}
		sparams, err := getSvcParams(svc)
		if err != nil {
			t.Errorf("error getting service params")
		}
		if sparams.Version != "v1" || sparams.Prefix != "example" {
			t.Errorf("Did not get expected service params %v", sparams)
		}
		for _, meth := range svc.Methods {
			mparams, err := getMethodParams(meth)
			if err != nil {
				t.Errorf("error getting method params")
			}
			switch *meth.Name {
			case "noncrudsvc_create":
				if mparams.Oper != "CreateOper" {
					t.Errorf("expecting CreateOper got %s", mparams.Oper)
				}
			case "noncrudsvc_update":
				if mparams.Oper != "UpdateOper" {
					t.Errorf("expecting UpdateOper got %s", mparams.Oper)
				}
			case "noncrudsvc_get":
				if mparams.Oper != "GetOper" {
					t.Errorf("expecting GetOper got %s", mparams.Oper)
				}
			case "noncrudsvc_delete":
				if mparams.Oper != "DeleteOper" {
					t.Errorf("expecting DeleteOper got %s", mparams.Oper)
				}
			case "noncrudsvc_list":
				if mparams.Oper != "ListOper" {
					t.Errorf("expecting ListOper got %s", mparams.Oper)
				}
			case "noncrudsvc_action":
				if mparams.Oper != "CreateOper" {
					t.Errorf("expecting CreateOper got %s", mparams.Oper)
				}
			case "noncrudsvc_watch":
				if mparams.Oper != "WatchOper" {
					t.Errorf("expecting WatchOper got %s", mparams.Oper)
				}
			default:
				t.Errorf("unknown type %s", mparams.Oper)
			}
			if *meth.Name == "noncrudsvc_watch" {
				continue
			}
			restOper, err := getAutoRestOper(meth)
			if err != nil {
				t.Errorf("error getting method rest oper for [%s]", *meth.Name)
			}
			switch *meth.Name {
			case "noncrudsvc_create":
				if restOper != "POST" {
					t.Errorf("expecting POST got [%s]", restOper)
				}
			case "noncrudsvc_update":
				if restOper != "PUT" {
					t.Errorf("expecting PUT got [%s]", restOper)
				}
			case "noncrudsvc_get":
				if restOper != "GET" {
					t.Errorf("expecting GET got [%s]", restOper)
				}
			case "noncrudsvc_delete":
				if restOper != "DELETE" {
					t.Errorf("expecting DELETE got [%s]", restOper)
				}
			case "noncrudsvc_list":
				if restOper != "GET" {
					t.Errorf("expecting GET got [%s]", restOper)
				}
			case "noncrudsvc_action":
				if restOper != "POST" {
					t.Errorf("expecting POST got [%s]", restOper)
				}
			case "noncrudsvc_watch":
				if restOper != "" {
					t.Errorf("expecting [] got [%s]", restOper)
				}
			default:
				t.Errorf("unknown method [%s]", mparams.Oper)
			}

			result1 := URIKey{
				Ref: true,
				Str: "\"example/v1/qual\", in.nest1_field.embedded_field, \"/prefix-\", in.str_field, \"/\", in.O.Name",
			}
			msgResult1 := URIKey{
				Ref: true,
				Str: "\"example/qual\", in.nest1_field.embedded_field, \"/prefix-\", in.str_field, \"/\", in.O.Name",
			}
			keys, err := getURIKey(meth, "v1", false)
			if err != nil {
				t.Errorf("error getting method URI key for [%s](%s)", *meth.Name, err)
			}
			msgKeys, err := getMsgURIKey(meth.RequestType, "example")
			if err != nil {
				t.Errorf("error getting message URI key for [%s.%s](%s)", *meth.Name, *meth.RequestType.Name, err)
			}
			switch *meth.Name {
			case "noncrudsvc_get", "noncrudsvc_delete", "noncrudsvc_list", "noncrudsvc_create", "noncrudsvc_update":
				if !reflect.DeepEqual(result1, keys) {
					t.Errorf("key components do not match for [%s] got [%+v] want [%+v]", *meth.Name, keys, result1)
				}
				if !reflect.DeepEqual(msgResult1, msgKeys) {
					t.Errorf("key components (msg) do not match for [%s] got [%+v] want [%+v]", *meth.Name, msgKeys, result1)
				}
			case "noncrudsvc_action":
				if !isActionMethod(meth) {
					t.Errorf("expection actionMethod to be true")
				}
				target, err := getActionTarget(meth)
				if err != nil {
					t.Errorf("Expecting to succeed got (%s)", err)
				}
				if target != "Nest1" {
					t.Errorf("expecint [Nest1] got [%v]", target)
				}
			}
			if !isRestExposed(meth) {
				t.Errorf("RestExposed returned false expecting true for [%s]", *meth.Name)
			}
			if !common.IsAutoGenMethod(meth) {
				t.Errorf("autoGenMethod returned false expecting true for [%s]", *meth.Name)
			}
		}
	}
	pmap, keys := genPathsMap(file)
	if len(pmap) != 2 || len(keys) != 2 {
		t.Fatalf("retrieved wrong number of elements in paths map")
	}
}

func TestGetSwaggerFile(t *testing.T) {
	for _, v := range []struct{ input, expected string }{
		{input: "xyz.proto", expected: "xyz.swagger.json"},
		{input: "abc/xyz.proto", expected: "xyz.swagger.json"},
	} {
		output, err := getSwaggerFileName(v.input)
		if err != nil {
			t.Errorf("error getting swagger file name (%s)", err)
		}
		if output != v.expected {
			t.Errorf("expecting [%s] got [%s]", v.expected, output)
		}
	}

}

func TestGetNimbusManifest(t *testing.T) {
	mfest, err := genNimbusManifest("/tmp/nimbus", "Endpoint", "EndpointApi")
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}

	if len(mfest) == 0 {
		t.Errorf("genManifest failed (%s)", err)
	}

	ofile, err := os.OpenFile("/tmp/nimbus", os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0755)
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}

	for _, entry := range mfest {
		ofile.WriteString(entry.Object + " " + entry.Service + "\n")
	}
	ofile.Close()

	mfest, err = genNimbusManifest("/tmp/nimbus", "Interface", "InterfaceApi")
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}

	if len(mfest) != 2 {
		t.Errorf("genManifest failed (%s)", err)
	}

	mfest, err = getNimbusManifest("/tmp/nimbus")

	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}

	if len(mfest) == 2 {
		t.Errorf("genManifest failed (%s)", err)
	}
	mfest, err = getNimbusManifest("/tmp/nimbusXXX")
	if err == nil {
		t.Errorf("got manifest for non-existent file")
	}
}

func TestGetManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example1.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileGrpcDest]: "localhost:8082">
		`,
		`
		name: 'example2.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileApiServerBacked]: false>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example1.proto", "example2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	file, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	filepath := "/nonexistent/filenameXXXX"
	manifest, err := genManifest(file, filepath, "example", "example1.proto")
	// Was empty file so we need to have a manifest with single element.
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	if len(manifest) != 1 {
		t.Errorf("expecting 1 entry found %d", len(manifest))
	}
	if v, ok := manifest["example1.proto"]; !ok || v.Pkg != "example" || v.APIServer != true {
		t.Errorf("did not find key [%v] or [%v]", ok, v)
	}
	file, err = r.LookupFile("example2.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	manifest, err = genManifest(file, filepath, "example", "example2.proto")
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	if v, ok := manifest["example2.proto"]; !ok || v.Pkg != "example" || v.APIServer != false {
		t.Errorf("did not find key [%v] or [%+v]", ok, v)
	}
	// Now add a few existing entries
	fileinput := []byte("\nexample1.proto example1 true\nexample2.proto example2 false\n")
	manifest = parseManifestFile(fileinput)
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	if len(manifest) != 2 {
		t.Errorf("expecting 1 entry found %d", len(manifest))
	}
	if v, ok := manifest["example1.proto"]; !ok || v.Pkg != "example1" {
		t.Errorf("did not find key [%v] or [%v]", ok, v.Pkg)
	}
	if v, ok := manifest["example2.proto"]; !ok || v.Pkg != "example2" {
		t.Errorf("did not find key [%v] or [%v]", ok, v.Pkg)
	}
}

func TestGenPkgManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example1.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileGrpcDest]: "localhost:8082">
		`,
		`
		name: 'example2.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileApiServerBacked]: false>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example1.proto", "example2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	file, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	filepath := "/nonexistent/filenameXXXX"
	manifest, err := genPkgManifest(file, filepath, "example", "example1.proto")
	// Was empty file so we need to have a PkgManifest with single element.
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	if len(manifest) != 1 {
		t.Fatalf("expecting 1 entry found %d", len(manifest))
	}
	if p, ok := manifest["example"]; !ok {
		t.Errorf("did not find package in manifest")
	} else {
		if len(p.Files) != 1 {
			t.Errorf("unexpected number of files, exp 1 got %d[%s]", len(p.Files), p.Files)
		}
		if p.Files[0] != "example1.proto" {
			t.Errorf("Unexpected file in manifest [%s]", p.Files)
		}
		if p.APIServer != true {
			t.Errorf("invalid APIServer flag")
		}
	}
	fileinput := []byte("\ntrue example example1.proto example2.proto\ntrue example2 example2.proto xxx.proto yyy.proto zzz.proto\n")
	manifest = parsePkgManifest(fileinput)
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	if len(manifest) != 2 {
		t.Fatalf("invalid number of manifest entries while parsing %d[%v]", len(manifest), manifest)
	}
	if p, ok := manifest["example"]; !ok {
		t.Errorf("did not find package example in manifest")
	} else {
		if len(p.Files) != 2 {
			t.Errorf("expecting 2 files got %d", len(p.Files))
		}
		exp := []string{"example1.proto", "example2.proto"}
		if !reflect.DeepEqual(exp, p.Files) {
			t.Errorf("manigest files do not match [%v][%v]", p.Files, exp)
		}
	}
	if p, ok := manifest["example2"]; !ok {
		t.Errorf("did not find package example in manifest")
	} else {
		if len(p.Files) != 4 {
			t.Errorf("expecting 2 files got %d", len(p.Files))
		}
		exp := []string{"example2.proto", "xxx.proto", "yyy.proto", "zzz.proto"}
		if !reflect.DeepEqual(exp, p.Files) {
			t.Errorf("manigest files do not match [%v][%v]", p.Files, exp)
		}
	}

	_, err = getSwaggerMD(file, filepath)
	if err != nil {
		t.Fatalf("could not get swagger MD")
	}

	_, err = getAPIRefMD(file, filepath)
	if err != nil {
		t.Fatalf("could not get API ref MD")
	}
}

func TestGetSvcKeyFromManifest(t *testing.T) {
	manifiest := `
	{
		"test-service": {
			"Svcs": {
	
				"OrchestratorV1": {
					"Version": "v1",
					"Messages": [
						"Orchestrator"
					],
					"Properties": {
						"Orchestrator": {
							"Scopes": [
								"cluster"
							],
							"RestMethods": [
								"put",
								"get",
								"delete",
								"post",
								"list",
								"watch"
							],
							"URI": "/configs/orchestrator/v1/orchestrator"
						},
						"Role": {
							"Scopes": [
								"tenant"
							],
							"RestMethods": [
								"get",
								"put",
								"delete",
								"post",
								"list",
								"watch"
							],
							"URI": "/configs/auth/v1/roles"
						},
						"RoleBinding": {
							"Scopes": [
								"tenant"
							],
							"RestMethods": [
								"get",
								"put",
								"delete",
								"post",
								"list",
								"watch"
							],
							"URI": "/configs/auth/v1/role-bindings"
						}
					}
				}
			},
			"Files": [
				"svc_testn.proto"
			]
		}
	}
	`
	f, err := os.Create("/tmp/genmanifest")
	if err != nil {
		t.Fatal("Error creating file")
	}
	defer f.Close()
	f.WriteString(manifiest)
	if err != nil {
		t.Fatal("Error writing file")
	}

	key, err := getServiceKey("/tmp/genmanifest", "test-service", "OrchestratorV1", "Orchestrator")
	if err != nil {
		t.Fatal("Error getting service key")
	}

	if key != "orchestrator" {
		t.Fatal("Service key don't match")
	}

	key, err = getServiceKey("/tmp/genmanifest", "test-service", "OrchestratorV1", "RoleBinding")
	if err != nil {
		t.Fatal("Error getting service key")
	}

	if key != "role-bindings" {
		t.Fatal("Service key don't match")
	}

	key, err = getServiceKey("/tmp/genmanifest", "test-service", "OrchestratorV1", "Role")
	if err != nil {
		t.Fatal("Error getting service key")
	}

	if key != "roles" {
		t.Fatal("Service key don't match")
	}
	_, err = getServiceKey("/tmp/genmanifestXXXX", "test-service", "OrchestratorV1", "Orchestrator")
	if err == nil {
		t.Fatal("Got non-existent service key")
	}
}

func TestGetSvcManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
			field <
				name: 'O'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.ObjectMeta'
				number: 3
			>
			options:<[venice.objectPrefix]:{Collection: "prefix", Path:"xx"}>
		>
		message_type <
			name: 'testmsg'
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
			field <
				name: 'O'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.ObjectMeta'
				number: 3
			>
			options:<[venice.objectPrefix]:{Collection: "prefix", Path:"xx"}>
		>
		message_type <
			name: 'ObjectMeta'
			field <
				name: 'Name'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
		>
		service <
			name: 'full_crudservice'
			method: <
				name: 'dummy_method_to_simulated_mutated_proto'
				input_type: '.example.testmsg'
				output_type: '.example.testmsg'
				options:<[venice.methodOper]:"create">
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1", [venice.apiAction]:{Object:"Nest1", Action: "TestAction"}>
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'noncrudsvc'
				input_type: '.example.Nest1'
				output_type: '.example.Nest1'
				options:<[venice.methodOper]:"create">
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg" [venice.apiRestService]: {Object: "Nest1", Method: [ "put", "post" ], Pattern: "/testpattern"}>
		>
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
		>
		service <
			name: 'crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest2">
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	defer os.Remove("nonexistentfile")
	manifest, err := genServiceManifest("nonexistentfile", file)
	if err != nil {
		t.Errorf("failed to genServiceManifest (%s)", err)
	}
	expected := make(map[string]packageDef)

	pkg := packageDef{Svcs: make(map[string]serviceDef)}
	pkg.Files = append(expected["example"].Files, "example.proto")
	pkg.Svcs["hybrid_crudservice"] = serviceDef{
		Version:  "v1",
		Messages: []string{"Nest1", "testmsg"},
		Properties: map[string]messageDef{
			"Nest1": {
				Scopes:      []string{"cluster"},
				RestMethods: []string{"put", "post"},
				URI:         "/configs/example/v1/xx/prefix/{O.Name}",
			},
			"testmsg": {
				Scopes: []string{"cluster"},
				URI:    "/configs/example/v1/xx/prefix/{O.Name}",
			},
		},
	}
	pkg.Svcs["full_crudservice"] = serviceDef{
		Version:  "v1",
		Messages: []string{"Nest1"},
		Properties: map[string]messageDef{
			"Nest1": {
				Scopes:  []string{"cluster"},
				Actions: []string{"TestAction"},
				URI:     "/configs/example/v1/xx/prefix/{O.Name}",
			},
		},
	}
	expected["example"] = pkg

	ret, err := json.MarshalIndent(expected, "", "  ")
	if err != nil {
		t.Fatalf("Json Unmarshall of svc manifest file failed")
	}
	if manifest != string(ret) {
		t.Errorf("result does not match [%+v] [%v]", manifest, string(ret))
	}

	_, err = genObjectURIs("nonexistentfile", file)
	if err != nil {
		t.Fatalf("could not get ObjectURIs")
	}
}

func TestGetAutoTypes(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'Nest1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
			>
		>
                message_type <
                        name: 'Auto_ListNest1'
                        field <
                                name: 'meta'
                                label: LABEL_OPTIONAL
                                type: TYPE_MESSAGE
                                type_name: '.example.Nest1'
                                number: 2
                        >
                        field <
                                name: 'Items'
                                label: LABEL_OPTIONAL
                                type: TYPE_MESSAGE
                                type_name: '.example.Nest1'
                                number: 2
                        >
                        options:<[venice.objectAutoGen]: "listhelper">
                >
		message_type <
			name: 'Auto_ListNest2'
		>
		message_type <
			name: 'Auto_WatchNest1'
			field <
				name: "Events"
				label: LABEL_REPEATED
				type: TYPE_MESSAGE
				type_name: '.example.Auto_WatchNest1.WatchEvent'
				number: 1
			>
			nested_type: <
				name: "WatchEvent"
				field <
						name: 'meta'
						label: LABEL_OPTIONAL
						type: TYPE_MESSAGE
						type_name: '.example.Nest1'
						number: 2
					>
				field <
						name: 'Object'
						label: LABEL_OPTIONAL
						type: TYPE_MESSAGE
						type_name: '.example.Nest1'
						number: 2
				>
				nested_type: <
					name: "DoubleNest"
					field <
							name: 'test'
							label: LABEL_OPTIONAL
							type: TYPE_MESSAGE
							type_name: '.example.Nest1'
							number: 2
						>
				>
			>
			options:<[venice.objectAutoGen]: "watchhelper">
		>
		service <
			name: 'hybrid_crudservice'
			method: <
				name: 'AutoCrudList'
				input_type: '.example.Nest1'
				output_type: '.example.Auto_ListNest1'
				options:<[venice.methodOper]:"list" [venice.methodAutoGen]: true>
			>
			method: <
				name: 'AutoCrudWatch'
				input_type: '.example.Nest1'
				output_type: '.example.Auto_ListNest1'
				options:<[venice.methodOper]:"watch" [venice.methodAutoGen]: true>
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1" [venice.apiGrpcCrudService]:"testmsg" [venice.apiRestService]: {Object: "Nest1", Method: [ "put", "post" ], Pattern: "/testpattern"}>
		>
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
		>
		message_type <
			name: 'MapMessage'
			field <
				name: 'key'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			options:<map_entry:true>
		>
		service <
			name: 'crudservice'
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest2">
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	for _, v := range file.Services[0].Methods {
		if *v.Name == "AutoCrudList" {
			if !isAutoList(v) || isAutoWatch(v) {
				t.Errorf("check for isAutoList failed")
			}
		}
		if *v.Name == "AutoCrudWatch" {
			if isAutoList(v) || !isAutoWatch(v) {
				t.Errorf("check for isAutoList failed")
			}
		}
	}
	lmsg2, err := r.LookupMsg("", ".example.Auto_ListNest2")
	if err != nil {
		t.Errorf("error finding ListNest2 auto message")
	}
	_, err = getMsgMetricOptionsHdlr(lmsg2)
	if err == nil {
		t.Errorf("got metric option for non-type message")
	}
	_, err = getListType(lmsg2, false)
	if err == nil {
		t.Errorf("failed to retrieve list type")
	}
	_, err = getListTypeMsg(lmsg2)
	if err == nil {
		t.Errorf("failed to retrieve list type")
	}
	lmsg, err := r.LookupMsg("", ".example.Auto_ListNest1")
	if err != nil {
		t.Errorf("error finding WatchNest1 auto message")
	}
	if !isListHelper(lmsg) {
		t.Errorf("test for list helper message failed")
	}
	if isWatchHelper(lmsg) {
		t.Errorf("test for watch helper message passed for list helper")
	}
	if str, err := getListType(lmsg, false); err != nil {
		t.Errorf("getListType returned error (%s)", err)
	} else {
		if str != "Nest1" {
			t.Errorf("list type expected [Nest1] got [%s]", str)
		}
	}
	if str, err := getListType(lmsg, true); err != nil {
		t.Errorf("getListType returned error (%s)", err)
	} else {
		if str != "example.Nest1" {
			t.Errorf("list type expected [example.Nest1] got [%s]", str)
		}
	}
	lmsgType, err := r.LookupMsg("", ".example.Nest1")
	if err != nil {
		t.Errorf("error finding nes1 message")
	}
	lmsgt, err := getListTypeMsg(lmsg)
	if err != nil {
		t.Errorf("failed to retrieve list type")
	}
	if lmsgt != lmsgType {
		t.Errorf("returned message type did not match")
	}
	wmsg, err := r.LookupMsg("", ".example.Auto_WatchNest1")
	if err != nil {
		t.Errorf("error finding WatchNest1 auto message")
	}
	if !isWatchHelper(wmsg) {
		t.Errorf("test for watch helper message failed")
	}
	if isListHelper(wmsg) {
		t.Errorf("test for list helper message passed for watch helper")
	}
	if str, err := getWatchType(wmsg, false); err != nil {
		t.Errorf("getWatchType returned error (%s)", err)
	} else {
		if str != "Nest1" {
			t.Errorf("watch type expected [Nest1] got [%s]", str)
		}
	}
	if str, err := getWatchType(wmsg, true); err != nil {
		t.Errorf("getWatchType returned error (%s)", err)
	} else {
		if str != "example.Nest1" {
			t.Errorf("watch type expected [example.Nest1] got [%s]", str)
		}
	}
	wmsgType, err := r.LookupMsg("", ".example.Nest1")
	if err != nil {
		t.Errorf("error finding nes1 message")
	}
	wmsgt, err := getWatchTypeMsg(wmsg)
	if err != nil {
		t.Errorf("failed to retrieve watch type")
	}
	if wmsgt != wmsgType {
		t.Errorf("returned message type did not match")
	}
	t.Logf("Test map_entry message")
	testmsg, err := r.LookupMsg("", ".example.MapMessage")
	if !isMapEntry(testmsg) {
		t.Errorf("Map entry message returned false")
	}

	t.Logf("Test Nested message")
	nmessage, err := r.LookupMsg("", ".example.Auto_WatchNest1.WatchEvent")
	if err != nil {
		t.Fatalf("error retrieving nested message (%s)", err)
	}
	if !isNestedMessage(nmessage) {
		t.Errorf("  Nested message returned false")
	}
	nname, err := getNestedMsgName(nmessage)
	if err != nil {
		t.Fatalf("error retrieving nested message name (%s)", err)
	}
	if nname != "Auto_WatchNest1_WatchEvent" {
		t.Errorf("expecting [Auto_WatchNest1_WatchEvent], got [%s]", nname)
	}

	nmessage, err = r.LookupMsg("", ".example.Auto_WatchNest1.WatchEvent.DoubleNest")
	if err != nil {
		t.Fatalf("error retrieving double nested message (%s)", err)
	}
	if !isNestedMessage(nmessage) {
		t.Errorf("  Nested message returned false")
	}
	nname, err = getNestedMsgName(nmessage)
	if err != nil {
		t.Fatalf("error retrieving nested message name (%s)", err)
	}
	if nname != "Auto_WatchNest1_WatchEvent_DoubleNest" {
		t.Errorf("expecting [Auto_WatchNest1_WatchEvent_DoubleNest], got [%s]", nname)
	}
}

func TestGetFileName(t *testing.T) {
	var tests = []struct {
		input    string
		expected string
	}{
		{"/trythis.proto", "Trythis"},
		{"/TryThis.exa", "TryThis"},
		{"aaa/this", "This"},
	}
	for _, test := range tests {
		ret := getFileName(test.input)
		if ret != test.expected {
			t.Errorf("expecting [%s] found [%s]", test.expected, ret)
		}
	}
}

func TestGetGrpcDestination(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileGrpcDest]: "localhost:8082">
		`,
		`
		name: 'example2.proto'
		package: 'example'
		syntax: 'proto3'
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "example2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	var tests = []struct {
		input    string
		expected string
	}{
		{input: "example.proto", expected: "localhost:8082"},
		{input: "example2.proto", expected: ""},
	}
	for _, test := range tests {
		file, err := r.LookupFile(test.input)
		if err != nil {
			t.Fatalf("Could not find file %s in request", test.input)
		}
		if test.expected != getGrpcDestination(file) {
			t.Errorf("expecting [%s] found [%s]", test.expected, getGrpcDestination(file))
		}
	}
}

func TestGetAPIOperType(t *testing.T) {
	cases := []string{"CreateOper", "UpdateOper", "GetOper", "DeleteOper", "ListOper", "WatchOper", "UnknowOper"}
	var pass bool
	for _, v := range cases {
		r, err := getAPIOperType(v)
		switch v {
		case "CreateOper":
			pass = (r == "create") && (err == nil)
		case "UpdateOper":
			pass = (r == "update") && (err == nil)
		case "GetOper":
			pass = (r == "get") && (err == nil)
		case "DeleteOper":
			pass = (r == "delete") && (err == nil)
		case "ListOper":
			pass = (r == "list") && (err == nil)
		case "WatchOper":
			pass = (r == "watch") && (err == nil)
		default:
			pass = (r == "unknown") && (err != nil)
		}
		if !pass {
			t.Errorf("failed for case %v", v)
		}
	}
}

func TestGetValidatorManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'msg1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.check]: "v1:OneStr(str)">
			>
			field <
				name: 'repeated_field'
				label: LABEL_REPEATED
				type: TYPE_STRING
				options:<[venice.check]: "MulInt(1,3)" [venice.check]: "v2:OneInt(11)">
				number: 2
			>
		>
		message_type <
			name: 'parentmsg'
			field <
				name: 'nestedfield'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.msg1'
				number: 2
			>
		>
		message_type <
			name: 'parentmsgtoanotherfile'
			field <
				name: 'nestedfield'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg1'
				number: 2
			>
		>
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'anothermsg1'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.check]: "v1:OneStr(str)">
			>
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	scratchMap := common.ValidatorArgMap
	common.ValidatorArgMap = map[string][]common.CheckArgs{
		"OneStr": {common.IsString},
		"MulStr": {common.IsString, common.IsString, common.IsString},
		"OneInt": {govldtr.IsInt},
		"MulInt": {govldtr.IsInt, govldtr.IsInt},
		"StrInt": {common.IsString, govldtr.IsInt},
	}
	exp := validators{
		Fmap: true,
		Map: map[string]validateMsg{
			"msg1": {
				Fields: map[string]validateFields{
					"nest1_field": {
						Repeated: false,
						Pointer:  false,
						Validators: []common.ValidateField{
							{
								Fn:   "OneStr",
								Ver:  "v1",
								Args: []string{"str"},
							},
						},
					},
					"repeated_field": {
						Repeated: true,
						Pointer:  false,
						Validators: []common.ValidateField{
							{
								Fn:   "MulInt",
								Ver:  "all",
								Args: []string{"1", "3"},
							},
							{
								Fn:   "OneInt",
								Ver:  "v2",
								Args: []string{"11"},
							},
						},
					},
				},
			},
			"parentmsg": {
				Fields: map[string]validateFields{
					"nestedfield": {
						Repeated:   false,
						Pointer:    true,
						Validators: []common.ValidateField{},
					},
				},
			},
			"parentmsgtoanotherfile": {
				Fields: map[string]validateFields{
					"nestedfield": {
						Repeated:   false,
						Pointer:    true,
						Validators: []common.ValidateField{},
					},
				},
			},
		},
	}

	v, err := getValidatorManifest(file)
	if err != nil {
		t.Fatalf("Could not generate validator manifest (%s)", err)
	}

	if !reflect.DeepEqual(v, exp) {
		t.Fatalf("generated manifest does not match")
	}

	common.ValidatorArgMap = scratchMap
}

func TestGetEumVMaps(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		enum_type:<
			name:"Enum1"
			value:<name:"value1" number:0 options:<[venice.enumValueStr]: "ModValue1"> >
			value:<name:"Value2" number:1 >
			value:<name:"VALUE3" number:2 options:<[venice.enumValueStr]: "mod-value-3"> >
		>
		message_type <
			name: 'Nest1'
			field <
				name: 'embedded_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			enum_type:<
			name:"EnumInner"
			value:<name:"Nestvalue1" number:0 options:<[venice.enumValueStr]: "NestModValue1"> >
			value:<name:"NestValue2" number:1 >
			value:<name:"NestVALUE3" number:2 options:<[venice.enumValueStr]: "nest-mod-value-3"> >
			>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}

	cases := []struct {
		in  string
		ok  bool
		exp string
	}{
		{
			in:  "TestMsg.TestEnum",
			ok:  true,
			exp: "TestMsg_TestEnum_vvalue",
		},
		{
			in:  "TestEnum",
			ok:  true,
			exp: "TestEnum_vvalue",
		},
		{
			in:  ".anotherproto.TestEnum",
			ok:  true,
			exp: "anotherproto.TestEnum_vvalue",
		},
	}
	for _, c := range cases {
		input := []string{c.in}
		ret, err := getEnumStrVMap(file, input)
		if (err == nil) != c.ok {
			t.Errorf("expecting success [%v] got [%v]", c.ok, (err == nil))
		}
		if ret != c.exp {
			t.Errorf("expecting result [%v] got [%v]", c.exp, ret)
		}
	}

	expNames := map[string]string{
		"Enum1":     "Enum1_vvalue",
		"EnumInner": "Nest1_EnumInner_vvalue",
	}
	for _, e := range file.Enums {
		exp, ok := expNames[*e.Name]
		if !ok {
			t.Fatalf("unexpected enum [%v]", *e.Name)
		}
		name, err := getEnumVValueName(e)
		if err != nil {
			t.Fatalf("Could not get getEnumVValueName(%s)", err)
		}
		if name != exp {
			t.Fatalf("got [%v] expecting [%v]", name, exp)
		}
	}

	expNames = map[string]string{
		"Enum1":     "Enum1_vname",
		"EnumInner": "Nest1_EnumInner_vname",
	}
	for _, e := range file.Enums {
		exp, ok := expNames[*e.Name]
		if !ok {
			t.Fatalf("unexpected enum [%v]", *e.Name)
		}
		name, err := getEnumVNameName(e)
		if err != nil {
			t.Fatalf("Could not get getEnumVNameName(%s)", err)
		}
		if name != exp {
			t.Fatalf("got [%v] expecting [%v]", name, exp)
		}
	}

	expVals := map[string]string{
		"EnumInner": "0:\"NestModValue1\",\n1:\"nestvalue2\",\n2:\"nest-mod-value-3\",\n",
		"Enum1":     "0:\"ModValue1\",\n1:\"value2\",\n2:\"mod-value-3\",\n",
	}
	for _, e := range file.Enums {
		exp, ok := expVals[*e.Name]
		if !ok {
			t.Fatalf("unexpected enum [%v]", *e.Name)
		}
		name, err := getEnumVNameValues(e)
		if err != nil {
			t.Fatalf("Could not get getEnumVNameName(%s)", err)
		}
		if name != exp {
			t.Fatalf("got [%v] expecting [%v]", name, exp)
		}
	}
	expVals = map[string]string{
		"EnumInner": "\"NestModValue1\":0,\n\"nestvalue2\":1,\n\"nest-mod-value-3\":2,\n",
		"Enum1":     "\"ModValue1\":0,\n\"value2\":1,\n\"mod-value-3\":2,\n",
	}
	for _, e := range file.Enums {
		exp, ok := expVals[*e.Name]
		if !ok {
			t.Fatalf("unexpected enum [%v]", *e.Name)
		}
		name, err := getEnumVValueValues(e)
		if err != nil {
			t.Fatalf("Could not get getEnumVNameName(%s)", err)
		}
		if name != exp {
			t.Fatalf("got [%v] expecting [%v]", name, exp)
		}
		t.Logf("got %s ", name)
	}
}

func TestGetEumStrMap(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	cases := []struct {
		in  string
		ok  bool
		exp string
	}{
		{
			in:  "TestMsg.TestEnum",
			ok:  true,
			exp: "TestMsg_TestEnum_vvalue",
		},
		{
			in:  "TestEnum",
			ok:  true,
			exp: "TestEnum_vvalue",
		},
		{
			in:  ".anotherproto.TestEnum",
			ok:  true,
			exp: "anotherproto.TestEnum_vvalue",
		},
	}
	for _, c := range cases {
		input := []string{c.in}
		ret, err := getEnumStrMap(file, input)
		if (err == nil) != c.ok {
			t.Errorf("expecting success [%v] got [%v]", c.ok, (err == nil))
		}
		if ret != c.exp {
			t.Errorf("expecting result [%v] got [%v]", c.exp, ret)
		}
	}
}

func TestGetEventTypes(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		enum_type:<
			name:"EventTypes"
			value:<
				name:"TYPE1"
				number:0
				options:<[eventtypes.severity]:INFO [eventtypes.category]:Cluster [eventtypes.desc]:"Type 1" > 
			>
			value:<
				name:"TYPE2"
				number:0
				options:<[eventtypes.severity]:INFO [eventtypes.category]:Cluster [eventtypes.desc]:"Type 2" > 
			>
			value:<
				name:"TYPE3"
				number:0
				options:<[eventtypes.severity]:WARN [eventtypes.category]:System [eventtypes.desc]:"Type 3" > 
			>
			value:<
				name:"TYPE4"
				number:0
				options:<[eventtypes.severity]:DEBUG [eventtypes.category]:Network [eventtypes.desc]:"Type 4" > 
			>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}

	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}

	eTypes, err := getEventTypes(file)
	testutils.AssertOk(t, err, "failed to get event types from the proto")

	expectedResponse := []*EventType{
		{EType: "TYPE1", Severity: eventattrs.Severity_INFO.String(), Category: eventattrs.Category_Cluster.String(),
			Desc: "Type 1"},
		{EType: "TYPE2", Severity: eventattrs.Severity_INFO.String(), Category: eventattrs.Category_Cluster.String(), Desc: "Type 2"},
		{EType: "TYPE3", Severity: eventattrs.Severity_WARN.String(), Category: eventattrs.Category_System.String(),
			Desc: "Type 3"},
		{EType: "TYPE4", Severity: eventattrs.Severity_DEBUG.String(), Category: eventattrs.Category_Network.String(),
			Desc: "Type 4"},
	}
	testutils.Assert(t, reflect.DeepEqual(eTypes, expectedResponse), "plugin did not generate expected event types")
}

func TestGetEumNormalizedMap(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		enum_type:<
			name:"Enum1"
			value:<name:"value1" number:0 options:<[venice.enumValueStr]: "value1mod"> >
			value:<name:"Value2" number:1 >
			value:<name:"VALUE3" number:2 options:<[venice.enumValueStr]: "Value3-mod">>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	cases := []struct {
		in  string
		ok  bool
		exp string
	}{
		{
			in:  "TestMsg.TestEnum",
			ok:  true,
			exp: "TestMsg_TestEnum_normal",
		},
		{
			in:  "TestEnum",
			ok:  true,
			exp: "TestEnum_normal",
		},
		{
			in:  ".anotherproto.TestEnum",
			ok:  true,
			exp: "anotherproto.TestEnum_normal",
		},
	}
	for _, c := range cases {
		input := []string{c.in}
		ret, err := getEnumStrNormalMap(file, input)
		if (err == nil) != c.ok {
			t.Errorf("expecting success [%v] got [%v]", c.ok, (err == nil))
		}
		if ret != c.exp {
			t.Errorf("expecting result [%v] got [%v]", c.exp, ret)
		}
	}

	e := file.Enums[0]
	name, err := getNormalizedEnumName(e)
	if err != nil {
		t.Fatalf("Could not get getNormalizedEnumName(%s)", err)
	}
	if name != "Enum1_normal" {
		t.Fatalf("expecting Enum1_normal got [%v]", name)
	}

	e.Outers = []string{"Out1", "Out2"}
	name, err = getNormalizedEnumName(e)
	if err != nil {
		t.Fatalf("Could not get getNormalizedEnumName(%s)", err)
	}
	if name != "Out1_Out2_Enum1_normal" {
		t.Fatalf("expecting Enum1_normal got [%v]", name)
	}

	values, err := getNormalizedEnum(e)
	if err != nil {
		t.Fatalf("Could not get getNormalizedEnumName(%s)", err)
	}
	values = strings.TrimSpace(values)
	values = strings.Replace(values, " ", "", -1)
	expVal := "\"Value3-mod\":\"Value3-mod\",\n\"value1mod\":\"value1mod\",\n\"value2\":\"value2\",\n\"value3-mod\":\"Value3-mod\","
	if values != expVal {
		t.Fatalf("got[%v]", values)
	}
}

func TestScratchVars(t *testing.T) {
	scratch.setInt(100, 0)
	scratch.setInt(201, 1)
	scratch.setInt(302, 2)
	scratch.setBool(true, 0)
	scratch.setBool(false, 1)
	scratch.setStr("value1", 0)
	scratch.setStr("value2", 2)

	if scratch.getInt(0) != 100 || scratch.getInt(1) != 201 || scratch.getInt(2) != 302 {
		t.Errorf("Scratch integer get failed")
	}
	if scratch.getBool(0) != true || scratch.getBool(1) != false {
		t.Errorf("Scratch bool get failed")
	}
	if scratch.getStr(0) != "value1" || scratch.getStr(2) != "value2" {
		t.Errorf("Scratch string get failed")
	}
}

func TestDefaulterManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'msg1'
				field <
					name: 'field1'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
					options:<[venice.default]: "v1:default1">
				>
				field <
					name: 'repeated_field'
					label: LABEL_REPEATED
					type: TYPE_STRING
					options:<[venice.default]: "repeated" [venice.default]: "v2:repeatedV2">
					number: 2
				>
				field <
					name: 'pointerMsg'
					label: LABEL_REPEATED
					type: TYPE_MESSAGE
					type_name: '.example.msg2'
					options:<[gogoproto.nullable]:true>
					number: 3
				>
				field <
					name: 'embeddedMsg'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.msg2'
					options:<[gogoproto.nullable]:false [gogoproto.embed]:true>
					number: 4
				>
				field <
					name: 'pointerMsg2'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.msg3'
					number: 5
				>
				field <
					name: 'anothermsg1'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.anothermsg1'
					number: 5
				>
				field <
					name: 'simpleMsg'
					label: LABEL_REPEATED
					type: TYPE_MESSAGE
					type_name: '.example.msg4'
					number: 6
				>
			>
			message_type <
				name: 'msg2'
				field <
					name: 'field1'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
					options:<[venice.default]: "default1">
				>
			>
			message_type <
				name: 'msg3'
				field <
					name: 'pointerMsg'
					label: LABEL_REPEATED
					type: TYPE_MESSAGE
					type_name: '.example.msg2'
					options:<[gogoproto.nullable]:true>
					number: 1
				>
			>
			message_type <
					name: 'msg4'
					field <
						name: 'field1'
						label: LABEL_OPTIONAL
						type: TYPE_STRING
						number: 1
					>
				>
			`, `
			name: 'another.proto'
			package: 'example'
			message_type <
				name: 'anothermsg1'
				field <
					name: 'field1'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
					options:<[venice.default]: "anotherDefault">
				>
			>
			syntax: "proto3"
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	v, err := getDefaulterManifest(file)
	if err != nil {
		t.Fatalf("get validator manifest failed []%s)", err)
	}
	expected := fileDefaults{
		Fmap: true,
		Map: map[string]msgDefaults{
			"msg1": {
				Fields: map[string]common.Defaults{
					"field1": {
						Repeated: false,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"v1": "\"default1\""},
					},
					"repeated_field": {
						Repeated: true,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"v2": "\"repeatedV2\"", "all": "\"repeated\""},
					},
					"pointerMsg": {
						Repeated: true,
						Pointer:  true,
						Nested:   true,
					},
					"msg2": {
						Repeated: false,
						Pointer:  false,
						Nested:   true,
					},
					"pointerMsg2": {
						Repeated: false,
						Pointer:  true,
						Nested:   true,
					},
				},
				Versions: map[string]map[string]VerDefaults{
					"all": {
						"repeated_field": {
							Repeated: true,
							Pointer:  false,
							Nested:   false,
							Val:      "\"repeated\"",
						},
					},
					"v1": {
						"field1": {
							Repeated: false,
							Pointer:  false,
							Nested:   false,
							Val:      "\"default1\"",
						},
					},
					"v2": {
						"repeated_field": {
							Repeated: true,
							Pointer:  false,
							Nested:   false,
							Val:      "\"repeatedV2\"",
						},
					},
				},
			},
			"msg2": {
				Fields: map[string]common.Defaults{
					"field1": {
						Repeated: false,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"all": "\"default1\""},
					},
				},
				Versions: map[string]map[string]VerDefaults{
					"all": {
						"field1": {
							Repeated: false,
							Pointer:  false,
							Nested:   false,
							Val:      "\"default1\"",
						},
					},
				},
			},
			"msg3": {
				Fields: map[string]common.Defaults{
					"pointerMsg": {
						Repeated: true,
						Pointer:  true,
						Nested:   true,
					},
				},
				Versions: map[string]map[string]VerDefaults{},
			},
		},
	}
	if !reflect.DeepEqual(v, expected) {
		t.Fatalf("Default manifest does not match \n[%+v]\n[%+v]", v, expected)
	}
}

// --- Storage Transformers ---

func TestParseStorageTransformers(t *testing.T) {
	scratchMap := storageTransformerArgMap
	storageTransformerArgMap = map[string][]storageTransformerArgs{
		"OneStr": {common.IsString},
		"MulStr": {common.IsString, common.IsString, common.IsString},
		"OneInt": {govldtr.IsInt},
		"MulInt": {govldtr.IsInt, govldtr.IsInt},
		"StrInt": {common.IsString, govldtr.IsInt},
		"NoArgs": {},
	}

	cases := []struct {
		str string
		ret *storageTransformerField
	}{
		{str: "OneStr(arg1)", ret: &storageTransformerField{Fn: "OneStr", Args: []string{"arg1"}}},
		{str: "MulStr(arg1, arg2, arg3)", ret: &storageTransformerField{Fn: "MulStr", Args: []string{"arg1", "arg2", "arg3"}}},
		{str: "OneInt(1)", ret: &storageTransformerField{Fn: "OneInt", Args: []string{"1"}}},
		{str: "MulInt(1,3)", ret: &storageTransformerField{Fn: "MulInt", Args: []string{"1", "3"}}},
		{str: "NoArgs()", ret: &storageTransformerField{Fn: "NoArgs", Args: []string{}}},
		{str: "NoArgs(arg1)", ret: nil},
		{str: "Unknown()", ret: nil},
		{str: "NoArgs", ret: nil},
		{str: ":NoArgs()", ret: nil},
	}
	for _, c := range cases {
		v, err := parseStorageTransformers(c.str)
		if v == nil && err == nil {
			t.Errorf("[%s] - parser failed without returning an error", c.str)
		}
		if !reflect.DeepEqual(c.ret, v) {
			t.Errorf("[%s] - returned transformer does not match [%+v]/[%+v]", c.str, c.ret, v)
		}
	}
	storageTransformerArgMap = scratchMap
}

func TestGetStorageTransformersManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'msg1'
			field <
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.storageTransformer]: "OneStr(str)">
			>
			field <
				name: 'repeated_field'
				label: LABEL_REPEATED
				type: TYPE_BYTES
				options:<[venice.storageTransformer]: "OneStr(str)" [venice.storageTransformer]: "NoParams()">
				number: 2
			>
		>
		message_type <
			name: 'parentmsg'
			field <
				name: 'nestedfield'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.msg1'
				number: 2
			>
		>
		message_type <
			name: 'parentmsgtoanotherfile'
			field <
				name: 'nestedfield'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg1'
				number: 2
			>
		>
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'anothermsg1'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.storageTransformer]: "NoParams()">
			>
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	scratchMap := storageTransformerArgMap
	storageTransformerArgMap = map[string][]storageTransformerArgs{
		"OneStr":   {common.IsString},
		"NoParams": {},
	}
	exp := storageTransformers{
		Fmap: true,
		Map: map[string]storageTransformerMsg{
			"msg1": {
				HasTransformers: true,
				Fields: map[string]storageTransformerFields{
					"nest1_field": {
						Repeated: false,
						Pointer:  false,
						TypeCast: "string",
						Transformers: []storageTransformerField{
							{
								Fn:   "OneStr",
								Args: []string{"str"},
							},
						},
					},
					"repeated_field": {
						Repeated: true,
						Pointer:  false,
						TypeCast: "[]byte",
						Transformers: []storageTransformerField{
							{
								Fn:   "OneStr",
								Args: []string{"str"},
							},
							{
								Fn:   "NoParams",
								Args: []string{},
							},
						},
					},
				},
			},
			"parentmsg": {
				HasTransformers: true,
				Fields: map[string]storageTransformerFields{
					"nestedfield": {
						Repeated:     false,
						Pointer:      true,
						Transformers: []storageTransformerField{},
					},
				},
			},
			"parentmsgtoanotherfile": {
				HasTransformers: true,
				Fields: map[string]storageTransformerFields{
					"nestedfield": {
						Repeated:     false,
						Pointer:      true,
						Transformers: []storageTransformerField{},
					},
				},
			},
		},
	}

	v, err := getStorageTransformersManifest(file)
	if err != nil {
		t.Fatalf("Could not generate storageTransformer manifest (%s)", err)
	}

	if !reflect.DeepEqual(*v, exp) {
		want, _ := json.MarshalIndent(exp, "", "  ")
		have, _ := json.MarshalIndent(v, "", "  ")
		t.Fatalf("generated manifest does not match:\n have: %v\n want: %v", string(have), string(want))
	}

	storageTransformerArgMap = scratchMap
}

func TestStorageTransformerNegativeCases(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'test1.proto'
		package: 'test'
		syntax: 'proto3'
		message_type <
			name: 'msg1'
			field <
				name: 'invalidFieldTypeInt'
				type: TYPE_INT32
				number: 1
				options:<[venice.storageTransformer]: "OneStr(str)">
			>
		>
		`, `
		name: 'test2.proto'
		package: 'test'
		syntax: "proto3"
		message_type <
			name: 'msg2'
			field <
				name: 'invalidFieldTypeMessage'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.test1.invalidFieldTypeInt'
				number: 1
				options:<[venice.storageTransformer]: "NoParams()">
			>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"test1.proto", "test2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Error loading files: %v", err)
	}
	scratchMap := storageTransformerArgMap
	storageTransformerArgMap = map[string][]storageTransformerArgs{
		"OneStr":   {common.IsString},
		"NoParams": {},
	}

	file, err := r.LookupFile("test1.proto")
	if err != nil {
		t.Fatalf("Could not find file test1.proto")
	}
	_, err = getStorageTransformersManifest(file)
	if err == nil ||
		!strings.Contains(err.Error(), "\"strings\" and \"bytes\" only") ||
		!strings.Contains(err.Error(), "invalidFieldTypeInt") {
		t.Fatalf("Expected type error while parsing storage transformers manifest, got: %v", err)
	}

	file, err = r.LookupFile("test2.proto")
	if err != nil {
		t.Fatalf("Could not find file test2.proto")
	}
	_, err = getStorageTransformersManifest(file)
	if err == nil ||
		!strings.Contains(err.Error(), "scalar types only") ||
		!strings.Contains(err.Error(), "invalidFieldTypeMessage") {
		t.Fatalf("Expected type error while parsing storage transformers manifest, got: %v", err)
	}

	storageTransformerArgMap = scratchMap
}

func TestFileOptions(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example1.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileGrpcDest]: "localhost:8082" [venice.fileCategory]: "test">
		message_type <
			name: 'msg1'
			field <
				name: 'invalidFieldTypeInt'
				type: TYPE_INT32
				number: 1
				options:<[venice.storageTransformer]: "OneStr(str)">
			>
		>
		`,
		`
		name: 'example2.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileApiServerBacked]: false>
		message_type <
			name: 'msg1'
			field <
				name: 'invalidFieldTypeInt'
				type: TYPE_INT32
				number: 1
				options:<[venice.storageTransformer]: "OneStr(str)">
			>
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example1.proto", "example2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	file, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	ret, err := getRelPath(file)
	if err != nil {
		t.Errorf("error getting relPath (%s)", err)
	}
	if ret != "github.com/pensando/sw/api" {
		t.Fatalf("relpath does not match [%s]", ret)
	}
	ret, err = getFileCategory(file.Messages[0])
	if err != nil {
		t.Errorf("error getting FileCategory (%s)", err)
	}
	if ret != "test" {
		t.Fatalf("FileCategory does not match [%s]", ret)
	}
	file2, err := r.LookupFile("example2.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	ret, err = getFileCategory(file2.Messages[0])
	if err != nil {
		t.Errorf("error getting FileCategory (%s)", err)
	}
	if ret != globals.ConfigURIPrefix {
		t.Fatalf("FileCategory does not match [%s]", ret)
	}
}

func TestGetMsgMap(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		enum_type:<
			name:"Enum1"
			value:<name:"Value1" number:0 >
			value:<name:"Value2" number:1 >
		>
		message_type <
			name: 'Struct1'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[gogoproto.jsontag]:"jsonfield1,omitempty" >
			>
			field <
				name: 'repeated_scalar'
				label: LABEL_REPEATED
				type: TYPE_STRING
				number: 2
			>
			field <
				name : 'repeated_struct'
				label: LABEL_REPEATED
				type: TYPE_MESSAGE
				type_name: '.example.Struct2'
				number: 3
				>
			field <
				name: 'enum_field'
				type: TYPE_ENUM
				type_name: '.example.Enum1'
				number: 4
			>
			field <
				name: 'repeated_enum'
				label: LABEL_REPEATED
				type: TYPE_ENUM
				type_name: '.example.Enum1'
				number: 5
			>
			field <
				name: 'map_string_string'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.MapMessageStringString'
				number: 6
				options:<[gogoproto.nullable]:false [gogoproto.jsontag]:"map-string-string" >
			>
			field <
				name: 'map_string_struct'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.MapMessageStringStruct'
				number: 7
			>
			field <
				name: 'map_string_enum'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.MapMessageStringEnum'
				number: 8
			>
		>
		message_type <
			name: 'Struct2'
			field <
				name: 'inlineFld'
				label: LABEL_OPTIONAL
				type:  TYPE_MESSAGE
				type_name: '.example.Struct2Spec'
				number: 1
				options:<[gogoproto.embed]: true, [gogoproto.jsontag]:"inline-field,inline" >
			>
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
			field <
				name: 'Spec'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Struct2Spec'
				options:<[gogoproto.jsontag]:"spec" >
				number: 3
			>
			field <
				name: 'Status'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Struct2Status'
				options:<[gogoproto.jsontag]:"status" >
				number: 4
			>
		>
		message_type <
			name: 'Struct2Spec'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'field2'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
		>
		message_type <
			name: 'Struct2Status'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'field2'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
		>
		message_type <
			name: 'MapMessageStringString'
			field <
				name: 'key'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'value'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
			options:<map_entry:true>
		>
		message_type <
			name: 'MapMessageStringStruct'
			field <
				name: 'key'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'value'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Struct2'
				number: 2
			>
			options:<map_entry:true>
		>
		message_type <
			name: 'MapMessageStringEnum'
			field <
				name: 'key'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'value'
				label: LABEL_OPTIONAL
				type: TYPE_ENUM
				type_name: '.example.Enum1'
				number: 2
			>
			options:<map_entry:true>
		>
		source_code_info:<
		location:<path:4 path:1 path:2 path:0 leading_comments:"cli-tags: ins=inline">
		location:<path:4 path:1 path:2 path:2 leading_comments:"cli-tags: ins=spec">
		location:<path:4 path:1 path:2 path:3 leading_comments:"cli-tags: ins=status">
		location:<path:4 path:2 leading_comments:"Struct2Spec comments" >
		location:<path:4 path:2 path:2 path:0 leading_comments:"cli-tags: verbose-only=true ins=test\ncli-help: Test string">
		location:<path:4 path:2 path:2 path:1 leading_comments:"cli-tags: id=TestKey" >
		location:<path:4 path:3 leading_comments:"Struct2Status comments" >
		location:<path:4 path:3 path:2 path:0 leading_comments:"cli-tags: verbose-only=false ins=test\ncli-help: Test Status string">
		location:<path:4 path:3 path:2 path:1 leading_comments:"cli-tags: ins=test1 id=TestKey2" >
		>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	msgs, keys, err := genMsgMap(file)
	if err != nil {
		t.Fatalf("Failed (%v)", err)
	}

	exp := map[string]Struct{
		"example.Struct1": {
			Fields: map[string]Field{
				"field1":            {Name: "field1", CLITag: cliInfo{tag: "jsonfield1"}, JSONTag: "jsonfield1", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"repeated_scalar":   {Name: "repeated_scalar", CLITag: cliInfo{tag: "repeated_scalar"}, JSONTag: "", Pointer: false, Slice: true, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"repeated_struct":   {Name: "repeated_struct", CLITag: cliInfo{tag: "repeated_struct"}, JSONTag: "", Pointer: true, Slice: true, Map: false, Mutable: true, KeyType: "", Type: "example.Struct2"},
				"enum_field":        {Name: "enum_field", CLITag: cliInfo{tag: "enum_field"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_ENUM"},
				"repeated_enum":     {Name: "repeated_enum", CLITag: cliInfo{tag: "repeated_enum"}, JSONTag: "", Pointer: false, Slice: true, Map: false, Mutable: true, KeyType: "", Type: "TYPE_ENUM"},
				"map_string_string": {Name: "map_string_string", CLITag: cliInfo{tag: "map-string-string"}, JSONTag: "map-string-string", Pointer: false, Slice: false, Map: true, Mutable: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},
				"map_string_struct": {Name: "map_string_struct", CLITag: cliInfo{tag: "map_string_struct"}, JSONTag: "", Pointer: true, Slice: false, Map: true, Mutable: true, KeyType: "TYPE_STRING", Type: "example.Struct2"},
				"map_string_enum":   {Name: "map_string_enum", CLITag: cliInfo{tag: "map_string_enum"}, JSONTag: "", Pointer: true, Slice: false, Map: true, Mutable: true, KeyType: "TYPE_STRING", Type: "TYPE_ENUM"},
			},
		},
		"example.Struct2": {
			Fields: map[string]Field{
				"Struct2Spec": {Name: "Struct2Spec", CLITag: cliInfo{tag: "inline-field", ins: "inline"}, JSONTag: "inline-field", Pointer: true, Slice: false, Map: false, Inline: true, Embed: true, Mutable: true, KeyType: "", Type: "example.Struct2Spec"},
				"field2":      {Name: "field2", CLITag: cliInfo{tag: "field2"}, JSONTag: "", Pointer: false, Slice: false, Map: false, FromInline: true, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"field1":      {Name: "field1", CLITag: cliInfo{tag: "field1"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"Spec":        {Name: "Spec", CLITag: cliInfo{tag: "spec", ins: "spec"}, JSONTag: "spec", Pointer: true, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "example.Struct2Spec"},
				"Status":      {Name: "Status", CLITag: cliInfo{tag: "status", ins: "status"}, JSONTag: "status", Pointer: true, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "example.Struct2Status"},
			},
		},
		"example.Struct2Spec": {
			Fields: map[string]Field{
				"field1": {Name: "field1", CLITag: cliInfo{tag: "field1", skip: true, ins: "test", help: "Test string"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"field2": {Name: "field2", CLITag: cliInfo{tag: "TestKey", ins: ""}, JSONTag: "", Pointer: false, Slice: false, Map: false, KeyType: "", Mutable: true, Type: "TYPE_STRING"},
			},
		},
		"example.Struct2Status": {
			Fields: map[string]Field{
				"field1": {Name: "field1", CLITag: cliInfo{tag: "field1", ins: "test", help: "Test Status string"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"field2": {Name: "field2", CLITag: cliInfo{tag: "TestKey2", ins: "test1"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.MapMessageStringString": {
			Fields: map[string]Field{
				"key":   {Name: "key", CLITag: cliInfo{tag: "key"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", CLITag: cliInfo{tag: "value"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.MapMessageStringStruct": {
			Fields: map[string]Field{
				"key":   {Name: "key", CLITag: cliInfo{tag: "key"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", CLITag: cliInfo{tag: "value"}, JSONTag: "", Pointer: true, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "example.Struct2"},
			},
		},
		"example.MapMessageStringEnum": {
			Fields: map[string]Field{
				"key":   {Name: "key", CLITag: cliInfo{tag: "key"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", CLITag: cliInfo{tag: "value"}, JSONTag: "", Pointer: false, Slice: false, Map: false, Mutable: true, KeyType: "", Type: "TYPE_ENUM"},
			},
		},
	}
	cliExp := map[string]cliInfo{
		"field1":          {tag: "field1", cliType: "String", path: "field1", ins: "", skip: false, help: ""},
		"field2":          {tag: "field2", cliType: "String", path: "field2", ins: "", skip: false, help: ""},
		"inline-field1":   {tag: "field1", cliType: "String", path: "Struct2Spec.field1", ins: "test", skip: true, help: "Test string"},
		"inline-TestKey":  {tag: "TestKey", cliType: "String", path: "Struct2Spec.field2", ins: "", skip: false, help: ""},
		"spec-field1":     {tag: "field1", cliType: "String", path: "Spec.field1", ins: "test", skip: true, help: "Test string"},
		"spec-TestKey":    {tag: "TestKey", cliType: "String", path: "Spec.field2", ins: "", skip: false, help: ""},
		"status-field1":   {tag: "field1", cliType: "String", path: "Status.field1", ins: "test", skip: false, help: "Test Status string"},
		"status-TestKey2": {tag: "TestKey2", cliType: "String", path: "Status.field2", ins: "test1", skip: false, help: ""},
	}
	if len(msgs) != len(exp) {
		t.Fatalf("expecting %v messages got %v", len(exp), len(msgs))
	}
	if len(keys) != len(exp) {
		t.Fatalf("expecting %v keys got %v", len(exp), len(keys))
	}

	for k, v := range exp {
		strct, ok := msgs[k]
		if !ok {
			t.Fatalf("did not find [%v] in msgs", k)
		}
		if !reflect.DeepEqual(strct.Fields, v.Fields) {
			if len(strct.Fields) != len(v.Fields) {
				t.Fatalf("strct %v len does not match, got %d, want %d\n got [%+v]", k, len(strct.Fields), len(v.Fields), strct.Fields)
			}
			for kf := range v.Fields {
				if !reflect.DeepEqual(strct.Fields[kf], v.Fields[kf]) {
					t.Fatalf("Field [%s][%s] does not match got [%+v] want [%+v]", k, kf, strct.Fields[kf], v.Fields[kf])
				}
			}
			t.Fatalf("Fields in %v did not match got[%+v]", k, strct.Fields)
		}
	}

	// Generate CLI tags and verify
	strct := msgs["example.Struct2"]
	getCLITags(strct, "", "", msgs, strct.CLITags)
	if !reflect.DeepEqual(strct.CLITags, cliExp) {
		if len(strct.CLITags) != len(cliExp) {
			t.Fatalf("len does not match, got %d want %d \ngot [%+v]", len(strct.CLITags), len(cliExp), strct.CLITags)
		}
		for k, v := range strct.CLITags {
			if !reflect.DeepEqual(v, cliExp[k]) {
				t.Fatalf("field [%v] does not match got [%+v] want [%+v]", k, v, cliExp[k])
			}
		}
	}

	cliFlagMap := getCLIFlagMap(file)
	if len(cliFlagMap) != 1 {
		t.Fatalf("expecting only 1 cli flag map %+v", cliFlagMap)
	} else if cf, ok := cliFlagMap["example.Struct2"]; !ok {
		t.Fatalf("cli flags not found %+v", cliFlagMap)
	} else if len(cf) != 1 && cf[0].ID != "TestKey" && cf[0].Type != "String" {
		t.Fatalf("invalid cli flag %+v", cliFlagMap)
	}
}

func TestCLITagRegex(t *testing.T) {
	good := []string{
		"foobar",
		"foo_bar",
		"_foo-bar",
		"BArFoo",
	}
	bad := []string{
		"s*&^Bar",
		"ss.asd",
		"foo,bar",
		"foo ,bar",
		"foo	,bar",
		"	foo	,bar",
		"foo/bar",
	}
	for _, v := range good {
		if !validateCLITag(v) {
			t.Errorf("[%s] expected to pass validation", v)
		}
	}
	for _, v := range bad {
		if validateCLITag(v) {
			t.Errorf("[%s] expected to fail validation", v)
		}
	}
}

func TestStreaming(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'Nest1'
				field <
					name: 'nest1_field'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.Nest2'
					number: 1
				>
			>
			message_type <
				name: 'testmsg'
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
			>
			service <
				name: 'crudservice'
				method: <
					name: 'Cstream'
					input_type: '.example.Nest1'
					output_type: '.example.Nest1'
					client_streaming: true
					options:<[venice.methodOper]:"create">
				>
				method: <
					name: 'Sstream'
					input_type: '.example.Nest1'
					output_type: '.example.Nest1'
					server_streaming: true
					options:<[venice.methodOper]:"create">
				>
				method: <
					name: 'Bistream'
					input_type: '.example.Nest1'
					output_type: '.example.Nest1'
					client_streaming: true
					server_streaming: true
					options:<[venice.methodOper]:"create">
				>
				method: <
					name: 'Nonstream'
					input_type: '.example.Nest1'
					output_type: '.example.Nest1'
					options:<[venice.methodOper]:"create">
				>
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	svc := file.Services[0]
	cases := map[string]struct{ c, s bool }{
		"Cstream":   {true, true},
		"Sstream":   {false, true},
		"Bistream":  {true, true},
		"Nonstream": {false, false},
	}
	for _, m := range svc.Methods {
		if v, ok := cases[m.GetName()]; ok {
			c, err := isClientStreaming(m)
			if err != nil {
				t.Errorf("got error checking isClientStreaming for method %v", m.GetName())
			}
			s, err := isStreaming(m)
			if err != nil {
				t.Errorf("got error checking isStreaming for method %v", m.GetName())
			}
			if v.c != c || v.s != s {
				t.Errorf("Does not match - Want (%v/%v) got (%v/%v)", v.c, v.s, c, s)
			}
		} else {
			t.Errorf("unknown method %v", m.GetName())
		}
	}
}

func TestIsTenanted(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'msg1'
				field <
					name: 'leaf_field'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
				>
				field <
					name: 'O'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.oMeta'
					number: 2
					options:<[gogoproto.embed]:true>
				>
				options:<[venice.objectPrefix]:{Collection: "prefix", Path:"{O.Tenant}/{leaf_field}"}>
			>
			message_type <
				name: 'msg2'
				field <
					name: 'leaf_field'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
				>
				field <
					name: 'nest2_field'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 2
				>
				options:<[venice.objectPrefix]:{Collection: "prefix", Path:"{nest2_field}/{leaf_field}"}>
			>
			message_type <
				name: 'msg3'
				field <
					name: 'leaf_field'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 1
				>
				field <
					name: 'O'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.oMeta'
					number: 2
					options:<[gogoproto.embed]:true>
				>
				options:<[venice.objectPrefix]:{Collection: "prefix", Path:"{O.Namespace}/{leaf_field}"}>
			>
			message_type <
			name: 'oMeta'
			field <
				name: 'Tenant'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				>
			field <
				name: 'Namespace'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
				>
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	cases := []struct {
		msg  string
		tval bool
		nval bool
	}{
		{"example.msg1", true, false},
		{"example.msg2", false, false},
		{"example.msg3", false, true},
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("could not find file (%s)", err)
	}
	for _, c := range cases {
		msg, err := r.LookupMsg("", c.msg)
		if err != nil {
			t.Fatalf("could not find message [%v](%s)", c.msg, err)
		}
		if v, err := isTenanted(msg); err != nil || v != c.tval {
			t.Fatalf("did not get [%v] for message [%v](%v)", c.tval, c.msg, err)
		}
		oname := strings.TrimPrefix(c.msg, "example.")
		if v, err := isObjTenanted(file, oname); err != nil || v != c.tval {
			t.Fatalf("did not get [%v] for message [%v](%v)", c.tval, c.msg, err)
		}
		if v, err := isNamespaced(msg); err != nil || v != c.nval {
			t.Fatalf("did not get [%v] for message [%v](%v)", c.nval, c.msg, err)
		}
		if v, err := isObjNamespaced(file, oname); err != nil || v != c.nval {
			t.Fatalf("did not get [%v] for message [%v](%v)", c.nval, c.msg, err)
		}
	}
}

func TestGetProxyPaths(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'Nest1'
				field <
					name: 'nest1_field'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.Nest2'
					number: 1
				>
			>
			service <
				name: 'crudservice'
				method: <
					name: 'TestMethod'
					input_type: '.example.Nest1'
					output_type: '.example.Nest1'
				>
				options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.proxyPrefix]:{PathPrefix: "/backapi1", Path: "/test1", Backend: "localhost:9999"} [venice.proxyPrefix]:{PathPrefix: "/backapi2", Path: "/test2", Backend: "resolved-svc"} >
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	svc := file.Services[0]
	pp, err := getProxyPaths(svc)
	if err != nil {
		t.Fatalf("failed to get proxy paths")
	}
	exp := []ProxyPath{
		{Prefix: "/backapi1", TrimPath: "/configs/example/v1/", Path: "test1", FullPath: "/configs/example/v1/test1", Backend: "localhost:9999"},
		{Prefix: "/backapi2", TrimPath: "/configs/example/v1/", Path: "test2", FullPath: "/configs/example/v1/test2", Backend: "resolved-svc"},
	}
	if !reflect.DeepEqual(pp, exp) {
		t.Fatalf("Proxy paths does not match exp[%+v] got [%+v]", exp, pp)
	}
}

func TestGetReqsManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'msg1'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
				options:<[venice.objRelation]: {Type: "NamedRef" To:"example/anothermsg1"}>
			>
			field <
				name: 'repeated_field'
				label: LABEL_REPEATED
				type: TYPE_STRING
				options:<[venice.objRelation]: {Type: "NamedRef" To:"example/anothermsg1"}>
				number: 2
			>
			field <
				name: 'nullable_field'
				label: LABEL_REPEATED
				type: TYPE_STRING
				options:<[venice.objRelation]: {Type: "NamedRef" To:"example/anothermsg1"} [gogoproto.nullable]:true>
				number: 3
			>
			field <
				name: 'nullable_msg'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg'
				number: 4
			>
			field <
				name: 'non_nullable_msg'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg'
				options:<[gogoproto.nullable]:false>
				number: 5
			>
			field <
				name: 'repeated_nullable_msg'
				label: LABEL_REPEATED
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg'
				number: 6
			>
			field <
				name: 'repeated_non_nullable_msg'
				label: LABEL_REPEATED
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg'
				options:<[gogoproto.nullable]:false>
				number: 7
			>
			field <
				name: 'embeded_msg'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.anothermsg'
				options:<[gogoproto.nullable]:false [gogoproto.embed]:true>
				number: 8
			>
			field <
				name: 'not_a_ref'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 9
			>
		>
		message_type <
			name: 'anothermsg'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.yetanothermsg'
				number: 1
			>
			field <
				name: 'field2'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				options:<[venice.objRelation]: {Type: "NamedRef" To:"example/anothermsg1"}>
				number: 2
			>
			field <
				name: 'not_a_ref'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 3
			>
		>
		message_type <
			name: 'yetanothermsg'
			field <
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				options:<[venice.objRelation]: {Type: "NamedRef" To:"example/anothermsg1"}>
				number: 2
			>
		>
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'anothermsg1'
			field <
				name: 'Name'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
			>
			field <
				name: 'Tenant'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
			options:<[venice.objectPrefix]:{Collection: "prefix", Path:"tenant/{Tenant}"}>
		>
		syntax: "proto3"
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto", "another.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	reqs, err := getRequirementsManifest(file)
	if err != nil {
		t.Fatalf("get requirements returned (%s)", err)
	}
	exp := apiReqs{
		Map: map[string]reqsMsg{
			"msg1": {
				Fields: map[string]reqsField{
					"field1":                    {RefType: "NamedRef", Pointer: false, Repeated: false, Scalar: true, Service: "example", Kind: "anothermsg1"},
					"repeated_field":            {RefType: "NamedRef", Pointer: false, Repeated: true, Scalar: true, Service: "example", Kind: "anothermsg1"},
					"nullable_field":            {RefType: "NamedRef", Pointer: true, Repeated: true, Scalar: true, Service: "example", Kind: "anothermsg1"},
					"nullable_msg":              {RefType: "", Pointer: true, Repeated: false, Scalar: false, Service: "", Kind: ""},
					"non_nullable_msg":          {RefType: "", Pointer: false, Repeated: false, Scalar: false, Service: "", Kind: ""},
					"repeated_nullable_msg":     {RefType: "", Pointer: true, Repeated: true, Scalar: false, Service: "", Kind: ""},
					"repeated_non_nullable_msg": {RefType: "", Pointer: false, Repeated: true, Scalar: false, Service: "", Kind: ""},
					"anothermsg":                {RefType: "", Pointer: false, Repeated: false, Scalar: false, Service: "", Kind: ""},
				},
			},
			"anothermsg": {
				Fields: map[string]reqsField{
					"field1": {RefType: "", Pointer: true, Repeated: false, Scalar: false, Service: "", Kind: ""},
					"field2": {RefType: "NamedRef", Pointer: false, Repeated: false, Scalar: true, Service: "example", Kind: "anothermsg1"},
				},
			},
			"yetanothermsg": {
				Fields: map[string]reqsField{
					"field1": {RefType: "NamedRef", Pointer: false, Repeated: false, Scalar: true, Service: "example", Kind: "anothermsg1"},
				},
			},
		},
	}
	if !reflect.DeepEqual(exp, reqs) {
		for k, m := range exp.Map {
			if m1, ok := reqs.Map[k]; !ok {
				t.Logf("did not find msg [%v]", k)
			} else {
				for fk, f := range m.Fields {
					if f1, ok := m1.Fields[fk]; !ok {
						t.Logf("did not find field [%v]", fk)
					} else {
						if !reflect.DeepEqual(f1, f) {
							t.Logf("field [%v][%v] did not match got:\n[%+v]\nwant\n[%+v]", k, fk, f1, f)
						}
					}
				}
			}
		}
		t.Fatalf("returned requirements does not match expectation: got \n[%+v]\nWant\n[%+v]", reqs, exp)
	}
	freqs := reqsField{Service: "example", Kind: "anothermsg1"}
	// fld := file.Messages[0].Fields[0]
	tstr, err := getRequirementPath(file, freqs, "intenant", "Name")
	if err != nil {
		t.Fatalf("getPath failed (%s)", err)
	}
	estr := "globals.ConfigRootPrefix + \"/example/\" + \"prefix/tenant/\" + intenant + \"/\" + Name"
	if tstr != estr {
		t.Fatalf("Got value \n[%s]\nWant\n[%s]", tstr, estr)
	}
}

func TestGetMsgKindsSchema(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'msg1'
				field <
					name: 'field'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.msg2'
					number: 1
				>
			>
			message_type <
				name: 'msg2'
				field <
					name: 'T'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.api.TypeMeta'
					number: 1
				>
			>
			message_type <
				name: 'msg3'
				field <
					name: 'T'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.api.TypeMeta'
					number: 1
				>
			>
			message_type <
				name: 'msg3List'
				field <
					name: 'T'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.example.msg2'
					number: 1
				>
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}

	exp := map[string]struct {
		hasTypeMeta   bool
		hasListHelper bool
	}{
		"msg1":     {false, false},
		"msg2":     {true, false},
		"msg3":     {true, true},
		"msg3List": {false, false},
	}
	for _, msg := range file.Messages {
		hast, hasl := hasTypeMeta(msg), hasListHelper(msg)
		if e, ok := exp[*msg.Name]; ok {
			if hast != e.hasTypeMeta || hasl != e.hasListHelper {
				t.Errorf("did not match want [%v/%v] got [%v/%v]", e.hasTypeMeta, e.hasListHelper, hast, hasl)
			}
		} else {
			t.Errorf("unknown message [%v]", *msg.Name)
		}
	}
}

func TestSplitSvcObj(t *testing.T) {
	if svcObj := splitSvcObj("example.msg1"); svcObj.Svc != "example" && svcObj.ObjName != "msg1" {
		t.Fatalf("unable to find svc and obj %+v", svcObj)
	}
	if svcObj := splitSvcObj("examplemsg1"); svcObj.Svc != "" {
		t.Fatalf("able to find svc and obj %+v", svcObj)
	}
}

func TestGetMetricsJSON(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
			name: 'example.proto'
			package: 'example'
			syntax: 'proto3'
			message_type <
				name: 'msg_1'
				options:<[venice.metricInfo]: {DisplayName: "TestMetricsJSON" Description:"Msg1Metrics is a test metric" Scope: PerASIC}>
				field <
					name: 'field1'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.delphi.Counter'
					number: 1
					options:<[venice.metricsField]: {DisplayName: "Field1Counter" Description:"Field1 is a counter" Units: Count ScaleMin: 0}>
				>
				field <
					name: 'field2'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.delphi.Gauge'
					number: 2
					options:<[venice.metricsField]: {DisplayName: "Field2Gauge" Description:"Field2 is a gauge" Units: Bytes ScaleMax: 1000}>
				>
				field <
					name: 'field3'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 3
					options:<[venice.metricsField]: {DisplayName: "Field3string" Description:"Field3 is a string"}>
				>
				field <
					name: 'field4'
					label: LABEL_OPTIONAL
					type: TYPE_INT32
					number: 4
					options:<[venice.metricsField]: {DisplayName: "Field4int32" Description:"Field4 is a int32"}>
				>
				field <
					name: 'field5'
					label: LABEL_OPTIONAL
					type: TYPE_INT64
					number: 5
					options:<[venice.metricsField]: {DisplayName: "Field5int64" Description:"Field5 is a int64"}>
				>
				field <
					name: 'field6'
					label: LABEL_OPTIONAL
					type: TYPE_BOOL
					number: 6
					options:<[venice.metricsField]: {DisplayName: "Field6bool" Description:"Field6 is a bool"}>
				>
				field <
					name: 'field7'
					label: LABEL_OPTIONAL
					type: TYPE_STRING
					number: 7
					options:<[venice.check]: "StrEnum(msg_1.Enum1)" [venice.metricsField]: {DisplayName: "Field7stringEnum" Description:"Field7 is a string Enum"}>
				>
				field <
					name: 'field8'
					label: LABEL_OPTIONAL
					type: TYPE_ENUM
					type_name: '.example.msg_1.Enum1'
					number: 8
					options:<[venice.metricsField]: {DisplayName: "Field8stringEnum" Description:"Field8 is a string Enum"}>
				>
				field <
					name: 'field9'
					label: LABEL_OPTIONAL
					type:  TYPE_MESSAGE
					type_name: '.delphi.Counter'
					number: 9
					options:<[venice.metricsField]: {DisplayName: "Field9Bitmap" Description:"Field9 is a Bitmap" Units: Bitmap AllowedVal: "msg_1.Enum1"}>
					>

				enum_type <
					name: "Enum1"
					value <name: "value1", number: 0>
					value <name: "value2", number: 1>
				>
			>
			message_type <
				name: 'msg2'
				field <
					name: 'T'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.api.TypeMeta'
					number: 1
				>
			>
			source_code_info:<
				location:<path:4 path:0 path:4 path: 0 path:2 path:0 leading_comments:"val1 comments\n ui-hint: hint1" >
				location:<path:4 path:0 path:4 path: 0 path:2 path:1 leading_comments:"val2 comments\n ui-hint: hint2" >
			>
			`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed")
	}
	file, err := r.LookupFile("example.proto")
	if err != nil {
		t.Fatalf("Could not find file")
	}
	retJSON, err := genFileMetricsJSON(file, "testfile")
	fmt.Printf("Return is \n %v", retJSON)
	got := fileMetricOptions{}
	err = json.Unmarshal([]byte(retJSON), &got)
	if err != nil {
		t.Fatalf("failed to unmarshal json (%s)", err)
	}
	exp := fileMetricOptions{
		FileName: "example.proto",
		Package:  "example",
		Prefix:   "testfile",
		Messages: []msgMetricOptions{
			{
				Name:        "Msg_1",
				Description: "Msg1Metrics is a test metric",
				DisplayName: "TestMetricsJSON",
				Scope:       "PerASIC",
				Fields: []fieldMetricOptions{
					{
						Name:        "Field1",
						DisplayName: "Field1Counter",
						Description: "Field1 is a counter",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "Counter",
					},
					{
						Name:        "Field2",
						DisplayName: "Field2Gauge",
						Description: "Field2 is a gauge",
						Units:       "Bytes",
						ScaleMin:    0,
						ScaleMax:    1000,
						BaseType:    "Gauge",
					},
					{
						Name:        "Field3",
						DisplayName: "Field3string",
						Description: "Field3 is a string",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "string",
					},
					{
						Name:        "Field4",
						DisplayName: "Field4int32",
						Description: "Field4 is a int32",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "int32",
					},
					{
						Name:        "Field5",
						DisplayName: "Field5int64",
						Description: "Field5 is a int64",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "int64",
					},
					{
						Name:        "Field6",
						DisplayName: "Field6bool",
						Description: "Field6 is a bool",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "bool",
					},
					{
						Name:        "Field7",
						DisplayName: "Field7stringEnum",
						Description: "Field7 is a string Enum",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "string",
						AllowedValues: []string{
							"value1",
							"value2",
						},
					},
					{
						Name:        "Field8",
						DisplayName: "Field8stringEnum",
						Description: "Field8 is a string Enum",
						Units:       "Count",
						ScaleMin:    0,
						ScaleMax:    0,
						BaseType:    "enum",
						AllowedValues: []string{
							"0",
							"1",
						},
					},
					{
						Name:        "Field9",
						DisplayName: "Field9Bitmap",
						Description: "Field9 is a Bitmap",
						Units:       "Bitmap",
						BaseType:    "Bitmap",
						AllowedValues: []string{
							"value1",
							"value2",
						},
					},
				},
			},
		},
	}

	if !reflect.DeepEqual(got, exp) {
		t.Fatal("Response did not match")
	}
}

func TestGenParamPrefix(t *testing.T) {
	flag.Parse()
	_, err := getGenParamsPrefix()
	if err == nil {
		t.Fatalf("should have returned error, but succeeded")
	}
	err = flag.CommandLine.Set("S_prefix", "testValue")
	if err != nil {
		t.Fatalf("could not set flag (%s)", err)
	}
	v, err := getGenParamsPrefix()
	if err != nil {
		t.Fatalf("should have suceeded, but failed with error (%s)", err)
	}
	if v != "testValue" {
		t.Fatalf("unexpected value [%v]", v)
	}
}

func TestGenMetricsManifest(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example1.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
				name: 'msg1'
				options:<[venice.metricInfo]: {DisplayName: "TestMetricsJSON" Description:"Msg1Metrics is a test metric"}>
				field <
					name: 'field1'
					label: LABEL_OPTIONAL
					type: TYPE_MESSAGE
					type_name: '.delphi.Counter'
					number: 1
					options:<[venice.metricsField]: {DisplayName: "Field1Counter" Description:"Field1 is a counter" Units: Count ScaleMin: 0}>
				>
		>
		`,
		`
		name: 'example2.proto'
		package: 'example'
		syntax: 'proto3'
		options:<[venice.fileApiServerBacked]: false>
		`,
	} {
		var fd descriptor.FileDescriptorProto
		if err := proto.UnmarshalText(src, &fd); err != nil {
			t.Fatalf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		req.ProtoFile = append(req.ProtoFile, &fd)
	}
	r := reg.NewRegistry()
	req.FileToGenerate = []string{"example1.proto", "example2.proto"}
	if err := r.Load(&req); err != nil {
		t.Fatalf("Load Failed (%s)", err)
	}
	file1, err := r.LookupFile("example1.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	file2, err := r.LookupFile("example2.proto")
	if err != nil {
		t.Fatalf("Could not find file in request")
	}
	filepath := "/nonexistent/filenameXXXX"
	manifest, err := genMetricsManifest(file1, filepath)
	// Was empty file so we need to have a PkgManifest with single element.
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	files := strings.Split(manifest, "\n")
	if len(files) != 2 {
		t.Fatalf("expecting 1 entry found %d", len(manifest))
	}
	if files[0] != "example1.proto" && files[1] != "" {
		t.Fatalf("unexpected output [%v]", manifest)
	}

	// file without metric defined
	manifest, err = genMetricsManifest(file2, filepath)
	// Was empty file so we need to have a PkgManifest with single element.
	if err != nil {
		t.Errorf("genManifest failed (%s)", err)
	}
	files = strings.Split(manifest, "\n")
	if len(files) != 1 {
		t.Fatalf("expecting 1 entry found %d [%v]", len(files), files)
	}
	if files[0] != "" {
		t.Fatalf("unexpected output [%v]", manifest)
	}

}

func TestApis(t *testing.T) {
	str := getCWD2()
	if str == "" {
		t.Fatalf("could not get current working directory")
	}
	err := createDir("/sw/", "dir1", "dir2")
	if err != nil {
		t.Fatalf("could not create directories")
	}
	cam, err := getMetaswitchMibTablesInfo()
	if err != nil {
		t.Errorf("Error reading MibTables")
	}
	if getFamFromCam(cam, "bgpRmEntTable") != "0x41000001" {
		t.Errorf("Invalid FAM for bgpRmEntTable")
	}
	if getFamFromCam(cam, "bgpRmEntTableXXXX") == "0x41000001" {
		t.Errorf("Got FAM for bgpRmEntTableXXXX")
	}
	if getStructFromCam(cam, "bgpRmEntTable") != "AMB_BGP_RM_ENT" {
		t.Errorf("did not get AMB_BGP_RM_ENT for bgpRmEntTable")
	}
	if getStructFromCam(cam, "bgpRmEntTableXXXX") == "AMB_BGP_RM_ENT" {
		t.Errorf("Got AMB_BGP_RM_ENT for bgpRmEntTableXXXX")
	}
	if getFieldIsKeyFromCam(cam, "AMB_BGP_RM_ENT", "index") == false {
		t.Errorf("Got wrong value")
	}
	if getFieldIsKeyFromCam(cam, "AMB_BGP_RM_ENT", "local_as") == true {
		t.Errorf("Got wrong value")
	}
	if getFieldLenFromCam(cam, "AMB_BGP_RM_ENT", "index") != 0 {
		t.Errorf("Got wrong value")
	}
	if getFieldIsReadOnlyFromCam(cam, "AMB_BGP_RM_ENT", "index") == true {
		t.Errorf("Got wrong value")
	}
	if getFieldIsReadOnlyFromCam(cam, "AMB_BGP_RM_ENT", "oper_status") == false {
		t.Errorf("Got wrong value")
	}
	if getFieldIdxFromCam(cam, "AMB_BGP_RM_ENTXX", "index") != "" {
		t.Errorf("Got wrong value")
	}
	if getFieldIdxFromCam(cam, "AMB_BGP_RM_ENT", "index") != "1" {
		t.Errorf("Got wrong value")
	}
	if getFieldIdxFromCam(cam, "AMB_BGP_RM_ENT", "local_as") != "6" {
		t.Errorf("Got wrong value")
	}
	if getFieldDataTypeFromCam(cam, "AMB_BGP_RM_ENTXX", "local_as") != "" {
		t.Errorf("Got wrong value")
	}
	if getFieldDataTypeFromCam(cam, "AMB_BGP_RM_ENT", "local_as") != "ulong" {
		t.Errorf("Got wrong value")
	}
	if isFieldInCamTable(cam, "AMB_BGP_RM_ENTXX", "local_as") != false {
		t.Errorf("Got wrong value")
	}
	if isFieldInCamTable(cam, "AMB_BGP_RM_ENT", "local_as") != true {
		t.Errorf("Got wrong value")
	}
	opt := pdsaFieldOpt{}
	opt.SetKeyOidLenIndex = ""
	if getPdsaCastSetFunc(gogoproto.FieldDescriptorProto_TYPE_FIXED32, "byteArray", opt) != "NBB_PUT_LONG" {
		t.Errorf("Got wrong value")
	}
	if getPdsaCastSetFunc(gogoproto.FieldDescriptorProto_TYPE_STRING, "byteArray", opt) != "pdsa_set_string_in_byte_array" {
		t.Errorf("Got wrong value")
	}
	opt.SetKeyOidLenIndex = "temp"
	if getPdsaCastSetFunc(gogoproto.FieldDescriptorProto_TYPE_STRING, "byteArray", opt) != "pdsa_set_string_in_byte_array_with_len" {
		t.Errorf("Got wrong value")
	}
	opt.GetKeyOidLenIndex = ""
	if getPdsaCastGetFunc(gogoproto.FieldDescriptorProto_TYPE_FIXED32, "byteArray", opt) != "pdsa_nbb_get_long" {
		t.Errorf("Got wrong value")
	}
	if getPdsaCastGetFunc(gogoproto.FieldDescriptorProto_TYPE_STRING, "byteArray", opt) != "pdsa_get_string_in_byte_array" {
		t.Errorf("Got wrong value")
	}
	opt.GetKeyOidLenIndex = "temp"
	if getPdsaCastGetFunc(gogoproto.FieldDescriptorProto_TYPE_STRING, "byteArray", opt) != "pdsa_get_string_in_byte_array_with_len" {
		t.Errorf("Got wrong value")
	}
}
