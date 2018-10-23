#ifndef __SYSMGR_CLIENT_HPP__
#define __SYSMGR_CLIENT_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"


namespace sysmgr
{

template <class T>
void mountKey(delphi::SdkPtr &delphi, std::string key)
{
    auto keyobj = std::make_shared<T>();
    keyobj->set_key(key);
    T::MountKey(delphi, keyobj, delphi::ReadWriteMode);
}


class Client : public delphi::objects::SysmgrShutdownReqReactor {
    public:
        Client(delphi::SdkPtr delphi, std::string name): delphi(delphi), name(name){
            delphi::objects::SysmgrShutdownReq::Mount(this->delphi, delphi::ReadMode);
            mountKey<delphi::objects::SysmgrServiceStatus>(this->delphi, name);
            mountKey<delphi::objects::SysmgrShutdownStatus>(this->delphi, name);
        };
        void init_done();
        void test_done();
    private:
        delphi::SdkPtr delphi;
        const std::string name;
        virtual delphi::error OnSysmgrShutdownReqCreate(delphi::objects::SysmgrShutdownReqPtr obj);
};


} // sysmgr


#endif
