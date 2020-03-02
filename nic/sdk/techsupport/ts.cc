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

static inline int
ts_get_exit_status (int rc)
{
    int ret = rc;

    if (WIFEXITED(rc)) {
        ret = WEXITSTATUS(rc);
    }
    return ret;
}

static void
techsupport_run_sys_cmds (vector<string> &cmds)
{
    int ret;
    vector<string>::iterator it;

    for (it = cmds.begin(); it < cmds.end(); it++) {
        cout << "Processing : " << *it << endl;
        ret = ts_get_exit_status(system(it->c_str()));
        if (ret != EXIT_SUCCESS) {
            cerr << "Processing : " << *it << " failed, err: " << ret << endl;
            exit(ret);
        }
    }
}

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
techsupport::process_config_(void) {
    boost::property_tree::ptree pt;

    // parse techsupport.json
    try {
        ifstream json_cfg(cmd_file_.c_str());
        read_json(json_cfg, pt);
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                      pt.get_child("tasks")) {
            tasks_.push_back(v.second.get_value<string>());
        }
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                      pt.get_child("cores")) {
            core_dir_.push_back(v.second.get_value<string>());
        }
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,
                      pt.get_child("logs")) {
            log_dir_.push_back(v.second.get_value<string>());
        }
    } catch (exception const &e) {
        cerr << "exception while processing " << cmd_file_ << e.what() << endl ;
        exit(EXIT_FAILURE);
    }
}

void
techsupport::setup_fsink_(void) {
    string file_name = dst_dir_ + "/" + work_dir_ + "/tasks.txt.gz";

    fsink_ = gzopen(file_name.c_str(), "w9b");
    if (fsink_ == NULL) {
        cerr << "could not open " << file_name << endl ;
        exit(EXIT_FAILURE);
    }
}

void
techsupport::setup_working_dir_(void) {
    string mkdir_cmd;
    char dir_template[] = "DSC_Techsupport-XXXXXX";
    vector<string> cmds;

    // create a directory to work on
    work_dir_.assign(mkdtemp(dir_template));
    mkdir_cmd = "mkdir -p " + dst_dir_ + "/" + work_dir_;
    cmds.push_back(mkdir_cmd);
    techsupport_run_sys_cmds(cmds);
}

static inline string
get_copy_dir_task (const string& src_dir, const string& dst_dir)
{
    string tar_cmd;
    string log_name(src_dir);

    // get the destination file name from src_dir
    replace(log_name.begin(), log_name.end(), '/', '_');
    tar_cmd = "tar -zcf " + dst_dir + log_name + ".tar.gz " + src_dir;

    return tar_cmd;
}

void
techsupport::add_log_collection_tasks_(void) {
    vector<string>::iterator it;
    string cmd;
    string log_dst_dir = dst_dir_ + "/" + work_dir_ + "/";

    for (it = log_dir_.begin(); it < log_dir_.end(); it++) {
        cmd = get_copy_dir_task(*it, log_dst_dir);
        tasks_.push_back(cmd);
    }
    // collect cores if requested
    if (!skip_core_) {
        for (it = core_dir_.begin(); it < core_dir_.end(); it++) {
            cmd = get_copy_dir_task(*it, log_dst_dir);
            tasks_.push_back(cmd);
        }
    }
}

void
techsupport::setup_(void) {
    setup_working_dir_();
    setup_fsink_();
    add_log_collection_tasks_();
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
    auto ret = ts_get_exit_status(pclose(pipefp));
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

void
techsupport::bundle_techsupport_(void) {
    string ts_tar_file, tar_cmd, tar_validate_cmd, tar_cwd;
    vector<string> cmds;

    // final techsupport tarball
    ts_tar_file = dst_dir_ + "/" + dst_file_;
    // set tar working dir to dst_dir_
    tar_cwd = " -C " + dst_dir_;
    tar_cmd = "tar -zcvf " + ts_tar_file + tar_cwd + " " + work_dir_;
    cmds.push_back(tar_cmd);

    // validate the tar file created
    tar_validate_cmd = "tar -tf " + ts_tar_file;
    cmds.push_back(tar_validate_cmd);
    techsupport_run_sys_cmds(cmds);
}

void
techsupport::teardown_(void) {
    vector<string> cmds;
    string rmdir_cmd;

    rmdir_cmd = "rm -rf " + dst_dir_ + "/" + work_dir_ + "/";
    cmds.push_back(rmdir_cmd);
    techsupport_run_sys_cmds(cmds);
    fsink_ = NULL;
}

int
techsupport::collect_techsupport(void) {
    int ret;

    setup_();
    // trigger the techsupport collection
    ret = run_tasks_();
    if (ret) {
        fprintf(stderr, "Errors while running some of the tasks\n");
    }
    // bundle all the collected info
    bundle_techsupport_();
    // cleanup
    teardown_();

    return ret;
}

}    // namespace ts
}    // namespace sdk
