//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains a tokenizer and parser
///
//----------------------------------------------------------------------------

#include "app_test_utils.hpp"
#include "script_parser.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

namespace test {
namespace athena_app {

script_parser_t::script_parser_t(const string& scripts_dir,
                                 const string& script_fname,
                                 token_parser_t& token_parser) :
    token_parser(token_parser),
    line_no_(0),
    line_consumed_(true),
    token_consumed_(true)
{
    string      full_fname;

    if (scripts_dir.empty()) {
        if (!script_fname.empty() && script_fname.front() == '/') {
            full_fname.assign(script_fname);
        } else {
            full_fname.assign("./" + script_fname);
        }
    } else {
        full_fname.assign(scripts_dir + "/" + script_fname);
    }

    file.open(full_fname.c_str());
    if (!file.is_open()) {
        TEST_LOG_ERR("failed to open script file %s\n", full_fname.c_str());
    }
}

script_parser_t::~script_parser_t()
{
    if (file.is_open()) {
        file.close();
    }
}

token_type_t
script_parser_t::parse(void)
{
    string              token;

    while (line_get()) {
        token = next_token_get();
        if (token_parser.is_eol(token)) {
            line_consume_set();
            token_consume_set();
            return TOKEN_TYPE_EOL;
        }
        if (token_parser.is_tuple_begin(token)) {
            token_consume_set();
            return TOKEN_TYPE_TUPLE_BEGIN;
        }
        if (token_parser.is_tuple_end(token)) {
            token_consume_set();
            return TOKEN_TYPE_TUPLE_END;
        }
        if (token_parser.is_num(token)) {
            return TOKEN_TYPE_NUM;
        }
        return TOKEN_TYPE_STR;
    }

    return TOKEN_TYPE_EOF;
}

bool
script_parser_t::eof(void)
{
    return file.is_open() ? file.eof() : true;
}

/*
 * Get a new line and automatically skip whitespaces and discard
 * empty/comment lines.
 */
bool
script_parser_t::line_get(void)
{
    if (file.is_open()) {
        if (!line_consumed()) {
            return true;
        }

        if (getline(file, line)) {
            line_no_++;
            token_parser(line);
            line_consume_clr();
            return true;
        }

        file.close();
    }

    return false;
}

/*
 * Get the next token from the current line.
 */
const string&
script_parser_t::next_token_get(void)
{
    if (token_consumed()) {
        next_token = token_parser.next_token_get();
        token_consume_clr();
    }
    return next_token;
}

/*
 * Parse small number (can be dec/hex/octal).
 */
bool
script_parser_t::parse_num(uint32_t *ret_num)
{
    const string&   token = next_token_get();
    const char      *sptr;
    char            *eptr;
    u_long          n;
    ipv4_addr_t     ipv4_addr;

    token_consume_set();
    sptr = token.c_str();

    /*
     * Include support for IPv4 address conversion (dot notation)
     */
    n = strtoul(sptr, &eptr, 0);
    *ret_num = n;
    if ((n == 0) && ((sptr == eptr))) {
        return false;
    }

    if (*eptr != '.') {
        return true;
    }

    if (str2ipv4addr(sptr, &ipv4_addr) < 0) {
        return false;
    }

    *ret_num = ipv4_addr;
    return true;
}

/*
 * Parse string
 */
bool
script_parser_t::parse_str(string *ret_str)
{
    ret_str->assign(next_token_get());
    token_consume_set();
    return !ret_str->empty();
}

/*
 * Token parser
 */
void
token_parser_t::operator()(const string& arg_line)
{
     line.assign(arg_line);
     token.clear();
     curr_pos = 0;
}

const string&
token_parser_t::next_token_get(void)
{
    size_t      matched_pos;

    token.clear();

    /*
     * next token begins at curr_pos until the 1st delim found,
     * or until end of {line}.
     */
    whitespaces_skip();
    if (!is_comment()) {
        while (curr_pos < line.size()) {
            matched_pos = line.find_first_of(combined_delims, curr_pos);
            if (matched_pos != string::npos) {

                /*
                 * Either found a delimiter or a whitespaces char
                 */
                if (matched_pos == curr_pos) {
                    matched_pos++;
                }
                token.assign(line.substr(curr_pos, matched_pos - curr_pos));
                curr_pos = matched_pos;
                break;
            }

            /*
             * Since whitespaces were initially skipped on function entry, the
             * next token had to have ended with a delimiter (above) or EOL.
             * If not, then the rest of the line constitutes the next token.
             */
            matched_pos = line.find_first_of(eol_delims, curr_pos);
            if (matched_pos == string::npos) {
                token.assign(line.substr(curr_pos));
                curr_pos = line.size();
                break;
            }

            /*
             * EOL, if found, is also returned as a token
             */
            if (matched_pos == curr_pos) {
                matched_pos++;
            }
            token.assign(line.substr(curr_pos, matched_pos - curr_pos));
            curr_pos = matched_pos;
            break;
        }
    }

    /*
     * An empty token on return also means end of line
     */
    return token;
}

/*
 * Skip all whitespaces from line to get to next token
 */
void
token_parser_t::whitespaces_skip(void)
{
    curr_pos = line.find_first_not_of(whitespaces, curr_pos);
    if (curr_pos == string::npos) {
        line.clear();
    }
}

}    // namespace athena_app
}    // namespace test

