// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_OBJECT_FACTORY_H_
#define _DELPHI_OBJECT_FACTORY_H_

#include "nic/delphi/utils/utils.hpp"

namespace delphi {

// forward declaration of base object
class BaseObject;
typedef std::shared_ptr<BaseObject> BaseObjectPtr;

class ObjectFactory
{
public:
    virtual BaseObjectPtr Create(const string &data) = 0;
};


// register a new kind of object
#define REGISTER_KIND(klass) \
    class klass##Factory : public delphi::ObjectFactory { \
    public: \
        klass##Factory() \
        { \
            delphi::BaseObject::RegisterKind(#klass, this); \
        } \
        delphi::BaseObjectPtr Create(const string& data) { \
            return make_shared<klass>(data); \
        } \
    }; \
    static klass##Factory global_##klass##Factory;

} // namespace delphi

#endif // _DELPHI_OBJECT_FACTORY_H_
