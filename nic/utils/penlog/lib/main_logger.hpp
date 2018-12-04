#ifndef __MAIN_LOGGER_H__
#define __MAIN_LOGGER_H__

#include <memory>
#include <map>

#include "gen/proto/penlog.delphi.hpp"
#include "gen/proto/penlog.pb.h"
#include "nic/delphi/sdk/delphi_sdk.hpp"

#include "penlog.hpp"

namespace penlog {
    class MainLogger: public Logger,
                      public delphi::Service,
                      public std::enable_shared_from_this<MainLogger>,
                      public delphi::objects::PenlogReqReactor {
    private:
        sinkptr sink;
        std::map<std::string, LoggerPtr> libloggers;
        delphi::SdkPtr delphi;
    private:
        void update_level(Level level);
    public:
        MainLogger(delphi::SdkPtr delphi, const std::string &name);
        virtual void OnMountComplete();
        void delphi_init();
        void register_lib_logger(LoggerPtr liblogger);
        virtual delphi::error OnPenlogReqCreate(
            delphi::objects::PenlogReqPtr obj);
        virtual delphi::error OnPenlogReqUpdate(
            delphi::objects::PenlogReqPtr obj);
    };
};

#endif
