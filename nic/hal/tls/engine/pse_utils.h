#ifndef __PSE_UTILS_H__
#define __PSE_UTILS_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>

int pse_init_log();
void pse_log(const char* format, ...); 

#define LOG(level, fmt_str, ...)            \
    do {                                    \
        pse_log("[%s][%s:%d:%s()] "fmt_str"\n",                      \
                level, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)


#define WARN(fmt_str, ...)                  \
    LOG("WARNING", fmt_str, ##__VA_ARGS__); 

#define INFO(fmt_str, ...)                  \
    LOG("INFO", fmt_str, ##__VA_ARGS__);    

#define MAX_LINE_SZ 128
static inline void pse_hex_dump(const uint8_t *buf, uint32_t len)
{
    char            line[MAX_LINE_SZ];
    char            *lineptr;
    uint32_t        idx = 0;
    uint32_t        lineoffset = 0;

    lineptr = &line[0];
    for (idx = 0; idx < len; idx++) {

        lineoffset += snprintf(lineptr + lineoffset, (MAX_LINE_SZ - lineoffset - 1),
                "%02hhx ", buf[idx]);

        if (((idx + 1) % 16) == 0) {
            INFO("%s", line);
            lineoffset = 0;
        }
    }
    if (lineoffset) {
        INFO("%s", line);
    }
}

#define HEX_DUMP(ptr, len)      \
    pse_hex_dump(ptr, len);

#endif /* __PSE_UTILS_H__ */
