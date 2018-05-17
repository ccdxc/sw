package plugin

import (
	"encoding/json"
	"errors"
	"io/ioutil"
	"os"
	"reflect"
	"strings"
	"testing"

	govldtr "github.com/asaskevich/govalidator"
	"github.com/gogo/protobuf/proto"
	descriptor "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	gogoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	reg "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	_ "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
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
	if item, ok := tmap["example.ToObjMsg"]; ok && len(item) != 2 {
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
	if len(relMap) != 1 {
		t.Fatalf("expected [2] messages, found [%d] [%+v]", len(relMap), relMap)
	}
	if len(relMap["example.ExternalRefMsg"]) != 1 {
		t.Fatalf("expected [1] relations, found [%d], [%+v]", len(relMap["example.ToObjMsg"]), relMap)
	}
	t.Logf("Writing relations with existing file")
	str, err = genRelMap(path)
	tmap = make(map[string][]relationRef)
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
			case "noncrudsvc_watch":
				if restOper != "" {
					t.Errorf("expecting [] got [%s]", restOper)
				}
			default:
				t.Errorf("unknown method [%s]", mparams.Oper)
			}

			result1 := URIKey{
				Ref: true,
				Str: "\"example/qual\", in.nest1_field.embedded_field, \"/prefix-\", in.str_field, \"/\", in.O.Name",
			}
			keys, err := getURIKey(meth, false)
			if err != nil {
				t.Errorf("error getting method URI key for [%s](%s)", *meth.Name, err)
			}
			switch *meth.Name {
			case "noncrudsvc_get", "noncrudsvc_delete", "noncrudsvc_list":
				if !reflect.DeepEqual(result1, keys) {
					t.Errorf("key components do not match for [%s] got [%+v] want [%+v]", *meth.Name, keys, result1)
				}
			case "noncrudsvc_create":
				if !reflect.DeepEqual(result1, keys) {
					t.Errorf("key components do not match for [%s] got [%+v] want [%+v]", *meth.Name, keys, result1)
				}
			case "noncrudsvc_update":
				if !reflect.DeepEqual(result1, keys) {
					t.Errorf("key components do not match for [%s] got [%+v]", *meth.Name, keys)
				}

			}
			if !isRestExposed(meth) {
				t.Errorf("RestExposed returned false expecting true for [%s]", *meth.Name)
			}
			if !isAutoGenMethod(meth) {
				t.Errorf("autoGenMethod returned false expecting true for [%s]", *meth.Name)
			}
		}
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
			name: 'full_crudservice'
			method: <
				name: 'dummy_method_to_simulated_mutated_proto'
				input_type: '.example.testmsg'
				output_type: '.example.testmsg'
				options:<[venice.methodOper]:"create">
			>
			options:<[venice.apiVersion]:"v1" [venice.apiPrefix]:"example" [venice.apiGrpcCrudService]:"Nest1">
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
	}
	pkg.Svcs["full_crudservice"] = serviceDef{
		Version:  "v1",
		Messages: []string{"Nest1"},
	}
	expected["example"] = pkg

	ret, err := json.MarshalIndent(expected, "", "  ")
	if err != nil {
		t.Fatalf("Json Unmarshall of svc manifest file failed")
	}
	if manifest != string(ret) {
		t.Errorf("result does not match [%+v] [%v]", manifest, string(ret))
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

func TestParsers(t *testing.T) {
	dummyval1 := "should not pass"
	dummyval2 := false
	{
		in := "test"
		out, err := parseStringOptions(&in)
		if err != nil {
			t.Fatalf("parseStringOptions failed (%s)", err)
		}
		if out.(string) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseStringOptions(&dummyval2); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := []string{"test", "test1"}
		out, err := parseStringSliceOptions(in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.([]string), in) {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseStringSliceOptions(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := int32(10)
		out, err := parseInt32Options(&in)
		if err != nil {
			t.Errorf("parseStringSliceOptions failed (%s)", err)
		}
		if out.(int32) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseInt32Options(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := false
		out, err := parseBoolOptions(&in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if out.(bool) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseBoolOptions(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := venice.ObjectRln{
			Type: "test",
			To:   "destination",
		}
		out, err := parseObjRelation(&in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if out.(venice.ObjectRln) != in {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseObjRelation(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
	}
	{
		in := venice.RestEndpoint{
			Object:  "test",
			Method:  []string{"put", "get"},
			Pattern: "/a/a/a/",
		}
		out, err := parseRestServiceOption(&in)
		if err != nil {
			t.Fatalf("parseStringSliceOptions failed (%s)", err)
		}
		if !reflect.DeepEqual(out.(venice.RestEndpoint), in) {
			t.Errorf("parseStringOptions returned value wrong [%v]", out)
		}
		if _, err := parseRestServiceOption(&dummyval1); err == nil {
			t.Errorf("parseStringOptions passed")
		}
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

func TestParseValidator(t *testing.T) {
	scratchMap := validatorArgMap
	validatorArgMap = map[string][]checkArgs{
		"OneStr": {isString},
		"MulStr": {isString, isString, isString},
		"OneInt": {govldtr.IsInt},
		"MulInt": {govldtr.IsInt, govldtr.IsInt},
		"StrInt": {isString, govldtr.IsInt},
		"NoArgs": {},
	}

	cases := []struct {
		str string
		res bool
		ret validateField
	}{
		{str: "v1:OneStr(arg1)", res: true, ret: validateField{Fn: "OneStr", Ver: "v1", Args: []string{"arg1"}}},
		{str: "MulStr(arg1, arg2, arg3)", res: true, ret: validateField{Fn: "MulStr", Ver: "all", Args: []string{"arg1", "arg2", "arg3"}}},
		{str: "MulStr(arg1, arg2, arg3)", res: true, ret: validateField{Fn: "MulStr", Ver: "all", Args: []string{"arg1", "arg2", "arg3"}}},
		{str: "MulStr(arg1, arg2)", res: false, ret: validateField{}},
		{str: "all:OneInt(arg1)", res: false, ret: validateField{}},
		{str: "all:OneInt(1)", res: true, ret: validateField{Fn: "OneInt", Ver: "all", Args: []string{"1"}}},
		{str: "*:MulInt(1,3)", res: true, ret: validateField{Fn: "MulInt", Ver: "all", Args: []string{"1", "3"}}},
		{str: "all:Unknown(arg1)", res: false, ret: validateField{}},
		{str: "NoArgs()", res: true, ret: validateField{Fn: "NoArgs", Ver: "all", Args: []string{}}},
		{str: "v1:NoArgs()", res: true, ret: validateField{Fn: "NoArgs", Ver: "v1", Args: []string{}}},
		{str: "Unknown(arg1)", res: false, ret: validateField{}},
	}
	for _, c := range cases {
		v, err := parseValidator(c.str)
		if c.res && !(err == nil) {
			t.Errorf("[%s] - Expecing result [%v] got [%v], (%v)", c.str, c.res, (err == nil), err)
		}
		if !reflect.DeepEqual(c.ret, v) {
			t.Errorf("[%s] - returned validate does not match [%+v]/[%+v]", c.str, c.ret, v)
		}
	}
	validatorArgMap = scratchMap
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
	scratchMap := validatorArgMap
	validatorArgMap = map[string][]checkArgs{
		"OneStr": {isString},
		"MulStr": {isString, isString, isString},
		"OneInt": {govldtr.IsInt},
		"MulInt": {govldtr.IsInt, govldtr.IsInt},
		"StrInt": {isString, govldtr.IsInt},
	}
	exp := validators{
		Fmap: true,
		Map: map[string]validateMsg{
			"msg1": {
				Fields: map[string]validateFields{
					"nest1_field": {
						Repeated: false,
						Pointer:  false,
						Validators: []validateField{
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
						Validators: []validateField{
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
						Validators: []validateField{},
					},
				},
			},
			"parentmsgtoanotherfile": {
				Fields: map[string]validateFields{
					"nestedfield": {
						Repeated:   false,
						Pointer:    true,
						Validators: []validateField{},
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

	validatorArgMap = scratchMap
}

func TestGEnumStrMap(t *testing.T) {
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
			exp: "TestMsg_TestEnum_value",
		},
		{
			in:  "TestEnum",
			ok:  true,
			exp: "TestEnum_value",
		},
		{
			in:  ".anotherproto.TestEnum",
			ok:  true,
			exp: "anotherproto.TestEnum_value",
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

func TestParseDefaulters(t *testing.T) {
	cases := []struct {
		in  string
		ver string
		val string
		err error
	}{
		{
			in:  "DefaultOnly",
			ver: "all",
			val: "DefaultOnly",
			err: nil,
		},
		{
			in:  ":DefaultOnly",
			ver: "all",
			val: ":DefaultOnly",
			err: nil,
		},
		{
			in:  "v1:DefaultOnly",
			ver: "v1",
			val: "DefaultOnly",
			err: nil,
		},
		{
			in:  "v1\\:DefaultOnly",
			ver: "all",
			val: "v1\\:DefaultOnly",
			err: nil,
		},
		{
			in:  "DefaultOnly:",
			ver: "DefaultOnly",
			val: "",
			err: nil,
		},
	}
	for _, c := range cases {
		ver, val, err := parseDefault(c.in)
		if ver != c.ver || val != c.val || c.err != err {
			t.Errorf("Does not match [%s][ %s / %s / %v ]", c.in, ver, val, err)
		}
	}
}

func TestDefaulters(t *testing.T) {
	var req gogoplugin.CodeGeneratorRequest
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		syntax: 'proto3'
		message_type <
			name: 'TestMsg'
			field <
				name: 'no_default_msg_field'
				type: TYPE_MESSAGE
				type_name: '.example.another_message'
				number: 1
			>
			field <
				name: 'msg_field'
				type: TYPE_MESSAGE
				type_name: '.example.another_message'
				options:<[venice.default]: "false" [venice.default]: "v2:true">
				number: 1
			>
			field <
				name: 'bool_field'
				type: TYPE_BOOL
				options:<[venice.default]: "false" [venice.default]: "v2:true">
				number: 1
			>
			field <
				name: 'enum_field'
				type: TYPE_ENUM
				type_name: '.example.Enum1'
				options:<[venice.default]: "EnumAll" [venice.default]: "v2:EnumONE">
				number: 2
			>
			field <
				name: 'int32_field'
				type: TYPE_INT32
				options:<[venice.default]: "10" [venice.default]: "v3:42">
				number: 3
			>
			field <
				name: 'int64_field'
				type: TYPE_INT64
				options:<[venice.default]: "1000000000000" [venice.default]: "v3:42000000000000">
				number: 4
			>
			field <
				name: 'sint32_field'
				type: TYPE_SINT32
				options:<[venice.default]: "-22" [venice.default]: "v3:-42">
				number: 5
			>
			field <
				name: 'sint64_field'
				type: TYPE_SINT64
				options:<[venice.default]: "-22000000000000" [venice.default]: "v3:-42000000000000">
				number: 6
			>
			field <
				name: 'sfixed32_field'
				type: TYPE_SFIXED32
				options:<[venice.default]: "-22" [venice.default]: "v3:-42">
				number: 7
			>
			field <
				name: 'sfixed64_field'
				type: TYPE_SFIXED64
				options:<[venice.default]: "-22000000000000" [venice.default]: "v3:-42000000000000">
				number: 8
			>
			field <
				name: 'uint32_field'
				type: TYPE_UINT32
				options:<[venice.default]: "10" [venice.default]: "v3:42">
				number: 9
			>
			field <
				name: 'uint64_field'
				type: TYPE_UINT64
				options:<[venice.default]: "1000000000000" [venice.default]: "v3:42000000000000">
				number: 10
			>
			field <
				name: 'fixed32_field'
				type: TYPE_FIXED32
				options:<[venice.default]: "22" [venice.default]: "v3:42">
				number: 11
			>
			field <
				name: 'fixed64_field'
				type: TYPE_FIXED64
				options:<[venice.default]: "22000000000000" [venice.default]: "v3:42000000000000">
				number: 12
			>
			field <
				name: 'float_field'
				type: TYPE_FLOAT
				options:<[venice.default]: "3.1415" [venice.default]: "v3:2.71828">
				number: 13
			>
			field <
				name: 'double_field'
				type: TYPE_DOUBLE
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:2000000000000.71828">
				number: 14
			>
			field <
				name: 'string_field'
				type: TYPE_STRING
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:unknown value">
				number: 15
			>
			field <
				name: 'str_enum_field'
				type: TYPE_STRING
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:unknown value" [venice.check]: "StrEnum(TestEnum.Value)" [venice.check]: "v2:StrEnum(.other.TestEnum.Value)">
				number: 16
			>
			field <
				name: 'str_enum_nodef_field'
				type: TYPE_STRING
				options:<[venice.check]: "StrEnum(TestEnum.Value)" [venice.check]: "v3:StrEnum(.other.TestEnum.Value)">
				number: 17
			>
			field <
				name: 'no_default_msg_field'
				type: TYPE_MESSAGE
				type_name: '.example.another_message'
				number: 18
			>
			field <
				name: 'no_default_int32_field'
				type: TYPE_INT32
				number: 19
			>
			field <
				name: 'bad_bool_field'
				type: TYPE_BOOL
				options:<[venice.default]: "unknown" [venice.default]: "v2:aaaa">
				number: 21
			>
			field <
				name: 'bad_int32_field'
				type: TYPE_INT32
				options:<[venice.default]: "10" [venice.default]: "v3:1000000000000">
				number: 23
			>
			field <
				name: 'bad_int64_field'
				type: TYPE_INT64
				options:<[venice.default]: "1000000000000a" [venice.default]: "v3:42000000000000">
				number: 24
			>
			field <
				name: 'bad_sint32_field'
				type: TYPE_SINT32
				options:<[venice.default]: "-22" [venice.default]: "v3:">
				number: 25
			>
			field <
				name: 'bad_sint64_field'
				type: TYPE_SINT64
				options:<[venice.default]: "true" [venice.default]: "v3:xyze">
				number: 26
			>
			field <
				name: 'bad_sfixed32_field'
				type: TYPE_SFIXED32
				options:<[venice.default]: "-22" [venice.default]: "v3:42000000000000.12333">
				number: 27
			>
			field <
				name: 'bad_sfixed64_field'
				type: TYPE_SFIXED64
				options:<[venice.default]: "-aa22000000000000" [venice.default]: "v3:-42000000000000">
				number: 28
			>
			field <
				name: 'bad_uint32_field'
				type: TYPE_UINT32
				options:<[venice.default]: "thousand" [venice.default]: ":">
				number: 29
			>
			field <
				name: 'bad_uint64_field'
				type: TYPE_UINT64
				options:<[venice.default]: "" [venice.default]: "v3:42000000000000">
				number: 210
			>
			field <
				name: 'bad_fixed32_field'
				type: TYPE_FIXED32
				options:<[venice.default]: "22" [venice.default]: "v3:0xaa">
				number: 211
			>
			field <
				name: 'bad_fixed64_field'
				type: TYPE_FIXED64
				options:<[venice.default]: "-22000000000000" [venice.default]: "v3:42000000000000">
				number: 212
			>
			field <
				name: 'bad_float_field'
				type: TYPE_FLOAT
				options:<[venice.default]: "a.1415" [venice.default]: "v3:2.71828">
				number: 213
			>
			field <
				name: 'bad_double_field'
				type: TYPE_DOUBLE
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3\\:2000000000000.71828">
				number: 214
			>
			field <
				name: 'dup_str_enum_field'
				type: TYPE_STRING
				options:<[venice.default]: "3000000000000.1415" [venice.default]: "v3:unknown value" [venice.check]: "StrEnum(TestEnum.Value)" [venice.check]: "v2:StrEnum(.other.TestEnum.Value)" [venice.default]: "v3:unknown value2">
				number: 215
			>

		>
		message_type <
			name: 'another_message'
			field <
				name: 'nestedfield'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
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
	testerr := errors.New("error")
	expected := map[string]struct {
		ret   Defaults
		err   error
		found bool
	}{
		"msg_field":              {err: testerr, found: true, ret: Defaults{}},
		"bool_field":             {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "false", "v2": "true"}}},
		"enum_field":             {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "EnumAll", "v2": "EnumONE"}}},
		"int32_field":            {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "10", "v3": "42"}}},
		"int64_field":            {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "1000000000000", "v3": "42000000000000"}}},
		"sint32_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22", "v3": "-42"}}},
		"sint64_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22000000000000", "v3": "-42000000000000"}}},
		"sfixed32_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22", "v3": "-42"}}},
		"sfixed64_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "-22000000000000", "v3": "-42000000000000"}}},
		"uint32_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "10", "v3": "42"}}},
		"uint64_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "1000000000000", "v3": "42000000000000"}}},
		"fixed32_field":          {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "22", "v3": "42"}}},
		"fixed64_field":          {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "22000000000000", "v3": "42000000000000"}}},
		"float_field":            {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "3.1415", "v3": "2.71828"}}},
		"double_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "3000000000000.1415", "v3": "2000000000000.71828"}}},
		"string_field":           {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"3000000000000.1415\"", "v3": "\"unknown value\""}}},
		"str_enum_field":         {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "\"3000000000000.1415\"", "v2": "other.TestEnum_Value_name[0]", "v3": "\"unknown value\""}}},
		"str_enum_nodef_field":   {err: nil, found: true, ret: Defaults{Map: map[string]string{"all": "TestEnum_Value_name[0]", "v3": "other.TestEnum_Value_name[0]"}}},
		"bad_bool_field":         {err: testerr, found: true, ret: Defaults{}},
		"bad_enum_field":         {err: testerr, found: true, ret: Defaults{}},
		"bad_int32_field":        {err: testerr, found: true, ret: Defaults{}},
		"bad_int64_field":        {err: testerr, found: true, ret: Defaults{}},
		"bad_sint32_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_sint64_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_sfixed32_field":     {err: testerr, found: true, ret: Defaults{}},
		"bad_sfixed64_field":     {err: testerr, found: true, ret: Defaults{}},
		"bad_uint32_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_uint64_field":       {err: testerr, found: true, ret: Defaults{}},
		"bad_fixed32_field":      {err: testerr, found: true, ret: Defaults{}},
		"bad_fixed64_field":      {err: testerr, found: true, ret: Defaults{}},
		"bad_float_field":        {err: testerr, found: true, ret: Defaults{}},
		"bad_double_field":       {err: testerr, found: true, ret: Defaults{}},
		"dup_str_enum_field":     {err: testerr, found: true, ret: Defaults{}},
		"no_default_msg_field":   {err: nil, found: false, ret: Defaults{}},
		"no_default_int32_field": {err: nil, found: false, ret: Defaults{}},
	}
	msg := file.Messages[0]
	for i := range msg.Fields {
		name := *msg.Fields[i].Name
		c := expected[name]
		if v, found, err := parseDefaults(file, msg.Fields[i]); (err == nil) != (c.err == nil) {
			t.Errorf("errors dont match for %s (%v/%v)[%+v]", name, err, c.err, v)
		} else if err == nil {
			if found != c.found {
				t.Errorf("[%v]found flag does not match[%v/%v]", name, found, c.found)
			}
			if found {
				if !reflect.DeepEqual(v, c.ret) {
					t.Errorf("[%v]returned value does not match expected [ %v]/ [%v]", name, v, c.ret)
				}
			}
		}
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
			"msg1": msgDefaults{
				Fields: map[string]Defaults{
					"field1": Defaults{
						Repeated: false,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"v1": "\"default1\""},
					},
					"repeated_field": Defaults{
						Repeated: true,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"v2": "\"repeatedV2\"", "all": "\"repeated\""},
					},
					"pointerMsg": Defaults{
						Repeated: true,
						Pointer:  true,
						Nested:   true,
					},
					"msg2": Defaults{
						Repeated: false,
						Pointer:  false,
						Nested:   true,
					},
					"pointerMsg2": Defaults{
						Repeated: false,
						Pointer:  true,
						Nested:   true,
					},
				},
				Versions: map[string]map[string]VerDefaults{
					"all": map[string]VerDefaults{
						"repeated_field": VerDefaults{
							Repeated: true,
							Pointer:  false,
							Nested:   false,
							Val:      "\"repeated\"",
						},
					},
					"v1": map[string]VerDefaults{
						"field1": VerDefaults{
							Repeated: false,
							Pointer:  false,
							Nested:   false,
							Val:      "\"default1\"",
						},
					},
					"v2": map[string]VerDefaults{
						"repeated_field": VerDefaults{
							Repeated: true,
							Pointer:  false,
							Nested:   false,
							Val:      "\"repeatedV2\"",
						},
					},
				},
			},
			"msg2": msgDefaults{
				Fields: map[string]Defaults{
					"field1": Defaults{
						Repeated: false,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"all": "\"default1\""},
					},
				},
				Versions: map[string]map[string]VerDefaults{
					"all": map[string]VerDefaults{
						"field1": VerDefaults{
							Repeated: false,
							Pointer:  false,
							Nested:   false,
							Val:      "\"default1\"",
						},
					},
				},
			},
			"msg3": msgDefaults{
				Fields: map[string]Defaults{
					"pointerMsg": Defaults{
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
		"OneStr": {isString},
		"MulStr": {isString, isString, isString},
		"OneInt": {govldtr.IsInt},
		"MulInt": {govldtr.IsInt, govldtr.IsInt},
		"StrInt": {isString, govldtr.IsInt},
		"NoArgs": {},
	}

	cases := []struct {
		str string
		res bool
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
		"OneStr":   {isString},
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
		"OneStr":   {isString},
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

func TestGetRelPath(t *testing.T) {
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
	ret, err := getRelPath(file)
	if err != nil {
		t.Errorf("error getting relPath (%s)", err)
	}
	if ret != "github.com/pensando/sw/api" {
		t.Fatalf("relpath does not match [%s]", ret)
	}
}

func (s *Struct) FindField(in string) (Field, bool) {
	f, ok := s.Fields[in]
	return f, ok
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
					options:<[gogoproto.jsontag]:"map-string-string,inline" >
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
				name: 'field1'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 1
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
		"example.Struct1": Struct{
			Fields: map[string]Field{
				"field1":            {Name: "field1", JSONTag: "jsonfield1", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"repeated_scalar":   {Name: "repeated_scalar", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"repeated_struct":   {Name: "repeated_struct", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "example.Struct2"},
				"enum_field":        {Name: "enum_field", JSONTag: "", Pointer: false, Slice: false, Map: false, KeyType: "", Type: "TYPE_ENUM"},
				"repeated_enum":     {Name: "repeated_enum", JSONTag: "", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_ENUM"},
				"map_string_string": {Name: "map_string_string", JSONTag: "map-string-string", Pointer: true, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},
				"map_string_struct": {Name: "map_string_struct", JSONTag: "", Pointer: true, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "example.Struct2"},
				"map_string_enum":   {Name: "map_string_enum", JSONTag: "", Pointer: true, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "TYPE_ENUM"},
			},
		},
		"example.Struct2": Struct{
			Fields: map[string]Field{
				"field1": {Name: "field1", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.MapMessageStringString": Struct{
			Fields: map[string]Field{
				"key":   {Name: "key", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.MapMessageStringStruct": Struct{
			Fields: map[string]Field{
				"key":   {Name: "key", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "example.Struct2"},
			},
		},
		"example.MapMessageStringEnum": Struct{
			Fields: map[string]Field{
				"key":   {Name: "key", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_ENUM"},
			},
		},
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
			t.Fatalf("Fields in %v did not match", k)
		}
	}
}
