#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nic/hal/test/fips_sha3_testvec_parser.h"


#define FIPS_TESTVEC_SHA3_DEBUG_EN      0
#define FIPS_TESTVEC_SHA3_ERR(...)      printf(__VA_ARGS__)
#define FIPS_TESTVEC_SHA3_TRACE(...)    printf(__VA_ARGS__)
#if (FIPS_TESTVEC_SHA3_DEBUG_EN == 1)
#define FIPS_TESTVEC_SHA3_DEBUG(...)    printf(__VA_ARGS__)
#define FIPS_TESTVEC_SHA3_HEXDUMP(m, d, l)  hex_dump(m, d, l)
#else
#define FIPS_TESTVEC_SHA3_DEBUG(format, ...)
#define FIPS_TESTVEC_SHA3_HEXDUMP(m, d, l)
#endif


fips_testvec_sha3_parser::fips_testvec_sha3_parser(const char *test_vector_file)
{
    char                line[FIPS_TESTVEC_SHA3_LINE_SZ_MAX];
    FILE*               fp;
    size_t              bn_len;
    uint32_t            digest_len, msg_len;
    int                 ret = 0;
    fips_sha3_group_t   group;
    uint16_t            msg_idx = 0;

    fp = fopen(test_vector_file, "r");
    if (fp == NULL) {
        return;
    }

    while ((fgets(line, FIPS_TESTVEC_SHA3_LINE_SZ_MAX, fp)) != NULL) {
        if (!strncmp(line, FIPS_TESTVEC_SHA3_DIGEST_LEN, strlen(FIPS_TESTVEC_SHA3_DIGEST_LEN))) {
            ret = fips_parse_dec_int(line + strlen(FIPS_TESTVEC_SHA3_DIGEST_LEN), &digest_len);
            if (ret) {
                FIPS_TESTVEC_SHA3_ERR("Failed to parse 'digest length'\n");
                break;
            }
            else {
                digest_len /= 8;
                FIPS_TESTVEC_SHA3_DEBUG("Digest Length: %d\n", digest_len);
                group.digest_len = digest_len;

            }
            if (msg_idx) {
                group.entry_count = msg_idx;
                fips_sha3_groups.push_back(group);
                msg_idx = 0;
            }
        }

        if (!strncmp(line, FIPS_TESTVEC_SHA3_MSG_LEN, strlen(FIPS_TESTVEC_SHA3_MSG_LEN))) {
            ret = fips_parse_dec_int(line + strlen(FIPS_TESTVEC_SHA3_MSG_LEN), &msg_len);
            if (ret) {
                FIPS_TESTVEC_SHA3_ERR("Failed to parse 'message length'\n");
                break;
            }
            msg_len /= 8; /* b to B */
            FIPS_TESTVEC_SHA3_DEBUG("Message length: %d\n", msg_len);
            group.entries[msg_idx].msg_len = msg_len;
        }

        if (!strncmp(line, FIPS_TESTVEC_SHA3_MSG, strlen(FIPS_TESTVEC_SHA3_MSG))) {
            bn_len = 16*1024;
            ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_SHA3_MSG),
                    group.entries[msg_idx].msg, &bn_len);
            if (ret) {
                FIPS_TESTVEC_SHA3_ERR("Failed to parse 'Msg': [%s]\n", line + strlen(FIPS_TESTVEC_SHA3_MSG));
                break;
            }
            FIPS_TESTVEC_SHA3_DEBUG("Parsed 'Msg' of len: %d\n", (int)bn_len);
            FIPS_TESTVEC_SHA3_HEXDUMP("Msg", group.entries[msg_idx].msg, bn_len);
            msg_idx++;
        }
    }
    if (msg_idx) {
        group.entry_count = msg_idx;
        fips_sha3_groups.push_back(group);
        msg_idx = 0;
    }
    fclose(fp);
    return;
}

void fips_testvec_sha3_parser::print_group_testvec(FILE *fp, fips_sha3_group_t &group)
{
    fprintf(fp, "[L = %d]\n\n", group.digest_len * 8);
    for (int idx = 0; idx < group.entry_count; idx++) {
        fprintf(fp, "Len = %d\n", group.entries[idx].msg_len * 8);
        fips_testvec_hex_output(fp, "Msg",
                group.entries[idx].msg,
                group.entries[idx].msg_len);
        fips_testvec_hex_output(fp, "MD",
                group.entries[idx].digest,
                group.digest_len);
        fprintf(fp, "\n");
    }
    return;
}


fips_testvec_sha3_monte_parser::fips_testvec_sha3_monte_parser(const char *test_vector_file)
{
    char                line[FIPS_TESTVEC_SHA3_LINE_SZ_MAX];
    FILE*               fp;
    size_t              bn_len;
    uint32_t            digest_len;
    int                 ret = 0;
    fips_sha3_monte_group_t   group;

    fp = fopen(test_vector_file, "r");
    if (fp == NULL) {
        return;
    }

    while ((fgets(line, FIPS_TESTVEC_SHA3_LINE_SZ_MAX, fp)) != NULL) {
        if (!strncmp(line, FIPS_TESTVEC_SHA3_DIGEST_LEN, strlen(FIPS_TESTVEC_SHA3_DIGEST_LEN))) {
            ret = fips_parse_dec_int(line + strlen(FIPS_TESTVEC_SHA3_DIGEST_LEN), &digest_len);
            if (ret) {
                FIPS_TESTVEC_SHA3_ERR("Failed to parse 'digest length'\n");
                break;
            }
            else {
                digest_len /= 8;
                FIPS_TESTVEC_SHA3_DEBUG("Digest Length: %d\n", digest_len);
                group.digest_len = digest_len;

            }
        }

        if (!strncmp(line, FIPS_TESTVEC_SHA3_SEED, strlen(FIPS_TESTVEC_SHA3_SEED))) {
            bn_len = 64;
            ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_SHA3_SEED),
                    group.seed, &bn_len);
            if (ret) {
                FIPS_TESTVEC_SHA3_ERR("Failed to parse 'Seed': [%s]\n", line + strlen(FIPS_TESTVEC_SHA3_SEED));
                break;
            }
            FIPS_TESTVEC_SHA3_DEBUG("Parsed 'Seed' of len: %d\n", (int)bn_len);
            FIPS_TESTVEC_SHA3_HEXDUMP("Seed", group.seed, bn_len);

            /* For now hardcode the entry count */
            group.entry_count = FIPS_SHA3_MONTE_ENTRIES_MAX;
            fips_sha3_monte_groups.push_back(group);
        }
    }
    fclose(fp);
    return;
}

void fips_testvec_sha3_monte_parser::print_group_testvec(FILE *fp, fips_sha3_monte_group_t &group)
{
    fprintf(fp, "[L = %d]\n\n", group.digest_len * 8);
    fips_testvec_hex_output(fp, "Seed",
            group.seed,
            group.digest_len);
    fprintf(fp, "\n");
    for (int idx = 0; idx < group.entry_count; idx++) {
        fprintf(fp, "COUNT = %d\n", idx);
        fips_testvec_hex_output(fp, "MD",
                group.entries[idx].digest,
                group.digest_len);
        fprintf(fp, "\n");
    }
    return;
}
