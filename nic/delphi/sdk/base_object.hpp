// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_BASE_OBJECTS_H_
#define _DELPHI_BASE_OBJECTS_H_

#include <map>

#include "nic/delphi/utils/utils.hpp"
#include "nic/delphi/messanger/proto/types.pb.h"
#include "object_factory.hpp"

namespace delphi {

using namespace std;

// forward declarations
class BaseReactor;
typedef std::shared_ptr<BaseReactor> BaseReactorPtr;

// ReactorList is a list of reactors for an object kind
class ReactorList {
public:
        vector<BaseReactorPtr> reactors;
};
typedef std::shared_ptr<ReactorList> ReactorListPtr;


// BaseObject is the base class for all delphi objects
class BaseObject {
public:
    // abstract methods all delphi objects have to implement
    virtual delphi::ObjectMeta *GetMeta() = 0;              // get object meta
    virtual string GetKey() = 0;                            // Get key for the object
    virtual ::google::protobuf::Message *GetMessage() = 0;  // Get protobuf message
    virtual BaseObjectPtr Clone() = 0;                      // clone the object
    virtual error TriggerEvent(BaseObjectPtr oldObj, ObjectOperation op, ReactorListPtr rl) = 0; // trigger event on the object

    // GetFactoryMap gets the factory
    static inline map<string, ObjectFactory*> *GetFactoryMap() {
        static map<string, ObjectFactory*> factories;
        return &factories;
    }

    // registers a factory class for each kind
    static inline void RegisterKind(string kind, ObjectFactory *factry) {
        try {
            map<string, ObjectFactory*> *fctries = BaseObject::GetFactoryMap();
            LogInfo("Registering object kind {}", kind);
            fctries->insert(std::pair<string, ObjectFactory*>(kind, factry));
        } catch (...){}
    }

    // creates an object of specific kind
    static inline BaseObjectPtr Create(const string &name, const string &data) {
        map<string, ObjectFactory*> fctries = *(BaseObject::GetFactoryMap());
        ObjectFactory *fctry = fctries[name];
        assert(fctry != NULL);

        return fctry->Create(data);
    }
};

// BaseReactor is the base class for all reactors
class BaseReactor {
public:
};

} // namespace delphi

#endif // _DELPHI_BASE_OBJECTS_H_
