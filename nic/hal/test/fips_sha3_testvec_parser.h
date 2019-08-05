#ifndef _FIPS_SHA3_TESTVEC_PARSER_H_
#define _FIPS_SHA3_TESTVEC_PARSER_H_
#include <vector>
#include "nic/hal/test/fips_testvec_parser_common.h"

using namespace std;

#define FIPS_TESTVEC_SHA3_LINE_SZ_MAX   (16 * 1024 * 2)


#define FIPS_TESTVEC_SHA3_DIGEST_LEN    "[L = "
#define FIPS_TESTVEC_SHA3_MSG_LEN       "Len = "
#define FIPS_TESTVEC_SHA3_MSG           "Msg = "
#define FIPS_TESTVEC_SHA3_SEED          "Seed = "


/******************************************************************************
 SHA3 Tests Helpers
******************************************************************************/

typedef struct fips_sha3_group_entries_s {
    uint16_t    msg_len;
    char        msg[16*1024];
    char        digest[64];
} fips_sha3_group_entries_t;

typedef struct fips_sha3_group_s {
    uint16_t    digest_len;
    uint16_t    entry_count;
    fips_sha3_group_entries_t   entries[200];
} fips_sha3_group_t;


class fips_testvec_sha3_parser: public fips_common_parser
{

    private:
        std::vector<fips_sha3_group_t> fips_sha3_groups;

    public:
        fips_testvec_sha3_parser(const char *test_vector_file);
        std::vector<fips_sha3_group_t> & fips_sha3_groups_get(void) { return fips_sha3_groups; }
        void print_group_testvec(FILE *fp, fips_sha3_group_t &group);



};

/******************************************************************************
 SHA3 Monte Carlo Tests Helpers
******************************************************************************/
#define FIPS_SHA3_MONTE_ENTRIES_MAX 100
typedef struct fips_sha3_monte_group_entries_s {
    char        digest[64];
} fips_sha3_monte_group_entries_t;

typedef struct fips_sha3_monte_group_s {
    uint16_t    digest_len;
    char        seed[64];
    uint16_t    entry_count;
    fips_sha3_monte_group_entries_t   entries[FIPS_SHA3_MONTE_ENTRIES_MAX];
} fips_sha3_monte_group_t;

class fips_testvec_sha3_monte_parser: public fips_common_parser
{
    private:
        std::vector<fips_sha3_monte_group_t> fips_sha3_monte_groups;

    public:
        fips_testvec_sha3_monte_parser(const char *test_vector_file);
        std::vector<fips_sha3_monte_group_t> & fips_sha3_groups_get(void) { return fips_sha3_monte_groups; }
        void print_group_testvec(FILE *fp, fips_sha3_monte_group_t &group);
};

#endif /* _FIPS_SHA3_TESTVEC_PARSER_H_ */
