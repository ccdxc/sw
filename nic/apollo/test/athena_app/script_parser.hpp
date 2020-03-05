#ifndef __TEST_ATHENA_SCRIPT_PARSER_HPP__
#define __TEST_ATHENA_SCRIPT_PARSER_HPP__

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <ctype.h>

using namespace std;

namespace test {
namespace athena_app {

typedef enum {
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_EOL,
    TOKEN_TYPE_TUPLE_BEGIN,
    TOKEN_TYPE_TUPLE_END,
    TOKEN_TYPE_NUM,
    TOKEN_TYPE_STR,
} token_type_t;

/*
 * Token parser
 */
class token_parser_t
{
public:
    token_parser_t() :
        whitespaces(" \f\t\v"),
        tuple_begin("{"),
        tuple_end("}"),
        eol_delims(";\r\n"),
        curr_pos(0)
    {
        combined_delims.assign(whitespaces + tuple_begin + tuple_end);
    }

    ~token_parser_t()
    {
    }

    void extra_whitespaces_add(const string& arg_whitespaces)
    {
        whitespaces.append(arg_whitespaces);
        combined_delims.assign(whitespaces + tuple_begin + tuple_end);
    }

    void extra_eol_delims_add(const string& arg_delims)
    {
         eol_delims.append(arg_delims);
    }

    bool is_comment(void)
    {
        return !line.empty() &&
            ((line.at(curr_pos) == '#') || (line.substr(curr_pos, 2) == "//"));
    }

    bool is_eol(const string& token)
    {
        return token.empty() || 
               (token.find_first_of(eol_delims) == 0);
    }

    bool is_tuple_begin(const string& token)
    {
        return !token.empty() && 
               (token.find_first_of(tuple_begin) == 0);
    }

    bool is_tuple_end(const string& token)
    {
        return !token.empty() &&
               (token.find_first_of(tuple_end) == 0);
    }

    bool is_num(const string& token)
    {
        return !token.empty() && isdigit(token.at(0));
    }

    void operator()(const string& arg_line);
    void whitespaces_skip(void);
    const string& next_token_get(void);

private:

    string              whitespaces;
    string              tuple_begin;
    string              tuple_end;
    string              combined_delims;
    string              eol_delims;
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

    token_type_t parse(void);
    bool eof(void);
    bool parse_num(uint32_t *ret_num);
    bool parse_str(string *ret_str);

    uint32_t line_no(void) { return line_no_; }
    void line_consume_set(void) { line_consumed_ = true; }
    void token_consume_set(void) { token_consumed_ = true; }

private:
    bool line_get(void);
    const string& next_token_get(void);

    void line_consume_clr(void) { line_consumed_ = false; }
    void token_consume_clr(void) { token_consumed_ = false; }
    bool line_consumed(void) { return line_consumed_; }
    bool token_consumed(void) { return token_consumed_; }

    token_parser_t&     token_parser;

    ifstream            file;
    string              line;
    string              next_token;
    uint32_t            line_no_;
    bool                line_consumed_;
    bool                token_consumed_;
};

}    // namespace athena_app
}    // namespace test

#endif   // __TEST_ATHENA_SCRIPT_PARSER_HPP__
