package descriptor

import (
	"fmt"
	"strings"
	"testing"

	"github.com/gogo/protobuf/proto"
	descriptor "github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
)

func TestGoPackageStandard(t *testing.T) {
	for _, spec := range []struct {
		pkg  GoPackage
		want bool
	}{
		{
			pkg:  GoPackage{Path: "fmt", Name: "fmt"},
			want: true,
		},
		{
			pkg:  GoPackage{Path: "encoding/json", Name: "json"},
			want: true,
		},
		{
			pkg:  GoPackage{Path: "github.com/gogo/protobuf/jsonpb", Name: "jsonpb"},
			want: false,
		},
		{
			pkg:  GoPackage{Path: "golang.org/x/net/context", Name: "context"},
			want: false,
		},
		{
			pkg:  GoPackage{Path: "github.com/pensando/grpc-gateway", Name: "main"},
			want: false,
		},
		{
			pkg:  GoPackage{Path: "github.com/google/googleapis/google/api/http.pb", Name: "http_pb", Alias: "htpb"},
			want: false,
		},
	} {
		if got, want := spec.pkg.Standard(), spec.want; got != want {
			t.Errorf("%#v.Standard() = %v; want %v", spec.pkg, got, want)
		}
	}
}

func TestGoPackageString(t *testing.T) {
	for _, spec := range []struct {
		pkg  GoPackage
		want string
	}{
		{
			pkg:  GoPackage{Path: "fmt", Name: "fmt"},
			want: `"fmt"`,
		},
		{
			pkg:  GoPackage{Path: "encoding/json", Name: "json"},
			want: `"encoding/json"`,
		},
		{
			pkg:  GoPackage{Path: "github.com/gogo/protobuf/jsonpb", Name: "jsonpb"},
			want: `"github.com/gogo/protobuf/jsonpb"`,
		},
		{
			pkg:  GoPackage{Path: "golang.org/x/net/context", Name: "context"},
			want: `"golang.org/x/net/context"`,
		},
		{
			pkg:  GoPackage{Path: "github.com/pensando/grpc-gateway", Name: "main"},
			want: `"github.com/pensando/grpc-gateway"`,
		},
		{
			pkg:  GoPackage{Path: "github.com/google/googleapis/google/api/http.pb", Name: "http_pb", Alias: "htpb"},
			want: `htpb "github.com/google/googleapis/google/api/http.pb"`,
		},
	} {
		if got, want := spec.pkg.String(), spec.want; got != want {
			t.Errorf("%#v.String() = %q; want %q", spec.pkg, got, want)
		}
	}
}

