package plugin

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"reflect"
	"testing"

	"github.com/gogo/protobuf/proto"
	descriptor "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
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
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example.ToObjMsg">>
			>
		>
		message_type <
			name: 'ToObjMsg'
			field <
				name: 'back_ref_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				options:<[venice.objRelation]:<Type:"BackRef" To:"example.RelationObj">>
				number: 1
			>
			field <
				name: 'another_ref__field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
				options:<[venice.objRelation]:<Type:"NamedRef" To:"example.ExternalRefMsg">>
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
				options:<[venice.objRelation]:<Type:"BackRef" To:"example.ToObjMsg">>
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
	if len(relMap["example.ToObjMsg"]) != 2 {
		t.Fatalf("expected [2] relations, found [%d]", len(relMap["example.ToObjMsg"]))
	}

	t.Logf("Writing relations Map")
	str, err := genRelMap(path)
	if err != nil {
		t.Fatalf("failed to generate relations map file (%s)", err)
	}
	tmap := make(map[string][]RelationRef)
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
	if item, ok := tmap["example.ToObjMsg"]; ok && len(item) != 2 {
		t.Fatalf("expected [2] relations, found [%d]", len(item))
	}

	// Write the the file
	err = ioutil.WriteFile(path, []byte(str), 0644)
	if _, err := os.Stat(path); os.IsNotExist(err) {
		t.Fatalf("relation map file not found")
	}

	t.Logf("add more relations with existing file")
	relMap = make(map[string][]RelationRef)

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
	if len(relMap) != 1 {
		t.Fatalf("expected [2] messages, found [%d] [%+v]", len(relMap), relMap)
	}
	if len(relMap["example.ExternalRefMsg"]) != 1 {
		t.Fatalf("expected [1] relations, found [%d], [%+v]", len(relMap["example.ToObjMsg"]), relMap)
	}
	t.Logf("Writing relations with existing file")
	str, err = genRelMap(path)
	tmap = make(map[string][]RelationRef)
	err = json.Unmarshal([]byte(str), &tmap)
	if err != nil {
		t.Fatalf("failed to unamarshal generated relations map (%s)", err)
	}
	if len(tmap) != 3 {
		t.Fatalf("expected [3] messages, found [%d] [%+v]", len(tmap), tmap)
	}
	if item, ok := tmap["example.RelationObj"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}
	if item, ok := tmap["example.ToObjMsg"]; ok && len(item) != 2 {
		t.Fatalf("expected [2] relations, found [%d]", len(item))
	}
	if item, ok := tmap["example.ExternalRefMsg"]; ok && len(item) != 1 {
		t.Fatalf("expected [1] relations, found [%d]", len(item))
	}

}
