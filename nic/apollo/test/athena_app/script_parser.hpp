#ifndef __TEST_ATHENA_SCRIPT_PARSER_HPP__
#define __TEST_ATHENA_SCRIPT_PARSER_HPP__

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>

using namespace std;

namespace test {
namespace athena_app {

/*
 * Token parser
 */
class token_parser_t
{
public:
    token_parser_t() :
        whitespaces(" \f\t\v\r\n"),
        delims(";"),
        curr_pos(0)
    {
    }

    ~token_parser_t()
    {
    }

    void extra_whitespaces_add(const string& arg_whitespaces)
    {
        whitespaces.append(arg_whitespaces);
    }

    void extra_delims_add(const string& arg_delims)
    {
         delims.append(arg_delims);
    }

    bool line_empty(void)
    {
        return line.empty();
    }

    bool line_is_comment(void)
    {
        return !line.empty() &&
               ((line.front() == '#') || (line.substr(0, 2) == "//"));
    }

    void operator()(const string& arg_line);
    void whitespaces_strip(void);
    const string& next_token_get(void);

private:

    string              whitespaces;
    string              delims;
    string              line;
    string              token;
    size_t              curr_pos;
};


/*
 * Parser
 */
class script_parser_t
{
public:
    script_parser_t(const string& scripts_dir,
                     const string& testvec_fname,
                     token_parser_t& token_parser);
    ~script_parser_t();

    string parse(void);
    bool eof(void);

private:
    bool line_get(void);
    const string& next_token_get(void);

    token_parser_t&     token_parser;

    ifstream            file;
    string              line;
    string              next_token;
};

}    // namespace athena_app
}    // namespace test

#endif   // __TEST_ATHENA_SCRIPT_PARSER_HPP__
