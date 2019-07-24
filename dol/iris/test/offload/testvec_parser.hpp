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

/*
 * Two methods of parsing are supported:
 * 1) Caller supplies a specific vector of token strings for which to parse.
 *    Depending on file position, different vectors may be supplied at
 *    different times.
 *
 * 2) Parser runs free and returns the token type ID it sees at the current
 *    line, which gives flexibility in how the caller wants to deal with any
 *    tokens it gets at a given time. For example, the caller can allocate
 *    a certain memory resource to handle the beginning of an expected token
 *    stream based on the returned token.
 */
typedef enum {
    PARSE_TOKEN_ID_VOID,
    PARSE_TOKEN_ID_UNKNOWN,
    PARSE_TOKEN_ID_EOF,
    PARSE_TOKEN_ID_RESULT,
    PARSE_TOKEN_ID_MODULUS,
    PARSE_TOKEN_ID_N,
    PARSE_TOKEN_ID_D,
    PARSE_TOKEN_ID_E,
    PARSE_TOKEN_ID_SHA_ALGO,
    PARSE_TOKEN_ID_MSG,
    PARSE_TOKEN_ID_S,
    PARSE_TOKEN_ID_SALT_VAL,
    PARSE_TOKEN_ID_PRANDOM,
    PARSE_TOKEN_ID_QRANDOM,
    PARSE_TOKEN_ID_Qx,
    PARSE_TOKEN_ID_Qy,
    PARSE_TOKEN_ID_K,
    PARSE_TOKEN_ID_R,
} parser_token_id_t;

/*
 * The following string tokens assume the parser has stripped off
 * all whitespaces (including the [] characters) from the current
 * line prior to parsing.
 */
#define PARSE_TOKEN_STR_RESULT          "Result="
#define PARSE_TOKEN_STR_MODULUS         "mod="
#define PARSE_TOKEN_STR_N               "n="
#define PARSE_TOKEN_STR_D               "d="
#define PARSE_TOKEN_STR_E               "e="
#define PARSE_TOKEN_STR_SHA_ALGO        "SHAAlg="
#define PARSE_TOKEN_STR_MSG             "Msg="
#define PARSE_TOKEN_STR_S               "S="
#define PARSE_TOKEN_STR_SALT_VAL        "SaltVal="
#define PARSE_TOKEN_STR_PRANDOM         "prandom="
#define PARSE_TOKEN_STR_QRANDOM         "qrandom="
#define PARSE_TOKEN_STR_Qx              "Qx="
#define PARSE_TOKEN_STR_Qy              "Qy="
#define PARSE_TOKEN_STR_K               "k="
#define PARSE_TOKEN_STR_R               "R="

/*
 * Unstripped versions
 */
#define PARSE_STR_RESULT_PREFIX         "Result = "
#define PARSE_STR_RESULT_SUFFIX         "\n"
#define PARSE_STR_MODULUS_PREFIX        "[ mod = "
#define PARSE_STR_MODULUS_SUFFIX        "]\n"
#define PARSE_STR_N_PREFIX              "n = "
#define PARSE_STR_N_SUFFIX              "\n"
#define PARSE_STR_D_PREFIX              "d = "
#define PARSE_STR_D_SUFFIX              "\n"
#define PARSE_STR_E_PREFIX              "e = "
#define PARSE_STR_SHA_ALGO_PREFIX       "SHAAlg = "
#define PARSE_STR_MSG_PREFIX            "Msg = "
#define PARSE_STR_S_PREFIX              "S = "
#define PARSE_STR_S_SUFFIX              "\n"
#define PARSE_STR_SALT_VAL_PREFIX       "SaltVal = "
#define PARSE_STR_PRANDOM_PREFIX        "prandom = "
#define PARSE_STR_QRANDOM_PREFIX        "qrandom = "
#define PARSE_STR_Qx_PREFIX             "Qx = "
#define PARSE_STR_Qy_PREFIX             "Qy = "
#define PARSE_STR_K_PREFIX              "K = "
#define PARSE_STR_R_PREFIX              "R = "

class testvec_parser_t;

typedef bool (testvec_parser_t::*line_parse_fn_t)(void *dst);

/*
 * Simple token bit mask
 */
class parser_token_mask_t
{
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
 * Parse params
 */
class testvec_parse_params_t
{
public:

    testvec_parse_params_t() :
        skip_unconsumed_line_(false)
    {
    }

    testvec_parse_params_t&
    skip_unconsumed_line(bool skip_unconsumed_line)
    {
        skip_unconsumed_line_ = skip_unconsumed_line;
        return *this;
    }

    bool skip_unconsumed_line(void) { return skip_unconsumed_line_; }

private:
    bool                        skip_unconsumed_line_;
};


/*
 * Token and its corresponding parse function
 */
class testvec_parse_token_t
{
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
class testvec_parser_t: protected fips_common_parser
{
public:
    testvec_parser_t(const string& scripts_dir,
                     const string& testvec_fname);
    ~testvec_parser_t();

    // Method #1 of parsing
    parser_token_mask_t parse(vector<testvec_parse_token_t>& token_vec,
                              testvec_parse_params_t params);
    // Method #2 of parsing
    parser_token_id_t parse(testvec_parse_params_t params);

    bool eof(void);
    bool line_parse_hex_bn(dp_mem_t *bn);
    bool line_parse_hex_bn(void *dst);
    bool line_parse_ulong(u_long *n);
    bool line_parse_ulong(void *dst);
    bool line_parse_string(string *str);
    bool line_parse_string(void *dst);

private:
    string token_isolate(void);
    bool get_line(void);
    void line_whitespaces_strip(void);

    ifstream            file;
    string              line;
    bool                line_consumed;
};

}  // namespace tests

#endif   // _TESTVEC_PARSER_HPP_
