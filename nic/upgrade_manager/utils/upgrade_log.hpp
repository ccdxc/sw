 // {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

 #ifndef _UPGRADE_LOG_H_
 #define _UPGRADE_LOG_H_

 #include <stdio.h>
 #include <iostream>
 #include <spdlog/fmt/ostr.h>
 #include <spdlog/spdlog.h>

 namespace upgrade {
 typedef std::shared_ptr<spdlog::logger> Logger;

 // GetLogger returns the current logger instance
 Logger GetLogger();

 #ifdef DEBUG_ENABLE
 #define UPG_LOG_DEBUG(args...) upgrade::GetLogger()->debug(args)
 #else
 #define UPG_LOG_DEBUG(args...) while (0) { upgrade::GetLogger()->debug(args); }
 #endif // DEBUG_ENABLE

 #define UPG_LOG_INFO(args...) upgrade::GetLogger()->info(args)
 #define UPG_LOG_WARN(args...) upgrade::GetLogger()->warn(args)
 #define UPG_LOG_ERROR(args...) upgrade::GetLogger()->error(args)
 #define UPG_LOG_FATAL(args...) { upgrade::GetLogger()->error(args); assert(0); }

 } // namespace upgrade

 #endif
