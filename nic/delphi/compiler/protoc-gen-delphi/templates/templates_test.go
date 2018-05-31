package templates

import (
	"strings"
	"testing"

	"github.com/golang/glog"
	"github.com/golang/protobuf/proto"
	google_protobuf "github.com/golang/protobuf/protoc-gen-go/descriptor"
	plugin "github.com/golang/protobuf/protoc-gen-go/plugin"
	"github.com/pensando/sw/nic/delphi/compiler/protoc-gen-delphi/descriptor"
	"github.com/pensando/sw/nic/delphi/compiler/protoc-gen-delphi/generator"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// runTemplates applies all templates against a proto request
func runTemplates(req *plugin.CodeGeneratorRequest) (map[string]string, error) {
	ret := make(map[string]string)
	// registry
	reg := descriptor.NewRegistry()

	// load the files into registry
	err := reg.Load(req)
	if err != nil {
		return nil, err
	}

	for _, target := range req.FileToGenerate {
		file, lerr := reg.LookupFile(target)
		if lerr != nil {
			glog.Fatal(lerr)
		}

		// apply header template
		code, terr := generator.ApplyTemplate(file, ".delphi.hpp", HeaderTemplate)
		if terr != nil {
			return nil, terr
		}
		ret[".delphi.hpp"] = code

		// apply src template
		code, terr = generator.ApplyTemplate(file, ".delphi.cc", SrcTemplate)
		if terr != nil {
			return nil, terr
		}
		ret[".delphi.cc"] = code

		// apply utest template
		code, terr = generator.ApplyTemplate(file, ".delphi_utest.hpp", UtestTemplate)
		if terr != nil {
			return nil, terr
		}
		ret[".delphi_utest.hpp"] = code
	}

	return ret, nil
}

func checkForString(t *testing.T, ret map[string]string, prefix, expected string) {
	code, ok := ret[prefix]
	Assert(t, ok, prefix+" code wasnt generated", ret)
	Assert(t, strings.Contains(code, expected), expected+" string not found", code)
}

func checkStringDoesntExist(t *testing.T, ret map[string]string, prefix, expected string) {
	code, ok := ret[prefix]
	Assert(t, ok, prefix+" code wasnt generated", ret)
	Assert(t, !strings.Contains(code, expected), expected+" string still found", code)
}

func TestDelphicTemplateBasic(t *testing.T) {

	opts := google_protobuf.MessageOptions{}
	proto.SetRawExtension(&opts, 70000, []byte{1, 1})

	fldOpts := google_protobuf.FieldOptions{}
	proto.SetRawExtension(&fldOpts, 70000, []byte{1, 1})

	// proto definition
	req := plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name:    proto.String("InterfaceSpec"),
						Options: &opts,
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
							{
								Name:     proto.String("AdminStatus"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_STRING.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_STRING.String()),
								Options:  &fldOpts,
							},
						},
					},
				},
			},
		},
	}

	ret, err := runTemplates(&req)
	AssertOk(t, err, "Error running templates")

	// look for specific strings
	checkForString(t, ret, ".delphi.hpp", "class InterfaceSpec : public BaseObject, public test::InterfaceSpec")
	checkForString(t, ret, ".delphi.hpp", "virtual delphi::ObjectMeta *GetMeta() {\n        return this->mutable_meta();\n    }")
	checkForString(t, ret, ".delphi.hpp", "virtual string GetKey() {\n        string out_str;\n        const google::protobuf::FieldDescriptor *fld =  this->GetDescriptor()->FindFieldByName(\"Key\");\n        google::protobuf::TextFormat::PrintFieldValueToString(*this, fld, -1, &out_str);\n        return out_str;\n    }")
	checkForString(t, ret, ".delphi.hpp", "virtual error OnInterfaceSpecUpdate(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.hpp", "virtual error OnAdminStatus(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.cc", "if (this->adminstatus() != exObj->adminstatus()) {")

}

