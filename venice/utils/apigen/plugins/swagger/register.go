package plugins

import (
	"fmt"
	"regexp"
	"sort"
	"strings"

	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/golang/glog"
	"github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
	gwplugins "github.com/pensando/grpc-gateway/protoc-gen-grpc-gateway/plugins"
	"github.com/pensando/grpc-gateway/protoc-gen-swagger/genswagger"
	reg "github.com/pensando/grpc-gateway/protoc-gen-swagger/plugins"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/apigen/annotations"
	mutator "github.com/pensando/sw/venice/utils/apigen/autogrpc"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
)

//--- Mutators functions ---//
func reqMutator(req *plugin.CodeGeneratorRequest) {
	mutator.AddAutoGrpcEndpoints(req)
}

func specFinalizer(obj *genswagger.SwaggerObject, file *descriptor.File, reg *descriptor.Registry) error {
	var pkg, version string
	pkg = strings.Title(file.GoPkg.Name)
	path := make(genswagger.SwaggerPathsObject)
	crudObjs := make(map[string]bool)

	apiResp := genswagger.SwaggerSchemaObject{
		SchemaCore: genswagger.SchemaCore{
			Type: "object",
		},
		Properties: genswagger.SwaggerSchemaObjectProperties{
			{
				Key: "status-code",
				Value: genswagger.SchemaCore{
					Type: "integer",
				},
			},
			{
				Key: "error",
				Value: genswagger.SchemaCore{
					Type: "string",
				},
			},
			{
				Key: "references",
				Value: genswagger.SchemaCore{
					Type: "array",
					Items: &genswagger.SwaggerItemsObject{
						Type: "string",
					},
				},
			},
		},
	}

	svcs := file.Services
	for _, s := range svcs {
		i, err := gwplugins.GetExtension("venice.apiVersion", s)
		if err == nil && i != nil {
			version = i.(string)
		}
		r, err := gwplugins.GetExtension("venice.apiRestService", s)
		if err == nil && r != nil {
			for _, v := range r.([]*venice.RestEndpoint) {
				crudObjs[v.Object] = true
			}
		}
		ext, err := gwplugins.GetExtension("venice.naplesRestService", s)
		if err != nil {
			// Ignore error here as swagger gen is not parsing the errors
			continue
		}
		glog.V(1).Infof("EXTS: %v", ext)
		obj.Definitions["netprotoApiResponse"] = apiResp

		naplesRestServices, ok := ext.([]*venice.RestEndpoint)
		if !ok {
			glog.V(1).Infof("Failed casting to naples rest endpoint object")
		}

		for _, n := range naplesRestServices {
			for _, m := range n.Method {
				if m == "post" || m == "list" {
					postResp := make(genswagger.SwaggerResponsesObject)
					listResp := make(genswagger.SwaggerResponsesObject)
					params := []genswagger.SwaggerParameterObject{
						{
							Name:     "body",
							In:       "body",
							Required: true,
							Schema: &genswagger.SwaggerSchemaObject{
								SchemaCore: genswagger.SchemaCore{
									Ref: fmt.Sprintf("#/definitions/netproto%s", n.Object),
								},
							},
						},
					}
					postResp["200"] = genswagger.SwaggerResponseObject{
						Schema: genswagger.SwaggerSchemaObject{
							SchemaCore: genswagger.SchemaCore{
								Ref: "#/definitions/netprotoApiResponse",
							},
						},
					}
					postResp["500"] = genswagger.SwaggerResponseObject{
						Schema: genswagger.SwaggerSchemaObject{
							SchemaCore: genswagger.SchemaCore{
								Ref: "#/definitions/netprotoApiResponse",
							},
						},
					}

					listResp["200"] = genswagger.SwaggerResponseObject{
						Schema: genswagger.SwaggerSchemaObject{
							SchemaCore: genswagger.SchemaCore{
								Type: "array",
								Items: &genswagger.SwaggerItemsObject{
									Ref: fmt.Sprintf("#/definitions/netproto%s", n.Object),
								},
							},
						},
					}

					p := genswagger.SwaggerPathItemObject{
						Post: &genswagger.SwaggerOperationObject{
							Summary:    fmt.Sprintf("Creates a %s Object", n.Object),
							Tags:       []string{n.Object},
							Parameters: params,
							Responses:  postResp,
						},
						Get: &genswagger.SwaggerOperationObject{
							Summary:   fmt.Sprintf("List all %s Objects", n.Object),
							Tags:      []string{n.Object},
							Responses: listResp,
						},
					}
					path[fmt.Sprintf("/%s/", n.Prefix)] = p
					obj.Paths = path

				}

				if m == "put" || m == "delete" {
					resp := make(genswagger.SwaggerResponsesObject)
					//var schema *genswagger.SwaggerSchemaObject
					putSchema := &genswagger.SwaggerSchemaObject{
						SchemaCore: genswagger.SchemaCore{
							Ref: fmt.Sprintf("#/definitions/netproto%sSpec", n.Object),
						},
					}
					deleteSchema := &genswagger.SwaggerSchemaObject{
						SchemaCore: genswagger.SchemaCore{
							Type: "object",
						},
						Properties: genswagger.SwaggerSchemaObjectProperties{
							{
								Key: "kind",
								Value: genswagger.SchemaCore{
									Type: "string",
								},
							},
							{
								Key: "meta",
								Value: genswagger.SchemaCore{
									Ref: "#/definitions/apiObjectMeta",
								},
							},
						},
					}
					putParams := []genswagger.SwaggerParameterObject{
						{
							Name:     "body",
							In:       "body",
							Required: true,
							Schema:   putSchema,
						},
					}
					glog.V(1).Infof("Put Parameters: %v ", putParams)
					deleteParams := []genswagger.SwaggerParameterObject{
						{
							Name:     "body",
							In:       "body",
							Required: true,
							Schema:   deleteSchema,
						},
					}
					resp["200"] = genswagger.SwaggerResponseObject{
						Schema: genswagger.SwaggerSchemaObject{
							SchemaCore: genswagger.SchemaCore{
								Ref: "#/definitions/netprotoApiResponse",
							},
						},
					}
					resp["500"] = genswagger.SwaggerResponseObject{
						Schema: genswagger.SwaggerSchemaObject{
							SchemaCore: genswagger.SchemaCore{
								Ref: "#/definitions/netprotoApiResponse",
							},
						},
					}
					p := genswagger.SwaggerPathItemObject{
						// ToDo enable PUT when updates are supported
						//Put: &genswagger.SwaggerOperationObject{
						//	Tags:       []string{n.Object},
						//	Parameters: putParams,
						//	Responses:  resp,
						//},
						Delete: &genswagger.SwaggerOperationObject{
							Summary:    fmt.Sprintf("Deletes a %s Object", n.Object),
							Tags:       []string{n.Object},
							Parameters: deleteParams,
							Responses:  resp,
						},
					}
					path[fmt.Sprintf("/%s%s", n.Prefix, n.Pattern)] = p
					obj.Paths = path
				}
			}
		}

	}
	obj.Info.Title = pkg + " API reference"
	obj.Info.Version = version
	desc, err := common.GetLocation(file.SourceCodeInfo, []int{common.PackageType})
	if err == nil {
		obj.Info.Description = ""
		for _, line := range strings.Split(desc.GetLeadingComments(), "\n") {
			line = strings.TrimSpace(line)
			if strings.HasPrefix(line, "CrudObjs:") {
				line = strings.TrimPrefix(line, "CrudObjs:")
				objs := strings.Split(line, ",")
				for _, obj := range objs {
					crudObjs[strings.TrimSpace(obj)] = true
				}
			} else {
				obj.Info.Description = obj.Info.Description + line + "\n"
			}
		}
		if len(crudObjs) > 0 {
			keys := []string{}
			for k := range crudObjs {
				keys = append(keys, k)
			}

			sort.Strings(keys)
			obj.Info.Description = obj.Info.Description + " API Objects are: "
			for _, k := range keys {
				obj.Info.Description = obj.Info.Description + k + " "
			}
		}
	}
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
	ret := []*descriptor.Message{m, m1, m2}
	// Find any messages that are annotated
	for _, n := range reg.GetAllFQMNs() {
		msg, err := reg.LookupMsg("", n)
		if err != nil {
			glog.Fatalf("message not found [%v]", n)
		}
		i, err := gwplugins.GetExtension("venice.forceDoc", msg)
		if err == nil && i.(bool) {
			ret = append(ret, msg)
		}
	}
	return ret, nil
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
	category := globals.ConfigURIPrefix
	if ext, err := gwplugins.GetExtension("venice.fileCategory", method.Service.File); err == nil {
		category = ext.(string)
	}

	prefix := "/" + category + "/" + svcPrefix + "/" + version
	if svcPrefix == "" {
		prefix = "/" + category + "/" + version
	}

	if !strings.HasPrefix(*path, prefix) {
		*path = strings.TrimSuffix(prefix, "/") + "/" + strings.TrimLeft(*path, "/")
	}
	addErrors := func(op *genswagger.SwaggerOperationObject) {
		for k, v := range defaultErrorResponses {
			op.Responses[k] = v
		}
	}
	tag := svcPrefix + "/" + version
	if svcPrefix == "" {
		tag = method.Service.File.GoPkg.Name + "/" + version
	}
	re := regexp.MustCompile(fmt.Sprintf("^%s(_[\\d]+)*$", method.GetName()))
	if obj != nil {
		if common.IsAutoGenMethod(method) {
			if obj.Get != nil && re.Match([]byte(obj.Get.OperationID)) {
				obj.Get.OperationID = strings.TrimPrefix(obj.Get.OperationID, "Auto")
				obj.Get.Tags[0] = tag
				addErrors(obj.Get)
			}
			if obj.Post != nil && re.Match([]byte(obj.Post.OperationID)) {
				obj.Post.OperationID = strings.TrimPrefix(obj.Post.OperationID, "Auto")
				obj.Post.Tags[0] = tag
				addErrors(obj.Post)
			}
			if obj.Put != nil && re.Match([]byte(obj.Put.OperationID)) {
				obj.Put.OperationID = strings.TrimPrefix(obj.Put.OperationID, "Auto")
				obj.Put.Tags[0] = tag
				addErrors(obj.Put)
			}
			if obj.Delete != nil && re.Match([]byte(obj.Delete.OperationID)) {
				obj.Delete.OperationID = strings.TrimPrefix(obj.Delete.OperationID, "Auto")
				obj.Delete.Tags[0] = tag
				addErrors(obj.Delete)
			}
			if obj.Patch != nil && re.Match([]byte(obj.Patch.OperationID)) {
				obj.Patch.OperationID = strings.TrimPrefix(obj.Patch.OperationID, "Auto")
				obj.Patch.Tags[0] = tag
				addErrors(obj.Patch)
			}
		} else {
			if obj.Get != nil && re.Match([]byte(obj.Get.OperationID)) {
				obj.Get.OperationID = "Get" + obj.Get.OperationID
				obj.Get.Tags[0] = tag
			}
			if obj.Post != nil && re.Match([]byte(obj.Post.OperationID)) {
				obj.Post.OperationID = "Post" + obj.Post.OperationID
				obj.Post.Tags[0] = tag
			}
			if obj.Put != nil && re.Match([]byte(obj.Put.OperationID)) {
				obj.Put.OperationID = "Put" + obj.Put.OperationID
				obj.Put.Tags[0] = tag
			}
			if obj.Delete != nil && re.Match([]byte(obj.Delete.OperationID)) {
				obj.Post.OperationID = "Delete" + obj.Delete.OperationID
				obj.Delete.Tags[0] = tag
			}
			if obj.Patch != nil && re.Match([]byte(obj.Patch.OperationID)) {
				obj.Patch.OperationID = "Patch" + obj.Patch.OperationID
				obj.Patch.Tags[0] = tag
			}
		}
	}
	return nil
}

