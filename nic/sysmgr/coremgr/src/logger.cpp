#include <stdio.h>
#include <iostream>

#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

#include "logger.hpp"

// GetLogger returns a logger instance
Logger GetLogger() {
    static Logger _logger = spdlog::stdout_color_mt("coremgr");
    static bool initDone = false;

    if (!initDone) {
        struct stat sb;
        char filename[PATH_MAX];
        int fd;

        _logger->set_pattern("%L [%Y-%m-%d %H:%M:%S.%f] %P/%n: %v");
#ifdef DEBUG_ENABLE
        spdlog::set_level(spdlog::level::debug);
#endif
        initDone = true;

        if ((stat(LOG_DIRECTORY, &sb) == 0) && S_ISDIR(sb.st_mode))
        {
            snprintf(filename, PATH_MAX, "%s/coremgr.log", LOG_DIRECTORY);
            fd = open(filename, O_WRONLY | O_APPEND | O_CREAT);
            if (fd != -1)
            {
                dup2(fd, STDOUT_FILENO);   
            }
        }
    }

    return _logger;
}