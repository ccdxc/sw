#include <stdio.h>
#include <iostream>
#include "boost/property_tree/json_parser.hpp"
#include "upgrade_metadata.hpp"

namespace upgrade {

using boost::property_tree::ptree;
using namespace std;

bool IsNumber(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

bool GetUpgCtxFromMeta(string metafile, UpgCtx& ctx) {
    ptree             pt;

    std::ifstream json_cfg(metafile.c_str());
    read_json(json_cfg, pt);
    try {
        std::string version = pt.get<std::string>("version");
        if (!IsNumber(version)) {
            LogInfo("Invalid version number %s", version);
            return false;
        }
        ctx.fromVer = stoi(version);
    } catch (std::exception const& e) {
        LogInfo("Unable to parse upgrade_metadata.json %s", e.what());
        return false;
    }
    return true;
}

}
