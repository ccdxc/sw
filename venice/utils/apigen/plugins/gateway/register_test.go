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

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
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
	// Get Json Tags
	msg, err := r.LookupMsg("", ".example.testmsg")
	if err != nil {
		t.Errorf("expecting to find testmsg")
	}
	fld := msg.Fields[0]
	tag := getJSONTag(fld)
	if tag != "metadata" {
		t.Errorf("failed to get JSON tag")
	}
	fld = msg.Fields[1]
	tag = getJSONTag(fld)
	if tag != "" {
		t.Errorf("failed to get JSON tag")
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
			}
			if !isRestExposed(meth) {
				t.Errorf("RestExposed returned false expecting true for [%s]", *meth.Name)
			}
			if !common.IsAutoGenMethod(meth) {
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
				Fields: map[string]common.Defaults{
					"field1": common.Defaults{
						Repeated: false,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"v1": "\"default1\""},
					},
					"repeated_field": common.Defaults{
						Repeated: true,
						Pointer:  false,
						Nested:   false,
						Map:      map[string]string{"v2": "\"repeatedV2\"", "all": "\"repeated\""},
					},
					"pointerMsg": common.Defaults{
						Repeated: true,
						Pointer:  true,
						Nested:   true,
					},
					"msg2": common.Defaults{
						Repeated: false,
						Pointer:  false,
						Nested:   true,
					},
					"pointerMsg2": common.Defaults{
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
				Fields: map[string]common.Defaults{
					"field1": common.Defaults{
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
				Fields: map[string]common.Defaults{
					"pointerMsg": common.Defaults{
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
				options:<[gogoproto.jsontag]:"inline-field,inline" >
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
				number: 3
			>
			field <
				name: 'Status'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Struct2Status'
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
		"example.Struct1": Struct{
			Fields: map[string]Field{
				"field1":            {Name: "field1", CLITag: cliInfo{tag: "jsonfield1"}, JSONTag: "jsonfield1", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"repeated_scalar":   {Name: "repeated_scalar", CLITag: cliInfo{tag: "repeated_scalar"}, JSONTag: "", Pointer: true, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"repeated_struct":   {Name: "repeated_struct", CLITag: cliInfo{tag: "repeated_struct"}, JSONTag: "", Pointer: true, Slice: true, Map: false, KeyType: "", Type: "example.Struct2"},
				"enum_field":        {Name: "enum_field", CLITag: cliInfo{tag: "enum_field"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_ENUM"},
				"repeated_enum":     {Name: "repeated_enum", CLITag: cliInfo{tag: "repeated_enum"}, JSONTag: "", Pointer: true, Slice: true, Map: false, KeyType: "", Type: "TYPE_ENUM"},
				"map_string_string": {Name: "map_string_string", CLITag: cliInfo{tag: "map-string-string"}, JSONTag: "map-string-string", Pointer: false, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "TYPE_STRING"},
				"map_string_struct": {Name: "map_string_struct", CLITag: cliInfo{tag: "map_string_struct"}, JSONTag: "", Pointer: true, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "example.Struct2"},
				"map_string_enum":   {Name: "map_string_enum", CLITag: cliInfo{tag: "map_string_enum"}, JSONTag: "", Pointer: true, Slice: false, Map: true, KeyType: "TYPE_STRING", Type: "TYPE_ENUM"},
			},
		},
		"example.Struct2": Struct{
			Fields: map[string]Field{
				"Struct2Spec": {Name: "Struct2Spec", CLITag: cliInfo{tag: "inline-field"}, JSONTag: "inline-field", Pointer: true, Slice: false, Map: false, Inline: true, KeyType: "", Type: "example.Struct2Spec"},
				"field2":      {Name: "field2", CLITag: cliInfo{tag: "field2"}, JSONTag: "", Pointer: true, Slice: false, Map: false, FromInline: true, KeyType: "", Type: "TYPE_STRING"},
				"field1":      {Name: "field1", CLITag: cliInfo{tag: "field1"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"Spec":        {Name: "Spec", CLITag: cliInfo{tag: "Spec"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "example.Struct2Spec"},
				"Status":      {Name: "Status", CLITag: cliInfo{tag: "Status"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "example.Struct2Status"},
			},
		},
		"example.Struct2Spec": Struct{
			Fields: map[string]Field{
				"field1": {Name: "field1", CLITag: cliInfo{tag: "test-field1", skip: true, ins: "test", help: "Test string"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"field2": {Name: "field2", CLITag: cliInfo{tag: "TestKey", ins: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.Struct2Status": Struct{
			Fields: map[string]Field{
				"field1": {Name: "field1", CLITag: cliInfo{tag: "test-field1", ins: "test", help: "Test Status string"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"field2": {Name: "field2", CLITag: cliInfo{tag: "test1-TestKey2", ins: "test1"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.MapMessageStringString": Struct{
			Fields: map[string]Field{
				"key":   {Name: "key", CLITag: cliInfo{tag: "key"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", CLITag: cliInfo{tag: "value"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
			},
		},
		"example.MapMessageStringStruct": Struct{
			Fields: map[string]Field{
				"key":   {Name: "key", CLITag: cliInfo{tag: "key"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", CLITag: cliInfo{tag: "value"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "example.Struct2"},
			},
		},
		"example.MapMessageStringEnum": Struct{
			Fields: map[string]Field{
				"key":   {Name: "key", CLITag: cliInfo{tag: "key"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
				"value": {Name: "value", CLITag: cliInfo{tag: "value"}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_ENUM"},
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
			t.Fatalf("Fields in %v did not match got[%+v]", k, strct.Fields)
		}
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
				name: 'nest1_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest2'
				number: 1
				>
			>
			message_type <
			name: 'oMeta'
			field <
				name: 'Tenant'
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
	cases := []struct {
		msg string
		val bool
	}{
		{"example.msg1", true},
		{"example.msg2", false},
		{"example.msg3", false},
	}
	for _, c := range cases {
		msg, err := r.LookupMsg("", c.msg)
		if err != nil {
			t.Fatalf("could not find message [%v](%s)", c.msg, err)
		}
		if v, err := isTenanted(msg); err != nil || v != c.val {
			t.Fatalf("did not get [%v] for message [%v](%v)", c.val, c.msg, err)
		}
	}
}
