#include "testvec_parser.hpp"

namespace tests {

testvec_parser_t::testvec_parser_t(string testvec_file) :
    file(testvec_file),
    line_consumed(true)
{
    if (!file.is_open()) {
        OFFL_FUNC_ERR("failed to open testvec file {}", testvec_file);
    }
}

testvec_parser_t::~testvec_parser_t()
{
    if (file.is_open()) {
        file.close();
    }
}

/*
 * Parse from the current line for as many lines as there are
 * matches in the argument token_vec.
 */
parser_token_mask_t
testvec_parser_t::parse(vector<testvec_parse_token_t>& token_vec,
                        bool skip_unconsumed_line)
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
            if (!skip_unconsumed_line) {
                break;
            }

            OFFL_FUNC_DEBUG("skipping line {}", line);
            line_consumed = true;
        }
    }

done:
    return token_mask;
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
testvec_parser_t::line_parse_hex_bn(void *dst)
{
    dp_mem_t    *bn = static_cast<dp_mem_t *>(dst);
    size_t      bn_len;
    int         ret;

    bn_len = bn->line_size_get();
    assert(bn_len);
    ret = fips_parse_hex_bn(line.c_str(), (char *)bn->read(), &bn_len);
    if (ret == 0) {
        bn->content_size_set(bn_len);
        bn->write_thru();
        return true;
    }
    return false;
}

/*
 * Parse small number (can be dec/hex/octal).
 */
bool
testvec_parser_t::line_parse_ulong(void *dst)
{
    u_long      *n = (u_long *)dst;
    char        *endptr;

    *n = strtoul(line.c_str(), &endptr, 0);
    return (*n != 0) || (line.c_str() != endptr);
}

/*
 * Parse string
 */
bool
testvec_parser_t::line_parse_string(void *dst)
{
    string      *str = static_cast<string *>(dst);

    str->assign(line);
    return !str->empty();
}

} // namespace tests
