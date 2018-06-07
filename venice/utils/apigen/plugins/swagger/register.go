package plugins

import (
	"fmt"
	"strings"

	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/golang/glog"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	gwplugins "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	"github.com/pensando/grpc-gateway/protoc-gen-swagger/genswagger"
	reg "github.com/pensando/grpc-gateway/protoc-gen-swagger/plugins"

	mutator "github.com/pensando/sw/venice/utils/apigen/autogrpc"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
)

//--- Mutators functions ---//
func reqMutator(req *plugin.CodeGeneratorRequest) {
	mutator.AddAutoGrpcEndpoints(req)
}

func specFinalizer(obj *genswagger.SwaggerObject, file *descriptor.File, reg *descriptor.Registry) error {
	return nil
}

var defaultErrorResponses = genswagger.SwaggerResponsesObject{
	"400": genswagger.SwaggerResponseObject{
		Description: "Bad request parameters",
		Schema:      genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiStatus")}},
	},
	"401": genswagger.SwaggerResponseObject{
		Description: "Unauthorized request",
		Schema:      genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiStatus")}},
	},
	"409": genswagger.SwaggerResponseObject{
		Description: "Conflict while processing request",
		Schema:      genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiStatus")}},
	},
	"412": genswagger.SwaggerResponseObject{
		Description: "Pre-condition failed",
		Schema:      genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiStatus")}},
	},
	"500": genswagger.SwaggerResponseObject{
		Description: "Internal server error",
		Schema:      genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiStatus")}},
	},
	"501": genswagger.SwaggerResponseObject{
		Description: "Request not implemented",
		Schema:      genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiStatus")}},
	},
}

func initializer(reg *descriptor.Registry) ([]*descriptor.Message, []*descriptor.Enum) {
	m, err := reg.LookupMsg("", ".api.Status")
	if err != nil {
		panic(fmt.Errorf("could not find status object(%s)", err))
	}
	m1, err := reg.LookupMsg("", ".api.StatusResult")
	if err != nil {
		panic(fmt.Errorf("could not find StatusResult object(%s)", err))
	}
	m2, err := reg.LookupMsg("", ".api.ObjectRef")
	if err != nil {
		panic(fmt.Errorf("could not find ObjectRef object(%s)", err))
	}
	return []*descriptor.Message{m, m1, m2}, nil
}

func methFinalizer(obj *genswagger.SwaggerPathItemObject, path *string, method *descriptor.Method, reg *descriptor.Registry) error {
	glog.V(1).Infof("called method Finalizer for %v/%v", *method.Name, *path)
	var svcPrefix, version string
	i, err := gwplugins.GetExtension("venice.apiVersion", method.Service)
	if err == nil && i != nil {
		version = i.(string)
	} else {
		panic(fmt.Sprintf("got error (%s)", err))
	}
	i, err = gwplugins.GetExtension("venice.apiPrefix", method.Service)
	if err == nil && i != nil {
		svcPrefix = i.(string)
	}
	prefix := "/" + version + "/" + svcPrefix
	if !strings.HasPrefix(*path, prefix) {
		*path = prefix + *path
	}
	addErrors := func(op *genswagger.SwaggerOperationObject) {
		for k, v := range defaultErrorResponses {
			op.Responses[k] = v
		}
	}
	if obj != nil {
		if common.IsAutoGenMethod(method) {
			if obj.Get != nil && obj.Get.OperationID == method.GetName() {
				obj.Get.OperationID = strings.TrimPrefix(obj.Get.OperationID, "Auto")
				obj.Get.Tags = append(obj.Get.Tags, "version:"+version)
				addErrors(obj.Get)
			}
			if obj.Post != nil && obj.Post.OperationID == method.GetName() {
				obj.Post.OperationID = strings.TrimPrefix(obj.Post.OperationID, "Auto")
				obj.Post.Tags = append(obj.Post.Tags, "version:"+version)
				addErrors(obj.Post)
			}
			if obj.Put != nil && obj.Put.OperationID == method.GetName() {
				obj.Put.OperationID = strings.TrimPrefix(obj.Put.OperationID, "Auto")
				obj.Put.Tags = append(obj.Put.Tags, "version:"+version)
				addErrors(obj.Put)
			}
			if obj.Delete != nil && obj.Delete.OperationID == method.GetName() {
				obj.Delete.OperationID = strings.TrimPrefix(obj.Delete.OperationID, "Auto")
				obj.Delete.Tags = append(obj.Delete.Tags, "version:"+version)
				addErrors(obj.Delete)
			}
			if obj.Patch != nil && obj.Patch.OperationID == method.GetName() {
				obj.Patch.OperationID = strings.TrimPrefix(obj.Patch.OperationID, "Auto")
				obj.Patch.Tags = append(obj.Patch.Tags, "version:"+version)
				addErrors(obj.Patch)
			}
		}
	}
	return nil
}

