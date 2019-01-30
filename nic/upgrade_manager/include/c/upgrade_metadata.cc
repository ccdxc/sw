#include <stdio.h>
#include <iostream>
#include "boost/property_tree/json_parser.hpp"
#include "upgrade_metadata.hpp"
#include "nic/upgrade_manager/utils/upgrade_log.hpp"

namespace upgrade {

using boost::property_tree::ptree;
using namespace std;

void myprint(ptree const& pt)
{
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        myprint(it->second);
    }
}

bool GetUpgCtxTablesFromMeta(string metafile,
                             ImageInfo& meta,
                             bool isVerFromCache) {
    ptree             root;

    std::ifstream json_cfg(metafile.c_str());
    memset(&meta, 0, sizeof(meta));
    if (isVerFromCache) {
        try {
            read_json(json_cfg, root);
            for (ptree::value_type sysimg : root.get_child("mainfwa.system_image")) {
                if (!strcmp(sysimg.first.c_str(), "nicmgr_version")) {
                    meta.nicmgrVersion = sysimg.second.get_value<std::string>();
                }
            }
        } catch (std::exception const& e) {
            UPG_LOG_DEBUG("Unable to parse upgrade_metadata.json {}", e.what());
            return false;
        }
    } else {
        try {
            read_json(json_cfg, root);
            for (ptree::value_type item : root) {
                if (!strcmp(item.first.c_str(), "nicmgr_version")) {
                    meta.nicmgrVersion = item.second.get_value<std::string>();;
                }
            }
        } catch (std::exception const& e) {
            UPG_LOG_DEBUG("Unable to parse upgrade_metadata.json {}", e.what());
            return false;
        }
    }

    return true;
}

bool GetUpgCtxFromMeta(UpgCtx& ctx) {
    bool ret = true;

    string premetafile = "/sw/nic/upgrade_manager/meta/upgrade_metadata.json";
    ret = GetUpgCtxTablesFromMeta(premetafile, ctx.preUpgMeta, true);

    string postmetafile = "/sw/nic/upgrade_manager/meta/MANIFEST.json";
    ret = GetUpgCtxTablesFromMeta(postmetafile, ctx.postUpgMeta, false);

    return ret;
}

}
