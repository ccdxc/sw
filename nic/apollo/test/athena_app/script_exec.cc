//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains script execution capability
///
//----------------------------------------------------------------------------

#include "app_test_utils.hpp"
#include "script_parser.hpp"
#include "session_aging.hpp"
#include "conntrack_aging.hpp"
#include "combined_aging.hpp"

namespace test {
namespace athena_app {

const static map<string,test_fn_t>  name2fn_map =
{
    SESSION_AGING_NAME2FN_MAP
    CONNTRACK_AGING_NAME2FN_MAP
    COMBINED_AGING_NAME2FN_MAP
    APP_TEST_NAME2FN_MAP_ENTRY(APP_TEST_EXIT_FN),
};

class test_entry_t
{
public:
    test_entry_t(const string test_name,
                 test_fn_t test_fn,
                 test_vparam_t vparam) :
        test_name(test_name),
        test_fn(test_fn),
        vparam(vparam),
        test_success(false)
    {
    }

    string                      test_name;
    test_fn_t                   test_fn;
    test_vparam_t               vparam;
    bool                        test_success;
};

static vector<test_entry_t>     test_suite;

static test_fn_t
name2fn_find(const string &token)
{
    auto iter = name2fn_map.find(token);
    if (iter != name2fn_map.end()) {
        return iter->second;
    }
    return nullptr;
}

void
timeval_delta(struct timeval& end,
              const struct timeval& start)
{
    uint64_t end_us = ((uint64_t)end.tv_sec * USEC_PER_SEC) +
                      (uint64_t)end.tv_usec;
    uint64_t start_us = ((uint64_t)start.tv_sec * USEC_PER_SEC) +
                        (uint64_t)start.tv_usec;

    end.tv_sec = 0;
    end.tv_usec = 0;
    if (end_us > start_us) {
        end_us -= start_us;
        end.tv_sec = end_us / USEC_PER_SEC;
        end.tv_usec = end_us % USEC_PER_SEC;
    }
}

sdk_ret_t
script_exec(const string& scripts_dir,
            const string& script_fname)
{
    script_parser_t     *script_parser;
    test_fn_t           test_fn;
    token_parser_t      token_parser;
    string              test_name;
    test_vparam_t       vparam;
    test_param_tuple_t  tuple;
    token_type_t        token_type;
    struct timeval      start;
    struct timeval      delta;
    size_t              tcid;
    bool                in_tuple;
    bool                has_app_exit;
    bool                overall_success;
    sdk_ret_t           ret = SDK_RET_INVALID_ARG;

    script_parser = new script_parser_t(scripts_dir, script_fname,
                                        token_parser);
    while (!script_parser->eof()) {
        token_type = script_parser->parse();
        if (token_type == TOKEN_TYPE_EOF) {
            break;
        }
        if (token_type == TOKEN_TYPE_EOL) {
            continue;
        }

        // First token should be a test name
        if ((token_type != TOKEN_TYPE_STR) ||
            !script_parser->parse_str(&test_name)) {

            TEST_LOG_ERR("Script line %u: first token must be a valid "
                         "test function name\n", script_parser->line_no());
            goto done;
        }

        // loop and build params vector
        in_tuple = false;
        while (!script_parser->eof()) {
            token_type = script_parser->parse();
            if ((token_type == TOKEN_TYPE_EOL) ||
                (token_type == TOKEN_TYPE_EOF)) {

                if (in_tuple) {
                    TEST_LOG_ERR("Script line %u: missing tuple terminator\n",
                                 script_parser->line_no());
                    goto done;
                }
                break;
            }

            switch (token_type) {

            case TOKEN_TYPE_TUPLE_BEGIN: {
                if (in_tuple) {
                    TEST_LOG_ERR("Script line %u: tuple cannot be nested\n",
                                 script_parser->line_no());
                    goto done;
                }
                in_tuple = true;
                break;
            }

            case TOKEN_TYPE_TUPLE_END: {
                if (!in_tuple) {
                    TEST_LOG_ERR("Script line %u: out of place tuple terminator\n",
                                 script_parser->line_no());
                    goto done;
                }
                vparam.push_back(test_param_t(tuple));
                tuple.clear();
                in_tuple = false;
                break;
            }
            case TOKEN_TYPE_STR: {
                string      param_str;

                if (!script_parser->parse_str(&param_str)) {
                    TEST_LOG_ERR("Script line %u: invalid string parameter\n",
                                 script_parser->line_no());
                    goto done;
                }
                if (in_tuple) {
                    tuple.push_back(test_param_t(param_str));
                } else {
                    vparam.push_back(test_param_t(param_str));
                }
                break;
            }

            case TOKEN_TYPE_NUM: {
                uint32_t    param_num;

                if (!script_parser->parse_num(&param_num)) {
                    TEST_LOG_ERR("Script line %u: invalid numeric parameter\n",
                                 script_parser->line_no());
                    goto done;
                }
                if (in_tuple) {
                    tuple.push_back(test_param_t(param_num));
                } else {
                    vparam.push_back(test_param_t(param_num));
                }
                break;
            }

            default:
                break;
            }
        }

        test_fn = name2fn_find(test_name);
        if (!test_fn) {
            test_name.append(" (null)");
        }
        test_suite.push_back(test_entry_t(test_name, test_fn, vparam));
        tuple.clear();
        vparam.clear();
    }

    delete script_parser;
    script_parser = nullptr;
    ret = SDK_RET_OK;

    if (test_suite.size() == 0) {
        TEST_LOG_INFO("Script file not present or is empty; "
                      "testing is hereby skipped\n");
        goto done;
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
                   test_entry.test_fn(test_entry.vparam) : true;

        gettimeofday(&delta, NULL);
        timeval_delta(delta, start);
        TEST_LOG_INFO(" Finished test #: %d name: %s status %u time %u secs %u usecs\n",
                      (int)tcid, test_entry.test_name.c_str(),
                      test_entry.test_success, (unsigned)delta.tv_sec,
                      (unsigned)delta.tv_usec);
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

        vparam.clear();
        vparam.push_back(test_param_t((uint32_t)overall_success));
        test_entry.test_fn(vparam);
    }

done:
    if (script_parser) {
        delete script_parser;
    }
    return ret;
}

}    // namespace athena_app
}    // namespace test

