package autogrpc

import (
	"errors"
	"fmt"
	"regexp"
	"sort"
	"strings"

	"reflect"

	"github.com/golang/glog"

	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
)

func getMethodOperOption(pb *descriptor.MethodDescriptorProto) *proto.ExtensionDesc {
	desc, err := proto.ExtensionDescs(pb)
	if err != nil {
		return nil
	}
	for _, d := range desc {
		if d.Name == "venice.methodOper" {
			return d
		}
	}
	return nil
}

func getExtensionDesc(pb proto.Message, extname string) (*proto.ExtensionDesc, error) {
	desc := proto.RegisteredExtensions(pb)
	for _, d := range desc {
		if d.Name == extname {
			return d, nil
		}
	}
	return nil, errors.New("ExtensionDesc not found")
}

func getExtension(pb proto.Message, extname string) (interface{}, error) {
	d, err := getExtensionDesc(pb, extname)
	if err != nil {
		return nil, err
	}
	e, err := proto.GetExtension(pb, d)
	if err != nil {
		return nil, err
	}
	return e, err
}

func trimSlash(in string) string {
	ret := strings.TrimSuffix(in, "/")
	ret = strings.TrimPrefix(ret, "/")
	return ret
}

func insertMethod(svc *descriptor.ServiceDescriptorProto, name, intype, outtype, oper string, watch bool, restopt *googapi.HttpRule) error {
	operDesc, err := getExtensionDesc(&descriptor.MethodOptions{}, "venice.methodOper")
	if err != nil {
		glog.V(1).Infof("Get methodOper desc failed (%s)\n", err)
		return err
	}
	autoDesc, err := getExtensionDesc(&descriptor.MethodOptions{}, "venice.methodAutoGen")
	if err != nil {
		glog.V(1).Infof("Get methodAutoGen desc failed (%s)\n", err)
		return err
	}
	httpOpt, err := getExtensionDesc(&descriptor.MethodOptions{}, "google.api.http")
	if err != nil {
		glog.V(1).Infof("Get google.api.http desc failed (%s)\n", err)
		return err
	}
	autogen := true
	m := descriptor.MethodDescriptorProto{
		Name:       &name,
		InputType:  &intype,
		OutputType: &outtype,
		Options:    &descriptor.MethodOptions{},
	}

	if watch {
		streaming := true
		m.ServerStreaming = &streaming
	}

	if err = proto.SetExtension(m.GetOptions(), operDesc, &oper); err != nil {
		glog.V(1).Infof("Failed to set Extension (%s)\n", err)
		return err
	}
	if err = proto.SetExtension(m.GetOptions(), autoDesc, &autogen); err != nil {
		glog.V(1).Infof("Failed to set Extension (%s)\n", err)
		return err
	}
	if restopt != nil {
		if err = proto.SetExtension(m.GetOptions(), httpOpt, restopt); err != nil {
			glog.V(1).Infof("Failed to set Rest Extension (%s)\n", err)
		}
	}
	svc.Method = append(svc.Method, &m)
	return nil
}

