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
	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
	mutator "github.com/pensando/sw/venice/utils/apigen/autogrpc"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
)

//--- Mutators functions ---//
func reqMutator(req *plugin.CodeGeneratorRequest) {
	mutator.AddAutoGrpcEndpoints(req)
}

func fieldCheck(fields genswagger.SwaggerSchemaObjectProperties) bool {
	var speccheck, statuscheck, metacheck bool
	for _, v := range fields {
		if strings.ToLower(v.Key) == "meta" {
			metacheck = true
		} else if strings.ToLower(v.Key) == "spec" {
			speccheck = true
		} else if strings.ToLower(v.Key) == "status" {
			statuscheck = true
		}
	}
	return metacheck && speccheck && statuscheck
}

func generateCreateUpdateObjects(m map[string]string, str string, obj *genswagger.SwaggerObject) {
	for _, v := range m {
		if objGroup, ok := obj.Definitions[v]; ok {
			indexToRemove := -1
			objUpdateGroup := objGroup
			if fieldCheck(objGroup.Properties) {
				if str == "Update" {
					objUpdateGroup.Properties = make([]genswagger.KeyVal, len(objGroup.Properties))
					copy(objUpdateGroup.Properties, objGroup.Properties)
					for i, v := range objUpdateGroup.Properties {
						if v.Key == "meta" {
							v.Value = genswagger.SwaggerSchemaObject{SchemaCore: genswagger.SchemaCore{Ref: fmt.Sprintf("#/definitions/apiObjectMetaUpdate")}}
							objUpdateGroup.Properties[i].Value = v.Value
						}
					}
				}
				for i, p := range objUpdateGroup.Properties {
					if strings.ToLower(p.Key) == "status" {
						indexToRemove = i
					}
				}
				if indexToRemove != -1 {
					objUpdateGroup.Properties[indexToRemove] = objUpdateGroup.Properties[len(objGroup.Properties)-1]
					objUpdateGroup.Properties = objUpdateGroup.Properties[:len(objGroup.Properties)-1]
				}
			}
			obj.Definitions[v+str] = objUpdateGroup
		}
	}
}