func TestDelphicTemplateKeyOrHandle(t *testing.T) {

	fldOpts := google_protobuf.FieldOptions{}
	proto.SetRawExtension(&fldOpts, 70000, []byte{1, 1})

	// proto definition
	req := plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceSpec"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("key_or_handle"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".InterfaceSpecKH"),
							},
							{
								Name:     proto.String("IfState"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".InterfaceState"),
								Options:  &fldOpts,
							},
						},
					},
				},
			},
		},
	}

	ret, err := runTemplates(&req)
	AssertOk(t, err, "Error running templates")

	// look for specific strings
	checkForString(t, ret, ".delphi.hpp", "class InterfaceSpec : public BaseObject, public test::InterfaceSpec")
	checkForString(t, ret, ".delphi.hpp", "virtual delphi::ObjectMeta *GetMeta() {\n        return this->mutable_meta();\n    }")
	checkForString(t, ret, ".delphi.hpp", "virtual string GetKey() {\n          return this->key_or_handle().ShortDebugString();\n      }")
	checkStringDoesntExist(t, ret, ".delphi.hpp", "virtual error OnInterfaceSpecUpdate(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.hpp", "virtual error OnIfState(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.cc", "if (this->ifstate().ShortDebugString() != exObj->ifstate().ShortDebugString()) {")

}

func TestDelphicTemplateSingleton(t *testing.T) {

	opts := google_protobuf.MessageOptions{}
	proto.SetRawExtension(&opts, 70001, []byte{1, 1})

	fldOpts := google_protobuf.FieldOptions{}
	proto.SetRawExtension(&fldOpts, 70000, []byte{1, 1})

	// proto definition
	req := plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name:    proto.String("InterfaceSpec"),
						Options: &opts,
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("IfState"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".InterfaceState"),
								Options:  &fldOpts,
							},
						},
					},
				},
			},
		},
	}

	ret, err := runTemplates(&req)
	AssertOk(t, err, "Error running templates")

	// look for specific strings
	checkForString(t, ret, ".delphi.hpp", "class InterfaceSpec : public BaseObject, public test::InterfaceSpec")
	checkForString(t, ret, ".delphi.hpp", "virtual delphi::ObjectMeta *GetMeta() {\n        return this->mutable_meta();\n    }")
	checkForString(t, ret, ".delphi.hpp", "virtual string GetKey() {\n        return \"default\";\n    }")
	checkStringDoesntExist(t, ret, ".delphi.hpp", "virtual error OnInterfaceSpecUpdate(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.hpp", "virtual error OnIfState(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.cc", "if (this->ifstate().ShortDebugString() != exObj->ifstate().ShortDebugString()) {")

}

func TestDelphicTemplateKeyError(t *testing.T) {

	// check we get error if there is no key field
	req := plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceSpec"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
						},
					},
				},
			},
		},
	}

	_, err := runTemplates(&req)
	Assert(t, (err != nil), "running templates did not return error", err)
	Assert(t, strings.Contains(err.Error(), "test.proto : InterfaceSpec :  does not have Key field"), "Invalid error msg", err)

	// check for error if "Key" and "key_or_handle" both are defined
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceSpec"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
							{
								Name:     proto.String("key_or_handle"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".InterfaceSpecKH"),
							},
						},
					},
				},
			},
		},
	}

	_, err = runTemplates(&req)
	Assert(t, (err != nil), "running templates did not return error", err)
	Assert(t, strings.Contains(err.Error(), "test.proto : InterfaceSpec :  multiple key fields or singleton"), "Invalid error msg", err)

	// check for error if "singleton" and "Key" both are defined
	opts := google_protobuf.MessageOptions{}
	proto.SetRawExtension(&opts, 70001, []byte{1, 1})
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name:    proto.String("InterfaceSpec"),
						Options: &opts,
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
						},
					},
				},
			},
		},
	}

	_, err = runTemplates(&req)
	Assert(t, (err != nil), "running templates did not return error", err)
	Assert(t, strings.Contains(err.Error(), "test.proto : InterfaceSpec :  multiple key fields or singleton"), "Invalid error msg", err)

	// check for error if ObjectMeta field has incorrect name
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceSpec"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
						},
					},
				},
			},
		},
	}

	_, err = runTemplates(&req)
	Assert(t, (err != nil), "running templates did not return error", err)
	Assert(t, strings.Contains(err.Error(), "test.proto : InterfaceSpec : meta :  invalid name for ObjectMeta field"), "Invalid error msg", err)

	// verify key field can not be repeated field
	// check for error if ObjectMeta field has incorrect name
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceSpec"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Meta"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.ObjectMeta"),
							},
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
								Label:    google_protobuf.FieldDescriptorProto_LABEL_REPEATED.Enum(),
							},
						},
					},
				},
			},
		},
	}

	_, err = runTemplates(&req)
	Assert(t, (err != nil), "running templates did not return error", err)
	Assert(t, strings.Contains(err.Error(), "test.proto : InterfaceSpec : Key :  Key field can not be repeated"), "Invalid error msg", err)

}
