#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <inttypes.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "lzrw.h"
#include "0-lzrw.h"

//typedef unsigned long long uint64_t;

#ifndef USEC_PER_SEC
#define USEC_PER_SEC            1000000L
#endif

#define BUF_SIZE_MAX            32768
#define RAND_LEN_MIN            3
#define DIFF_DUMP_LINE_BYTES    16

typedef int (*lzrw1a_non_smooth_t)(uint32_t action,
                                   uint8_t *hash,
                                   uint8_t *data,
                                   uint32_t size,
                                   uint8_t *result,
                                   uint32_t* p_dst_len,
                                   uint32_t thresh);
typedef int (*lzrw1a_smooth_t)(uint32_t action,
                               uint8_t *hash,
                               uint8_t *data,
                               uint32_t size,
                               uint8_t *result,
                               uint32_t* p_dst_len,
                               uint32_t thresh,
                               uint32_t *p_adler32_chksum);

static uint8_t  scratch_buf[BUF_SIZE_MAX];
static uint8_t  input_buf[BUF_SIZE_MAX];
static uint8_t  cp_buf[BUF_SIZE_MAX];
static uint8_t  dc_buf[BUF_SIZE_MAX];

static uint8_t  kb_line[128];

#ifndef min
#define min(a, b) (a) < (b) ? (a) : (b)
#define max(a, b) (a) > (b) ? (a) : (b)
#endif

static uint32_t rand_len_randomize(uint32_t max_size)
{
    uint32_t rand_len = rand() % (max_size + 1);
    return rand_len ? rand_len : 1;
}

static void input_randomize(uint32_t buf_size, uint32_t rand_len)
{
    uint8_t *p;
    uint32_t rem_len, cpy_len;
    uint32_t i;

    if (rand_len == 0) {
        rand_len = buf_size;
    }
    buf_size = min(buf_size, BUF_SIZE_MAX);
    rand_len = min(buf_size, rand_len);

    for (i = 0; i < rand_len; i++) {
        input_buf[i] = rand();
    }

    p = &input_buf[rand_len];
    rem_len = buf_size - rand_len;
    while (rem_len) {
        cpy_len = min(rem_len, rand_len);
        memcpy(p, input_buf, cpy_len);
        p += cpy_len;
        rem_len -= cpy_len;
    }
}

int input_hexascii_file_read(const char *fname)
{
    uint8_t local_buf[256];
    uint8_t *local_p;
    int fd;
    int rd_bytes;
    int count = 0;
    int total = 0;
    uint8_t byte = 0;
    uint8_t nibble_shift = 0;
    uint8_t nibble;
    uint8_t c;

    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        printf("ERROR cannot open file %s\n", fname);
        goto done;
    }

    while (TRUE) {
        rd_bytes = read(fd, local_buf, sizeof(local_buf));
        if (rd_bytes < 0) {
            printf("ERROR reading file %s\n", fname);
            total = 0;
            goto done;
        }

        // Strip 0x prefix if any
        local_p = local_buf;
        count = rd_bytes;
        if ((count >= 2) && !memcmp(local_p, "0x", 2)) {
            local_p += 2;
            count -= 2;
        }

        while (count && (total < BUF_SIZE_MAX)) {
            c = *local_p++;
            count--;
            if (isspace(c)) {
                continue;
            }
            if (!isxdigit(c)) {
                printf("ERROR invalid hex digit %c in file %s\n", c, fname);
                total = 0;
                goto done;
            }

            nibble = isalpha(c) ? (tolower(c) - 'a') + 10 : c - '0';
            byte = (byte << nibble_shift) | nibble;
            if (nibble_shift == 0) {
                nibble_shift = 4;
                continue;
            }

            input_buf[total++] = byte;
            byte = nibble_shift = 0;
        }

        if (rd_bytes < sizeof(local_buf)) {
            break;
        }
    }

    if (nibble_shift) {
        printf("ERROR odd number of hex digits in file %s\n", fname);
        total = 0;
        goto done;
    }

    if (count) {
        printf("ERROR file %s expresses a pattern longer than %u\n",
               fname, BUF_SIZE_MAX);
        total = 0;
        goto done;
    }

done:
   if (fd >= 0) {
       close(fd);
   }

   return total;
}

