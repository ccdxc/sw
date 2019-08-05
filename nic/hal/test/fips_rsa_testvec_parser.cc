#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "nic/hal/test/fips_rsa_testvec_parser.h"

#define FIPS_RSA_TESTVEC_DEBUG_EN  1
#define FIPS_RSA_TESTVEC_ERR(...) printf(__VA_ARGS__)
#define FIPS_RSA_TESTVEC_TRACE(...) printf(__VA_ARGS__)
#ifdef FIPS_RSA_TESTVEC_DEBUG_EN
#define FIPS_RSA_TESTVEC_DEBUG(...) printf(__VA_ARGS__)
#else
#define FIPS_RSA_DEBUG(format, ...)
#endif

/******************************************************************************
 SigGen15 Helpers
******************************************************************************/
fips_rsa_siggen15_testvec_parser::fips_rsa_siggen15_testvec_parser(const char *test_vector_file)
{
    char            line[FIPS_TESTVEC_LINE_SZ_MAX];
    FILE*           fp;
    size_t          bn_len, sha_algo_len;
    uint32_t        modulus_len;
    int             ret = 0;
    fips_rsa_siggen15_group_t   group;

    fp = fopen(test_vector_file, "r");
    if (fp == NULL) {
        return;
    }

    while ((fgets(line, FIPS_TESTVEC_LINE_SZ_MAX, fp)) != NULL) {
        if (!strncmp(line, FIPS_TESTVEC_RSA_MODULUS, strlen(FIPS_TESTVEC_RSA_MODULUS))) {
            ret = fips_parse_dec_int(line + strlen(FIPS_TESTVEC_RSA_MODULUS), &modulus_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'modulus_len' \n");
                break;
            }
            else {
                FIPS_RSA_TESTVEC_DEBUG("Modulus len: %d\n", modulus_len);
                group.modulus_len = modulus_len;
                /* Parse the key info */
                while ((fgets(line, FIPS_TESTVEC_LINE_SZ_MAX, fp)) != NULL) {
                    FIPS_RSA_TESTVEC_DEBUG("%s", line);
                    if (!strncmp(line, FIPS_TESTVEC_RSA_N, strlen(FIPS_TESTVEC_RSA_N))) {
                        bn_len = 512;
                        ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_N), group.n, &bn_len);
                        if (ret) {
                            FIPS_RSA_TESTVEC_ERR("Failed to parse 'n' \n");
                            break;
                        }
                        else {
                            FIPS_RSA_TESTVEC_TRACE("Parsed 'n'\n");
                            //hex_dump("n", group.n, bn_len);
                        }
                    }
                    if (!strncmp(line, FIPS_TESTVEC_RSA_E, strlen(FIPS_TESTVEC_RSA_E))) {
                        bn_len = 512;
                        ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_E), group.e, &bn_len);
                        if (ret) {
                            FIPS_RSA_TESTVEC_ERR("Failed to parse 'e' \n");
                            break;
                        }
                        else {
                            FIPS_RSA_TESTVEC_TRACE("Parsed 'e'\n");
                            hex_dump("e", group.e, bn_len);
                        }
                    }
                    if (!strncmp(line, FIPS_TESTVEC_RSA_D, strlen(FIPS_TESTVEC_RSA_D))) {
                        bn_len = 512;
                        ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_D), group.d, &bn_len);
                        if (ret) {
                            FIPS_RSA_TESTVEC_ERR("Failed to parse 'd' \n");
                            break;
                        }
                        else {
                            FIPS_RSA_TESTVEC_TRACE("Parsed 'd'\n");
                            hex_dump("d", group.d, bn_len);
                        }
                        break; /* We have the key information, start the input parsing loop */
                    }
                }

                /* Parse the inputs */
