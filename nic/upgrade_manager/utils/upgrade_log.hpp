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
 #define LogDebug(args...) upgrade::GetLogger()->debug(args)
 #else
 #define LogDebug(args...) while (0) { upgrade::GetLogger()->debug(args); }
 #endif // DEBUG_ENABLE

 #define LogInfo(args...) upgrade::GetLogger()->info(args)
 #define LogWarn(args...) upgrade::GetLogger()->warn(args)
 #define LogError(args...) upgrade::GetLogger()->error(args)
 #define LogFatal(args...) { upgrade::GetLogger()->error(args); assert(0); }

 } // namespace upgrade

 #endif
