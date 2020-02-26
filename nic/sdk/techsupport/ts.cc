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
techsupport_get_fsink (const string& dst_file)
{
    gzFile file_sink;

    file_sink = gzopen(dst_file.c_str(), "w9b");
    if (file_sink == NULL) {
        cerr << "could not open " << dst_file << endl ;
        exit(EXIT_FAILURE);
    }

    return file_sink;
}

void
techsupport::process_config_(void) {
    fsink_ = techsupport_get_fsink(dst_file_);
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
    gzprintf(fsink_, "exit_status : %d\n", ret);

    return ret;
}

int
techsupport::run_tasks(void) {
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

}    // namespace ts
}    // namespace sdk