#define MAX_RSA_INPUT_COUNT 30
                uint16_t        input_count = 0;
                group.entry_count = MAX_RSA_INPUT_COUNT;
                while ((fgets(line, FIPS_TESTVEC_LINE_SZ_MAX, fp)) != NULL) {
                    if (!strncmp(line, FIPS_TESTVEC_RSA_SHAALG, strlen(FIPS_TESTVEC_RSA_SHAALG))) {
                        sha_algo_len = RSA_SHA_ALGO_MAX_SZ;
                        ret = fips_parse_sha_algo(line + strlen(FIPS_TESTVEC_RSA_SHAALG),
                                group.entries[input_count].sha_algo, &sha_algo_len);
                        if (ret) {
                            FIPS_RSA_TESTVEC_ERR("Failed to parse 'SHAAlg' \n");
                            break;
                        }
                        FIPS_RSA_TESTVEC_DEBUG("SHAAlg: %s\n", group.entries[input_count].sha_algo);
                    }
                    if (!strncmp(line, FIPS_TESTVEC_RSA_MSG, strlen(FIPS_TESTVEC_RSA_MSG))) {
                        bn_len = 512;
                        ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_MSG),
                                group.entries[input_count].msg, &bn_len);
                        if (ret) {
                            FIPS_RSA_TESTVEC_ERR("Failed to parse 'Msg' \n");
                            break;
                        }
                        group.entries[input_count].msg_len = bn_len;
                        //hex_dump("Msg", group.entries[input_count].msg, bn_len);
                        input_count++;
                        if (input_count == MAX_RSA_INPUT_COUNT) {
                            /* add to the vector */
                            fips_rsa_siggen15_groups.push_back(group);
                            break;
                        }
                    }
                }
            }
        }
    }
    fclose(fp);
    return;
}

void fips_rsa_siggen15_testvec_parser::print_group(fips_rsa_siggen15_group_t &group)
{
    uint16_t        entry_idx;
    uint16_t        byte_count;

    FIPS_RSA_TESTVEC_DEBUG("Modulus Len:%d\n", group.modulus_len);
    FIPS_RSA_TESTVEC_DEBUG("Entry Count:%d\n", group.entry_count);

    byte_count = group.modulus_len/8;

    hex_dump("n", group.n, byte_count);
    hex_dump("e", group.e, byte_count);
    hex_dump("d", group.d, byte_count);

    for (entry_idx = 0; entry_idx < group.entry_count; entry_idx++) {
        FIPS_RSA_TESTVEC_DEBUG("Entry: %d\n", entry_idx);
        FIPS_RSA_TESTVEC_DEBUG("SHAAlg: %s\n", group.entries[entry_idx].sha_algo);
        hex_dump("msg", group.entries[entry_idx].msg, group.entries[entry_idx].msg_len);
    }
}

/******************************************************************************
 SigVer15 Helpers
******************************************************************************/
fips_rsa_sigver15_testvec_parser::fips_rsa_sigver15_testvec_parser(const char *test_vector_file)
{
    char            line[FIPS_TESTVEC_LINE_SZ_MAX];
    FILE*           fp;
    size_t          bn_len, sha_algo_len;
    uint32_t        modulus_len, modulus_group_idx = 0, modulus_group_entry_idx = 0;
    int             ret = 0;
    fips_rsa_sigver15_group_t   group;

    fp = fopen(test_vector_file, "r");
    if (fp == NULL) {
        return;
    }

    

    while ((fgets(line, FIPS_TESTVEC_LINE_SZ_MAX, fp)) != NULL) {
        if (!strncmp(line, FIPS_TESTVEC_RSA_MODULUS, strlen(FIPS_TESTVEC_RSA_MODULUS))) {
            ret = fips_parse_dec_int(line + strlen(FIPS_TESTVEC_RSA_MODULUS), &modulus_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'modulus_len' \n");
                break;
            }
            else {
                FIPS_RSA_TESTVEC_DEBUG("Modulus len: %d\n", modulus_len);
                group.modulus_len = modulus_len;
            }
            if (modulus_group_idx) {
                group.entry_count = modulus_group_idx + 1;
                fips_rsa_sigver15_groups.push_back(group);
                modulus_group_idx = 0;
            }
        }

        /* Parse the modulus */
        if (!strncmp(line, FIPS_TESTVEC_RSA_N, strlen(FIPS_TESTVEC_RSA_N))) {
            bn_len = 512;
            ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_N),
                    group.entries[modulus_group_idx].n, &bn_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'n' \n");
                break;
            }
            else {
                FIPS_RSA_TESTVEC_TRACE("Parsed 'n'\n");
                //hex_dump("n", group.n, bn_len);
                /* New modulus group */
                if (modulus_group_entry_idx) {
                    /* not the first iteration,store the modulus_group_entry_idx in modulus_group_entry_count */
                    group.entries[modulus_group_idx].modulus_group_entry_count = modulus_group_entry_idx + 1;
                    modulus_group_entry_idx = 0;
                }
                modulus_group_idx++;
                if (modulus_group_idx >= FIPS_RSA_SIGVER15_MODULUS_GROUPS_MAX) {
                    FIPS_RSA_TESTVEC_ERR("Exceeded FIPS_RSA_SIGVER15_MODULUS_GROUPS_MAX count: %d\n", modulus_group_idx);
                    break;
                }
            }
        }

        if (!strncmp(line, FIPS_TESTVEC_RSA_SHAALG, strlen(FIPS_TESTVEC_RSA_SHAALG))) {
            sha_algo_len = RSA_SHA_ALGO_MAX_SZ;
            ret = fips_parse_sha_algo(line + strlen(FIPS_TESTVEC_RSA_SHAALG),
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].sha_algo,
                    &sha_algo_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'SHAAlg' \n");
                break;
            }
            FIPS_RSA_TESTVEC_DEBUG("SHAAlg: %s\n",
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].sha_algo);
        }
        if (!strncmp(line, FIPS_TESTVEC_RSA_E, strlen(FIPS_TESTVEC_RSA_E))) {
            bn_len = 512;
            ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_E),
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].e,
                    &bn_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'e' \n");
                break;
            }
            else {
                FIPS_RSA_TESTVEC_TRACE("Parsed 'e'\n");
                hex_dump("e", group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].e, bn_len);
            }
        }
        if (!strncmp(line, FIPS_TESTVEC_RSA_MSG, strlen(FIPS_TESTVEC_RSA_MSG))) {
            bn_len = 512;
            ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_MSG),
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].msg,
                    &bn_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'Msg' \n");
                break;
            }
            group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].msg_len = bn_len;
            //hex_dump("Msg", group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].msg, bn_len);
        }
        if (!strncmp(line, FIPS_TESTVEC_RSA_S, strlen(FIPS_TESTVEC_RSA_S))) {
            bn_len = 512;
            ret = fips_parse_hex_bn(line + strlen(FIPS_TESTVEC_RSA_S),
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].sig,
                    &bn_len);
            if (ret) {
                FIPS_RSA_TESTVEC_ERR("Failed to parse 'S' \n");
                break;
            }
            else {
                FIPS_RSA_TESTVEC_TRACE("Parsed 'S'\n");
                hex_dump("S", group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].sig, bn_len);

                modulus_group_entry_idx++;
                if (modulus_group_entry_idx >= FIPS_RSA_SIGVER15_MODULUS_GROUP_ENTRY_COUNT_MAX) {
                    FIPS_RSA_TESTVEC_ERR("Exceeded FIPS_RSA_SIGVER15_MODULUS_GROUP_ENTRY_COUNT_MAX count: %d\n",
                            modulus_group_entry_idx);
                    break;
                }
            }
        }
    }
    fclose(fp);
    return;
}

