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
    UPG_LOG_DEBUG("Executing {}", cmd);
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

static
bool read_meta_for_image(ImageInfo& meta, ptree& root, string img)
{
    try {
        UPG_LOG_DEBUG("image {}", img);
        for (ptree::value_type sysimg : root.get_child(img)) {
            if (!strcmp(sysimg.first.c_str(), "nicmgr_compat_version")) {
                meta.nicmgrVersion = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running nicmgr version: {}", meta.nicmgrVersion);
            } else if (!strcmp(sysimg.first.c_str(), "kernel_compat_version")) {
                meta.kernelVersion = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running kernel version: {}", meta.kernelVersion);
            } else if (!strcmp(sysimg.first.c_str(), "pcie_compat_version")) {
                meta.pcieVersion = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running pcie version: {}", meta.pcieVersion);
            } else if (!strcmp(sysimg.first.c_str(), "dev_conf_compat_version")) {
                meta.devConfVersion = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running devconf version: {}", meta.devConfVersion);
            } else if (!strcmp(sysimg.first.c_str(), "build_date")) {
                meta.buildDate = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running build date: {}", meta.buildDate);
            } else if (!strcmp(sysimg.first.c_str(), "base_version")) {
                meta.baseVersion = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running base version: {}", meta.baseVersion);
            } else if (!strcmp(sysimg.first.c_str(), "software_version")) {
                meta.softwareVersion = sysimg.second.get_value<string>();
                UPG_LOG_DEBUG("running software version: {}", meta.softwareVersion);
            }
        }
    } catch (exception const& e) {
        UPG_LOG_DEBUG("PreMeta Unable to parse {} {}", img, e.what());
        return false;
    }

    return true;
}

bool GetUpgCtxTablesFromMeta(string metafile,
                             ImageInfo& meta,
                             bool isVerFromCache) {
    ptree             root;

    ifstream json_cfg(metafile.c_str());
    memset(&meta, 0, sizeof(meta));
    if (isVerFromCache) {
        try {
            string img = "mainfwa.system_image";
            string not_img = "mainfwb.system_image";
            read_json(json_cfg, root);
            if (exists("/nic/tools/fwupdate")) {
                UPG_LOG_DEBUG("this is the image {}", exec("/nic/tools/fwupdate -r"));
                if (exec("/nic/tools/fwupdate -r") == "mainfwb\n") {
                   img = "mainfwb.system_image";
                   not_img = "mainfwa.system_image";
                }
            }
            if (!read_meta_for_image(meta, root, img)) {
                read_meta_for_image(meta, root, not_img);
            }
        } catch (exception const& e) {
            UPG_LOG_DEBUG("PreMeta Unable to parse {} {}", metafile, e.what());
            return false;
        }
    } else {
        try {
            read_json(json_cfg, root);
            for (ptree::value_type item : root) {
                if (!strcmp(item.first.c_str(), "nicmgr_compat_version")) {
                    meta.nicmgrVersion = item.second.get_value<string>();;
                    UPG_LOG_DEBUG("upgrade nicmgr version: {}", meta.nicmgrVersion);
                } else if (!strcmp(item.first.c_str(), "kernel_compat_version")) {
                    meta.kernelVersion = item.second.get_value<string>();;
                    UPG_LOG_DEBUG("upgrade kernel version: {}", meta.kernelVersion);
                } else if (!strcmp(item.first.c_str(), "pcie_compat_version")) {
                    meta.pcieVersion = item.second.get_value<string>();;
                    UPG_LOG_DEBUG("upgrade pcie version: {}", meta.pcieVersion);
                } else if (!strcmp(item.first.c_str(), "dev_conf_compat_version")) {
                    meta.devConfVersion = item.second.get_value<string>();
                    UPG_LOG_DEBUG("running devconf version: {}", meta.devConfVersion);
                } else if (!strcmp(item.first.c_str(), "build_date")) {
                    meta.buildDate = item.second.get_value<string>();
                    UPG_LOG_DEBUG("upgrade build date: {}", meta.buildDate);
                } else if (!strcmp(item.first.c_str(), "base_version")) {
                    meta.baseVersion = item.second.get_value<string>();
                    UPG_LOG_DEBUG("upgrade base version: {}", meta.baseVersion);
                } else if (!strcmp(item.first.c_str(), "software_version")) {
                    meta.softwareVersion = item.second.get_value<string>();
                    UPG_LOG_DEBUG("upgrade software version: {}", meta.softwareVersion);
                }
            }
        } catch (exception const& e) {
            UPG_LOG_DEBUG("PostMeta Unable to parse {} {}", metafile, e.what());
            return false;
        }
    }

    return true;
}

bool GetUpgCtxFromMeta(UpgCtx& ctx) {
    bool ret = true;
    if (ctx.ctxUpdated) {
        return ret;
    }
    string premetafile = "/data/running_meta.json";
    string postmetafile = "/data/upg_meta.json";

    if (exists("/nic/tools/fwupdate")) {
        if (!exists(premetafile.c_str())) {
            string result = exec("/nic/tools/fwupdate -L");
            mkfile(result, premetafile.c_str());
	}
        ret = GetUpgCtxTablesFromMeta(premetafile, ctx.preUpgMeta, true);
        if (!exists(postmetafile.c_str())) {
            string postmetacmd = "/bin/tar xfO /update/" + ctx.firmwarePkgName + " MANIFEST";
            string result = exec(postmetacmd.c_str());
            mkfile(result, postmetafile.c_str());
        }
        ret = GetUpgCtxTablesFromMeta(postmetafile, ctx.postUpgMeta, false);
    } else {
        string premetafile = "/sw/nic/upgrade_manager/meta/upgrade_metadata.json";
        ret = GetUpgCtxTablesFromMeta(premetafile, ctx.preUpgMeta, true);

        string postmetafile = "/sw/nic/upgrade_manager/meta/MANIFEST.json";
        ret = GetUpgCtxTablesFromMeta(postmetafile, ctx.postUpgMeta, false);
    }
    ctx.ctxUpdated = true;
    return ret;
}

bool IsPrePostImageMetaSame(UpgCtx& ctx) {
    if (ctx.postUpgMeta.nicmgrVersion == ctx.preUpgMeta.nicmgrVersion &&
        ctx.postUpgMeta.kernelVersion == ctx.preUpgMeta.kernelVersion &&
        ctx.postUpgMeta.pcieVersion == ctx.preUpgMeta.pcieVersion &&
        ctx.postUpgMeta.buildDate == ctx.preUpgMeta.buildDate &&
        ctx.postUpgMeta.baseVersion == ctx.preUpgMeta.baseVersion &&
        ctx.postUpgMeta.softwareVersion == ctx.preUpgMeta.softwareVersion) {
        return true;
    }
    return false;
}

}
