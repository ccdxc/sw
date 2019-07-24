#include "testvec_output.hpp"
#include <sys/stat.h>

namespace tests {

const static string     dole_rsp_subdir_name = "dole_rsp";

testvec_output_t::testvec_output_t(const string& scripts_dir,
                                   const string& testvec_fname,
                                   const string& mem_type_str) :
    fp(nullptr)
{
    string      dole_rsp_fulldir;
    string      dole_rsp_subdir;
    string      fname_only;
    string      output_fname;
    struct stat dir_stat;
    size_t      find_pos;

    /*
     * Extract the last subdir component
     */
    fname_only = testvec_fname;
    find_pos = testvec_fname.find_last_of("/");
    if (find_pos != string::npos) {
        dole_rsp_subdir = testvec_fname.substr(0, find_pos + 1);
        fname_only = testvec_fname.substr(find_pos + 1, string::npos);
    }
    dole_rsp_subdir = dole_rsp_subdir + dole_rsp_subdir_name;

    /*
     * Create response directory if necessary
     */
    dole_rsp_fulldir = scripts_dir.empty() ? 
                       "./" + dole_rsp_subdir :
                       scripts_dir + "/" + dole_rsp_subdir;
    if (stat(dole_rsp_fulldir.c_str(), &dir_stat)) {
        if (mkdir(dole_rsp_fulldir.c_str(),
                  S_IRWXU | S_IRWXG | S_IRWXO)) {
            OFFL_FUNC_ERR("failed to create directory {}",
                          dole_rsp_fulldir);
            assert(0);
        }
    }

    /*
     * Replace .txt/req/rsp with .rsp in fname_only
     */
    find_pos = fname_only.find_last_of(".");
    if ((find_pos != string::npos) &&
        ((fname_only.compare(find_pos, string::npos, ".txt") == 0) ||
         (fname_only.compare(find_pos, string::npos, ".req") == 0) ||
         (fname_only.compare(find_pos, string::npos, ".rsp") == 0))) {

        output_fname = fname_only.substr(0, find_pos); 
    } else {
        output_fname = fname_only;
    }
    output_fname = mem_type_str.empty() ?
                   output_fname + ".rsp" :
                   output_fname + "-" + mem_type_str + ".rsp";
    output_fname = dole_rsp_fulldir + "/" + output_fname;
    OFFL_FUNC_INFO("creating {}", output_fname);

    fp = fopen(output_fname.c_str(), "w");
    if (!fp) {
        OFFL_FUNC_ERR("failed to create testvec output file {}",
                      output_fname);
        assert(fp);
    }
}

testvec_output_t::~testvec_output_t()
{
    if (fp) {
        fclose(fp);
    }
}

void
testvec_output_t::dec(const string& prefix,
                      u_long val,
                      const string& suffix)
{
    if (fp) {
        fprintf(fp, "%s%lu%s\n", prefix.c_str(), val, suffix.c_str());
    }
}

void
testvec_output_t::str(const string& prefix,
                      const string& val,
                      const string& suffix)
{
    if (fp) {
        fprintf(fp, "%s%s%s\n", prefix.c_str(), val.c_str(), suffix.c_str());
    }
}

void
testvec_output_t::hex_bn(const string& prefix,
                         dp_mem_t *val,
                         const string& suffix)
{
    uint8_t     *buf;
    uint32_t    len;

    if (fp) {
        buf = val->read_thru();
        len = val->content_size_get();
        fprintf(fp, "%s", prefix.c_str());
        while (len--) {
            fprintf(fp, "%02x", *buf++);
        }
        fprintf(fp, "%s\n", suffix.c_str());
    }
}

} // namespace tests