func TestFieldPath(t *testing.T) {
	var fds []*descriptor.FileDescriptorProto
	for _, src := range []string{
		`
		name: 'example.proto'
		package: 'example'
		message_type <
			name: 'Nest'
			field <
				name: 'nest2_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: 'Nest2'
				number: 1
			>
			field <
				name: 'terminal_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
			>
		>
		syntax: "proto3"
		`, `
		name: 'another.proto'
		package: 'example'
		message_type <
			name: 'Nest2'
			field <
				name: 'nest_field'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: 'Nest'
				number: 1
			>
			field <
				name: 'terminal_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 2
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
	nest := &Message{
		DescriptorProto: fds[0].MessageType[0],
		Fields: []*Field{
			{FieldDescriptorProto: fds[0].MessageType[0].Field[0]},
			{FieldDescriptorProto: fds[0].MessageType[0].Field[1]},
		},
	}
	nest2 := &Message{
		DescriptorProto: fds[1].MessageType[0],
		Fields: []*Field{
			{FieldDescriptorProto: fds[1].MessageType[0].Field[0]},
			{FieldDescriptorProto: fds[1].MessageType[0].Field[1]},
		},
	}
	file1 := &File{
		FileDescriptorProto: fds[0],
		GoPkg:               GoPackage{Path: "example", Name: "example"},
		Messages:            []*Message{nest},
	}
	file2 := &File{
		FileDescriptorProto: fds[1],
		GoPkg:               GoPackage{Path: "example", Name: "example"},
		Messages:            []*Message{nest2},
	}
	crossLinkFixture(file1)
	crossLinkFixture(file2)

	c1 := FieldPathComponent{
		Name:   "nest_field",
		Target: nest2.Fields[0],
	}
	if got, want := c1.LHS(), "GetNestField()"; got != want {
		t.Errorf("c1.LHS() = %q; want %q", got, want)
	}
	if got, want := c1.RHS(), "NestField"; got != want {
		t.Errorf("c1.RHS() = %q; want %q", got, want)
	}

	c2 := FieldPathComponent{
		Name:   "nest2_field",
		Target: nest.Fields[0],
	}
	if got, want := c2.LHS(), "Nest2Field"; got != want {
		t.Errorf("c2.LHS() = %q; want %q", got, want)
	}
	if got, want := c2.LHS(), "Nest2Field"; got != want {
		t.Errorf("c2.LHS() = %q; want %q", got, want)
	}

	fp := FieldPath{
		c1, c2, c1, FieldPathComponent{
			Name:   "terminal_field",
			Target: nest.Fields[1],
		},
	}
	if got, want := fp.RHS("resp"), "resp.GetNestField().Nest2Field.GetNestField().TerminalField"; got != want {
		t.Errorf("fp.RHS(%q) = %q; want %q", "resp", got, want)
	}

	fp2 := FieldPath{
		c2, c1, c2, FieldPathComponent{
			Name:   "terminal_field",
			Target: nest2.Fields[1],
		},
	}
	if got, want := fp2.RHS("resp"), "resp.Nest2Field.GetNestField().Nest2Field.TerminalField"; got != want {
		t.Errorf("fp2.RHS(%q) = %q; want %q", "resp", got, want)
	}

	var fpEmpty FieldPath
	if got, want := fpEmpty.RHS("resp"), "resp"; got != want {
		t.Errorf("fpEmpty.RHS(%q) = %q; want %q", "resp", got, want)
	}
}

func TestFindMessageField(t *testing.T) {
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
			field <
				name: 'leaf_field'
				label: LABEL_OPTIONAL
				type: TYPE_STRING
				number: 3
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
	r := NewRegistry()
	r.loadFile(fds[0])
	r.loadFile(fds[1])
	testmsg, err := r.LookupMsg("", ".example.testmsg")
	if err != nil {
		t.Fatalf("Could not lookup testmsg (%v)", err)
		return
	}
	nest1, err := r.LookupMsg("", ".example.Nest1")
	if err != nil {
		t.Fatalf("Could not lookup nest1 (%v)", err)
		return
	}
	nest2, err := r.LookupMsg("", ".example.Nest2")
	if err != nil {
		t.Fatalf("Could not lookup nest2 (%v)", err)
		return
	}

	t.Logf("Test finding scalar field in message [leaf_field]")
	rr, e, f := testmsg.FindMessageField("leaf_field")
	if !f || e || rr != testmsg {
		t.Errorf("\t**FindMessageField failed for leaf_field[%v/%v]", f, e)
	}

	t.Logf("Test finding nested field in message [real_field]")
	rr, e, f = testmsg.FindMessageField("real_field")
	if !f || e || rr != nest1 {
		t.Errorf("\t**FindMessageField failed for [real_field][%v/%v]", f, e)
	}

	t.Logf("Test finding embedded nested field in message [embedded_field]")
	rr, e, f = testmsg.FindMessageField("embedded_field")
	if !f || !e || rr != nest2 {
		t.Errorf("\t**FindMessageField failed for [embedded_field][%v/%v]", f, e)
	}

	t.Logf("Test finding embedded nested message [embedded_msg]")
	rr, e, f = testmsg.FindMessageField("embedded_msg")
	if !f || !e || rr != nest2 {
		t.Errorf("\t**FindMessageField failed for [embedded_field][%v/%v]", f, e)
	}

	t.Logf("Test finding nested structure field in message [dummy_field")
	rr, e, f = testmsg.FindMessageField("dummy_field")
	if f {
		t.Errorf("\t**FindMessageField found a fictitious field[dummy_field]! failed!")
	}
}

func ExampleNestedFindMessageField() {
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
			field <
				name: 'nest3_msg'
				label: LABEL_OPTIONAL
				type: TYPE_MESSAGE
				type_name: '.example.Nest3'
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
			name: 'Nest3'
			field <
				name: 'nest3_field'
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
			fmt.Printf("proto.UnmarshalText(%s, &fd) failed with %v; want success", src, err)
		}
		fds = append(fds, &fd)
	}
	r := NewRegistry()
	r.loadFile(fds[0])
	r.loadFile(fds[1])
	for idx, testpath := range []string{
		// Valid path.
		"real_field.nest2_field.embedded_field",
		// Invalaid path.
		"real_field.dummy_field.embedded_field",
		// Conflicting composition
		"leaf_field",
		// Conflicting composition, select anonymous member
		"embedded_msg.leaf_field",
		// Valid path with embedded field.
		"nest3_msg.nest3_field",
	} {
		fmt.Printf("Path %v: [%s]\n", idx, testpath)
		testmsg, err := r.LookupMsg("", ".example.testmsg")
		if err != nil {
			fmt.Errorf("Could not lookup testmsg (%v)", err)
			return
		}
		var f bool
		for _, val := range strings.Split(testpath, ".") {
			testmsg, _, f = testmsg.FindMessageField(val)

			if !f {
				fmt.Printf(" >>failed to find [%s]\n", val)
				break
			}
			fmt.Printf(" >>found [%s]\n", val)
		}
		if !f {
			fmt.Printf(" >>>FindMessageField failed for path %v\n", idx)
		}
	}

	// Output:
	// Path 0: [real_field.nest2_field.embedded_field]
	//  >>found [real_field]
	//  >>found [nest2_field]
	//  >>found [embedded_field]
	// Path 1: [real_field.dummy_field.embedded_field]
	//  >>found [real_field]
	//  >>failed to find [dummy_field]
	//  >>>FindMessageField failed for path 1
	// Path 2: [leaf_field]
	//  >>found [leaf_field]
	// Path 3: [embedded_msg.leaf_field]
	//  >>found [embedded_msg]
	//  >>found [leaf_field]
	// Path 4: [nest3_msg.nest3_field]
	//  >>found [nest3_msg]
	//  >>found [nest3_field]
}
