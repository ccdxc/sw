#ifndef _TESTVEC_OUTPUT_HPP_
#define _TESTVEC_OUTPUT_HPP_

#include <stdint.h>
#include <string>
#include <vector>
#include <assert.h>
#include "logger.hpp"
#include "dp_mem.hpp"

using namespace std;
using namespace dp_mem;

namespace tests {

/*
 * Test vector output
 */
class testvec_output_t
{
public:
    testvec_output_t(const string& scripts_dir,
                     const string& testvec_fname,
                     const string& fname_suffix);
    ~testvec_output_t();

    void dec(const string& prefix,
             u_long val,
             const string& suffix="",
             bool eol=true);
    void str(const string& prefix,
             const string& val,
             const string& suffix="",
             bool eol=true);
    void text(const string& prefix,
              const string& text,
              const string& suffix="",
              bool eol=true);
    void text_vec(const string& prefix,
                  const vector<string>& text_vec,
                  const string& suffix="",
                  bool eol=true);
    void hex_bn(const string& prefix,
                dp_mem_t *val,
                const string& suffix="");

private:
    FILE                *fp;
};

}  // namespace tests

#endif   // _TESTVEC_PARSER_HPP_
