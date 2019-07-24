#include "testvec_parser.hpp"

namespace tests {

const static map<string, parser_token_id_t>   token_str_id_map =
{
    {PARSE_TOKEN_STR_RESULT,    PARSE_TOKEN_ID_RESULT},
    {PARSE_TOKEN_STR_MODULUS,   PARSE_TOKEN_ID_MODULUS},
    {PARSE_TOKEN_STR_N,         PARSE_TOKEN_ID_N},
    {PARSE_TOKEN_STR_D,         PARSE_TOKEN_ID_D},
    {PARSE_TOKEN_STR_E,         PARSE_TOKEN_ID_E},
    {PARSE_TOKEN_STR_SHA_ALGO,  PARSE_TOKEN_ID_SHA_ALGO},
    {PARSE_TOKEN_STR_MSG,       PARSE_TOKEN_ID_MSG},
    {PARSE_TOKEN_STR_S,         PARSE_TOKEN_ID_S},
    {PARSE_TOKEN_STR_SALT_VAL,  PARSE_TOKEN_ID_SALT_VAL},
    {PARSE_TOKEN_STR_PRANDOM,   PARSE_TOKEN_ID_PRANDOM},
    {PARSE_TOKEN_STR_QRANDOM,   PARSE_TOKEN_ID_QRANDOM},
    {PARSE_TOKEN_STR_Qx,        PARSE_TOKEN_ID_Qx},
    {PARSE_TOKEN_STR_Qy,        PARSE_TOKEN_ID_Qy},
    {PARSE_TOKEN_STR_K,         PARSE_TOKEN_ID_K},
    {PARSE_TOKEN_STR_R,         PARSE_TOKEN_ID_R},
};

static inline parser_token_id_t
token_id_find(const string &token)
{
    auto iter = token_str_id_map.find(token);
    if (iter != token_str_id_map.end()) {
        return iter->second;
    }
    return PARSE_TOKEN_ID_UNKNOWN;
}

testvec_parser_t::testvec_parser_t(const string& scripts_dir,
                                   const string& testvec_fname) :
    line_consumed(true)
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
 * Method #1 of parsing:
 *
 * Parse from the current line for as many lines as there are
 * matches in the argument token_vec.
 */
parser_token_mask_t
testvec_parser_t::parse(vector<testvec_parse_token_t>& token_vec,
                        testvec_parse_params_t params)
{
    parser_token_mask_t token_mask;
    bool                success;
    parser_token_mask_t done_mask(token_vec.size());

    while (get_line()) {

        //OFFL_FUNC_DEBUG("line {}", line);
        for (uint32_t i = 0; i < token_vec.size(); i++) {
            auto entry = token_vec.at(i);

            if (line.compare(0, entry.token.length(), entry.token) == 0) {
                line_consumed = true;
                line = line.substr(entry.token.length(), string::npos);

                success = (this->*entry.line_parse_fn)(entry.dst);
                if (!success) {
                    OFFL_FUNC_ERR("failed to parse token {}", entry.token);
                    goto done;
                }

                token_mask.set_bit(i);
                break;
            }
        }

        if (line_consumed) {
            if (token_mask.eq(done_mask)) {
                break;
            }
        } else {

            /*
             * For unconsumed line, leave this parse invocation to let the
             * caller retry with a new set of tokens, unless we're told to
             * just skip the line.
             */
            if (!params.skip_unconsumed_line()) {
                break;
            }

            OFFL_FUNC_DEBUG("skipping line {}", line);
            line_consumed = true;
        }
    }

done:
    return token_mask;
}

/*
 * Method #2 of parsing:
 *
 * Parser runs free and returns the token type ID it sees at the current line,
 * which gives flexibility in how the caller wants to deal with any tokens
 * it gets at a given time.
 */
parser_token_id_t
testvec_parser_t::parse(testvec_parse_params_t params)
{
    parser_token_id_t   token_id;
    string              token;

    while (get_line()) {
        token = token_isolate();
        token_id = token_id_find(token);
        if (token_id == PARSE_TOKEN_ID_UNKNOWN) {
            if (params.skip_unconsumed_line()) {
                OFFL_FUNC_DEBUG("skipping line {}", line);
                line_consumed = true;
                continue;
            }
            return PARSE_TOKEN_ID_UNKNOWN;
        }

        /*
         * Move cursor past the token
         */
        line = line.substr(token.length(), string::npos);
        line_consumed = true;
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
 * For testvector parsing, white spaces include all the usual suspects
 * plus the square brackets (e.g., used by [mod = xyz]). 
 */
void
testvec_parser_t::line_whitespaces_strip(void)
{
    string      whitespaces(" []\f\t\v\r\n");
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

/*
 * Parse for a token (a sequence of characters terminated by
 * well known delimiters).
 */
string
testvec_parser_t::token_isolate(void)
{
    string      delimiters("=");
    string      token;
    size_t      matched_pos;

    /*
     * Line has already been stripped of whitespaces so token now
     * should be at the beginning of the line.
     */
    matched_pos = line.find_first_of(delimiters);
    if (matched_pos != string::npos) {

        /*
         * Isolate the token and also keep its delimiter
         */
        token.assign(line.substr(0, matched_pos + 1));
    }
    return token;
}

/*
 * Get a new line and automatically strip whitespaces and discard
 * empty/comment lines.
 */
bool
testvec_parser_t::get_line(void)
{
    if (file.is_open()) {
        if (!line_consumed) {
            return true;
        }

        while (getline(file, line)) {
            line_whitespaces_strip();

            /*
             * Discard empty or comment line
             */
            if (!line.empty() && (line.front() != '#')) {
                line_consumed = false;
                return true;
            }
        }

        file.close();
    }

    return false;
}

/*
 * Parse hex big number.
 */
bool
testvec_parser_t::line_parse_hex_bn(dp_mem_t *bn)
{
    size_t      bn_len;
    int         ret;

    bn_len = bn->line_size_get();
    assert(bn_len);

    /*
     * fips_parse_hex_bn() needs a little assistance to correctly parsed
     * an odd length hex string. Here we prepend a "0".
     */
    if (line.size() & 1) {
        line.assign("0" + line);
    }
    ret = fips_parse_hex_bn(line.c_str(), (char *)bn->read(), &bn_len);
    if (ret == 0) {
        bn->content_size_set(bn_len);
        bn->write_thru();
        return true;
    }
    return false;
}

bool
testvec_parser_t::line_parse_hex_bn(void *dst)
{
    return line_parse_hex_bn(static_cast<dp_mem_t *>(dst));
}

/*
 * Parse small number (can be dec/hex/octal).
 */
bool
testvec_parser_t::line_parse_ulong(u_long *n)
{
    char        *endptr;

    *n = strtoul(line.c_str(), &endptr, 0);
    return (*n != 0) || (line.c_str() != endptr);
}

bool
testvec_parser_t::line_parse_ulong(void *dst)
{
    return line_parse_ulong((u_long *)dst);
}

/*
 * Parse string
 */
bool
testvec_parser_t::line_parse_string(string *str)
{
    str->assign(line);
    return !str->empty();
}

bool
testvec_parser_t::line_parse_string(void *dst)
{
    return line_parse_string(static_cast<string *>(dst));
}

} // namespace tests
