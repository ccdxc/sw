#ifndef _FIPS_RSA_TESTVEC_PARSER_H_
#define _FIPS_RSA_TESTVEC_PARSER_H_
#include <vector>
#include "nic/hal/test/fips_testvec_parser_common.h"

using namespace std;

#define FIPS_TESTVEC_LINE_SZ_MAX    2048

#define FIPS_TESTVEC_RSA_COMMENT        "# "
#define FIPS_TESTVEC_RSA_EMPTY_LINE     "\r\n"
#define FIPS_TESTVEC_RSA_MODULUS        "[mod = "
#define FIPS_TESTVEC_RSA_N              "n = "
#define FIPS_TESTVEC_RSA_E              "e = "
#define FIPS_TESTVEC_RSA_D              "d = "
#define FIPS_TESTVEC_RSA_SHAALG         "SHAAlg = "
#define FIPS_TESTVEC_RSA_MSG            "Msg = "
#define FIPS_TESTVEC_RSA_S              "S = "


/******************************************************************************
    RSA Common Helpers
******************************************************************************/

class fips_rsa_testvec_parser: public fips_common_parser
{

    protected:
#define RSA_SHA_ALGO_MAX_SZ     32
        int fips_parse_sha_algo(char *ascii_in, char *sha_algo, size_t *len)
        {
            size_t      pend_len = *len - 1;
            uint16_t    sha_algo_ptr = 0;

            while ((*ascii_in != '\r') && (*ascii_in != '\n') && pend_len) {
                sha_algo[sha_algo_ptr] = *ascii_in;
                ascii_in++;
                pend_len--;
                sha_algo_ptr++;
                (*len)++;
            }
            sha_algo[sha_algo_ptr] = '\0';
            (*len)++;
            return 0;
        }
};


/******************************************************************************
 SigGen15 Helpers
******************************************************************************/

typedef struct fips_rsa_siggen15_group_entries_s {
    char        sha_algo[32];
    uint16_t    msg_len;
    char        msg[128];
    char        sig[512];
} fips_rsa_siggen15_group_entries_t;

typedef struct fips_rsa_siggen15_group_s {
    uint16_t    modulus_len;
    uint16_t    entry_count;
    char        n[512];
    char        e[512];
    char        d[512];
    fips_rsa_siggen15_group_entries_t   entries[50];
} fips_rsa_siggen15_group_t;


class fips_rsa_siggen15_testvec_parser: public fips_rsa_testvec_parser
{

    private:
        std::vector<fips_rsa_siggen15_group_t> fips_rsa_siggen15_groups;

    public:
        fips_rsa_siggen15_testvec_parser(const char *test_vector_file);
        std::vector<fips_rsa_siggen15_group_t> & fips_rsa_siggen15_groups_get(void) { return fips_rsa_siggen15_groups; }
        void print_group(fips_rsa_siggen15_group_t &group);


};

/******************************************************************************
 SigVer15 Helpers
******************************************************************************/
typedef struct fips_rsa_sigver15_modulus_group_entries_s {
    char        sha_algo[32];
    char        e[512];
    uint16_t    msg_len;
    char        msg[128];
    char        sig[512];
} fips_rsa_sigver15_modulus_group_entries_t;

#define FIPS_RSA_SIGVER15_MODULUS_GROUP_ENTRY_COUNT_MAX 6
typedef struct fips_rsa_sigver15_modulus_group_s {
    char        n[512];
    uint16_t    modulus_group_entry_count;
    fips_rsa_sigver15_modulus_group_entries_t   modulus_group_entries[FIPS_RSA_SIGVER15_MODULUS_GROUP_ENTRY_COUNT_MAX];
} fips_rsa_sigver15_modulus_group_t;

#define FIPS_RSA_SIGVER15_MODULUS_GROUPS_MAX    9
typedef struct fips_rsa_sigver15_group_s {
    uint16_t    modulus_len;
    uint16_t    entry_count;
    fips_rsa_sigver15_modulus_group_t   entries[FIPS_RSA_SIGVER15_MODULUS_GROUPS_MAX];
} fips_rsa_sigver15_group_t;


class fips_rsa_sigver15_testvec_parser: public fips_rsa_testvec_parser
{

    private:
        std::vector<fips_rsa_sigver15_group_t> fips_rsa_sigver15_groups;

    public:
        fips_rsa_sigver15_testvec_parser(const char *test_vector_file);
        std::vector<fips_rsa_sigver15_group_t> & fips_rsa_sigver15_groups_get(void) { return fips_rsa_sigver15_groups; }
        void print_group(fips_rsa_sigver15_group_t &group);


};


#endif  /*_FIPS_RSA_TESTVEC_PARSER_H_*/
