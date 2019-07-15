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

class testvec_parser_t;

typedef bool (testvec_parser_t::*line_parse_fn_t)(void *dst);

/*
 * Simple token bit mask
 */
class parser_token_mask_t {

public:
    parser_token_mask_t() :
        mask(0)
    {
    }

    parser_token_mask_t(uint32_t num_bits)
    {
        fill(num_bits);
    }

    void set_bit(uint32_t bit)
    {
        assert(bit < (sizeof(mask) * BITS_PER_BYTE));
        mask |= (uint64_t)1 << bit;
    }

    void clr_bit(uint32_t bit)
    {
        assert(bit < (sizeof(mask) * BITS_PER_BYTE));
        mask &= ~((uint64_t)1 << bit);
    }

    bool tst_bit(uint32_t bit)
    {
        assert(bit < (sizeof(mask) * BITS_PER_BYTE));
        return !!(mask & ((uint64_t)1 << bit));
    }

    bool empty(void)
    {
        return mask == 0;
    }

    bool eq(parser_token_mask_t val)
    {
        return mask == val.mask;
    }

    void assign(parser_token_mask_t val)
    {
        mask = val.mask;
    }

    void fill(uint32_t num_bits)
    {
        assert(num_bits < (sizeof(mask) * BITS_PER_BYTE));
        mask = ((uint64_t)1 << num_bits) - 1;
    }

private:
    uint64_t                    mask;
};

/*
 * Token and its corresponding parse function
 */
class testvec_parse_token_t {

public:
    testvec_parse_token_t(const string& token,
                          line_parse_fn_t line_parse_fn,
                          void *dst) :
        token(token),
        line_parse_fn(line_parse_fn),
        dst(dst)
    {
    }

    friend class testvec_parser_t;

private:
    const string&               token;
    line_parse_fn_t             line_parse_fn;
    void                        *dst;
};

/*
 * Test vector parser
 */
class testvec_parser_t: protected fips_common_parser {

public:
    testvec_parser_t(string testvec_file);
    ~testvec_parser_t();

    parser_token_mask_t parse(vector<testvec_parse_token_t>& token_vec,
                              bool skip_unconsumed_line=false);
    bool eof(void);
    bool line_parse_hex_bn(void *dst);
    bool line_parse_ulong(void *dst);
    bool line_parse_string(void *dst);

private:
    bool get_line(void);
    void line_whitespaces_strip(void);

    ifstream            file;
    string              line;
    bool                line_consumed;
};

}  // namespace tests

#endif   // _TESTVEC_PARSER_HPP_
