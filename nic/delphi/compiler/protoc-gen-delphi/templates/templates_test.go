package templates

import (
	"fmt"
	"runtime"
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
         _, file, line, _ := runtime.Caller(1)
	lineStr := fmt.Sprintf("(%v:%v)", file, line)
	code, ok := ret[prefix]
	Assert(t, ok, prefix+" code wasnt generated "+lineStr, ret)
	Assert(t, strings.Contains(code, expected), expected+" string not found "+lineStr, code)
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
	checkForString(t, ret, ".delphi.hpp", "static inline InterfaceSpecPtr FindObject(SdkPtr sdk, InterfaceSpecPtr objkey) {")
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
	checkForString(t, ret, ".delphi.hpp", "static inline InterfaceSpecPtr FindObject(SdkPtr sdk) {")
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

func TestDelphicMetricsBasic(t *testing.T) {

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
					{
						Name: proto.String("InterfaceMetrics"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
							{
								Name:     proto.String("PktCounter"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Counter"),
							},
							{
								Name:     proto.String("PktRate"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Gauge"),
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
	checkForString(t, ret, ".delphi.hpp", "static inline InterfaceSpecPtr FindObject(SdkPtr sdk, InterfaceSpecPtr objkey) {")
	checkForString(t, ret, ".delphi.hpp", "virtual error OnInterfaceSpecUpdate(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.hpp", "virtual error OnAdminStatus(InterfaceSpecPtr obj)")
	checkForString(t, ret, ".delphi.cc", "if (this->adminstatus() != exObj->adminstatus()) {")

	checkForString(t, ret, ".delphi.hpp", "class InterfaceMetrics : public delphi::metrics::DelphiMetrics {")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::CounterPtr   PktCounter_;")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::GaugePtr     PktRate_;")
	checkForString(t, ret, ".delphi.hpp", "InterfaceMetrics(int32_t key, char *ptr);")
	checkForString(t, ret, ".delphi.hpp", "InterfaceMetrics(char *kptr, char *vptr) : InterfaceMetrics(*(int32_t *)kptr, vptr){ };")
	checkForString(t, ret, ".delphi.hpp", "int32_t GetKey() { return key_; }")
	checkForString(t, ret, ".delphi.hpp", "static InterfaceMetricsPtr NewInterfaceMetrics(int32_t key);")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::CounterPtr PktCounter() { return PktCounter_; };")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::GaugePtr PktRate() { return PktRate_; };")
	checkForString(t, ret, ".delphi.hpp", "REGISTER_METRICS(InterfaceMetrics);")
	checkForString(t, ret, ".delphi.hpp", "class InterfaceMetricsIterator {")
	checkForString(t, ret, ".delphi.hpp", "explicit InterfaceMetricsIterator(delphi::shm::TableIterator tbl_iter) {")
	checkForString(t, ret, ".delphi.hpp", "inline InterfaceMetricsPtr Get() {")
	checkForString(t, ret, ".delphi.hpp", "int32_t *key = (int32_t *)tbl_iter_.Key();")
	checkForString(t, ret, ".delphi.hpp", "return make_shared<InterfaceMetrics>(*key, tbl_iter_.Value())")

	checkForString(t, ret, ".delphi.cc", "InterfaceMetrics::InterfaceMetrics(int32_t key, char *ptr) {")
	checkForString(t, ret, ".delphi.cc", "PktCounter_ = make_shared<delphi::metrics::Counter>((uint64_t *)ptr);\n    ptr += delphi::metrics::Counter::Size();")
	checkForString(t, ret, ".delphi.cc", "PktRate_ = make_shared<delphi::metrics::Gauge>((double *)ptr);\n    ptr += delphi::metrics::Gauge::Size();")
	checkForString(t, ret, ".delphi.cc", "InterfaceMetricsPtr InterfaceMetrics::NewInterfaceMetrics(int32_t key) {")

}

func TestDelphicMetricsKeyType(t *testing.T) {

	// proto definition
	req := plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceMetrics"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String("InterfaceIdx"),
							},
							{
								Name:     proto.String("PktCounter"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Counter"),
							},
							{
								Name:     proto.String("PktRate"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Gauge"),
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
	checkForString(t, ret, ".delphi.hpp", "class InterfaceMetrics : public delphi::metrics::DelphiMetrics {")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::CounterPtr   PktCounter_;")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::GaugePtr     PktRate_;")
	checkForString(t, ret, ".delphi.hpp", "InterfaceMetrics(test::InterfaceIdx key, char *ptr);")
	checkForString(t, ret, ".delphi.hpp", "InterfaceMetrics(char *kptr, char *vptr) : InterfaceMetrics(getInterfaceIdxFromPtr(kptr), vptr){ };")
	checkForString(t, ret, ".delphi.hpp", "InterfaceIdx GetKey() { return key_; };")
	checkForString(t, ret, ".delphi.hpp", "static InterfaceMetricsPtr NewInterfaceMetrics(test::InterfaceIdx key);")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::CounterPtr PktCounter() { return PktCounter_; };")
	checkForString(t, ret, ".delphi.hpp", "delphi::metrics::GaugePtr PktRate() { return PktRate_; };")
	checkForString(t, ret, ".delphi.hpp", "REGISTER_METRICS(InterfaceMetrics);")
	checkForString(t, ret, ".delphi.hpp", "class InterfaceMetricsIterator {")
	checkForString(t, ret, ".delphi.hpp", "explicit InterfaceMetricsIterator(delphi::shm::TableIterator tbl_iter) {")
	checkForString(t, ret, ".delphi.hpp", "inline InterfaceMetricsPtr Get() {")
	checkForString(t, ret, ".delphi.hpp", "char *keyptr = tbl_iter_.Key();")
	checkForString(t, ret, ".delphi.hpp", "test::InterfaceIdx key;")
	checkForString(t, ret, ".delphi.hpp", "key.ParseFromString(keystr)")
	checkForString(t, ret, ".delphi.hpp", "return make_shared<InterfaceMetrics>(key, tbl_iter_.Value());")

	checkForString(t, ret, ".delphi.cc", "InterfaceMetrics::InterfaceMetrics(test::InterfaceIdx key, char *ptr) {")
	checkForString(t, ret, ".delphi.cc", "PktCounter_ = make_shared<delphi::metrics::Counter>((uint64_t *)ptr);\n    ptr += delphi::metrics::Counter::Size();")
	checkForString(t, ret, ".delphi.cc", "PktRate_ = make_shared<delphi::metrics::Gauge>((double *)ptr);\n    ptr += delphi::metrics::Gauge::Size();")
	checkForString(t, ret, ".delphi.cc", "InterfaceMetricsPtr InterfaceMetrics::NewInterfaceMetrics(test::InterfaceIdx key) {")

}

func TestDelphicMetricsError(t *testing.T) {
	// metrics definition without key
	req := plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceMetrics"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("PktCounter"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Counter"),
							},
							{
								Name:     proto.String("PktRate"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Gauge"),
							},
						},
					},
				},
			},
		},
	}

	ret, err := runTemplates(&req)
	Assert(t, (err != nil), "Metrics without key was accepted", ret)
	Assert(t, strings.Contains(err.Error(), "metrics does not have Key field"), "incorrect error", err)

	// metrics definition without key
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceMetrics"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Counter"),
							},
							{
								Name:     proto.String("PktCounter"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Counter"),
							},
							{
								Name:     proto.String("PktRate"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Gauge"),
							},
						},
					},
				},
			},
		},
	}

	ret, err = runTemplates(&req)
	Assert(t, (err != nil), "Metrics with invalid key type was accepted", ret)
	Assert(t, strings.Contains(err.Error(), "Key field type can not be counter or gauge"), "incorrect error", err)

	// metrics with invalid field type
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceMetrics"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
							{
								Name:     proto.String("PktCounter"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Invalid"),
							},
							{
								Name:     proto.String("PktRate"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_MESSAGE.Enum(),
								TypeName: proto.String(".delphi.Gauge"),
							},
						},
					},
				},
			},
		},
	}

	ret, err = runTemplates(&req)
	Assert(t, (err != nil), "Metrics with invalid field type accepted", ret)
	Assert(t, strings.Contains(err.Error(), "Invalid field type for"), "incorrect error", err)

	// metrics with invalid field type
	req = plugin.CodeGeneratorRequest{
		FileToGenerate: []string{"test.proto"},
		ProtoFile: []*google_protobuf.FileDescriptorProto{
			{
				Name:    proto.String("test.proto"),
				Package: proto.String("test"),
				MessageType: []*google_protobuf.DescriptorProto{
					{
						Name: proto.String("InterfaceMetrics"),
						Field: []*google_protobuf.FieldDescriptorProto{
							{
								Name:     proto.String("Key"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
							{
								Name:     proto.String("PktCounter"),
								Type:     google_protobuf.FieldDescriptorProto_TYPE_INT32.Enum(),
								TypeName: proto.String(google_protobuf.FieldDescriptorProto_TYPE_INT32.String()),
							},
						},
					},
				},
			},
		},
	}

	ret, err = runTemplates(&req)
	Assert(t, (err != nil), "Metrics with invalid field type accepted", ret)
	Assert(t, strings.Contains(err.Error(), "Invalid field type for"), "incorrect error", err)
}
