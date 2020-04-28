#pragma once
#include <string>
// concatenated string versions
#define xstr(s) str(s)
#define str(s) #s

#define IONICCONFIG_VERSION_EXTENSION       xstr(IONIC_VERSIONINFO_EXTENSION)
#define IONICCONFIG_PRODUCT_NAME            "Pensando IonicConfig Driver Configuration Tool" 
#define IONICCONFIG_TOOL_NAME               "IonicConfig.exe" // xstr(IONIC_TOOL_NAME)
#define IONICCONFIG_VERSION_STRING          xstr(IONIC_MAJOR_TOOL_VERSION) "." xstr(IONIC_MINOR_TOOL_VERSION) "." xstr(IONIC_SP_TOOL_VERSION) "." xstr(IONIC_BUILD_TOOL_VERSION)
#define IONICCONFIG_PRODVERSION_STRING      xstr(IONIC_MAJOR_TOOL_VERSION) "." xstr(IONIC_MINOR_TOOL_VERSION) "." xstr(IONIC_SP_TOOL_VERSION)

#define IONICCONFIG_VENDOR_TOOL_VERSION   ((IONIC_MAJOR_TOOL_VERSION << 16) | IONIC_MINOR_TOOL_VERSION)
