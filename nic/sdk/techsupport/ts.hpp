//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TS_HPP__
#define __TS_HPP__

#include <zlib.h>
#include <string>
#include <vector>

using namespace std;

namespace sdk {
namespace ts {

#define BUFFER_SIZE 8192
#define NO_OF_TASKS 64

typedef struct techsupport_spec_t_ {
    string cmd_file;    ///< command filename with absolute path
    string dst_dir;     ///< absolute path of destination dir to copy out
    string ts_file;     ///< gzipped techsupport filename
    bool skip_core;     ///< skip cores in final tar
} techsupport_spec_t;

class techsupport {
public:
    /// \brief factory method to allocate & initialize techsupport instance
    /// \return new instance of techsupport or NULL, in case of error
    static techsupport *factory(const techsupport_spec_t& spec);

    /// \brief free memory allocated to techsupport instance
    /// \param[in] techsupport_inst pointer to techsupport instance
    static void destroy(techsupport *techsupport_inst);

    /// \brief collects & tarballs techsupport
    /// \return 0 on success, error code on failure
    int collect_techsupport(void);

private:
    /// \brief constructor
    techsupport() {}

    /// \brief parameterized constructor
    techsupport(const techsupport_spec_t& spec) {
        cmd_file_ = spec.cmd_file;
        dst_dir_ = spec.dst_dir;
        dst_file_ = spec.ts_file;
        skip_core_ = spec.skip_core;
        fsink_ = NULL;
        tasks_.reserve(NO_OF_TASKS);
    }

    /// \brief destructor
    ~techsupport() {}

    /// \brief add tasks for collecting logs/cores
    void add_log_collection_tasks_(void);

    /// \brief bundle the files collected for techsupport
    void bundle_techsupport_(void);

    /// \brief run all tasks
    /// \return 0 on success, error code on failure
    int run_tasks_(void);

    /// \brief run single task and redirect to fsink
    /// \return 0 on success, error code on failure
    int execute_task_(const string& task);

    /// \brief parses json file and get list of tasks
    void process_command_file_(void);

    /// \brief process config & initialize techsupport instance
    void process_config_(void);

    /// \brief sets up a working directory
    void setup_working_dir_(void);

    /// \brief creates a file sink for on-the-fly compression
    void setup_fsink_(void);

    /// \brief sets up the pre-requisites for techsupport collection
    void setup_(void);

    /// \brief cleans up the temporary artefacts
    void teardown_(void);

private:
    string cmd_file_;         ///< json file with list of tasks
    string dst_dir_;          ///< destination directory for techsupport
    string dst_file_;         ///< gzipped techsupport file
    string work_dir_;         ///< working dir to copy out log files
    bool skip_core_;          ///< to skip core
    vector<string> tasks_;    ///< list of tasks
    vector<string> core_dir_; ///< core locations
    vector<string> log_dir_;  ///< log file locations
    gzFile fsink_;            ///< destination file sink

};

}    // namespace ts
}    // namespace sdk
#endif   // __TS_HPP__