func specFinalizer(obj *genswagger.SwaggerObject, file *descriptor.File, reg *descriptor.Registry, opts genswagger.Opts) error {
	var pkg, version string
	pkg = strings.Title(file.GoPkg.Name)
	path := make(genswagger.SwaggerPathsObject)
	crudObjs := make(map[string]bool)

	glog.V(1).Infof("processing for file [%s]", *file.Name)
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

	if opts.Mode == genswagger.External {
		//Creating ApiObjecMetaUpdate object without all properties except labels
		if metaFin, ok := obj.Definitions["apiObjectMeta"]; ok {
			indexOfLabels := -1
			for i, v := range metaFin.Properties {
				if v.Key == "labels" {
					indexOfLabels = i
				}
			}
			if indexOfLabels != -1 {
				metaFin.Properties = metaFin.Properties[indexOfLabels : indexOfLabels+1]
				obj.Definitions["apiObjectMetaUpdate"] = metaFin
			}
		}

		//Creating Update and Create Objects for top level objects
		var putMap = make(map[string]string)
		var postMap = make(map[string]string)
		re := regexp.MustCompile("definitions/(.*)")
		for _, v := range obj.Paths {
			if v.Put != nil {
				putRef := v.Put.Parameters[len(v.Put.Parameters)-1].Schema.Ref
				match := re.FindStringSubmatch(putRef)
				putMap[match[1]] = match[1]
			}
			if v.Post != nil && v.Post.Parameters != nil && len(v.Post.Parameters) != 0 {
				postRef := v.Post.Parameters[len(v.Post.Parameters)-1].Schema.Ref
				match := re.FindStringSubmatch(postRef)
				postMap[match[1]] = match[1]
			}
		}

		generateCreateUpdateObjects(postMap, "Create", obj)
		generateCreateUpdateObjects(putMap, "Update", obj)
		// Modifying schema definitions for put and post methods, and creating
		for _, v := range obj.Paths {
			if v.Post != nil && v.Post.Parameters != nil && len(v.Post.Parameters) != 0 {
				v.Post.Parameters[len(v.Post.Parameters)-1].Schema.Ref = v.Post.Parameters[len(v.Post.Parameters)-1].Schema.Ref + "Create"
			}
		}
		for _, v := range obj.Paths {
			if v.Put != nil && v.Put.Parameters != nil && len(v.Put.Parameters) != 0 {
				v.Put.Parameters[len(v.Put.Parameters)-1].Schema.Ref = v.Put.Parameters[len(v.Put.Parameters)-1].Schema.Ref + "Update"
			}
		}
	}

	svcs := file.Services
	for _, s := range svcs {
		glog.V(1).Infof("processing for Service [%s]", *s.Name)
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
		// Add any proxy paths if defined.
		pp, err := common.GetProxyPaths(s)
		if err != nil {
			glog.Fatalf("failed to get Proxy paths (%s)", err)
		}
		if len(pp) > 0 {
			glog.V(1).Infof("got Proxy paths [%+v]", pp)
			for _, ppath := range pp {
				params := []genswagger.SwaggerParameterObject{}
				for _, fd := range ppath.FormParams {
					params = append(params, genswagger.SwaggerParameterObject{In: "formData", Name: fd.Name, Description: fd.Description, Type: fd.Type, Required: fd.Required})
				}
				respObj := strings.Join(strings.Split(ppath.Response, "."), "")
				p := genswagger.SwaggerPathItemObject{
					Post: &genswagger.SwaggerOperationObject{
						Summary:     ppath.DocString,
						Parameters:  params,
						Tags:        []string{fmt.Sprintf("%s/%s", strings.ToLower(pkg), version)},
						OperationID: strings.Join(strings.Split(strings.TrimPrefix(ppath.Path, "/"), "/"), "_"),
						Responses: genswagger.SwaggerResponsesObject{
							"200": genswagger.SwaggerResponseObject{
								Schema: genswagger.SwaggerSchemaObject{
									SchemaCore: genswagger.SchemaCore{
										Ref: "#/definitions/" + respObj,
									},
								},
							},
						},
					},
				}
				obj.Paths[strings.TrimSuffix(ppath.FullPath, "/")] = p
				glog.V(1).Infof("adding new proxy path [%v][%+v]", ppath.Path, p)
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
					glog.V(1).Infof("Put Parameters: %v", putParams)
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

func initializer(reg *descriptor.Registry, opts genswagger.Opts) ([]*descriptor.Message, []*descriptor.Enum) {
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

func methFinalizer(obj *genswagger.SwaggerPathItemObject, path *string, method *descriptor.Method, reg *descriptor.Registry, opts genswagger.Opts) error {
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

func messageFinalizer(obj *genswagger.SwaggerSchemaObject, message *descriptor.Message, reg *descriptor.Registry, opts genswagger.Opts) error {
	// Adding the required field
	req := []string{}
	del := []int{}
	for _, f := range message.Fields {
		// ignore any fields that are to be ignored by json
		if common.GetJSONTag(f) == "-" {
			continue
		}
		profile, err := common.GenerateVeniceCheckFieldProfile(f, reg)
		if err == nil && profile != nil {
			// TODO: Use swagger version instead of all
			if profile.Required["all"] {
				tag := common.GetJSONTag(f)
				req = append(req, tag)
			}
		}
	}
	for i, o := range obj.Properties {
		// remove any fields that are to be ignored by json
		if o.Key == "-" {
			del = append(del, i)
		}
	}
	if len(del) > 0 {
		props := obj.Properties
		obj.Properties = nil
		index := 0
		for i, f := range props {
			if i == del[index] {
				index++
				continue
			}
			obj.Properties = append(obj.Properties, f)
		}
	}
	if len(req) != 0 {
		obj.Required = req
	}
	return nil
}

func fieldFinalizer(obj *genswagger.SwaggerSchemaObject, field *descriptor.Field, reg *descriptor.Registry, opts genswagger.Opts) error {
	glog.V(1).Infof("called Field Finalizer for %v", *field.Name)
	profile, err := common.GenerateVeniceCheckFieldProfile(field, reg)
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
		if val, ok := profile.DocStrings["all"]; ok {
			description := ""
			for _, s := range val {
				temp := strings.TrimSpace(s)
				if len(temp) == 0 {
					continue
				}
				temp = strings.ToUpper(string(temp[0])) + temp[1:]
				if !strings.HasSuffix(temp, ".") {
					temp = temp + "."
				}
				description += temp + " "
			}
			description = strings.TrimSpace(description)
			obj.Description += description
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

func fieldNameFinalizer(field *descriptor.Field, opts genswagger.Opts) string {
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