func insertGrpcCRUD(svc *descriptor.ServiceDescriptorProto, m, pkg string, resteps map[string]string) {
	var restopt *googapi.HttpRule

	glog.V(1).Infof("adding Grpc CRUD endpoints for %s [ %+v ]", m, resteps)
	// Add method
	if v, ok := resteps["post"]; ok {
		opt := googapi.HttpRule_Post{Post: v}
		restopt = &googapi.HttpRule{Pattern: &opt, Body: "*"}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoAdd%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"create", false, restopt)

	// Update method
	if v, ok := resteps["put"]; ok {
		opt := googapi.HttpRule_Put{Put: v}
		restopt = &googapi.HttpRule{Pattern: &opt, Body: "*"}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoUpdate%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"update", false, restopt)

	// Get method
	if v, ok := resteps["get"]; ok {
		opt := googapi.HttpRule_Get{Get: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoGet%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"get", false, restopt)

	// Delete method
	if v, ok := resteps["delete"]; ok {
		opt := googapi.HttpRule_Delete{Delete: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoDelete%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"delete", false, restopt)

	// List method
	if v, ok := resteps["list"]; ok {
		opt := googapi.HttpRule_Get{Get: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoList%s", m),
		".api.ListWatchOptions",
		fmt.Sprintf(".%s.%sList", pkg, m),
		"list", false, restopt)

	// Watch method
	// Rest Endpoint on Watch is not currently supported. Will be ignored if specified.
	insertMethod(svc,
		fmt.Sprintf("AutoWatch%s", m),
		".api.ListWatchOptions",
		fmt.Sprintf(".%s.AutoMsg%sWatchHelper", pkg, m),
		"watch", true, nil)
	glog.V(1).Infof("Generated AutoGrpc for [%s][%s]\n", *svc.Name, m)
}

func insertGrpcAutoMsgs(f *descriptor.FileDescriptorProto, msg string) {
	autoDesc, err := getExtensionDesc(&descriptor.MessageOptions{}, "venice.objectAutoGen")
	if err != nil {
		glog.V(1).Infof("Get objectAutoGen desc failed (%s)\n", err)
		return
	}
	embedDesc, err := getExtensionDesc(&descriptor.FieldOptions{}, "gogoproto.embed")
	if err != nil {
		glog.V(1).Infof("Get objectAutoGen desc failed (%s)\n", err)
		return
	}
	nullDesc, err := getExtensionDesc(&descriptor.FieldOptions{}, "gogoproto.nullable")
	if err != nil {
		glog.V(1).Infof("Get objectAutoGen desc failed (%s)\n", err)
		return
	}
	// Watch helper message
	{
		name := fmt.Sprintf("AutoMsg%sWatchHelper", msg)
		autoption := "watchhelper"

		mtype := fmt.Sprintf(".%s.%s", *f.Package, msg)
		fldname1 := "Type"
		fldname2 := "Object"

		var fldnum1 int32 = 1
		var fldnum2 int32 = 2

		var fldlabel1 = descriptor.FieldDescriptorProto_LABEL_OPTIONAL
		var fldlabel2 = descriptor.FieldDescriptorProto_LABEL_OPTIONAL

		var fldtype1 = descriptor.FieldDescriptorProto_TYPE_STRING
		var fldtype2 = descriptor.FieldDescriptorProto_TYPE_MESSAGE

		field1 := descriptor.FieldDescriptorProto{
			Name:   &fldname1,
			Number: &fldnum1,
			Label:  &fldlabel1,
			Type:   &fldtype1,
		}
		field2 := descriptor.FieldDescriptorProto{
			Name:     &fldname2,
			Number:   &fldnum2,
			Label:    &fldlabel2,
			Type:     &fldtype2,
			TypeName: &mtype,
		}

		var fields []*descriptor.FieldDescriptorProto
		fields = append(fields, &field1)
		fields = append(fields, &field2)

		msg := descriptor.DescriptorProto{
			Name:    &name,
			Field:   fields,
			Options: &descriptor.MessageOptions{},
		}
		proto.SetExtension(msg.GetOptions(), autoDesc, &autoption)
		f.MessageType = append(f.MessageType, &msg)
	}
	// List helper message
	{
		name := fmt.Sprintf("%sList", msg)
		autoption := "listhelper"
		embed := true
		nullable := false
		// mtype1 := ".api.ObjectMeta"
		mtype2 := ".api.TypeMeta"
		mtype3 := ".api.ListMeta"
		mtype := fmt.Sprintf(".%s.%s", *f.Package, msg)
		// fldname1 := "O"
		fldname2 := "T"
		fldname3 := "ListMeta"
		fldname4 := "Items"

		// var fldnum1 int32 = 1
		var fldnum2 int32 = 2
		var fldnum3 int32 = 3
		var fldnum4 int32 = 4

		var fldlabel2 = descriptor.FieldDescriptorProto_LABEL_OPTIONAL
		var fldlabel3 = descriptor.FieldDescriptorProto_LABEL_OPTIONAL
		var fldlabel4 = descriptor.FieldDescriptorProto_LABEL_REPEATED

		var fldtype2 = descriptor.FieldDescriptorProto_TYPE_MESSAGE
		var fldtype3 = descriptor.FieldDescriptorProto_TYPE_MESSAGE
		var fldtype4 = descriptor.FieldDescriptorProto_TYPE_MESSAGE

		field2 := descriptor.FieldDescriptorProto{
			Name:     &fldname2,
			Number:   &fldnum2,
			Label:    &fldlabel2,
			Type:     &fldtype2,
			TypeName: &mtype2,
			Options:  &descriptor.FieldOptions{},
		}
		proto.SetExtension(field2.GetOptions(), embedDesc, &embed)
		proto.SetExtension(field2.GetOptions(), nullDesc, &nullable)

		field3 := descriptor.FieldDescriptorProto{
			Name:     &fldname3,
			Number:   &fldnum3,
			Label:    &fldlabel3,
			Type:     &fldtype3,
			TypeName: &mtype3,
			Options:  &descriptor.FieldOptions{},
		}
		proto.SetExtension(field3.GetOptions(), embedDesc, &embed)
		proto.SetExtension(field3.GetOptions(), nullDesc, &nullable)

		field4 := descriptor.FieldDescriptorProto{
			Name:     &fldname4,
			Number:   &fldnum4,
			Label:    &fldlabel4,
			Type:     &fldtype4,
			TypeName: &mtype,
		}
		var fields []*descriptor.FieldDescriptorProto
		fields = append(fields, &field2)
		fields = append(fields, &field3)
		fields = append(fields, &field4)
		msg := descriptor.DescriptorProto{
			Name:    &name,
			Field:   fields,
			Options: &descriptor.MessageOptions{},
		}
		proto.SetExtension(msg.GetOptions(), autoDesc, &autoption)
		f.MessageType = append(f.MessageType, &msg)
	}
}

func isSingleton(m *descriptor.DescriptorProto) (bool, error) {
	objpreext, err := getExtension(m.GetOptions(), "venice.objectPrefix")
	if err != nil {
		return false, fmt.Errorf("not found")
	}
	o := objpreext.(*venice.ObjectPrefix)
	if o.GetSingleton() != "" {
		return true, nil
	}
	return false, nil
}

// getMessageURIPrefix returns the message URI sans the service prefix and object ID
func getMessageURIPrefix(m *descriptor.DescriptorProto) (string, error) {
	opts := m.GetOptions()
	if opts == nil {
		return "", fmt.Errorf("not found")
	}
	objpreext, err := getExtension(m.GetOptions(), "venice.objectPrefix")
	if err != nil {
		return "", fmt.Errorf("not found")
	}
	o := objpreext.(*venice.ObjectPrefix)
	path := trimSlash(o.Path)
	prfx := ""
	if prfx = o.GetCollection(); prfx == "" {
		prfx = o.GetSingleton()
	}
	container := trimSlash(prfx)
	if path != "" {
		container = "/" + path + "/" + container
	} else {
		container = "/" + container
	}
	return container, nil
}

// GetMessageURI returns the message URI sans the service prefix
func GetMessageURI(m *descriptor.DescriptorProto) (string, error) {
	ret, err := getMessageURIPrefix(m)
	if err != nil {
		return "", err
	}
	if s, _ := isSingleton(m); s {
		return ret, nil
	}
	return ret + "/{O.Name}", nil
}

func processActions(f *descriptor.FileDescriptorProto, s *descriptor.ServiceDescriptorProto, msgMap map[string]*descriptor.DescriptorProto) {
	opts := s.GetOptions()
	act, err := getExtension(opts, "venice.apiAction")
	if err != nil {
		glog.V(1).Infof("No action endpoints defined for %s (%s)", *s.Name, err)
	} else {
		for _, r := range act.([]*venice.ActionEndpoint) {
			path := ""
			if t := r.GetCollection(); t != "" {
				if _, ok := msgMap[t]; !ok {
					glog.Fatalf("unknown message [%s] in action definition", t)
				}
				path, err = getMessageURIPrefix(msgMap[t])
				if err != nil {
					glog.Fatalf("could not get message URI prefix for collection [%s]", t)
				}
			} else if t := r.GetObject(); t != "" {
				if _, ok := msgMap[t]; !ok {
					glog.Fatalf("unknown message [%s] in action definition", t)
				}
				path, err = GetMessageURI(msgMap[t])
				if err != nil {
					glog.Fatalf("could not get message URI for target [%s]", t)
				}
			} else {
				glog.Fatalf("empty Target")
			}
			if msgMap[r.Request] == nil || msgMap[r.Response] == nil {
				glog.Fatalf("unknown message in request or response")
			}
			name := strings.Title(r.GetAction())
			path = path + "/" + r.GetAction()
			opt := googapi.HttpRule_Post{Post: path}
			restopt := &googapi.HttpRule{Pattern: &opt, Body: "*"}
			reqType := "." + *f.Package + "." + r.Request
			respType := "." + *f.Package + "." + r.Response
			insertMethod(s, name, reqType, respType, "create", false, restopt)
		}
	}
}

// AddAutoGrpcEndpoints adds gRPC endpoints and types to the generation request
func AddAutoGrpcEndpoints(req *plugin.CodeGeneratorRequest) {

	msgMap := make(map[string]*descriptor.DescriptorProto)
	pkgMap := make(map[string]string)
	protoRe := regexp.MustCompile(`protos/([a-z]|[A-Z]|[0-9]|_|.)+.proto$`)
	for _, f := range req.GetProtoFile() {
		glog.V(1).Infof("Got Importsin [%s] as {%v}", *f.Name, f.GetDependency())
		if str := protoRe.FindString(f.GetName()); str != "" {
			pkgMap[str] = f.GetPackage()
		}
		for _, m := range f.MessageType {
			msgMap[*m.Name] = m
		}
	}
	glog.V(1).Infof("Got PkgMap as {%+v}", pkgMap)
	for _, files := range req.GetFileToGenerate() {
		for _, f := range req.GetProtoFile() {
			if files != *f.Name {
				continue
			}
			crudMsgMap := make(map[string]bool)
			glog.V(1).Infof("File is %s [%s]\n", *f.Name, *f.Package)
			glog.V(1).Infof("Before Mutation file is %+v", f)

			for _, m := range f.MessageType {
				msgMap[*m.Name] = m
				glog.V(1).Infof("Message [%s] - \n %+v\n\n", *m.Name, m)
				for _, fld := range m.Field {
					glog.V(1).Infof("Field: %s [%+v]\n", *fld.Name, fld)
				}
				for _, fld := range m.NestedType {
					glog.V(1).Infof("NestedField: %s [%+v]\n", *fld.Name, fld)
				}
			}
			for _, s := range f.Service {
				opts := s.GetOptions()
				glog.V(1).Infof("Opts type is [%s]\n", reflect.TypeOf(opts).Elem())
				if opts != nil {
					e, err := getExtension(opts, "venice.apiGrpcCrudService")
					if err != nil {
						glog.V(1).Infof("No CrudService extensions found %s (%s)\n", *s.Name, err)
						continue
					}
					msgs := e.([]string)

					resteps := make(map[string]map[string]string)
					rest, err := getExtension(opts, "venice.apiRestService")
					if err != nil {
						glog.V(1).Infof("No REST services defined for %s (%s)", *s.Name, err)
					} else {
						for _, r := range rest.([]*venice.RestEndpoint) {
							glog.V(1).Infof("REST endpoint- Object: %s, Methods: %v Pattern: %s", r.Object, r.Method, r.Pattern)
							if _, ok := resteps[r.Object]; !ok {
								resteps[r.Object] = make(map[string]string)
							}
							for _, meth := range r.Method {
								meth = strings.ToLower(meth)
								switch meth {
								case "put", "post", "get", "delete", "list":
								default:
									glog.Fatalf("unsupported REST verb %s", meth)
								}
								if m, ok := msgMap[r.Object]; !ok {
									// glog.Fatalf("unknown message [%s] in REST service definition", r.Object)
								} else {
									switch meth {
									case "list":
										if s, _ := isSingleton(m); s {
											glog.Fatalf("list not allowed on singleton object")
										}
										fallthrough
									case "post":
										path, err := getMessageURIPrefix(m)
										if err != nil {
											glog.Fatalf("Could not evaluate URI for [%s](%s)", r.Object, err)
										}
										resteps[r.Object][meth] = path
									case "put", "get", "delete":
										path, err := GetMessageURI(m)
										if err != nil {
											glog.Fatalf("Could not evaluate URI for [%s](%s)", r.Object, err)
										}
										resteps[r.Object][meth] = path
									}
								}
							}
						}
					}
					// Process any actions defined
					processActions(f, s, msgMap)

					for _, m := range msgs {
						if _, ok := msgMap[m]; ok {
							insertGrpcCRUD(s, m, *f.Package, resteps[m])
							crudMsgMap[m] = true
						} else {
							glog.V(1).Infof("*** Unknown Message [%s] defined for CRUD service\n", m)
						}
					}
				}
			}
			// Insert new message type for WatchEvents and List
			for v := range crudMsgMap {
				insertGrpcAutoMsgs(f, v)
			}

			// Sort the slices of interest to maintain stability in code.
			sort.Slice(f.Service, func(x, y int) bool {
				return *f.Service[x].Name < *f.Service[y].Name
			})
			sort.Slice(f.MessageType, func(x, y int) bool {
				return *f.MessageType[x].Name < *f.MessageType[y].Name
			})
			for _, s := range f.Service {
				sort.Slice(s.Method, func(x, y int) bool {
					return *s.Method[x].Name < *s.Method[y].Name
				})
			}
			glog.V(1).Infof("Mutated file is %+v", f)
		}
	}
}
