#ifndef _TESTVEC_PARSER_HPP_
#define _TESTVEC_PARSER_HPP_

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <assert.h>
#include "logger.hpp"
#include "dp_mem.hpp"
#include "fips_testvec_parser_common.h"

using namespace std;
using namespace dp_mem;

namespace tests {

enum {
    PARSE_TOKEN_ID_VOID,
    PARSE_TOKEN_ID_UNKNOWN,
    PARSE_TOKEN_ID_EOF,
    PARSE_TOKEN_ID_USER,
};

typedef int     parser_token_id_t;


/*
 * Token parser
 */
class token_parser_t
{
public:
    token_parser_t() :
        whitespaces(" \f\t\v\r\n"),
        delims(","),
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
    const string& next_token_get(void);

private:
    void whitespaces_strip(void);

    string              whitespaces;
    string              delims;
    string              line;
    string              token;
    size_t              curr_pos;
};


/*
 * Parse params
 */
class testvec_parse_params_t
{
public:

    testvec_parse_params_t() :
        skip_unknown_token_(false)
    {
    }

    testvec_parse_params_t&
    skip_unknown_token(bool skip_unknown_token)
    {
        skip_unknown_token_ = skip_unknown_token;
        return *this;
    }

    bool skip_unknown_token(void) { return skip_unknown_token_; }

private:
    bool                        skip_unknown_token_;
};


/*
 * Test vector parser
 */
class testvec_parser_t: protected fips_common_parser
{
public:
    testvec_parser_t(const string& scripts_dir,
                     const string& testvec_fname,
                     token_parser_t& token_parser,
                     const map<string,parser_token_id_t>& token2id_map);
    ~testvec_parser_t();

    parser_token_id_t parse(testvec_parse_params_t params);

    bool eof(void);
    bool parse_hex_bn(dp_mem_t *bn,
                      bool empty_ok=false);
    bool parse_ulong(u_long *n);
    bool parse_string(string *str);

    void line_consume_set(void) { line_consumed_ = true; }
    void token_consume_set(void) { token_consumed_ = true; }

private:
    bool line_get(void);
    const string& next_token_get(void);
    void line_consume_clr(void) { line_consumed_ = false; }
    void token_consume_clr(void) { token_consumed_ = false; }
    bool line_consumed(void) { return line_consumed_; }
    bool token_consumed(void) { return token_consumed_; }

    parser_token_id_t
    token_id_find(const string &token)
    {
        auto iter = token2id_map.find(token);
        if (iter != token2id_map.end()) {
            return iter->second;
        }
        return PARSE_TOKEN_ID_UNKNOWN;
    }

    token_parser_t&     token_parser;
    const map<string,parser_token_id_t>& token2id_map;
    ifstream            file;
    string              line;
    string              next_token;
    bool                line_consumed_;
    bool                token_consumed_;
};

}  // namespace tests

#endif   // _TESTVEC_PARSER_HPP_
