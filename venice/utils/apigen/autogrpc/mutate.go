package autogrpc

import (
	"errors"
	"fmt"
	"regexp"
	"sort"
	"strconv"
	"strings"

	"reflect"

	"github.com/golang/glog"

	"github.com/gogo/protobuf/gogoproto"
	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"
	plugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	googapi "github.com/pensando/grpc-gateway/third_party/googleapis/google/api"

	venice "github.com/pensando/sw/venice/utils/apigen/annotations"
	"github.com/pensando/sw/venice/utils/apigen/plugins/common"
)

const (
	customEnumStringParam = "custom_enumstr"
)

type codeInfo struct {
	comments string
	trailer  string
}

type msgSrcCodeInfo struct {
	codeInfo
	fields     map[string]codeInfo
	nestedMsgs map[string]msgSrcCodeInfo
	enums      map[string]enumSrcCodeInfo
}

type methSrcCodeInfo struct {
	codeInfo
}
type svcSrcCodeInfo struct {
	codeInfo
	methods map[string]codeInfo
}

type enumSrcCodeInfo struct {
	codeInfo
	values map[string]codeInfo
}
type srcCodeInfo struct {
	codeInfo
	msgs     map[string]msgSrcCodeInfo
	services map[string]svcSrcCodeInfo
	enums    map[string]enumSrcCodeInfo
}

func saveEnumSrcCodeInfo(file *descriptor.FileDescriptorProto, enum *descriptor.EnumDescriptorProto, path []int) enumSrcCodeInfo {
	ret := enumSrcCodeInfo{
		values: make(map[string]codeInfo),
	}
	loc, err := common.GetLocation(file.GetSourceCodeInfo(), path)
	if err != nil {
		panic(fmt.Errorf("error while saving source code info  for enum(%s)", err))
	}
	ret.comments = loc.GetLeadingComments()

	for idx, v := range enum.GetValue() {
		npath := append(path, common.EnumValType, idx)
		loc, err = common.GetLocation(file.GetSourceCodeInfo(), npath)
		if err != nil {
			panic(fmt.Errorf("error while saving source code info  for enum(%s) file [%s] enum[%s]", err, file.GetName(), enum.GetName()))
		}
		ret.values[v.GetName()] = codeInfo{comments: loc.GetLeadingComments(), trailer: loc.GetTrailingComments()}
	}
	return ret
}

func saveMsgSrcCodeInfo(file *descriptor.FileDescriptorProto, msg *descriptor.DescriptorProto, path []int) msgSrcCodeInfo {
	m := msgSrcCodeInfo{
		fields:     make(map[string]codeInfo),
		nestedMsgs: make(map[string]msgSrcCodeInfo),
		enums:      make(map[string]enumSrcCodeInfo),
	}
	loc, err := common.GetLocation(file.GetSourceCodeInfo(), path)
	if err != nil {
		// Just log a message here, since there can be elements like map_entry that dont have locs
		glog.V(1).Infof("error while saving source code info  for msg(%s) file [%s] msg[%s]", err, file.GetName(), msg.GetName())
	}
	if loc != nil {
		m.comments = loc.GetLeadingComments()
	}
	for fidx, fld := range msg.GetField() {
		fpath := append(path, []int{common.FieldType, fidx}...)
		loc, err = common.GetLocation(file.GetSourceCodeInfo(), fpath)
		if err != nil {
			glog.V(1).Infof("error while saving source code info for fld (%s)", err)
		}
		if loc != nil {
			m.fields[fld.GetName()] = codeInfo{comments: loc.GetLeadingComments()}
		} else {
			m.fields[fld.GetName()] = codeInfo{}
		}
	}
	// Save Nested Messages
	for idx, nmsg := range msg.GetNestedType() {
		npath := append(path, []int{common.NestedMsgType, idx}...)
		m.nestedMsgs[nmsg.GetName()] = saveMsgSrcCodeInfo(file, nmsg, npath)
	}

	// Save nested enums
	for idx, enum := range msg.GetEnumType() {
		npath := append(path, common.NestedEnumType, idx)
		m.enums[enum.GetName()] = saveEnumSrcCodeInfo(file, enum, npath)
	}
	return m
}

