//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#include <string>
#include <time.h>
#include "gen/proto/opersvc.grpc.pb.h"
#include "gen/proto/types.pb.h"
#include "oper.hpp"

static inline std::string
get_techsupport_filename (void)
{
    char timestring[PATH_MAX];
    char filename[PATH_MAX];
    time_t current_time = time(NULL);

    strftime(timestring, PATH_MAX, "%Y%m%d%H%M%S", gmtime(&current_time));
    snprintf(filename, PATH_MAX, "tech-support-%s.tar.gz", timestring);

    return std::string(filename);
}

static inline std::string
get_techsupport_binary (void)
{
    std::string ts_bin_path;
    std::string ts_bin = "/bin/techsupport";

    ts_bin_path = std::string(std::getenv("PDSPKG_TOPDIR"));
    if (ts_bin_path.empty()) {
        ts_bin_path = std::string("/nic/");
    }
    ts_bin_path += ts_bin;

    return ts_bin_path;
}

static inline std::string
get_techsupport_config (void)
{
    std::string ts_cfg_path;
    std::string ts_cfg = "/techsupport.json";

    ts_cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (ts_cfg_path.empty()) {
        ts_cfg_path = std::string("/nic/conf/");
    }
    ts_cfg_path += ts_cfg;

    return ts_cfg_path;
}

static inline std::string
get_techsupport_cmd (std::string ts_dir, std::string ts_file, bool skipcores)
{
    char ts_cmd[PATH_MAX];
    std::string bin_path, ts_bin_path;
    std::string ts_bin, ts_task;

    ts_bin = get_techsupport_binary();
    ts_task = get_techsupport_config();

    snprintf(ts_cmd, PATH_MAX, "%s -c %s -d %s -o %s %s", ts_bin.c_str(),
             ts_task.c_str(), ts_dir.c_str(), ts_file.c_str(),
             skipcores ? "-s" : "");

    return std::string(ts_cmd);
}

static inline int
get_exit_status (int rc)
{
    int ret = rc;

    if (WIFEXITED(rc)) {
        ret = WEXITSTATUS(rc);
    }
    return ret;
}

Status
OperSvcImpl::TechSupportCollect(ServerContext *context,
                                const TechSupportRequest *req,
                                TechSupportResponse *rsp) {
    int rc;
    std::string tsdir = "/data/techsupport/";
    auto tsrsp = rsp->mutable_response();
    auto tsfile = get_techsupport_filename();
    auto tscmd = get_techsupport_cmd(tsdir, tsfile, req->request().skipcores());

    rc = get_exit_status(system(tscmd.c_str()));
    // PDS_TRACE_DEBUG("Techsupport request {}, rc {}", tsfile, rc);
    if (rc != 0) {
        rsp->set_apistatus(types::ApiStatus::API_STATUS_ERR);
    } else {
        tsrsp->set_filepath(tsdir + tsfile);
        rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    }
    return Status::OK;
}