void fips_rsa_sigver15_testvec_parser::print_group(fips_rsa_sigver15_group_t &group)
{
    uint16_t        modulus_group_idx = 0, modulus_group_entry_idx = 0;
    uint16_t        byte_count;

    FIPS_RSA_TESTVEC_DEBUG("Modulus Len:%d\n", group.modulus_len);
    FIPS_RSA_TESTVEC_DEBUG("Entry Count:%d\n", group.entry_count);
    byte_count = group.modulus_len/8;

    for (modulus_group_idx = 0; modulus_group_idx < group.entry_count; modulus_group_idx++) {
        FIPS_RSA_TESTVEC_DEBUG("Modulus Group : %d\n", modulus_group_idx);
        hex_dump("n", group.entries[modulus_group_idx].n, byte_count);
        FIPS_RSA_TESTVEC_DEBUG("Modulus Group entry count: %d\n",
                group.entries[modulus_group_idx].modulus_group_entry_count);
        for (modulus_group_entry_idx = 0; modulus_group_entry_idx < group.entries[modulus_group_idx].modulus_group_entry_count; modulus_group_entry_idx++) {
            FIPS_RSA_TESTVEC_DEBUG("SHAAlg: %s\n",
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].sha_algo);
            FIPS_RSA_TESTVEC_DEBUG("Modulus Group Entry: %d\n", modulus_group_entry_idx);
            hex_dump("e", group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].e,
                    byte_count);
            hex_dump("msg",
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].msg,
                    group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].msg_len);
            hex_dump("S", group.entries[modulus_group_idx].modulus_group_entries[modulus_group_entry_idx].sig,
                    byte_count);
        }
    }
}

#if 0


int main(int argc, char *argv[])
{
    int         ret = -1;
    std::vector<fips_rsa_siggen15_group_t> groups;

    if (argc != 2) {
        printf("Usage: parser <filename>\n");
        exit(-1);
    }

    fips_rsa_siggen15_testvec_parser rsa_testvec_parser(argv[1]);
    groups = rsa_testvec_parser.fips_rsa_siggen15_groups_get();


    for (std::vector<fips_rsa_siggen15_group_t>::iterator it = groups.begin(); it != groups.end(); it++) {
        rsa_testvec_parser.print_group(*it);
    }
}
#endif
