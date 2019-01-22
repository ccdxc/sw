#include "sysmgr_client.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "nic/utils/penlog/lib/penlog.hpp"

using namespace std;
using namespace sysmgr;

static penlog::LoggerPtr logger = penlog::logger_init_for_lib("sysmgr");

template <class T>
void mountKey(delphi::SdkPtr &delphi, std::string key)
{
    auto keyobj = std::make_shared<T>();
    keyobj->set_key(key);
    T::MountKey(delphi, keyobj, delphi::ReadWriteMode);
}

ClientPtr sysmgr::CreateClient(delphi::SdkPtr delphi, std::string name)
{
    ClientPtr client = make_shared<Client>(delphi, name);

    client->register_mounts();

    return client;
}

Client::Client(delphi::SdkPtr delphi, std::string name):
    delphi(delphi), name(name)
{
}

void Client::register_mounts()
{
    delphi::objects::SysmgrShutdownReq::Mount(this->delphi, delphi::ReadMode);
    mountKey<delphi::objects::SysmgrServiceStatus>(this->delphi, name);
    mountKey<delphi::objects::SysmgrShutdownStatus>(this->delphi, name);
  
    delphi::objects::SysmgrServiceStatus::Mount(this->delphi, delphi::ReadMode);
    delphi::objects::SysmgrServiceStatus::Watch(this->delphi, shared_from_this());

    this->delphi->WatchMountComplete(shared_from_this());
};

void Client::init_done()
{
    auto obj = make_shared<delphi::objects::SysmgrServiceStatus>();

    obj->set_key(this->name);
    obj->set_pid(getpid());
    this->delphi->QueueUpdate(obj);
}

void Client::OnMountComplete()
{  
    for (auto obj: delphi::objects::SysmgrServiceStatus::List(this->delphi)) {
        this->service_up(obj->key());
    }
}


void Client::register_service_reactor(std::string name, ServiceStatusReactorPtr rctor)
{
    this->reactors.insert(std::pair<std::string, ServiceStatusReactorPtr>(name, rctor));
}

delphi::error Client::OnSysmgrServiceStatusCreate(delphi::objects::SysmgrServiceStatusPtr obj)
{
    this->service_up(obj->key());
    logger->debug("Got service create notification for {}", obj->key());
    return delphi::error::OK();
}

delphi::error Client::OnSysmgrServiceStatusUpdate(delphi::objects::SysmgrServiceStatusPtr obj)
{
    this->service_up(obj->key());
    logger->debug("Got service update notification for {}", obj->key());
    return delphi::error::OK();
}

void Client::service_up(std::string name)
{
    ServiceStatusReactorPtr r = this->reactors[name];
    if (r != nullptr) {
        r->ServiceUp(name);
        logger->debug("Found service up reactor for {}", name);
    } else {
        logger->debug("No service up reactor for {}", name);
    }
}

void Client::restart_system()
{
}