func saveSrcCodeInfo(file *descriptor.FileDescriptorProto) srcCodeInfo {
	ret := srcCodeInfo{
		msgs:     make(map[string]msgSrcCodeInfo),
		services: make(map[string]svcSrcCodeInfo),
		enums:    make(map[string]enumSrcCodeInfo),
	}
	if file.SourceCodeInfo == nil {
		return ret
	}
	// Messages
	for idx, msg := range file.MessageType {
		path := []int{common.MsgType, idx}
		ret.msgs[msg.GetName()] = saveMsgSrcCodeInfo(file, msg, path)
	}
	//Services
	for idx, svc := range file.Service {
		s := svcSrcCodeInfo{
			methods: make(map[string]codeInfo),
		}
		loc, err := common.GetLocation(file.GetSourceCodeInfo(), []int{common.SvcType, idx})
		if err != nil {
			panic(fmt.Errorf("error while saving source code info  for msg(%s)", err))
		}
		if loc != nil {
			s.comments = loc.GetLeadingComments()
		}
		for midx, meth := range svc.GetMethod() {
			loc, err = common.GetLocation(file.GetSourceCodeInfo(), []int{common.SvcType, idx, 2, midx})
			if err != nil {
				panic(fmt.Errorf("error while saving source code info for method (%s)", err))
			}
			if loc != nil {
				s.methods[meth.GetName()] = codeInfo{comments: loc.GetLeadingComments()}
			} else {
				s.methods[meth.GetName()] = codeInfo{}
			}
		}
		ret.services[svc.GetName()] = s
	}

	// Enums
	for idx, enum := range file.EnumType {
		path := []int{common.EnumType, idx}
		ret.enums[enum.GetName()] = saveEnumSrcCodeInfo(file, enum, path)
	}
	glog.V(1).Infof("Save Source code Info : [%+v]", ret)
	return ret
}

func restoreMsgSrcCodeInfo(f *descriptor.FileDescriptorProto, msg *descriptor.DescriptorProto, savedSci *srcCodeInfo, newSci *descriptor.SourceCodeInfo, path []int32) {
	cmsg, ok := savedSci.msgs[msg.GetName()]
	if ok {
		newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
			Path:            path,
			LeadingComments: &cmsg.comments,
		})
	}
	for fidx, fld := range msg.Field {
		cfld, ok := cmsg.fields[fld.GetName()]
		if ok {
			fldpath := append(path, common.FieldType, int32(fidx))
			newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
				Path:            fldpath,
				LeadingComments: &cfld.comments,
			})
		}
	}
	// Nested Enums
	for idx, enum := range msg.EnumType {
		cenum, ok := cmsg.enums[enum.GetName()]
		if ok {
			enumpath := append(path, common.NestedEnumType, int32(idx))
			newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
				Path:            enumpath,
				LeadingComments: &cenum.comments,
			})
			for id, v := range enum.Value {
				cval, ok := cenum.values[v.GetName()]
				if ok {
					valpath := append(enumpath, common.EnumValType, int32(id))
					newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
						Path:             valpath,
						LeadingComments:  &cval.comments,
						TrailingComments: &cval.trailer,
					})
				}
			}
		}
	}
	// Nested Messages
	for idx, nmsg := range msg.NestedType {
		npath := append(path, common.NestedMsgType, int32(idx))
		restoreMsgSrcCodeInfo(f, nmsg, savedSci, newSci, npath)
	}
}

func restoreScrCodeInfo(f *descriptor.FileDescriptorProto, savedSci srcCodeInfo) {
	newSci := &descriptor.SourceCodeInfo{}
	// Save Package comments
	{
		loc, err := common.GetLocation(f.SourceCodeInfo, []int{common.PackageType})
		if err == nil {
			newSci.Location = append(newSci.Location, loc)
		}
	}
	for idx, msg := range f.GetMessageType() {
		path := []int32{common.MsgType, int32(idx)}
		restoreMsgSrcCodeInfo(f, msg, &savedSci, newSci, path)
	}
	// services
	for idx, svc := range f.GetService() {
		cmsg, ok := savedSci.services[svc.GetName()]
		if ok {
			newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
				Path:            []int32{common.SvcType, int32(idx)},
				LeadingComments: &cmsg.comments,
			})
			for midx, meth := range svc.GetMethod() {
				cmtd, ok := cmsg.methods[meth.GetName()]
				if ok {
					newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
						Path:            []int32{common.SvcType, int32(idx), common.MethType, int32(midx)},
						LeadingComments: &cmtd.comments,
					})
				}
			}
		}
	}
	for idx, enum := range f.GetEnumType() {
		cenum, ok := savedSci.enums[enum.GetName()]
		if ok {
			newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
				Path:            []int32{common.EnumType, int32(idx)},
				LeadingComments: &cenum.comments,
			})
			for vidx, val := range enum.Value {
				cval, ok := cenum.values[val.GetName()]
				if ok {
					newSci.Location = append(newSci.Location, &descriptor.SourceCodeInfo_Location{
						Path:            []int32{common.EnumType, int32(idx), common.EnumValType, int32(vidx)},
						LeadingComments: &cval.comments,
					})
				}
			}
		}
	}
	f.SourceCodeInfo = newSci
}

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

