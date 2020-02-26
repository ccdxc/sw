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
    string ts_file;     ///< gzipped techsupport filename with absolute path
} techsupport_spec_t;

class techsupport {
public:
    /// \brief factory method to allocate & initialize techsupport instance
    /// \return new instance of techsupport or NULL, in case of error
    static techsupport *factory(const techsupport_spec_t& spec);

    /// \brief free memory allocated to techsupport instance
    /// \param[in] techsupport_inst pointer to techsupport instance
    static void destroy(techsupport *techsupport_inst);

    /// \brief run all tasks
    /// \return 0 on success, error code on failure
    int run_tasks(void);

private:
    /// \brief constructor
    techsupport() {}

    /// \brief parameterized constructor
    techsupport(const techsupport_spec_t& spec) {
        cmd_file_ = spec.cmd_file;
        dst_file_ = spec.ts_file;
        fsink_ = NULL;
        tasks_.reserve(NO_OF_TASKS);
    }

    /// \brief destructor
    ~techsupport() {}

    /// \brief run single task and redirect to fsink
    /// \return 0 on success, error code on failure
    int execute_task_(const string& task);

    /// \brief parses json file and get list of tasks
    void process_command_file_(void);

    /// \brief process config & initialize techsupport instance
    void process_config_(void);

private:
    string cmd_file_;         ///< json file with list of tasks
    string dst_file_;         ///< gzipped techsupport file
    vector<string> tasks_;    ///< list of tasks
    gzFile fsink_;            ///< destination file sink

};

}    // namespace ts
}    // namespace sdk
#endif   // __TS_HPP__
