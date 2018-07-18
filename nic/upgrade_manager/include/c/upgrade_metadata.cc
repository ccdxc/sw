#include <stdio.h>
#include <iostream>
#include "boost/property_tree/json_parser.hpp"
#include "upgrade_metadata.hpp"

namespace upgrade {

using boost::property_tree::ptree;
using namespace std;

bool GetUpgCtxFromMeta(UpgCtx& ctx) {
    ptree             pt;
    string metafile = "/sw/nic/upgrade_manager/meta/upgrade_metadata.json";

    std::ifstream json_cfg(metafile.c_str());
    read_json(json_cfg, pt);
    try {
        ctx.fromVer = pt.get<std::string>("version");
    } catch (std::exception const& e) {
        LogInfo("Unable to parse upgrade_metadata.json %s", e.what());
        return false;
    }
    return true;
}

}