func fixupJSONTag(file *descriptor.FileDescriptorProto) {
	for _, msg := range file.GetMessageType() {
		for _, f := range msg.GetField() {
			if tag := gogoproto.GetJsonTag(f); tag != nil {
				f.JsonName = tag
			}
		}
	}
}

func insertMethod(svc *descriptor.ServiceDescriptorProto, name, intype, outtype, oper string, watch, defTenant bool, action string, restopt *googapi.HttpRule) error {
	operDesc, err := getExtensionDesc(&descriptor.MethodOptions{}, "venice.methodOper")
	if err != nil {
		glog.V(1).Infof("Get methodOper desc failed (%s)", err)
		return err
	}
	defTenantDesc, err := getExtensionDesc(&descriptor.MethodOptions{}, "venice.methodTenantDefault")
	if err != nil {
		glog.V(1).Infof("Get methodTenantDefault desc failed (%s)", err)
		return err
	}
	autoDesc, err := getExtensionDesc(&descriptor.MethodOptions{}, "venice.methodAutoGen")
	if err != nil {
		glog.V(1).Infof("Get methodAutoGen desc failed (%s)", err)
		return err
	}
	httpOpt, err := getExtensionDesc(&descriptor.MethodOptions{}, "google.api.http")
	if err != nil {
		glog.V(1).Infof("Get google.api.http desc failed (%s)", err)
		return err
	}
	actOpt, err := getExtensionDesc(&descriptor.MethodOptions{}, "venice.methodActionObject")
	if err != nil {
		glog.V(1).Infof("Get venice.methodActionObject desc failed (%s)", err)
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
		glog.V(1).Infof("Failed to set Extension (%s)", err)
		return err
	}
	if err = proto.SetExtension(m.GetOptions(), defTenantDesc, &defTenant); err != nil {
		glog.V(1).Infof("Failed to set Extension (%s)", err)
		return err
	}
	if err = proto.SetExtension(m.GetOptions(), autoDesc, &autogen); err != nil {
		glog.V(1).Infof("Failed to set Extension (%s)", err)
		return err
	}
	if action != "" {
		if err = proto.SetExtension(m.GetOptions(), actOpt, &action); err != nil {
			glog.V(1).Infof("Failed to set Extension (%s)", err)
			return err
		}
	}
	if restopt != nil {
		if err = proto.SetExtension(m.GetOptions(), httpOpt, restopt); err != nil {
			glog.V(1).Infof("Failed to set Rest Extension (%s)", err)
		}
	}
	svc.Method = append(svc.Method, &m)
	return nil
}

