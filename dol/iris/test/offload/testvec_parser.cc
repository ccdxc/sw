#include "testvec_parser.hpp"

namespace tests {

testvec_parser_t::testvec_parser_t(const string& scripts_dir,
                                   const string& testvec_fname,
                                   token_parser_t& token_parser,
                                   const map<string,parser_token_id_t>& token2id_map) :
    token_parser(token_parser),
    token2id_map(token2id_map),
    line_consumed_(true),
    token_consumed_(true)
{
    string      full_fname;

    if (scripts_dir.empty()) {
        full_fname.assign("./" + testvec_fname);
    } else {
        full_fname.assign(scripts_dir + "/" + testvec_fname);
    }

    file.open(full_fname.c_str());
    if (!file.is_open()) {
        OFFL_FUNC_ERR("failed to open testvec file {}", full_fname);
    }
}

testvec_parser_t::~testvec_parser_t()
{
    if (file.is_open()) {
        file.close();
    }
}


/*
 * Parser runs free and returns the token type ID it sees at the current line,
 * which gives flexibility in how the caller wants to deal with any tokens
 * it gets at a given time.
 */
parser_token_id_t
testvec_parser_t::parse(testvec_parse_params_t params)
{
    parser_token_id_t   token_id;
    string              token;

    while (line_get()) {
        token = next_token_get();
        if (token.empty()) {
            line_consume_set();
            token_consume_set();
            continue;
        }

        token_id = token_id_find(token);
        if (token_id == PARSE_TOKEN_ID_UNKNOWN) {
            if (params.skip_unknown_token()) {
                OFFL_FUNC_DEBUG("skipping token {}", token);
                token_consume_set();
                continue;
            }
            return PARSE_TOKEN_ID_UNKNOWN;
        }

        token_consume_set();
        return token_id;
    }

    return PARSE_TOKEN_ID_EOF;
}


bool
testvec_parser_t::eof(void)
{
    return file.is_open() ? file.eof() : true;
}

/*
 * Get a new line and automatically strip whitespaces and discard
 * empty/comment lines.
 */
bool
testvec_parser_t::line_get(void)
{
    if (file.is_open()) {
        if (!line_consumed()) {
            return true;
        }

        while (getline(file, line)) {
            token_parser(line);

            /*
             * Discard empty or comment line
             */
            if (!token_parser.line_empty() && !token_parser.line_is_comment()) {
                line_consume_clr();
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
testvec_parser_t::next_token_get(void)
{
    if (token_consumed()) {
        next_token = token_parser.next_token_get();
        token_consume_clr();
    }
    return next_token;
}


/*
 * Parse hex big number.
 */
bool
testvec_parser_t::parse_hex_bn(dp_mem_t *bn)
{
    string      token;
    size_t      bn_len;
    int         ret;

    bn_len = bn->line_size_get();
    assert(bn_len);

    token = next_token_get();
    token_consume_set();

    if (!token.empty()) {

        /*
         * fips_parse_hex_bn() needs a little assistance to correctly parsed
         * an odd length hex string. Here we prepend a "0".
         */
        if (token.size() & 1) {
            token.assign("0" + token);
        }
        ret = fips_parse_hex_bn(token.c_str(), (char *)bn->read(), &bn_len);
        if (ret == 0) {
            bn->content_size_set(bn_len);
            bn->write_thru();
            return true;
        }
    }
    return false;
}


/*
 * Parse small number (can be dec/hex/octal).
 */
bool
testvec_parser_t::parse_ulong(u_long *n)
{
    const string&   token = next_token_get();
    const char      *sptr;
    char            *eptr;

    token_consume_set();

    sptr = token.c_str();
    *n = strtoul(sptr, &eptr, 0);
    return (*n != 0) || (sptr != eptr);
}

/*
 * Parse string
 */
bool
testvec_parser_t::parse_string(string *str)
{
    str->assign(next_token_get());
    token_consume_set();
    return !str->empty();
}


/*
 * Line parser
 */
void
token_parser_t::operator()(const string& arg_line)
{
     line.assign(arg_line);
     whitespaces_strip();
     //OFFL_FUNC_DEBUG("line {}", line);

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
    //OFFL_FUNC_DEBUG("token {}", token);
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


} // namespace tests

