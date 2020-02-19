//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains script execution capability
///
//----------------------------------------------------------------------------

#include "app_test.hpp"
#include "script_parser.hpp"
#include "session_aging.hpp"

namespace test {
namespace athena_app {

const static map<string,app_test_fn_with_param_t> name2fn_map =
{
    SESSION_AGING_NAME2FN_MAP
    APP_TEST_NAME2FN_MAP_ENTRY_NULL_PARAM(APP_TEST_EXIT_FN),
};

class test_entry_t
{
public:
    test_entry_t(const string test_name,
                 app_test_fn_t test_fn,
                 void *test_param) :
        test_name(test_name),
        test_fn(test_fn),
        test_param(test_param),
        test_success(false)
    {
    }

    string                      test_name;
    app_test_fn_t               test_fn;
    void                        *test_param;
    bool                        test_success;
};

static vector<test_entry_t>     test_suite;

static const app_test_fn_with_param_t *
name2fn_find(const string &token)
{
    auto iter = name2fn_map.find(token);
    if (iter != name2fn_map.end()) {
        return &iter->second;
    }
    return nullptr;
}

void
script_exec(const string& scripts_dir,
            const string& script_fname)
{
    script_parser_t     *script_parser;
    const app_test_fn_with_param_t *test_fn;
    token_parser_t      token_parser;
    string              test_name;
    struct timeval      start;
    struct timeval      end;
    size_t              tcid;
    bool                has_app_exit;
    bool                overall_success;

    script_parser = new script_parser_t(scripts_dir, script_fname,
                                        token_parser);    while (!script_parser->eof()) {
        test_name = script_parser->parse();
        if (test_name.empty()) {
            break;
        }
        test_fn = name2fn_find(test_name);
        if (test_fn && test_fn->first) {
            test_suite.push_back(test_entry_t(test_name, test_fn->first,
                                              test_fn->second));
        } else {
            test_name.append(" (null)");
            test_suite.push_back(test_entry_t(test_name, nullptr, nullptr));
        }
    }
    delete script_parser;

    if (test_suite.size() == 0) {
        TEST_LOG_INFO("Script file not present or is empty; "
                      "testing is hereby skipped\n");
        return;
    }

    has_app_exit = false;
    overall_success = true;
    for (tcid = 0; tcid < test_suite.size(); tcid++) {
        test_entry_t& test_entry = test_suite.at(tcid);

        /*
         * app_exit is a special test case and is to be run outside
         * of all script loops. It also designates the last test function
         * wherever it is encountered in the script.
         */
        if (test_entry.test_name == APP_TEST_EXIT_FN_STR) {
            has_app_exit = true;
            break;
        }

        gettimeofday(&start, NULL);
        TEST_LOG_INFO(" Starting test #: %d name: %s\n", (int)tcid,
                      test_entry.test_name.c_str());
        test_entry.test_success = test_entry.test_fn ? 
                   test_entry.test_fn(test_entry.test_param) : true;

        gettimeofday(&end, NULL);
        TEST_LOG_INFO(" Finished test #: %d name: %s status %u time %u\n",
                      (int)tcid, test_entry.test_name.c_str(),
                      test_entry.test_success, (unsigned)(end.tv_sec - start.tv_sec));
        overall_success &= test_entry.test_success;
    }

    TEST_LOG_INFO("\nConsolidated Test Report\n");
    TEST_LOG_INFO("-----------------------------------------------------------------------\n");
    TEST_LOG_INFO("Number\t\t%-40s\tResult\n\n", "Name");
    TEST_LOG_INFO("-----------------------------------------------------------------------\n");

    for (size_t i = 0; i < tcid; i++) {
        test_entry_t& test_entry = test_suite.at(i);
        TEST_LOG_INFO("%d\t\t%-40s\t%s\n", (int)i,
                      test_entry.test_name.c_str(),
                      test_entry.test_success ? "Success" : "Failure");
    }
    TEST_LOG_INFO("\nOverall Report: %s\n",
                  overall_success ? "SUCCESS" : "FAILURE");
    if (has_app_exit) {
        test_entry_t& test_entry = test_suite.at(tcid);
        test_entry.test_fn((void *)&overall_success);
    }
}

}    // namespace athena_app
}    // namespace test