func insertGrpcCRUD(svc *descriptor.ServiceDescriptorProto, sci *srcCodeInfo, m, pkg string, resteps map[string]string) {
	var restopt *googapi.HttpRule

	glog.V(1).Infof("adding Grpc CRUD endpoints for %s [ %+v ]", m, resteps)
	// Add method
	var defTenant bool
	if v, ok := resteps["post"]; ok {
		opt := googapi.HttpRule_Post{Post: v}
		restopt = &googapi.HttpRule{Pattern: &opt, Body: "*"}
		if v1, ok := resteps["post_defTenant"]; ok {
			dopt := googapi.HttpRule_Post{Post: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt, Body: "*"})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoAdd%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"create", false, defTenant, "", restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoAdd%s", m)] = codeInfo{
		comments: fmt.Sprintf("Create %s object", m),
	}
	// Label method
	if v, ok := resteps["label"]; ok {
		opt := googapi.HttpRule_Post{Post: v}
		restopt = &googapi.HttpRule{Pattern: &opt, Body: "*"}
		if v1, ok := resteps["label_defTenant"]; ok {
			dopt := googapi.HttpRule_Post{Post: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt, Body: "*"})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoLabel%s", m),
		".api.Label",
		fmt.Sprintf(".%s.%s", pkg, m),
		"label", false, defTenant, m, restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoLabel%s", m)] = codeInfo{
		comments: fmt.Sprintf("Label %s object", m),
	}
	// Update method
	if v, ok := resteps["put"]; ok {
		opt := googapi.HttpRule_Put{Put: v}
		restopt = &googapi.HttpRule{Pattern: &opt, Body: "*"}
		if v1, ok := resteps["put_defTenant"]; ok {
			dopt := googapi.HttpRule_Put{Put: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt, Body: "*"})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoUpdate%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"update", false, defTenant, "", restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoUpdate%s", m)] = codeInfo{
		comments: fmt.Sprintf("Update %s object", m),
	}
	// Get method
	if v, ok := resteps["get"]; ok {
		opt := googapi.HttpRule_Get{Get: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
		if v1, ok := resteps["get_defTenant"]; ok {
			dopt := googapi.HttpRule_Get{Get: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoGet%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"get", false, defTenant, "", restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoGet%s", m)] = codeInfo{
		comments: fmt.Sprintf("Get %s object", m),
	}
	// Delete method
	if v, ok := resteps["delete"]; ok {
		opt := googapi.HttpRule_Delete{Delete: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
		if v1, ok := resteps["delete_defTenant"]; ok {
			dopt := googapi.HttpRule_Delete{Delete: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoDelete%s", m),
		fmt.Sprintf(".%s.%s", pkg, m),
		fmt.Sprintf(".%s.%s", pkg, m),
		"delete", false, defTenant, "", restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoDelete%s", m)] = codeInfo{
		comments: fmt.Sprintf("Delete %s object", m),
	}
	// List method
	if v, ok := resteps["list"]; ok {
		opt := googapi.HttpRule_Get{Get: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
		if v1, ok := resteps["list_defTenant"]; ok {
			dopt := googapi.HttpRule_Get{Get: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoList%s", m),
		".api.ListWatchOptions",
		fmt.Sprintf(".%s.%sList", pkg, m),
		"list", false, defTenant, "", restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoList%s", m)] = codeInfo{
		comments: fmt.Sprintf("List %s objects", m),
	}

	// Watch method
	if v, ok := resteps["watch"]; ok {
		opt := googapi.HttpRule_Get{Get: v}
		restopt = &googapi.HttpRule{Pattern: &opt}
		if v1, ok := resteps["watch_defTenant"]; ok {
			dopt := googapi.HttpRule_Get{Get: v1}
			restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt})
			defTenant = true
		}
	} else {
		restopt = nil
	}
	insertMethod(svc,
		fmt.Sprintf("AutoWatch%s", m),
		".api.ListWatchOptions",
		fmt.Sprintf(".%s.AutoMsg%sWatchHelper", pkg, m),
		"watch", true, defTenant, "", restopt)
	defTenant = false
	sci.services[svc.GetName()].methods[fmt.Sprintf("AutoWatch%s", m)] = codeInfo{
		comments: fmt.Sprintf("Watch %s objects. Supports WebSockets or HTTP long poll", m),
	}
	glog.V(1).Infof("Generated AutoGrpc for [%s][%s]\n", *svc.Name, m)
}

func insertGrpcAutoMsgs(f *descriptor.FileDescriptorProto, sci *srcCodeInfo, msg string) {
	autoDesc, err := getExtensionDesc(&descriptor.MessageOptions{}, "venice.objectAutoGen")
	if err != nil {
		glog.V(1).Infof("Get objectAutoGen desc failed (%s)\n", err)
		return
	}
	embedDesc, err := getExtensionDesc(&descriptor.FieldOptions{}, "gogoproto.embed")
	if err != nil {
		glog.V(1).Infof("Get gogoproto.embed desc failed (%s)\n", err)
		return
	}
	nullDesc, err := getExtensionDesc(&descriptor.FieldOptions{}, "gogoproto.nullable")
	if err != nil {
		glog.V(1).Infof("Get gogoproto.nullable desc failed (%s)\n", err)
		return
	}
	jsontagDesc, err := getExtensionDesc(&descriptor.FieldOptions{}, "gogoproto.jsontag")
	if err != nil {
		glog.V(1).Infof("Get gogoproto.jsontag desc failed (%s)\n", err)
		return
	}
	// Watch helper message
	{
		name := fmt.Sprintf("AutoMsg%sWatchHelper", msg)
		nestedName := "WatchEvent"
		autoption := "watchhelper"

		mtype := fmt.Sprintf(".%s.%s", *f.Package, msg)
		nfldname1 := "Type"
		nfldname2 := "Object"

		nfldjsontag1 := "type,omitempty"
		nfldjsontag2 := "object,omitempty"
		var nfldnum1 int32 = 1
		var nfldnum2 int32 = 2

		var nfldlabel1 = descriptor.FieldDescriptorProto_LABEL_OPTIONAL
		var nfldlabel2 = descriptor.FieldDescriptorProto_LABEL_OPTIONAL

		var nfldtype1 = descriptor.FieldDescriptorProto_TYPE_STRING
		var nfldtype2 = descriptor.FieldDescriptorProto_TYPE_MESSAGE

		sci.msgs[name] = msgSrcCodeInfo{
			codeInfo: codeInfo{comments: fmt.Sprintf("%s is a wrapper object for watch events for %s objects", name, msg)},
			fields: map[string]codeInfo{
				nfldname1: codeInfo{
					comments: "Type of watch event",
				},
				nfldname2: codeInfo{
					comments: "Changed object details",
				},
			},
		}

		nfield1 := descriptor.FieldDescriptorProto{
			Name:    &nfldname1,
			Number:  &nfldnum1,
			Label:   &nfldlabel1,
			Type:    &nfldtype1,
			Options: &descriptor.FieldOptions{},
		}
		proto.SetExtension(nfield1.GetOptions(), jsontagDesc, &nfldjsontag1)
		nfield2 := descriptor.FieldDescriptorProto{
			Name:     &nfldname2,
			Number:   &nfldnum2,
			Label:    &nfldlabel2,
			Type:     &nfldtype2,
			TypeName: &mtype,
			Options:  &descriptor.FieldOptions{},
		}
		proto.SetExtension(nfield2.GetOptions(), jsontagDesc, &nfldjsontag2)
		var nestedFields []*descriptor.FieldDescriptorProto
		nestedFields = append(nestedFields, &nfield1)
		nestedFields = append(nestedFields, &nfield2)

		nestedMsg := descriptor.DescriptorProto{
			Name:  &nestedName,
			Field: nestedFields,
		}
		nestedMsgs := []*descriptor.DescriptorProto{
			&nestedMsg,
		}
		var fldname1 = "Events"
		var fldjsontag1 = "events"
		var fldnum1 int32 = 1
		var fldlabel1 = descriptor.FieldDescriptorProto_LABEL_REPEATED
		var fldtype2 = descriptor.FieldDescriptorProto_TYPE_MESSAGE
		fldtype1 := fmt.Sprintf(".%s.%s.WatchEvent", *f.Package, name)
		field1 := descriptor.FieldDescriptorProto{
			Name:     &fldname1,
			Number:   &fldnum1,
			Label:    &fldlabel1,
			Type:     &fldtype2,
			TypeName: &fldtype1,
			Options:  &descriptor.FieldOptions{},
		}
		proto.SetExtension(field1.GetOptions(), jsontagDesc, &fldjsontag1)
		fields := []*descriptor.FieldDescriptorProto{
			&field1,
		}
		msg := descriptor.DescriptorProto{
			Name:       &name,
			Field:      fields,
			NestedType: nestedMsgs,
			Options:    &descriptor.MessageOptions{},
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
		fldjsontag2 := ",inline"
		fldjsontag3 := "list-meta,inline"
		fldjsontag4 := "items"
		sci.msgs[name] = msgSrcCodeInfo{
			codeInfo: codeInfo{comments: fmt.Sprintf("%s is a container object for list of %s objects", name, msg)},
			fields: map[string]codeInfo{
				fldname4: codeInfo{
					comments: fmt.Sprintf("List of %s objects", msg),
				},
			},
		}
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
		proto.SetExtension(field2.GetOptions(), jsontagDesc, &fldjsontag2)

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
		proto.SetExtension(field3.GetOptions(), jsontagDesc, &fldjsontag3)

		field4 := descriptor.FieldDescriptorProto{
			Name:     &fldname4,
			Number:   &fldnum4,
			Label:    &fldlabel4,
			Type:     &fldtype4,
			TypeName: &mtype,
			Options:  &descriptor.FieldOptions{},
		}
		proto.SetExtension(field4.GetOptions(), jsontagDesc, &fldjsontag4)

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
	path = strings.Replace(path, "{O.Tenant}", "tenant/{O.Tenant}", 1)
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

func processActions(f *descriptor.FileDescriptorProto, s *descriptor.ServiceDescriptorProto, sci *srcCodeInfo, msgMap map[string]*descriptor.DescriptorProto) {
	opts := s.GetOptions()
	act, err := getExtension(opts, "venice.apiAction")
	if err != nil {
		glog.V(1).Infof("No action endpoints defined for %s (%s)", *s.Name, err)
	} else {
		for _, r := range act.([]*venice.ActionEndpoint) {
			path := ""
			targetObj := ""
			if t := r.GetCollection(); t != "" {
				if _, ok := msgMap[t]; !ok {
					glog.Fatalf("unknown message [%s] in action definition", t)
				}
				path, err = getMessageURIPrefix(msgMap[t])
				if err != nil {
					glog.Fatalf("could not get message URI prefix for collection [%s]", t)
				}
				targetObj = t
			} else if t := r.GetObject(); t != "" {
				if _, ok := msgMap[t]; !ok {
					glog.Fatalf("unknown message [%s] in action definition", t)
				}
				path, err = GetMessageURI(msgMap[t])
				if err != nil {
					glog.Fatalf("could not get message URI for target [%s]", t)
				}
				targetObj = t
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
			defTenant := false
			if strings.Contains(path, "tenant/{O.Tenant}") {
				npath := strings.Replace(path, "tenant/{O.Tenant}/", "", 1)
				dopt := googapi.HttpRule_Post{Post: npath}
				restopt.AdditionalBindings = append(restopt.AdditionalBindings, &googapi.HttpRule{Pattern: &dopt, Body: "*"})
				defTenant = true
			}
			insertMethod(s, name, reqType, respType, "create", false, defTenant, targetObj, restopt)
			sci.services[s.GetName()].methods[name] = codeInfo{
				comments: r.Desc,
			}
		}
	}
}

func addServiceWatcherMsg(f *descriptor.FileDescriptorProto, s *descriptor.ServiceDescriptorProto, msgMap map[string]*descriptor.DescriptorProto) {
	insertMethod(s,
		fmt.Sprintf("AutoWatchSvc%s", *s.Name),
		".api.ListWatchOptions",
		".api.WatchEventList",
		"watch", true, false, "", nil)
}

func insertFileDefaults(params map[string]string, file *descriptor.FileDescriptorProto) error {
	v, ok := params[customEnumStringParam]
	if !ok {
		return nil
	}
	if val, err := strconv.ParseBool(v); err != nil || !val {
		return nil
	}

	// skip generating String function for Enums, since we will be generating those ourselves
	enumStringOpt, err := getExtensionDesc(file.GetOptions(), "gogoproto.goproto_enum_stringer_all")
	if err != nil {
		glog.V(1).Infof("Get gogoproto.goproto_enum_stringer_all desc failed (%s)\n", err)
		return fmt.Errorf("failed to the get extension Descriptor(%s)", err)
	}
	enableStringer := false
	if file.GetOptions() == nil {
		file.Options = &descriptor.FileOptions{}
	}
	err = proto.SetExtension(file.GetOptions(), enumStringOpt, &enableStringer)
	if err != nil {
		return fmt.Errorf("failed to set extension (%s)", err)
	}
	return nil
}

func parseReqParam(param string, pkgMap map[string]string) error {
	if param == "" {
		return nil
	}
	for _, p := range strings.Split(param, ",") {
		spec := strings.SplitN(p, "=", 2)
		if len(spec) == 2 {
			name, value := spec[0], spec[1]
			pkgMap[name] = value
		}
	}
	return nil
}

// AddAutoGrpcEndpoints adds gRPC endpoints and types to the generation request
func AddAutoGrpcEndpoints(req *plugin.CodeGeneratorRequest) {
	msgMap := make(map[string]*descriptor.DescriptorProto)
	pkgMap := make(map[string]string)
	params := make(map[string]string)
	if req.Parameter != nil {
		err := parseReqParam(req.GetParameter(), params)
		if err != nil {
			glog.Fatalf("Error parsing flags: %v", err)
		}
	}

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

	apiMetaImport := "github.com/pensando/sw/api/meta.proto"
	glog.V(1).Infof("Got PkgMap as {%+v}", pkgMap)
	for _, files := range req.GetFileToGenerate() {
		for _, f := range req.GetProtoFile() {
			var savedSci srcCodeInfo
			if files != *f.Name {
				continue
			}
			// Before mutating save SourceCodeInfo paths and indices
			savedSci = saveSrcCodeInfo(f)
			// Add api/meta.prot if it is not in dependencies
			depFound := false

			// get all the filtered imports
			filterImports := []string{}
			opts := f.GetOptions()
			if opts != nil {
				fi, err := getExtension(opts, "venice.filterImport")
				if err == nil {
					filterImports = append(filterImports, fi.([]string)...)
				}
			}

			finalDeps := []string{}
			for _, d := range f.GetDependency() {
				filter := false
				if d == apiMetaImport {
					depFound = true
				}
				for _, fi := range filterImports {
					if fi == d {
						filter = true
						break
					}
				}
				if !filter {
					finalDeps = append(finalDeps, d)
				}
			}

			// Insert File level options
			err := insertFileDefaults(params, f)
			if err != nil {
				glog.Fatalf("failed to insert file options(%s)", err)
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
						// Still add the Svc Watch Method
						addServiceWatcherMsg(f, s, msgMap)
						continue
					}
					msgs := e.([]string)
					if _, ok := savedSci.services[s.GetName()]; !ok {
						savedSci.services[s.GetName()] = svcSrcCodeInfo{
							methods: make(map[string]codeInfo),
						}
					}
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
								case "put", "post", "get", "delete", "list", "watch", "label":
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
										if strings.Contains(path, "tenant/{O.Tenant}") {
											npath := strings.Replace(path, "tenant/{O.Tenant}/", "", 1)
											resteps[r.Object][meth+"_defTenant"] = npath
										}
									case "watch":
										path, err := getMessageURIPrefix(m)
										if err != nil {
											glog.Fatalf("Could not evaluate URI for [%s](%s)", r.Object, err)
										}
										resteps[r.Object][meth] = "/watch" + path
										if strings.Contains(path, "tenant/{O.Tenant}") {
											npath := "/watch" + strings.Replace(path, "tenant/{O.Tenant}/", "", 1)
											resteps[r.Object][meth+"_defTenant"] = npath
										}
									case "put", "get", "delete":
										path, err := GetMessageURI(m)
										if err != nil {
											glog.Fatalf("Could not evaluate URI for [%s](%s)", r.Object, err)
										}
										resteps[r.Object][meth] = path
										if strings.Contains(path, "tenant/{O.Tenant}") {
											npath := strings.Replace(path, "tenant/{O.Tenant}/", "", 1)
											resteps[r.Object][meth+"_defTenant"] = npath
										}
									case "label":
										path, err := GetMessageURI(m)
										path = path + "/label"
										if err != nil {
											glog.Fatalf("Could not evaluate URI for [%s](%s)", r.Object, err)
										}
										resteps[r.Object][meth] = path
										var npath string
										if strings.Contains(path, "tenant/{O.Tenant}") {
											npath = strings.Replace(path, "tenant/{O.Tenant}/", "", 1)
											resteps[r.Object][meth+"_defTenant"] = npath
										}
									}
								}
							}
						}
					}
					// Add Watch helper for the service Watch
					addServiceWatcherMsg(f, s, msgMap)
					// Process any actions defined
					processActions(f, s, &savedSci, msgMap)

					for _, m := range msgs {
						if _, ok := msgMap[m]; ok {
							insertGrpcCRUD(s, &savedSci, m, *f.Package, resteps[m])
							crudMsgMap[m] = true
						} else {
							glog.V(1).Infof("*** Unknown Message [%s] defined for CRUD service\n", m)
						}
					}
				}
			}

			if !depFound && (len(crudMsgMap) > 0 || len(f.Service) > 0) {
				finalDeps = append(finalDeps, apiMetaImport)
			}
			f.Dependency = finalDeps

			// Insert new message type for WatchEvents and List
			for v := range crudMsgMap {
				insertGrpcAutoMsgs(f, &savedSci, v)
			}

			// Fixup JSON tags in proto def. Works around a gogoproto bug that does not updated the proto tags correctly.
			fixupJSONTag(f)

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
			// Restore the source code info
			restoreScrCodeInfo(f, savedSci)

			glog.V(1).Infof("Mutated file is %+v", f)
		}
	}
}
