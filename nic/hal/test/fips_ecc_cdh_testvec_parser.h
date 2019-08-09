#ifndef _FIPS_ECC_CDH_TESTVEC_PARSER_H_
#define _FIPS_ECC_CDH_TESTVEC_PARSER_H_

#include <vector>
#include "nic/hal/test/fips_testvec_parser_common.h"

using namespace std;

#define FIPS_TESTVEC_ECC_CDH_LINE_SZ_MAX    256

#define FIPS_TESTVEC_ECC_CDH_KEY_SIZE   "[P-"
#define FIPS_TESTVEC_ECC_CDH_COUNT      "COUNT = "
#define FIPS_TESTVEC_ECC_CDH_QCAVSX     "QCAVSx = "
#define FIPS_TESTVEC_ECC_CDH_QCAVSY     "QCAVSy = "
#define FIPS_TESTVEC_ECC_CDH_QIUTX      "QIUTx = "
#define FIPS_TESTVEC_ECC_CDH_QIUTY      "QIUTy = "
#define FIPS_TESTVEC_ECC_CDH_DIUT       "dIUT = "

/******************************************************************************
    ECC-CDH Common Helpers
******************************************************************************/
#define FIPS_ECC_CDH_ENTRIES_MAX    25
#define FIPS_ECC_CDH_KEY_SIZE_MAX   66

typedef struct fips_ecc_cdh_group_entries_s {
    /* Public+Private Key Pair */
    char            qiutx[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            qiuty[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            diut[FIPS_ECC_CDH_KEY_SIZE_MAX];

    /* Peer Public key */
    char            qcavsx[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            qcavsy[FIPS_ECC_CDH_KEY_SIZE_MAX];

    /* Computed shared secret */
    char            ziut[FIPS_ECC_CDH_KEY_SIZE_MAX];
} fips_ecc_cdh_group_entries_t;

typedef struct fips_ecc_cdh_group_s {
    uint16_t        key_size;
    uint16_t        key_size_bytes;
    /* Domain Parameters */
    char            p[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            a[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            b[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            gx[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            gy[FIPS_ECC_CDH_KEY_SIZE_MAX];
    char            n[FIPS_ECC_CDH_KEY_SIZE_MAX];

    uint16_t        entry_count;
    fips_ecc_cdh_group_entries_t    entries[FIPS_ECC_CDH_ENTRIES_MAX];
} fips_ecc_cdh_group_t;



class fips_testvec_ecc_cdh_parser: public fips_common_parser
{
    private:
        std::vector<fips_ecc_cdh_group_t> fips_ecc_cdh_groups;

    public:
        fips_testvec_ecc_cdh_parser(const char *test_vector_file);
        std::vector<fips_ecc_cdh_group_t> &fips_ecc_cdh_groups_get(void) { return fips_ecc_cdh_groups; }
        void print_group_testvec(FILE *fp, fips_ecc_cdh_group_t &group);
};
#endif /* _FIPS_ECC_CDH_TESTVEC_PARSER_H_ */