func messageFinalizer(obj *genswagger.SwaggerSchemaObject, message *descriptor.Message, reg *descriptor.Registry) error {
	return nil
}

func fieldFinalizer(obj *genswagger.SwaggerSchemaObject, field *descriptor.Field, reg *descriptor.Registry) error {
	glog.V(1).Infof("called Field Finalizer for %v", *field.Name)
	r, err := gwplugins.GetExtension("venice.check", field)
	if err == nil {
		// We hae some options specified
		profile := common.FieldProfile{}
		profile.Init()
		for _, v := range r.([]string) {
			fldv, err := common.ParseValidator(v)
			if err != nil {
				return err
			}
			fn, ok := common.ValidatorProfileMap[fldv.Fn]
			if !ok {
				glog.Fatalf("Did not find entry in profile map for %v", fldv.Fn)
			}
			ver := fldv.Ver
			if ver == "" || ver == "*" {
				ver = "all"
			}
			fn(field, reg, ver, fldv.Args, &profile)
		}
		if len(profile.Enum) > 0 {
			obj.Enum = append(obj.Enum, profile.Enum["all"]...)
		}
		if len(profile.EnumHints) > 0 {
			obj.XUiHints = profile.EnumHints["all"]
		}
		if val, ok := profile.MinInt["all"]; ok {
			obj.Minimum = int(val)
		}
		if val, ok := profile.MaxInt["all"]; ok {
			obj.Maximum = int(val)
		}
		if val, ok := profile.MinLen["all"]; ok {
			obj.MinLength = int(val)
		}
		if val, ok := profile.MaxLen["all"]; ok {
			obj.MaxLength = int(val)
		}
		if val, ok := profile.Example["all"]; ok {
			obj.Example = val
		}
		if val, ok := profile.Default["all"]; ok {
			obj.Default = val
		}
		if val, ok := profile.DocString["all"]; ok {
			obj.Description = obj.Description + val
		}
	}
	// Add Default to the spec
	def, found, err := common.ParseDefaults(field.Message.File, field)
	if err != nil {
		return err
	}
	if found {
		if defval, ok := def.Map["all"]; ok {
			defval = strings.TrimSuffix(defval, "\"")
			defval = strings.TrimPrefix(defval, "\"")
			obj.Default = defval
		}
	}
	return nil
}

func fieldNameFinalizer(field *descriptor.Field) string {
	tag := common.GetJSONTag(field)
	if tag == "" {
		return field.GetName()
	}
	return tag
}

func init() {
	// Register Option Parsers
	common.RegisterOptionParsers()

	fnz := genswagger.Finalizer{}
	fnz.Init = initializer
	fnz.Spec = specFinalizer
	fnz.Method = methFinalizer
	fnz.Def = messageFinalizer
	fnz.Field = fieldFinalizer
	fnz.FieldName = fieldNameFinalizer

	// Register request mutators
	reg.RegisterReqMutator("pensando", reqMutator)
	reg.RegisterFinalizer("pensando", fnz)
}
