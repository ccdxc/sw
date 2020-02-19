//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains a tokenizer and parser
///
//----------------------------------------------------------------------------

#include "app_test.hpp"
#include "script_parser.hpp"

namespace test {
namespace athena_app {

script_parser_t::script_parser_t(const string& scripts_dir,
                                 const string& script_fname,
                                 token_parser_t& token_parser) :
    token_parser(token_parser)
{
    string      full_fname;

    if (scripts_dir.empty()) {
        full_fname.assign("./" + script_fname);
    } else {
        full_fname.assign(scripts_dir + "/" + script_fname);
    }

    file.open(full_fname.c_str());
    if (!file.is_open()) {
        TEST_LOG_INFO("failed to open script file %s\n", full_fname.c_str());
    }
}

script_parser_t::~script_parser_t()
{
    if (file.is_open()) {
        file.close();
    }
}

string
script_parser_t::parse(void)
{
    string              token;

    while (line_get()) {
        token = next_token_get();
        if (!token.empty()) {
            break;
        }
    }
    return token;
}

bool
script_parser_t::eof(void)
{
    return file.is_open() ? file.eof() : true;
}

/*
 * Get a new line and automatically strip whitespaces and discard
 * empty/comment lines.
 */
bool
script_parser_t::line_get(void)
{
    if (file.is_open()) {
        while (getline(file, line)) {
            token_parser(line);
            token_parser.whitespaces_strip();
            if (!token_parser.line_empty() && !token_parser.line_is_comment()) {
                return true;
            }
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
    next_token = token_parser.next_token_get();
    return next_token;
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
     * next token begins at curr_pos until the 1st delimiter found,
     * or until end of line.
     */
    while (curr_pos < line.size()) {
        matched_pos = line.find_first_of(delims, curr_pos);
        if (matched_pos == string::npos) {
            token.assign(line.substr(curr_pos));
            curr_pos = line.size();
            break;
        }

        token.assign(line.substr(curr_pos, matched_pos - curr_pos));
        curr_pos = matched_pos + 1;
        if (!token.empty()) {
            break;
        }
    }

    /*
     * An empty token on return means end of line
     */
    return token;
}

/*
 * Strip all whitespaces from line to make parsing easier.
 */
void
token_parser_t::whitespaces_strip(void)
{
    string      stripped_line;
    size_t      matched_pos;
    size_t      pos = 0;

    while (true) {
        matched_pos = line.find_first_of(whitespaces, pos);
        if (matched_pos == string::npos) {
            stripped_line.append(line.substr(pos, string::npos));
            break;
        }

        stripped_line.append(line.substr(pos, matched_pos - pos));
        pos = line.find_first_not_of(whitespaces, matched_pos);
        if (pos == string::npos) {
            break;
        }
    }

    line.assign(stripped_line);
}


}    // namespace athena_app
}    // namespace test