void output_hexascii_file_write(const char *fname, const uint8_t *buf,
                                uint32_t size, int write_binary)
{
    uint8_t *local_p;

    if (write_binary) {
        int fd = open(fname, O_WRONLY);
        if (fd < 0) {
            goto open_error;
        }
        write(fd, buf, size);
        close(fd);

    } else {

        FILE *fp = fopen(fname, "w");
        if (!fp) {
            goto open_error;
        }
        fprintf(fp, "0x");
        for (uint32_t i = 0; i < size; i++) {
            fprintf(fp, "%02x", buf[i]);
        }
        fclose(fp);
    }

    return;

open_error:
    printf("ERROR cannot open file %s\n", fname);
}

void dump(const uint8_t *buf, uint32_t size)
{
    printf("size %u\n", size);
    for (uint32_t i = 0; i < size; i++) {
        printf("%02x", buf[i]);
    }
}

void diff_line_dump(const uint8_t *line0, const uint8_t *line1,
                    uint32_t line_size, uint32_t offset)
{
    if (memcmp(line0, line1, line_size)) {
        printf("@%4u ", offset);
        for (uint32_t j = 0; j < line_size; j++) {
            printf("%02x ", line0[j]);
        }
        printf("\n@%4d ", offset);
        for (uint32_t j = 0; j < line_size; j++) {
            printf("%02x ", line1[j]);
        }
        printf("\n\n");
    }
}

void diff_dump(const uint8_t *buf0, const uint8_t *buf1, uint32_t size)
{
    printf("size %u\n", size);
    uint32_t n = size / DIFF_DUMP_LINE_BYTES;
    uint32_t offset = 0;

    for (uint32_t i = 0; i < n; i++) {
        diff_line_dump(buf0, buf1, DIFF_DUMP_LINE_BYTES, offset);
        buf0 += DIFF_DUMP_LINE_BYTES;
        buf1 += DIFF_DUMP_LINE_BYTES;
        offset += DIFF_DUMP_LINE_BYTES;
    }
    diff_line_dump(buf0, buf1, size % DIFF_DUMP_LINE_BYTES, offset);
}

void usage_help(void)
{
    printf("\nOptions:\n\n"
             "-r                      repetition count, default 100\n"
             "-s                      input buffer size, default 8K\n"
             "-l                      max random stream length, default 576\n"
             "--pattern-file or -f    read pattern from a hex ascii file and use it\n"
             "--compress-only         do only compression and write to output file\n"
             "--decompress-only       do only decompression and write to output file\n"
             "--smooth0               use release 0 of the smooth algorithm\n"
             "--no-smooth             do not use smooth algorithm\n"
             "--no-adler32            do not calculate adler32 checksum\n"
             "--no-verify             skip data and checksum verification\n"
             "--output-file or -o     name of output file\n"
             "--liveness-count or -v  liveness count, default 0 (0 means no liveness reporting)\n"
             "--help or -h            this help message\n\n");

}

static uint64_t elapsed_us(const struct timeval *start,
                           const struct timeval *end)
{
    uint64_t start_us;
    uint64_t end_us;

    start_us = ((uint64_t)start->tv_sec * USEC_PER_SEC) + start->tv_usec;
    end_us = ((uint64_t)end->tv_sec * USEC_PER_SEC) + end->tv_usec;

    return end_us - start_us;
}

