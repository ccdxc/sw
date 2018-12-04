#include <memory>

#include <assert.h>

#include <spdlog/spdlog.h>

#include "gen/proto/client.delphi.hpp"
#include "gen/proto/penlog.delphi.hpp"
#include "gen/proto/penlog.pb.h"
#include "nic/delphi/sdk/delphi_sdk.hpp"

#include "main_logger.hpp"
#include "penlog.hpp"

using namespace penlog;

MainLogger::MainLogger(delphi::SdkPtr delphi, const std::string &name) : Logger(name)
{
    const char *env_location = getenv("PENLOG_LOCATION");
    std::string location;
    if (env_location)
    {
        location = std::string(env_location);
    }
    else
    {
        location = std::string(PENLOG_LOCATION);
    }
    
    this->sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        location + "/" + name + ".log", PENLOG_LOGFILE_SIZE,
        PENLOG_ROTATE_COUNT);

    this->spd_init(this->sink, this->level);

    this->delphi = delphi;
}

void MainLogger::delphi_init()
{
    auto keyobj = std::make_shared<delphi::objects::PenlogReq>();
    keyobj->set_key(this->name);
    delphi::objects::PenlogReq::MountKey(this->delphi, keyobj, delphi::ReadMode);
    delphi::objects::PenlogReq::Watch(this->delphi, shared_from_this());
    this->delphi->WatchMountComplete(shared_from_this());
}

void MainLogger::update_level(Level level)
{
    switch (level)
    {
    case Critical:
        this->level = spdlog::level::critical;
        break;
    case Error:
        this->level = spdlog::level::err;
        break;
    case Warn:
        this->level = spdlog::level::warn;
        break;
    case Info:
        this->level = spdlog::level::info;
        break;
    case Debug:
        this->level = spdlog::level::debug;
        break;
    case Trace:
        this->level = spdlog::level::trace;
        break;
    default:
        assert(false);
    }

    this->set_level(this->level);

    for (auto liblogger: libloggers)
    {
        liblogger.second->set_level(this->level);
    }    
}

void MainLogger::OnMountComplete()
{
    for (auto obj: delphi::objects::PenlogReq::List(this->delphi))
    {
        if (obj->key() == this->name)
        {
            this->trace("level {}", obj->level());
            this->update_level(obj->level());
        }
    }
}

delphi::error MainLogger::OnPenlogReqCreate(delphi::objects::PenlogReqPtr obj)
{
    this->trace("level {}", obj->level());
    this->update_level(obj->level());
    return delphi::error::OK();
}

delphi::error MainLogger::OnPenlogReqUpdate(delphi::objects::PenlogReqPtr obj)
{
    this->trace("level {}", obj->level());
    this->update_level(obj->level());
    return delphi::error::OK();
}

void MainLogger::register_lib_logger(LoggerPtr liblogger)
{
    liblogger->spd_init(this->sink, this->level);
    this->libloggers.insert(std::pair<std::string, LoggerPtr>(
            liblogger->name, liblogger));
}
