package templates

// templates for delphi code generation
var (
	HeaderTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _{{ .FilePrefix | ToUpper }}_OBJECTS_DELPHI_H_
#define _{{ .FilePrefix | ToUpper }}_OBJECTS_DELPHI_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/shm/delphi_metrics.hpp"
#include "{{.FilePrefix}}.pb.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>
#include "nic/sdk/lib/pal/pal.hpp"

namespace delphi {
namespace objects {

{{$fileName := .GetName}}
{{$pkgName := .Package}}
{{$msgs := .Messages}}
{{range $msgs}}
  {{$msgName := .GetName}}
  {{$msg := .}}
  {{if not (HasSuffix .GetName "Metrics")}}
    {{if .HasFieldType ".delphi.ObjectMeta" }}
      {{if .HasField "Key" }}
        {{if or (.HasField "key_or_handle") (.HasExtOption "delphi.singleton")}} {{ ThrowError "multiple key fields or singleton" $fileName .GetName }}
        {{end}}
      {{else if .HasField "key_or_handle"}}
        {{if or (.HasField "Key") (.HasExtOption "delphi.singleton")}} {{ ThrowError "multiple key fields or singleton" $fileName .GetName }}
        {{end}}
      {{else if .HasExtOption "delphi.singleton" }}
        {{if or (.HasField "key_or_handle") (.HasField "key_or_handle")}} {{ ThrowError "multiple key fields or singleton" $fileName .GetName }}
        {{end}}
      {{else}} {{ ThrowError "does not have Key field" $fileName .GetName }}
      {{end}}

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
	{{.GetName}}(const {{$pkgName}}::{{.GetName}}& arg) : {{$pkgName}}::{{.GetName}}(arg) {
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
      {{if .HasExtOption "delphi.singleton" }}
    virtual string GetKey() {
        return "default";
    }
      {{end}}
      {{$fields := .Fields}}
      {{range $fields}}
        {{if (eq .GetName "Key") }}
          {{if .IsRepeated }} {{ ThrowError "Key field can not be repeated" $fileName $msgName .GetName }}
          {{end}}
          {{ if .TypeIsMessage }}
    virtual string GetKey() {
        return this->key().ShortDebugString();
    }
          {{else}}
    virtual string GetKey() {
        string out_str;
        const google::protobuf::FieldDescriptor *fld =  this->GetDescriptor()->FindFieldByName("Key");
        google::protobuf::TextFormat::PrintFieldValueToString(*this, fld, -1, &out_str);
        return out_str;
    }
          {{end}}
        {{end}}
        {{if (eq .GetName "key_or_handle") }}
          {{ if .TypeIsMessage }}
      virtual string GetKey() {
          return this->key_or_handle().ShortDebugString();
      }
          {{else}} {{ ThrowError "does not have Key field" $fileName $msgName .GetName }}
          {{end}}
        {{end}}
      {{end}}

    virtual ::google::protobuf::Message *GetMessage() {
        return this;
    }
    static error Mount(SdkPtr sdk, MountMode mode) {
        return sdk->MountKind("{{.GetName}}", mode);
    }
    static error MountKey(SdkPtr sdk, {{.GetName}}Ptr objkey, MountMode mode) {
        return sdk->MountKey("{{.GetName}}", objkey->GetKey(), mode);
    }
    static error Watch(SdkPtr sdk, {{.GetName}}ReactorPtr reactor);
    static vector<{{.GetName}}Ptr> List(SdkPtr sdk);
    virtual error TriggerEvent(BaseObjectPtr oldObj, ObjectOperation op, ReactorListPtr rl);

      {{if .HasExtOption "delphi.singleton" }}
    // FindObject finds the object for singletons
    static inline {{.GetName}}Ptr FindObject(SdkPtr sdk) {
        {{.GetName}}Ptr objkey = make_shared<{{.GetName}}>();
        BaseObjectPtr base_obj = sdk->FindObject(objkey);
        {{.GetName}}Ptr obj = static_pointer_cast<{{.GetName}}>(base_obj);

        return obj;
    }
      {{else}}
    // FindObject finds the object by key
    static inline {{.GetName}}Ptr FindObject(SdkPtr sdk, {{.GetName}}Ptr objkey) {
        BaseObjectPtr base_obj = sdk->FindObject(objkey);
        {{.GetName}}Ptr obj = static_pointer_cast<{{.GetName}}>(base_obj);

        return obj;
    }
      {{end}}

    virtual bool IsPersistent() {
      {{if .HasExtOption "delphi.persist" }}
        return true;
      {{else}}
        return false;
      {{end}}
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
      {{if .HasExtOption "delphi.update_event" }} {{$upd_ext := .GetExtOption "delphi.update_event"}}
        {{if IsTrue $upd_ext}}
    virtual error On{{ .GetName }}Update({{.GetName}}Ptr obj) {
        return error::OK();
    }
        {{end}} 
      {{end}}
      {{$fields := .Fields}}
      {{range $fields}}
        {{if and (eq .GetTypeName ".delphi.ObjectMeta") (ne .GetName "Meta") }} {{ ThrowError "invalid name for ObjectMeta field" $fileName $msgName .GetName }}
        {{end}}
        {{if and (ne .GetName "Meta") (ne .GetName "Key")}}
          {{ if .HasExtOption "delphi.event"}} {{$evt_ext := .GetExtOption "delphi.event"}}
            {{if IsTrue $evt_ext}}
    virtual error On{{ .GetName }}({{$msgName}}Ptr obj) {
        return error::OK();
    }
            {{end}}
          {{end}}
        {{end}}
      {{end}}
};

REGISTER_KIND({{.GetName}});
    {{end}}
  {{else}}
    {{if not (.HasField "Key")}} 
      {{if not (.HasExtOption "delphi.singleton")}} {{ ThrowError "metrics does not have Key field" $fileName .GetName }}
      {{end}}
    {{end}}
    {{if .HasField "Key" }}
      {{if (.HasExtOption "delphi.singleton")}} {{ ThrowError "multiple key fields or singleton" $fileName .GetName }}
      {{end}}
    {{end}}

// forward declaration
class {{.GetName}};
typedef std::shared_ptr<{{.GetName}}> {{.GetName}}Ptr;
class {{.GetName}}Iterator;

// {{.GetName | ToLower}}_t: c-struct for {{.GetName}}
typedef struct {{.GetName | ToLower}}_ {
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetTypeName ".delphi.Counter") }}
    uint64_t    {{.GetName}};
      {{else if (eq .GetTypeName ".delphi.Gauge") }}
    double      {{.GetName}};
      {{end}}
    {{end}}
} {{.GetName | ToLower}}_t;

// {{.GetName}} class
class {{.GetName}} : public delphi::metrics::DelphiMetrics {
private:
    char                          *shm_ptr_;
    uint64_t                      pal_addr_;
    {{if (.HasExtOption "delphi.singleton")}}
	uint32_t                            key_ = 0;
    {{end}}
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
        {{if .IsRepeated }} {{ ThrowError "Key field can not be repeated" $fileName $msgName .GetName }}
        {{end}}
        {{ if .TypeIsMessage }}
          {{if or (eq .GetTypeName ".delphi.Counter") (eq .GetTypeName ".delphi.Gauge") }} {{ ThrowError "Key field type can not be counter or gauge" $fileName $msgName .GetName }} 
          {{end}}
    {{$pkgName}}::{{.GetCppTypeName}}      key_;
        {{else}}
    {{.GetCppTypeName}}                       key_;
        {{end}}
      {{else}}
        {{if (eq .GetTypeName ".delphi.Counter") }}
    delphi::metrics::CounterPtr   {{.GetName}}_;
        {{else if (eq .GetTypeName ".delphi.Gauge") }}
    delphi::metrics::GaugePtr     {{.GetName}}_;
        {{else if (eq .GetTypeName ".delphi.ObjectMeta") }}
    delphi::ObjectMeta            {{.GetName}}_;
        {{else}} {{ ThrowError "Invalid field type for" $fileName $msgName .GetName .GetCppTypeName }}
        {{end}}
      {{end}}
    {{end}}

public:
    static int32_t Size();
    delphi::error Delete();
    string DebugString();
    string JSONString();
    static {{.GetName}}Iterator Iterator();
    void * Raw() { return shm_ptr_; };
    uint64_t GetPalAddr() { return pal_addr_; };
	static delphi::error  CreateTable();

    {{if (.HasExtOption "delphi.singleton")}}
	{{$msgName}}(char *ptr);
        {{$msgName}}(char *valptr, uint64_t pal_addr);
	{{$msgName}}(char *kptr, char *valptr) : {{$msgName}}(valptr){ };
	{{$msgName}}(char *kptr, char *valptr, uint64_t pal_addr) : {{$msgName}}(valptr, pal_addr){ };
    uint32_t GetKey() { return 0; };
      {{if $msg.HasExtOption "delphi.datapath_metrics" }}
    static {{$msgName}}Ptr New{{$msgName}}(uint64_t pal_addr);
      {{else}}
    static {{$msgName}}Ptr New{{$msgName}}();
      {{end}}
	static delphi::error Publish({{$msgName | ToLower}}_t *mptr);
	static {{$msgName}}Ptr Find();
        static void Release({{$msgName}}Ptr ptr);
    {{end}}
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
        {{ if .TypeIsMessage }}
    {{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key, char *valptr);
    {{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key, char *valptr, uint64_t pal_addr);
    inline {{$pkgName}}::{{.GetCppTypeName}} get{{.GetCppTypeName}}FromPtr(char *kptr) {
        string keystr(kptr);
        {{$pkgName}}::{{.GetCppTypeName}} key;
        key.ParseFromString(keystr);
        return key;
    }
    {{$msgName}}(char *kptr, char *valptr) : {{$msgName}}(get{{.GetCppTypeName}}FromPtr(kptr), valptr){ };
    {{$msgName}}(char *kptr, char *valptr, uint64_t pal_addr) : {{$msgName}}(get{{.GetCppTypeName}}FromPtr(kptr), valptr, pal_addr){ };
    {{$pkgName}}::{{.GetCppTypeName}} GetKey() { return key_; };
          {{if $msg.HasExtOption "delphi.datapath_metrics" }}
    static {{$msgName}}Ptr New{{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key, uint64_t pal_addr);
          {{else}}
    static {{$msgName}}Ptr New{{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key);
          {{end}}
	static delphi::error Publish({{$pkgName}}::{{.GetCppTypeName}} key, {{$msgName | ToLower}}_t *mptr);
	static {{$msgName}}Ptr Find({{$pkgName}}::{{.GetCppTypeName}} key);
	static delphi::error DeleteComposite({{$pkgName}}::{{.GetCppTypeName}} key);
        static void Release({{$msgName}}Ptr ptr);
        {{else}}
    {{$msgName}}({{.GetCppTypeName}} key, char *valptr);
    {{$msgName}}({{.GetCppTypeName}} key, char *valptr, uint64_t pal_addr);
	{{$msgName}}(char *kptr, char *valptr) : {{$msgName}}(*({{.GetCppTypeName}} *)kptr, valptr){ };
	{{$msgName}}(char *kptr, char *valptr, uint64_t pal_addr) : {{$msgName}}(*({{.GetCppTypeName}} *)kptr, valptr, pal_addr){ };
    {{.GetCppTypeName}} GetKey() { return key_; };
          {{if $msg.HasExtOption "delphi.datapath_metrics" }}
    static {{$msgName}}Ptr New{{$msgName}}({{.GetCppTypeName}} key, uint64_t pal_addr);
          {{else}}
    static {{$msgName}}Ptr New{{$msgName}}({{.GetCppTypeName}} key);
          {{end}}
	static delphi::error Publish({{.GetCppTypeName}} key, {{$msgName | ToLower}}_t *mptr);
	static {{$msgName}}Ptr Find({{.GetCppTypeName}} key);
        static void Release({{$msgName}}Ptr ptr);
        {{end}}
      {{end}}

      {{if (eq .GetTypeName ".delphi.Counter") }}
    delphi::metrics::CounterPtr {{.GetName}}() { return {{.GetName}}_; };

      {{else if (eq .GetTypeName ".delphi.Gauge") }}
    delphi::metrics::GaugePtr {{.GetName}}() { return {{.GetName}}_; };
      {{end}}
    {{end}}
};
REGISTER_METRICS({{.GetName}});

class {{.GetName}}Iterator {
public:
    explicit {{.GetName}}Iterator(delphi::shm::TableIterator tbl_iter) {
        tbl_iter_ = tbl_iter;
    }
    inline void Next() {
        tbl_iter_.Next();
    }
    inline {{.GetName}}Ptr Get() {
    {{if (.HasExtOption "delphi.singleton")}}
                if (!tbl_iter_.IsDpstats()) {
                    return make_shared<{{$msgName}}>(tbl_iter_.Value());
                } else {
                    uint64_t pal_addr = *(uint64_t *)tbl_iter_.Value();
                    return make_shared<{{$msgName}}>(tbl_iter_.Value(), pal_addr);
                }
    {{end}}
    {{$fields := .Fields}}
    {{range $fields}} 
      {{if (eq .GetName "Key") }}
        {{ if .TypeIsMessage }}
        char *keyptr = tbl_iter_.Key();
        string keystr(keyptr);
        {{$pkgName}}::{{.GetCppTypeName}} key;
        key.ParseFromString(keystr);

        if (!tbl_iter_.IsDpstats()) {
            return make_shared<{{$msgName}}>(key, tbl_iter_.Value());
        } else {
            uint64_t pal_addr = *(uint64_t *)tbl_iter_.Value();
            return make_shared<{{$msgName}}>(key, tbl_iter_.Value(), pal_addr);
        }
        {{else}}
        {{.GetCppTypeName}} *key = ({{.GetCppTypeName}} *)tbl_iter_.Key();
        if (!tbl_iter_.IsDpstats()) {
            return make_shared<{{$msgName}}>(*key, tbl_iter_.Value());
        } else {
            uint64_t pal_addr = *(uint64_t *)tbl_iter_.Value();
            return make_shared<{{$msgName}}>(*key, tbl_iter_.Value(), pal_addr);
        }
        {{end}}

      {{end}} 
    {{end}} 
    }
    inline bool IsNil() {
        return tbl_iter_.IsNil();
    }
    inline bool IsNotNil() {
        return tbl_iter_.IsNotNil();
    }
private:
    delphi::shm::TableIterator tbl_iter_;
};

  {{end}}
{{end}}
} // namespace objects
} // namespace delphi

#endif // _{{ .FilePrefix | ToUpper }}_OBJECTS_DELPHI_H_
`

	SrcTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream
#include "{{.FilePrefix}}.delphi.hpp"

namespace delphi {
namespace objects {

{{$fileName := .GetName}}
{{$pkgName := .Package}}
{{$msgs := .Messages}}
{{range $msgs}}
  {{$msgName := .GetName}}
  {{$msg := .}}
  {{if not (HasSuffix .GetName "Metrics")}}
    {{if .HasFieldType ".delphi.ObjectMeta" }}
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
      {{if .HasExtOption "delphi.update_event" }} {{$upd_ext := .GetExtOption "delphi.update_event"}} 
        {{if IsTrue $upd_ext}}
                // Trigger update event
                RETURN_IF_FAILED(rctr->On{{ .GetName }}Update(shared_from_this()));
        {{end}} 
      {{end}}
                // see which field changed
      {{$fields := .Fields}}
      {{range $fields}}
        {{if and (ne .GetName "Meta") (ne .GetName "Key")}} 
          {{ if .HasExtOption "delphi.event"}} {{$evt_ext := .GetExtOption "delphi.event"}} 
            {{if IsTrue $evt_ext}}
              {{if .IsRepeated }}
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
                } 
              {{else if .TypeIsMessage }}
                if (this->{{ .GetName | ToLower }}().ShortDebugString() != exObj->{{ .GetName | ToLower }}().ShortDebugString()) {
                    RETURN_IF_FAILED(rctr->On{{ .GetName }}(shared_from_this()));
                } 
              {{else}}
                if (this->{{ .GetName | ToLower }}() != exObj->{{ .GetName | ToLower }}()) {
                    RETURN_IF_FAILED(rctr->On{{ .GetName }}(shared_from_this()));
                }
              {{end}} 
            {{end}} 
          {{end}} 
        {{end}} 
      {{end}}
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
    vector<BaseObjectPtr> olist = sdk->ListKind("{{.GetName}}");

    // walk all objects and convert type
    for (vector<BaseObjectPtr>::iterator oi=olist.begin(); oi!=olist.end(); ++oi) {
        {{.GetName}}Ptr obj = static_pointer_cast<{{.GetName}}>(*oi);
        objlist.push_back(obj);
    }

    return objlist;
}
    {{end}}
  {{else}}
// {{.GetName}} metrics constructor
    {{if (.HasExtOption "delphi.singleton")}}
{{$msgName}}::{{$msgName}}(char *ptr) {
	shm_ptr_ = ptr;
    key_ = 0;
    {{end}}
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
{{$msgName}}::{{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key, char *ptr) {
	{{else}}
{{$msgName}}::{{$msgName}}({{.GetCppTypeName}} key, char *ptr) {
	{{end}}
    shm_ptr_ = ptr;
    key_ = key;
      {{end}}

      {{if (eq .GetTypeName ".delphi.Counter") }}
    {{.GetName}}_ = make_shared<delphi::metrics::Counter>((uint64_t *)ptr);
    ptr += delphi::metrics::Counter::Size();
      {{else if (eq .GetTypeName ".delphi.Gauge") }}
    {{.GetName}}_ = make_shared<delphi::metrics::Gauge>((double *)ptr);
    ptr += delphi::metrics::Gauge::Size();
      {{end}}
    {{end}}
}

// {{.GetName}} metrics constructor
    {{if (.HasExtOption "delphi.singleton")}}
{{$msgName}}::{{$msgName}}(char *valptr, uint64_t pal_addr) {
    pal_addr_ = pal_addr;
    shm_ptr_ = valptr;
    key_ = 0;
    {{end}}
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
{{$msgName}}::{{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key, char *valptr, uint64_t pal_addr) {
	{{else}}
{{$msgName}}::{{$msgName}}({{.GetCppTypeName}} key, char *valptr, uint64_t pal_addr) {
	{{end}}
    pal_addr_ = pal_addr;
    shm_ptr_ = valptr;
    key_ = key;
      {{end}}

      {{if (eq .GetTypeName ".delphi.Counter") }}
    {{.GetName}}_ = make_shared<delphi::metrics::Counter>(pal_addr);
    pal_addr += delphi::metrics::Counter::Size();
      {{else if (eq .GetTypeName ".delphi.Gauge") }}
    {{.GetName}}_ = make_shared<delphi::metrics::Gauge>(pal_addr);
    pal_addr += delphi::metrics::Gauge::Size();
      {{end}}
    {{end}}
}

int32_t {{.GetName}}::Size() {
    int32_t sz = 0;

    // calculate the shared memory size
    {{$fields := .Fields}}
    {{range $fields}}
      {{if not (eq .GetName "Key") }}
        {{if (eq .GetTypeName ".delphi.Counter") }}
    sz += delphi::metrics::Counter::Size();
        {{else if (eq .GetTypeName ".delphi.Gauge") }}
    sz += delphi::metrics::Gauge::Size();
        {{end}}
      {{end}}
    {{end}}

    return sz;
}

// CreateTable creates a table for metrics
delphi::error  {{.GetName}}::CreateTable() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{.GetName}}", DEFAULT_METRIC_TBL_SIZE);

    return delphi::error::OK();
}

    {{if (.HasExtOption "delphi.singleton")}}
      {{if $msg.HasExtOption "delphi.datapath_metrics" }}
// New{{.GetName}} creates a new metrics instance in PAL memory
{{$msgName}}Ptr {{$msgName}}::New{{$msgName}}(uint64_t pal_addr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{$msgName}}", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    uint32_t key = 0;
    char *valptr= tbl->CreateDpstats((char *)&key, sizeof(key), pal_addr, {{$msgName}}::Size());
    assert(valptr != NULL);

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(valptr, pal_addr);
}
      {{else}}
// New{{.GetName}} creates a new metrics instance
{{$msgName}}Ptr {{$msgName}}::New{{$msgName}}() {
	uint32_t key = 0;
	// get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{$msgName}}", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    char *shmptr = (char *)tbl->Create((char *)&key, sizeof(key), {{.GetName}}::Size());

    // return an instance of {{.GetName}}
    return make_shared<{{.GetName}}>(shmptr);
}
      {{end}} 
    {{end}}
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
        {{ if .TypeIsMessage }}
          {{if $msg.HasExtOption "delphi.datapath_metrics" }}
// New{{.GetName}} creates a new metrics instance in PAL memory
{{$msgName}}Ptr {{$msgName}}::New{{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key, uint64_t pal_addr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{$msgName}}", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    auto keystr = key.SerializeAsString();
    char *valptr = tbl->CreateDpstats((char *)keystr.c_str(), keystr.length(), pal_addr, {{$msgName}}::Size());
    assert(valptr != NULL);

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, valptr, pal_addr);
}
          {{else}}
// New{{.GetName}} creates a new metrics instance
{{$msgName}}Ptr {{$msgName}}::New{{$msgName}}({{$pkgName}}::{{.GetCppTypeName}} key) {

    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{$msgName}}", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    auto keystr = key.SerializeAsString();
    char *shmptr = (char *)tbl->Create((char *)keystr.c_str(), keystr.length(), {{$msgName}}::Size());

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, shmptr);
}
          {{end}}
        {{else}}
          {{if $msg.HasExtOption "delphi.datapath_metrics" }}
// New{{.GetName}} creates a new metrics instance in PAL memory
{{$msgName}}Ptr {{$msgName}}::New{{$msgName}}({{.GetCppTypeName}} key, uint64_t pal_addr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{$msgName}}", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    char *valptr = tbl->CreateDpstats((char *)&key, sizeof(key), pal_addr, {{$msgName}}::Size());
    assert(valptr != NULL);

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, valptr, pal_addr);
}
          {{else}}
// New{{.GetName}} creates a new metrics instance
{{$msgName}}Ptr {{$msgName}}::New{{$msgName}}({{.GetCppTypeName}} key) {

    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->CreateTable("{{$msgName}}", DEFAULT_METRIC_TBL_SIZE);

    // create an entry in hash table
    char *shmptr = (char *)tbl->Create((char *)&key, sizeof(key), {{$msgName}}::Size());

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, shmptr);
}
          {{end}}
        {{end}} 
      {{end}} 
    {{end}}
    {{if (.HasExtOption "delphi.singleton")}}
// Publish publishes a metric atomically
delphi::error {{$msgName}}::Publish({{$msgName | ToLower}}_t *mptr) {
	// get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    assert(tbl != NULL);

    // publish to hash table
	uint32_t key = 0;
    return tbl->Publish((char *)&key, sizeof(key), (char *)mptr, {{$msgName}}::Size());
}

// Find finds a metrics by key
{{$msgName}}Ptr {{$msgName}}::Find() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // find the key
	uint32_t key = 0;
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
      {{if $msg.HasExtOption "delphi.datapath_metrics" }}
    uint64_t *pal_addr = (uint64_t *)tbl->Find((char *)&key, sizeof(key));
    if (pal_addr == NULL) {
        return NULL;
    }

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>((char *)pal_addr, *pal_addr);
      {{else}}
    char *shmptr = (char *)tbl->Find((char *)&key, sizeof(key));
    if (shmptr == NULL) {
        return NULL;
    }

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(shmptr);
      {{end}}
}

// Release release the metrics object memory
void {{$msgName}}::Release({{$msgName}}Ptr ptr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    tbl->Release(ptr->Raw());
}

    {{end}}
    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
        {{ if .TypeIsMessage }}
// Publish publishes a metric atomically
delphi::error {{$msgName}}::Publish({{$pkgName}}::{{.GetCppTypeName}} key, {{$msgName | ToLower}}_t *mptr) {
	// get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    assert(tbl != NULL);

    // publish to hash table
    auto keystr = key.SerializeAsString();
    return tbl->Publish((char *)keystr.c_str(), keystr.length(), (char *)mptr, {{$msgName}}::Size());
}

// Find finds a metrics by key
{{$msgName}}Ptr {{$msgName}}::Find({{$pkgName}}::{{.GetCppTypeName}} key) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    auto keystr = key.SerializeAsString();

    // find the key
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
          {{if $msg.HasExtOption "delphi.datapath_metrics" }}
    uint64_t *pal_addr = (uint64_t *)tbl->Find((char *)keystr.c_str(), keystr.length());
    if (pal_addr == NULL) {
        return NULL;
    }

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, (char *)pal_addr, *pal_addr);
          {{else}}
    char *shmptr = (char *)tbl->Find((char *)keystr.c_str(), keystr.length());
    if (shmptr == NULL) {
        return NULL;
    }

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, shmptr);
          {{end}}
}

// DeleteComposite deletes the metric instance
delphi::error {{$msgName}}::DeleteComposite({{$pkgName}}::{{.GetCppTypeName}} key) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    auto keystr = key.SerializeAsString();

    // get the table
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    assert(tbl != NULL);

    // delete the key
    return tbl->Delete((char *)keystr.c_str(), keystr.length());
}

// Release release the metrics object memory
void {{$msgName}}::Release({{$msgName}}Ptr ptr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    tbl->Release(ptr->Raw());
}

        {{else}}
// Publish publishes a metric atomically
delphi::error {{$msgName}}::Publish({{.GetCppTypeName}} key, {{$msgName | ToLower}}_t *mptr) {
	// get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    assert(tbl != NULL);

    // publish to hash table
    return tbl->Publish((char *)&key, sizeof(key), (char *)mptr, {{$msgName}}::Size());
}

// Find finds a metrics by key
{{$msgName}}Ptr {{$msgName}}::Find({{.GetCppTypeName}} key) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // find the key
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
          {{if $msg.HasExtOption "delphi.datapath_metrics" }}
    uint64_t *pal_addr = (uint64_t *)tbl->Find((char *)&key, sizeof(key));
    if (pal_addr == NULL) {
        return NULL;
    }

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, (char *)pal_addr, *pal_addr);
          {{else}}
    char *shmptr = (char *)tbl->Find((char *)&key, sizeof(key));
    if (shmptr == NULL) {
        return NULL;
    }

    // return an instance of {{$msgName}}
    return make_shared<{{$msgName}}>(key, shmptr);
          {{end}}
}

// Release release the metrics object memory
void {{$msgName}}::Release({{$msgName}}Ptr ptr) {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{$msgName}}");
    tbl->Release(ptr->Raw());
}

        {{end}} 
      {{end}} 
    {{end}}


// Delete deletes the metric instance
delphi::error {{.GetName}}::Delete() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{.GetName}}");
    assert(tbl != NULL);

    // delete the key
    return tbl->Delete((char *)&key_, sizeof(key_));
}

// Iterator returns an iterator for metrics
{{.GetName}}Iterator {{.GetName}}::Iterator() {
    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table("{{.GetName}}");
    assert(tbl != NULL);

    return {{.GetName}}Iterator(tbl->Iterator());
}

// DebugString prints the contents of the metrics object
string {{.GetName}}::DebugString() {
    stringstream outstr;
    outstr << "{{.GetName}} {" << endl;

    {{$fields := .Fields}}
    {{range $fields}}
      {{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
	outstr << "    Key: " << key_.DebugString() << endl;
	{{else}}
	outstr << "    Key: " << key_ << endl;
	{{end}}
      {{else if (eq .GetTypeName ".delphi.Counter") }}
    outstr << "    {{.GetName}}: " << {{.GetName}}()->Get() << endl;
      {{else if (eq .GetTypeName ".delphi.Gauge") }}
    outstr << "    {{.GetName}}: " << {{.GetName}}()->Get() << endl;
      {{end}}
    {{end}}
    outstr << "}" << endl;

    return outstr.str();
}

// DebugString returns a string with the contents of the metrics object in JSON 
// format
string {{.GetName}}::JSONString() {
    stringstream outstr;
    outstr << "{" << endl;
    outstr << "  \"{{.GetName}}\": {" << endl;

    {{$fields := .Fields}}
    {{range $i, $e := $fields}}
      {{if (eq .GetName "Key") }}
	    {{ if .TypeIsMessage }}
          std::string json_string;
          google::protobuf::util::JsonPrintOptions options;
          options.add_whitespace = true;
          options.always_print_primitive_fields = true;
          options.preserve_proto_field_names = true;
          MessageToJsonString(key_, &json_string, options);
	      outstr << "    \"Key\": " << json_string;
	    {{else}}
	      outstr << "    \"Key\": " << key_;
	    {{end}}
      {{else if (eq .GetTypeName ".delphi.Counter") }}
        outstr << "    \"{{.GetName}}\": " << {{.GetName}}()->Get();
      {{else if (eq .GetTypeName ".delphi.Gauge") }}
        outstr << "    \"{{.GetName}}\": " << {{.GetName}}()->Get();
      {{end}}
      {{if IsLast $i $fields}}
        outstr << endl;
      {{else}}
        outstr << "," << endl;
      {{end}}
    {{end}}
    outstr << "  }" << endl;
    outstr << "}" << endl;

    return outstr.str();
}

  {{end}}
{{end}}
} // namespace objects
} // namespace delphi
`

	UtestTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include "{{.FilePrefix}}.delphi.hpp"
#include "nic/delphi/sdk/delphi_utest.hpp"

{{$fileName := .GetName}} 
{{$msgs := .Messages}}
{{range $msgs}}
  {{if .HasFieldType ".delphi.ObjectMeta" }}
#define {{ .GetName | ToUpper }}_REACTOR_TEST(test_name, reactor_class) \
class {{.GetName}}Service : public delphi::Service, public enable_shared_from_this<{{.GetName}}Service> { \
public: \
    {{.GetName}}Service(delphi::SdkPtr sk) { \
        this->sdk_ = sk; \
        sk->enterAdminMode(); \
        delphi::objects::{{.GetName}}::Mount(sdk_, delphi::ReadWriteMode); \
        reactor_class##var_ = make_shared<reactor_class>(sdk_); \
        delphi::objects::{{.GetName}}::Watch(sdk_, reactor_class##var_); \
    } \
    void OnMountComplete() { \
        vector<delphi::objects::{{.GetName}}Ptr> olist = delphi::objects::{{.GetName}}::List(sdk_); \
        for (vector<delphi::objects::{{.GetName}}Ptr>::iterator obj=olist.begin(); obj!=olist.end(); ++obj) { \
            reactor_class##var_->On{{.GetName}}Create(*obj); \
        } \
    } \
\
    std::shared_ptr<reactor_class>    reactor_class##var_; \
    delphi::SdkPtr                    sdk_; \
}; \
DELPHI_SERVICE_TEST(test_name, {{.GetName}}Service);

  {{end}}
{{end}}

`
)