int main(int argc, char **argv)
{
    char *output_fname = NULL;
    lzrw1a_non_smooth_t lzrw1a_non_smooth = &orig_lzrw1a_compress;
    lzrw1a_smooth_t lzrw1a_smooth = &lzrw1a_compress;
    struct timeval tv;
    struct timeval last_tv;
    uint32_t buf_size = 8192;
    uint32_t rand_len_highwater = 576;
    uint32_t cp_size, dc_size;
    uint32_t min_cp_size = ~0;
    uint32_t max_cp_size = 0;
    uint32_t min_dc_size = ~0;
    uint32_t max_dc_size = 0;
    uint32_t min_rand_len = ~0;
    uint32_t max_rand_len = 0;
    uint32_t rand_len;
    uint32_t cp_adler32 = 0;
    uint32_t dc_adler32 = 0;
    uint32_t *cp_adler32_p;
    uint32_t *dc_adler32_p;
    uint32_t pattern_size = 0;
    int num_data_miscompares = 0;
    int num_uncompressibles = 0;
    int num_chksum_mismatches = 0;
    int num_len_mismatches = 0;
    int num_successes = 0;
    int rep_count = 100;
    int liveness_count = 0;
    int use_pattern = 0;
    int failure;
    int cp_only = 0;
    int dc_only = 0;
    int use_smooth0_algo = 0;
    int no_smooth = 0;
    int no_adler32 = 0;
    int no_verify = 0;
    int cp_ret, dc_ret;
    int opt;
    int i;

    struct option longopts[] = {
       { "compress-only",       no_argument,        &cp_only, 1 },
       { "decompress-only",     no_argument,        &dc_only, 1 },
       { "smooth0",             no_argument,        &use_smooth0_algo, 1 },
       { "no-smooth",           no_argument,        &no_smooth, 1 },
       { "no-adler32",          no_argument,        &no_adler32, 1 },
       { "no-verify",           no_argument,        &no_verify, 1 },
       { "pattern-file",        required_argument,  NULL, 'f' },
       { "output-file",         required_argument,  NULL, 'o' },
       { "liveness-count",      required_argument,  NULL, 'v' },
       { "help",                no_argument,        NULL, 'h' },
       { 0,                     0,                  0,     0  }
    };

    while ((opt = getopt_long(argc, argv, ":r:s:l:v:f:o:h", longopts, NULL)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'r':
            rep_count = atoi(optarg);
            break;
        case 's':
            buf_size = atoi(optarg);
            if ((buf_size <= 0) || (buf_size > BUF_SIZE_MAX)) {
                printf("invalid buf_size %d, max is %d\n",
                       buf_size, BUF_SIZE_MAX);
                return 1;
            }
            break;
        case 'l':
            rand_len_highwater = atoi(optarg);
            if (rand_len_highwater < RAND_LEN_MIN) {
                printf("invalid random length %d, must be at least %d\n",
                       rand_len_highwater, RAND_LEN_MIN);
                return 1;
            }
            break;
        case 'v':
            liveness_count = atoi(optarg);
            if (liveness_count < 0) {
                printf("invalid liveness_count %d\n", liveness_count);
                return 1;
            }
            break;
        case 'f':
            pattern_size = input_hexascii_file_read(optarg);
            if (!pattern_size) {
                printf("no pattern to work with\n");
                return 1;
            }
            use_pattern = 1;
            break;
        case 'o':
            output_fname = optarg;
            break;
        case ':':
            printf("option -%c requires an operand\n", optopt);
            return 1;
            break;
        case 'h':
            usage_help();
            return 0;
            break;
        default:
            printf("unknown option\n");
            usage_help();
            return 1;
        }
    }

    if (cp_only && dc_only) {
        printf("ERROR only one of compress-only or decompress-only can be set\n");
        return 1;
    }

    if (no_smooth && use_smooth0_algo) {
        printf("ERROR smooth0 and no-smooth cannot both be set\n");
        return 1;
    }

    if (use_smooth0_algo && no_adler32) {
        printf("ERROR smooth0 cannot be used with no-adler32\n");
        return 1;
    }
    lzrw1a_smooth = use_smooth0_algo ? lzrw1a_compress_0 : lzrw1a_compress;

    cp_adler32_p = no_adler32 ? NULL : &cp_adler32;
    dc_adler32_p = no_adler32 ? NULL : &dc_adler32;

    gettimeofday(&tv, NULL);
    for (i = 0; i < rep_count; i++) {

        if (!use_pattern) {
            srand(tv.tv_sec + tv.tv_usec + i);
            rand_len = rand_len_randomize(rand_len_highwater);
            min_rand_len = min(min_rand_len, rand_len);
            max_rand_len = max(max_rand_len, rand_len);
            input_randomize(buf_size, rand_len);
        }

        if (dc_only) {
            if (!use_pattern) {
                printf("ERROR decompress-only must have a pattern file input\n");
                return 1;
            }
            memcpy(cp_buf, input_buf, pattern_size);
            cp_size = pattern_size;
            cp_ret = 1;
        } else {
            if (pattern_size) {
                buf_size = pattern_size;
            }
            //dump(input_buf, buf_size);
            cp_ret = no_smooth ?
                     (*lzrw1a_non_smooth)(COMPRESS_ACTION_COMPRESS, scratch_buf, input_buf,
                                          buf_size, cp_buf, &cp_size, buf_size - 8) :
                     (*lzrw1a_smooth)(COMPRESS_ACTION_COMPRESS, scratch_buf, input_buf,
                                      buf_size, cp_buf, &cp_size, buf_size - 8, cp_adler32_p);
        }
        if (cp_ret) {
            failure = 0;
            min_cp_size = min(min_cp_size, cp_size);
            max_cp_size = max(max_cp_size, cp_size);

            if (cp_only) {
                if (output_fname) {
                    output_hexascii_file_write(output_fname, cp_buf, cp_size, FALSE);

                    // do output only once no matter how many reps
                    output_fname = NULL;
                }

            } else {
                dc_ret = no_smooth ?
                         (*lzrw1a_non_smooth)(COMPRESS_ACTION_DECOMPRESS, scratch_buf, cp_buf,
                                              cp_size, dc_buf, &dc_size, buf_size) :
                         (*lzrw1a_smooth)(COMPRESS_ACTION_DECOMPRESS, scratch_buf, cp_buf,
                                          cp_size, dc_buf, &dc_size, buf_size, dc_adler32_p);
                min_dc_size = min(min_dc_size, dc_size);
                max_dc_size = max(max_dc_size, dc_size);

                //dump(dc_buf, dc_size);
                if (dc_only) {
                    if (output_fname) {
                        output_hexascii_file_write(output_fname, dc_buf, dc_size, FALSE);

                        // do output only once no matter how many reps
                        output_fname = NULL;
                    }

                } else if (!no_verify) {
                    if (buf_size != dc_size) {
                        printf("cp_ret %d dc_ret %d buf_size %u dc_cize %u\n",
                               cp_ret, dc_ret, buf_size, dc_size);
                        printf("ERROR lengths mismatched\n");
                        num_len_mismatches++;
                        failure |= 1;
                    } else if (memcmp(input_buf, dc_buf, buf_size)) {
                        printf("cp_ret %d dc_ret %d buf_size %u cp_size %u dc_size %u cp_adler32 %u dc_adler32 %u\n",
                               cp_ret, dc_ret, buf_size, cp_size, dc_size, cp_adler32, dc_adler32);
                        diff_dump(input_buf, dc_buf, buf_size);
                        printf("ERROR dc data mismatched\n");
                        num_data_miscompares++;
                        failure |= 1;
                    }
                    if (cp_adler32 != dc_adler32) {
                        printf("cp_ret %d dc_ret %d buf_size %u cp_adler32 %u dc_adler32 %u\n",
                               cp_ret, dc_ret, buf_size, cp_adler32, dc_adler32);
                        printf("ERROR adler32 mismatched\n");
                        num_chksum_mismatches++;
                        failure |= 1;
                    }
                }
            }

            if (!failure) {
                num_successes++;
            }

        } else {
            num_uncompressibles++;
        }

        if (liveness_count && ((i % liveness_count) == 0)) {
            printf("rep %d out of %d total\n", i, rep_count);
        }
    }
    gettimeofday(&last_tv, NULL);

    printf("\nSummary:\n"
             "--------\n");

    printf("          Buffer size: %u\n"
           "Number of repititions: %u\n"
           "            successes: %u\n"
           "      uncompressibles: %u\n"
           "    length mismatches: %u\n"
           "  checksum mismatches: %u\n"
           "     data miscompares: %u\n"
           "       execution time: %" PRIu64 " usecs\n\n",
           buf_size, rep_count, num_successes, num_uncompressibles,
           num_len_mismatches, num_chksum_mismatches, num_data_miscompares,
           elapsed_us(&tv, &last_tv));

    /*
     * If these values were never altered from their initial values,
     * set them to zero.
     */
    if (min_cp_size == ~0) {
        min_cp_size = 0;
    }
    if (min_dc_size == ~0) {
        min_dc_size = 0;
    }
    if (min_rand_len == ~0) {
        min_rand_len = 0;
    }

    printf("\nOther info:\n"
             "-----------\n");
    printf("    smallest CP result: %u bytes\n"
           "     largest CP result: %u bytes\n"
           "    smallest DC result: %u bytes\n"
           "     largest DC result: %u bytes\n"
           "       last CP adler32: 0x%x\n"
           "       last DC adler32: 0x%x\n"
           "smallest random length: %u bytes\n"
           " largest random length: %u bytes\n",
           min_cp_size, max_cp_size, min_dc_size, max_dc_size,
           cp_adler32, dc_adler32, min_rand_len, max_rand_len);

    return num_len_mismatches + num_data_miscompares + num_chksum_mismatches ? 1 : 0;
}
