#ifndef _FIPS_PARSER_COMMON_H_
#define _FIPS_PARSER_COMMON_H_

class fips_common_parser {

    protected:    
        int fips_parse_hex_bn(const char *ascii_in, char *out, size_t *len)
        {
            const char  *curr_ptr = ascii_in;
            char        *curr_out_ptr;
            uint8_t     byte = 0;
            size_t      pend_len = *len;

            curr_out_ptr = out;
            *len = 0;

            while ((*curr_ptr != '\0') && (*curr_ptr != '\n') && (*curr_ptr != '\r') && (pend_len)) {
                if ((*curr_ptr >= '0') && (*curr_ptr <= '9')) {
                    byte = ((*curr_ptr - '0') << 4);
                }
                else if ((*curr_ptr >= 'a') && (*curr_ptr <= 'f')) {
                    byte = ((10 + (*curr_ptr - 'a')) << 4);
                }
                else {
                    return -1;
                }
                curr_ptr++;
                if ((*curr_ptr >= '0') && (*curr_ptr <= '9')) {
                    byte |= (*curr_ptr - '0');
                }
                else if ((*curr_ptr >= 'a') && (*curr_ptr <= 'f')) {
                    byte |= (10 + (*curr_ptr - 'a'));
                }
                else {
                    return -1;
                }
                curr_ptr++;
                *curr_out_ptr = byte;
                curr_out_ptr++;
                (*len)++;
                pend_len--;
            }
            return 0;
        }

#define MAX_LINE_SZ 128
        static inline void hex_dump(const char *label, char *buf, uint16_t len)
        {
            char            line[MAX_LINE_SZ];
            char            *lineptr;
            uint16_t        idx = 0;
            uint16_t        lineoffset = 0;

            lineptr = &line[0];
            printf("%s:\n", label);
            for (idx = 0; idx < len; idx++) {

                lineoffset += snprintf(lineptr + lineoffset, (MAX_LINE_SZ - lineoffset - 1),
                        "%02hhx ", buf[idx]);

                if (((idx + 1) % 16) == 0) {
                    printf("%s\n", line);
                    lineoffset = 0;
                }
            }
            if (lineoffset) {
                printf("%s\n", line);
            }
        }

#define INT_STR_MAX_SZ  32
        int fips_parse_dec_int(const char *ascii_in, uint32_t *int_out)
        {
            char            int_str[INT_STR_MAX_SZ];
            uint16_t        int_str_ptr = 0, pend_len = (INT_STR_MAX_SZ - 1);


            while ((*ascii_in >= '0') && (*ascii_in <= '9') && pend_len) {
                int_str[int_str_ptr] = *ascii_in;
                pend_len--;
                int_str_ptr++;
                ascii_in++;
            }
            int_str[int_str_ptr] = '\0';

            *int_out = atoi((const char*)int_str);
            return 0;
        }

    public:
        int fips_testvec_hex_output(FILE *f, const char *label, char *buf, uint16_t len)
        {
            int         idx = 0;

            fprintf(f, "%s = ", label);

            for (idx = 0; idx < len; idx++) {
                fprintf(f, "%02hhx", buf[idx]);
            }
            fprintf(f, "\n");
            return 0;
        }

        int fips_testvec_hex_output_padded(FILE *f, const char *label, char *buf,
                uint16_t len, uint16_t pad_len)
        {
            int         idx = 0;

            fprintf(f, "%s = ", label);

            for (idx = 0; idx < pad_len; idx++) {
                fprintf(f, "%02hhx", 0);
            }
            for (idx = 0; idx < len; idx++) {
                fprintf(f, "%02hhx", buf[idx]);
            }
            fprintf(f, "\n");
            return 0;
        }

};


#endif  /*_FIPS_PARSER_COMMON_H_*/
