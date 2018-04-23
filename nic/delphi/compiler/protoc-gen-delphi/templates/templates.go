package templates

// templates for delphi code generation
var (
	HeaderTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _{{ .FilePrefix | ToUpper }}_OBJECTS_DELPHI_H_
#define _{{ .FilePrefix | ToUpper }}_OBJECTS_DELPHI_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "{{.FilePrefix}}.pb.h"
#include <google/protobuf/text_format.h>

namespace delphi {
namespace objects {

{{$fileName := .GetName}} {{$pkgName := .Package}} {{$msgs := .Messages}}{{range $msgs}}{{if .HasFieldType ".delphi.ObjectMeta" }} {{if not (.HasField "Key") }} {{ ThrowError "does not have Key field" $fileName .GetName }}{{end}}

class {{.GetName}};
typedef std::shared_ptr<{{.GetName}}> {{.GetName}}Ptr;
class {{.GetName}}Reactor;
typedef std::shared_ptr<{{.GetName}}Reactor> {{.GetName}}ReactorPtr;

class {{.GetName}} : public BaseObject, public {{$pkgName}}::{{.GetName}}, public enable_shared_from_this<{{.GetName}}> {
public:
    // constructor
    {{.GetName}}() {
        delphi::ObjectMeta *meta = this->mutable_meta();
        meta->set_kind(this->GetDescriptor()->name());
    }
    {{.GetName}}(const {{.GetName}}& arg) {
        this->CopyFrom(arg);
    }
    virtual BaseObjectPtr Clone() {
        return make_shared<{{.GetName}}>(*this);
    }
    {{.GetName}}(const string &data) {
        this->ParseFromString(data);
    }
    virtual delphi::ObjectMeta *GetMeta() {
        return this->mutable_meta();
    };
    {{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}} {{if (eq .GetName "Key") }} {{ if .TypeIsMessage }}
    virtual string GetKey() {
        return this->key().ShortDebugString();
    } {{else}}
    virtual string GetKey() {
        string out_str;
        const google::protobuf::FieldDescriptor *fld =  this->GetDescriptor()->FindFieldByName("Key");
        google::protobuf::TextFormat::PrintFieldValueToString(*this, fld, -1, &out_str);
        return out_str;
    }
    {{end}} {{end}} {{end}}

    virtual ::google::protobuf::Message *GetMessage() {
        return this;
    }
    static error Mount(SdkPtr sdk, MountMode mode) {
        return sdk->MountKind("{{.GetName}}", mode);
    }
    static error Watch(SdkPtr sdk, {{.GetName}}ReactorPtr reactor);
    static vector<{{.GetName}}Ptr> List(SdkPtr sdk);
    virtual error TriggerEvent(BaseObjectPtr oldObj, ObjectOperation op, ReactorListPtr rl);

    // FindObject finds the object by key
    static inline {{.GetName}}Ptr FindObject(SdkPtr sdk, {{.GetName}}Ptr objkey) {
        BaseObjectPtr base_obj = sdk->FindObject(objkey);
        {{.GetName}}Ptr obj = static_pointer_cast<{{.GetName}}>(base_obj);

        return obj;
    }
};

class {{.GetName}}Reactor : public BaseReactor {
public:
    virtual error On{{.GetName}}Create({{.GetName}}Ptr obj) {
        return error::OK();
    }
    virtual error On{{.GetName}}Delete({{.GetName}}Ptr obj) {
        return error::OK();
    }
    {{if .HasExtOption "delphi.update_event" }} {{$upd_ext := .GetExtOption "delphi.update_event"}} {{if IsTrue $upd_ext}}
    virtual error On{{ .GetName }}Update({{.GetName}}Ptr obj) {
        return error::OK();
    }
    {{end}} {{end}}
    {{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}} {{if and (eq .GetTypeName ".delphi.ObjectMeta") (ne .GetName "Meta") }} {{ ThrowError "invalid name for ObjectMeta field" $fileName $msgName .GetName }} {{end}} {{if and (ne .GetName "Meta") (ne .GetName "Key")}} {{ if .HasExtOption "delphi.event"}} {{$evt_ext := .GetExtOption "delphi.event"}} {{if IsTrue $evt_ext}}
    virtual error On{{ .GetName }}({{$msgName}}Ptr obj) {
        return error::OK();
    }
    {{end}} {{end}} {{end}} {{end}}
};

REGISTER_KIND({{.GetName}});
{{end}} {{end}}
} // namespace objects
} // namespace delphi

#endif // _{{ .FilePrefix | ToUpper }}_OBJECTS_DELPHI_H_
`

	SrcTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "{{.FilePrefix}}.delphi.hpp"

namespace delphi {
namespace objects {

{{$fileName := .GetName}} {{$msgs := .Messages}}{{range $msgs}} {{if .HasFieldType ".delphi.ObjectMeta" }}
// Watch watches a kind of objects
error {{.GetName}}::Watch(SdkPtr sdk, {{.GetName}}ReactorPtr reactor) {
    return sdk->WatchKind("{{.GetName}}", reactor);
}

// TriggerEvent trigger event based on object difference
error {{.GetName}}::TriggerEvent(BaseObjectPtr oldObj, ObjectOperation op, ReactorListPtr rl) {
    {{.GetName}}Ptr exObj = static_pointer_cast<{{.GetName}}>(oldObj);

    // loop for each reactor
    for (vector<BaseReactorPtr>::iterator iter=rl->reactors.begin(); iter!=rl->reactors.end(); ++iter) {
        {{.GetName}}ReactorPtr rctr = static_pointer_cast<{{.GetName}}Reactor>(*iter);

        // check event type
        if (op == delphi::SetOp) {
            // See if this is a new object thats created
            if (oldObj == NULL) {
                RETURN_IF_FAILED(rctr->On{{.GetName}}Create(shared_from_this()));
            } else if (this->ShortDebugString() == exObj->ShortDebugString()) {
                // if nothing changed, we are done
                return error::OK();
            } else {
                {{if .HasExtOption "delphi.update_event" }} {{$upd_ext := .GetExtOption "delphi.update_event"}} {{if IsTrue $upd_ext}}
                // Trigger update event
                RETURN_IF_FAILED(rctr->On{{ .GetName }}Update(shared_from_this()));
                {{end}} {{end}}
                // see which field changed
                {{$fields := .Fields}} {{range $fields}} {{if and (ne .GetName "Meta") (ne .GetName "Key")}} {{ if .HasExtOption "delphi.event"}} {{$evt_ext := .GetExtOption "delphi.event"}} {{if IsTrue $evt_ext}}
                {{ if .IsRepeated }}
                {
                    string new_str;
                    string old_str;
                    const google::protobuf::FieldDescriptor *nfld =  this->GetDescriptor()->FindFieldByName("{{.GetName}}");
                    google::protobuf::TextFormat::PrintFieldValueToString(*this, nfld, -1, &new_str);
                    const google::protobuf::FieldDescriptor *ofld =  exObj->GetDescriptor()->FindFieldByName("{{.GetName}}");
                    google::protobuf::TextFormat::PrintFieldValueToString(*exObj, ofld, -1, &old_str);

                    if (new_str != old_str) {
                        RETURN_IF_FAILED(rctr->On{{ .GetName }}(shared_from_this()));
                    }
                } {{ else if .TypeIsMessage }}
                if (this->{{ .GetName | ToLower }}().ShortDebugString() != exObj->{{ .GetName | ToLower }}().ShortDebugString()) {
                    RETURN_IF_FAILED(rctr->On{{ .GetName }}(shared_from_this()));
                } {{ else }}
                if (this->{{ .GetName | ToLower }}() != exObj->{{ .GetName | ToLower }}()) {
                    RETURN_IF_FAILED(rctr->On{{ .GetName }}(shared_from_this()));
                }
                {{end}} {{end}} {{end}} {{end}} {{end}}
            }
        // Handle deletes
        } else if (op == delphi::DeleteOp) {
            RETURN_IF_FAILED(rctr->On{{.GetName}}Delete(shared_from_this()));
        }
    }

    return error::OK();
}

// List returns a list of all {{.GetName}} objects in the mounted db
vector<{{.GetName}}Ptr> {{.GetName}}::List(SdkPtr sdk) {
    vector<{{.GetName}}Ptr> objlist;
    vector<BaseObjectPtr> olist = sdk->ListKind("InterfaceSpec");

    // walk all objects and convert type
    for (vector<BaseObjectPtr>::iterator oi=olist.begin(); oi!=olist.end(); ++oi) {
        {{.GetName}}Ptr obj = static_pointer_cast<{{.GetName}}>(*oi);
        objlist.push_back(obj);
    }

    return objlist;
}
{{end}} {{end}}
} // namespace objects
} // namespace delphi
`

	UtestTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "{{.FilePrefix}}.delphi.hpp"
#include "nic/delphi/sdk/delphi_utest.hpp"

{{$fileName := .GetName}} {{$msgs := .Messages}}{{range $msgs}}{{if .HasFieldType ".delphi.ObjectMeta" }}
#define {{ .GetName | ToUpper }}_REACTOR_TEST(test_name, reactor_class) \
class {{.GetName}}Service : public delphi::Service, public enable_shared_from_this<{{.GetName}}Service> { \
public: \
    {{.GetName}}Service(delphi::SdkPtr sk) { \
        this->sdk_ = sk; \
        delphi::objects::{{.GetName}}::Mount(sdk_, delphi::ReadWriteMode); \
        reactor_class##var_ = make_shared<reactor_class>(sdk_); \
        delphi::objects::{{.GetName}}::Watch(sdk_, reactor_class##var_); \
    } \
    void OnMountComplete() { \
        vector<delphi::objects::InterfaceSpecPtr> iflist = delphi::objects::InterfaceSpec::List(sdk_); \
        for (vector<delphi::objects::InterfaceSpecPtr>::iterator intf=iflist.begin(); intf!=iflist.end(); ++intf) { \
            reactor_class##var_->OnInterfaceSpecCreate(*intf); \
        } \
    } \
private: \
    std::shared_ptr<reactor_class>    reactor_class##var_; \
    delphi::SdkPtr                    sdk_; \
}; \
DELPHI_SERVICE_TEST(test_name, {{.GetName}}Service);

{{end}} {{end}}

`
)
