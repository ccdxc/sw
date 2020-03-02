//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <iostream>
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ts.hpp"

using namespace std;

namespace sdk {
namespace ts {

techsupport *
techsupport::factory(const techsupport_spec_t& spec) {
    techsupport *techsupport_inst = new techsupport(spec);

    techsupport_inst->process_config_();
    return techsupport_inst;
}

void
techsupport::destroy(techsupport *techsupport_inst) {
    delete techsupport_inst;
}

void
techsupport::process_command_file_(void) {
    boost::property_tree::ptree pt;

    // parse techsupport.json
    try {
        ifstream json_cfg(cmd_file_.c_str());
        read_json(json_cfg, pt);
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                      pt.get_child("tasks")) {
            tasks_.push_back(v.second.get_value<string>());
        }
    } catch (exception const &e) {
        cerr << "exception while processing " << cmd_file_ << e.what() << endl ;
        exit(EXIT_FAILURE);
    }
}

static inline gzFile
techsupport_get_fsink (const string& dst_dir)
{
    gzFile file_sink;
    string file_name = dst_dir + "/tasks.txt.gz";

    file_sink = gzopen(file_name.c_str(), "w9b");
    if (file_sink == NULL) {
        cerr << "could not open " << file_name << endl ;
        exit(EXIT_FAILURE);
    }

    return file_sink;
}

void
techsupport::process_config_(void) {
    fsink_ = techsupport_get_fsink(dst_dir_);
    process_command_file_();
}

int
techsupport::execute_task_(const string& task) {
    char buffer[BUFFER_SIZE];

    gzprintf(fsink_, "=== Task ===\n");
    // record the task being run
    gzprintf(fsink_, "%s\n", task.c_str());
    auto pipefp = popen(task.c_str(), "r");
    if (pipefp == NULL) {
        gzprintf(fsink_, "Failed to execute task : %s\n", strerror(errno));
        return errno;
    }

    // record the task's output
    while (!feof(pipefp)) {
        if (fgets(buffer, BUFFER_SIZE, pipefp) != NULL) {
            gzprintf(fsink_, "%s", buffer);
        }
    }

    // record the task's exit code
    auto ret = pclose(pipefp);
    if (WIFEXITED(ret)){
        ret = WEXITSTATUS(ret);
    }
    gzprintf(fsink_, "exit_status : %d\n", ret);

    return ret;
}

int
techsupport::run_tasks_(void) {
    int ret, rc = 0;
    vector<string>::iterator it;

    for (it = tasks_.begin(); it < tasks_.end(); it++) {
        cout << "Processing : " << *it << endl;
        ret = execute_task_(*it);
        if (ret != EXIT_SUCCESS) {
            cerr << "Processing : " << *it << " failed, err: " << ret << endl;
            rc = ret;
        }
    }
    gzflush(fsink_, Z_FINISH);
    gzclose(fsink_);

    return rc;
}

static void
techsupport_run_sys_cmds (vector<string> &cmds)
{
    int ret;
    vector<string>::iterator it;

    for (it = cmds.begin(); it < cmds.end(); it++) {
        cout << "Processing : " << *it << endl;
        ret = system(it->c_str());
        if (ret != EXIT_SUCCESS) {
            cerr << "Processing : " << *it << " failed, err: " << ret << endl;
            exit(1);
        }
    }
}

static inline string
techsupport_get_tar_cmd (const string& dst_dir, const string& dst_file,
                         const string& ts_file, bool skip_core)
{
    string tar_cmd, tar_excludes;
    string tar_exclude_option = " --exclude=";

    // exclude final tar ball as it gets written to same directory
    tar_excludes = tar_exclude_option + dst_file;
    if (skip_core) {
        // exclude core dir if skip_core is set
        tar_excludes += tar_exclude_option + "core";
    }
    tar_cmd = "tar " + tar_excludes + " -zcvf " + ts_file + " " + dst_dir;
    return tar_cmd;
}

void
techsupport::bundle_techsupport_(void) {
    string ts_file, touch_cmd, tar_cmd, tar_list_cmd;
    vector<string> cmds;

    ts_file = dst_dir_ + "/" + dst_file_;
    // touch tar file to exclude as it gets written to same dir
    touch_cmd = "touch " + ts_file;
    cmds.push_back(touch_cmd);
    tar_cmd = techsupport_get_tar_cmd(dst_dir_, dst_file_, ts_file, skip_core_);
    cmds.push_back(tar_cmd);
    // validate the tar file created
    tar_list_cmd = "tar -tf " + ts_file;
    cmds.push_back(tar_list_cmd);
    techsupport_run_sys_cmds(cmds);
}

int
techsupport::collect_techsupport(void) {
    int ret;

    ret = run_tasks_();
    if (ret) {
        fprintf(stderr, "Errors while running some of the tasks\n");
    }
    bundle_techsupport_();
    return ret;
}

}    // namespace ts
}    // namespace sdk