func messageFinalizer(obj *genswagger.SwaggerSchemaObject, message *descriptor.Message, reg *descriptor.Registry) error {
	// Adding the required field
	req := []string{}
	for _, f := range message.Fields {
		profile, err := generateVeniceCheckFieldProfile(f, reg)
		if err == nil && profile != nil {
			// TODO: Use swagger version instead of all
			if profile.Required["all"] {
				tag := common.GetJSONTag(f)
				req = append(req, tag)
			}
		}
	}
	if len(req) != 0 {
		obj.Required = req
	}
	return nil
}

func generateVeniceCheckFieldProfile(field *descriptor.Field, reg *descriptor.Registry) (*common.FieldProfile, error) {
	r, err := gwplugins.GetExtension("venice.check", field)
	if err == nil {
		// We have some options specified
		profile := common.FieldProfile{}
		profile.Init()
		for _, v := range r.([]string) {
			fldv, err := common.ParseValidator(v)
			if err != nil {
				return nil, err
			}
			fn, ok := common.ValidatorProfileMap[fldv.Fn]
			if !ok {
				glog.Fatalf("Did not find entry in profile map for %v", fldv.Fn)
			}
			ver := fldv.Ver
			if ver == "" || ver == "*" {
				ver = "all"
			}
			err = fn(field, reg, ver, fldv.Args, &profile)
			if err != nil {
				glog.Fatalf("cannot parse validator (%s)", err)
			}
		}
		return &profile, nil
	}
	return nil, nil
}

func fieldFinalizer(obj *genswagger.SwaggerSchemaObject, field *descriptor.Field, reg *descriptor.Registry) error {
	glog.V(1).Infof("called Field Finalizer for %v", *field.Name)
	profile, err := generateVeniceCheckFieldProfile(field, reg)
	// TODO: Use swagger version instead of all
	if err == nil && profile != nil {
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
		if val, ok := profile.Pattern["all"]; ok {
			obj.Pattern = val
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
