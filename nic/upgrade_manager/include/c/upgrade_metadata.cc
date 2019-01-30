#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
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
        cout << it->first << ": " << it->second.get_value<string>() << endl;
        myprint(it->second);
    }
}

void mkfile (string result, const char* file) {
    std::ofstream ofile (file);
    ofile << result;
    ofile.close();
}

string exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

inline bool exists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

bool GetUpgCtxTablesFromMeta(string metafile,
                             ImageInfo& meta,
                             bool isVerFromCache) {
    ptree             root;

    ifstream json_cfg(metafile.c_str());
    memset(&meta, 0, sizeof(meta));
    if (isVerFromCache) {
        try {
            read_json(json_cfg, root);
            for (ptree::value_type sysimg : root.get_child("mainfwa.system_image")) {
                if (!strcmp(sysimg.first.c_str(), "nicmgr_version")) {
                    meta.nicmgrVersion = sysimg.second.get_value<string>();
                }
            }
        } catch (exception const& e) {
            UPG_LOG_DEBUG("Unable to parse upgrade_metadata.json {}", e.what());
            return false;
        }
    } else {
        try {
            read_json(json_cfg, root);
            for (ptree::value_type item : root) {
                if (!strcmp(item.first.c_str(), "nicmgr_version")) {
                    meta.nicmgrVersion = item.second.get_value<string>();;
                }
            }
        } catch (exception const& e) {
            UPG_LOG_DEBUG("Unable to parse upgrade_metadata.json {}", e.what());
            return false;
        }
    }

    return true;
}

bool GetUpgCtxFromMeta(UpgCtx& ctx) {
    bool ret = true;

    if (exists("/nic/tools/fwupdate")) {
        string result = exec("/nic/tools/fwupdate -L");
        string premetafile = "/tmp/running_meta.json";
        mkfile(result, premetafile.c_str());
        ret = GetUpgCtxTablesFromMeta(premetafile, ctx.preUpgMeta, true);

        string postmetacmd = "tar xvO /update/" + ctx.firmwarePkgName;
        result = exec(postmetacmd.c_str());
        string postmetafile = "/tmp/upg_meta.json";
        mkfile(result, postmetafile.c_str());
        ret = GetUpgCtxTablesFromMeta(postmetafile, ctx.postUpgMeta, false);

        //remove(premetafile.c_str());
        //remove(postmetafile.c_str());
    } else {
        string premetafile = "/sw/nic/upgrade_manager/meta/upgrade_metadata.json";
        ret = GetUpgCtxTablesFromMeta(premetafile, ctx.preUpgMeta, true);

        string postmetafile = "/sw/nic/upgrade_manager/meta/MANIFEST.json";
        ret = GetUpgCtxTablesFromMeta(postmetafile, ctx.postUpgMeta, false);
    }
    return ret;
}

}
