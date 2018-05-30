
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "dtls.h"
#include <readline/readline.h>
#include <regex.h>
#include <map>
#include <vector>
#include "parse.h"
#include "cli.h"
#include "clidtls.h"
#include "editbuf.h"

#define FILTER_DEBUG 0

static int cli_disp_so;
static int cli_exp_level;
static int cli_comp_start;
static int cli_comp_len;

static char **
cli_display_filter_level(int level, char **matches, int count, int max_len,
        int *out_countp, int *out_maxlenp)
{
    int out_maxlen, out_count;
    char **out_matches;
    char obuf[128];

    if ((size_t)max_len + 1 > sizeof (obuf)) {
        throw std::runtime_error("obuf too small");
    }
    for (int pass = 1; pass <= 2; pass++) {
        obuf[0] = '\0';
        out_count = 0;
        out_maxlen = 0;
        for (int i = 1; i <= count; i++) {
            char *s = matches[i];
            int j = 0;
            char c;
            while ((c = *s++) != '\0') {
                if (c == '_') {
                    if (++j == level) {
                        break;
                    }
                }
            }
            size_t len = s - matches[i];
            if (memcmp(matches[i], obuf, len) == 0) {
                continue;   // same as previous; skip
            }
            memcpy(obuf, matches[i], len);
            obuf[len] = '\0';
            ++out_count;
            if (pass == 2) {
                out_matches[out_count] = strdup(obuf);
                if (len > (size_t)out_maxlen) {
                    out_maxlen = len;
                }
            }
        }
        if (pass == 1) {
            out_matches = (char **)malloc(sizeof (char *) * (out_count + 1));
        }
    }
    *out_countp = out_count;
    *out_maxlenp = out_maxlen;
    return out_matches;
}

static void
cli_display_free_matches(char **matches, int count)
{
    for (int i = 1; i <= count; i++) {
        free(matches[i]);
    }
    free(matches);
}

void
cli_register_display(char **matches, int count, int max_len)
{
    int start_level = cli_exp_level;
    int out_count, out_maxlen, level, rows, cols;
    char **out_matches;

    //rl_crlf();
    for (level = start_level;; level++) {
        out_matches = cli_display_filter_level(level, matches, count,
                max_len, &out_count, &out_maxlen);
        cols = 80 / (out_maxlen + 2);
        cols = cols ? cols : 1;
        rows = (out_count + cols - 1) / cols;
        if (out_count == count || rows > 25) {
            break;
        }
        cli_display_free_matches(out_matches, out_count);
    }
    if (rows > 25 && level > start_level) {
        cli_display_free_matches(out_matches, out_count);
        out_matches = cli_display_filter_level(level - 1, matches, count,
                max_len, &out_count, &out_maxlen);
    } else if (out_count == count) {
        cli_display_free_matches(out_matches, out_count);
        out_matches = cli_display_filter_level(99, matches, count,
                max_len, &out_count, &out_maxlen);
    }
    cols = 80 / (out_maxlen + 2);
    cols = cols ? cols : 1;
    rows = (out_count + cols - 1) / cols;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            if (idx < out_count) {
                printf("%-*s%s", out_maxlen, out_matches[idx + 1],
                        (c == cols - 1) ? "" : "  ");
            }
        }
        putchar('\n');
    }
    cli_display_free_matches(out_matches, out_count);
    rl_forced_update_display();
}

static std::vector<std::string>
cli_make_wordlist(const std::string& is, int start, int len, int *widx)
{
    std::vector<std::string> v;
    size_t pos = 0;
    int this_widx = -1;
    do {
        size_t si = is.find_first_not_of(" \t", pos);
        if (si == std::string::npos) {
            break;
        }
        size_t ei = is.find_first_of(" \t", si);
        size_t ll;
        if (ei == std::string::npos) {
            ll = is.size() - si;
        } else {
            ll = ei - si;
        }
        if ((int)si >= start && (int)ll <= len) {
            this_widx = (int)v.size();
        }
        v.push_back(is.substr(si, ll));
        pos = (ei == std::string::npos) ? ei : ei + 1;
    } while (pos != std::string::npos);
    *widx = this_widx;
    return v;
}

static char *
cli_register_generator(const char *s, int state)
{
    static struct parse_info inf;
    static CVDBReg reg;
    static enum { IT_REGNAMES, IT_FIELDNAMES } mode;
    static std::string fieldname;
    static int fidx;
    static int field_so;
    char *match = NULL;

    if (state == 0) {
        reg = CVDBReg::end();
        if (s[0] == '.') {
            //
            // Build a vector of words in the line so far and then find
            // the index of the word being completed.  If the word's index
            // >= 2, then we assume a command of the format:
            //      command(0) regname(1) fieldname(2)
            // ... and we want to complete the fieldnames in the context
            // of the register name.
            //
            int this_widx;
            std::vector<std::string> v = cli_make_wordlist(rl_line_buffer,
                    cli_comp_start, cli_comp_len, &this_widx);
            if (this_widx >= 2) {
                // command(0) regname(1) fieldname(2)
                // try to parse word 1 as a register
                if (parse_word(v[1].c_str(), &inf) < 0) {
                    return NULL;
                }
                reg = CVDBReg::get_byname(inf.regname.c_str());
                if (reg == CVDBReg::end()) {
                    return NULL;
                }
                if (reg.is_array() && !(inf.flags & PIF_IDX)) {
                    return NULL;
                }
            } else {
                // complete fields for the current register
                if (editbuf_info.valid) {
                    reg = editbuf_info.inf.reg;
                }
            }
            if (reg == CVDBReg::end()) {
                return NULL;
            }
            fieldname = std::string(s + 1);
            field_so = 1;
            fidx = 0;
            mode = IT_FIELDNAMES;
        } else {
            if (parse_word(s, &inf) < 0) {
                return NULL;
            }
            if (!(inf.flags & PIF_FULLREG)) {
                reg = CVDBReg::first_byname(inf.regname);
                mode = IT_REGNAMES;

                cli_exp_level = 0;
                for (size_t i = 0; i < inf.regname.size(); i++ ) {
                    if (inf.regname[i] == '_') {
                        ++cli_exp_level;
                    }
                }
                ++cli_exp_level;
                    
                //rl_completion_display_matches_hook = cli_register_display;
                cli_disp_so = 0;
            }
        }
    }

    if (mode == IT_REGNAMES) {
        if (reg == CVDBReg::end()) {
            return NULL;
        }
        match = strdup(reg.name());
        reg = reg.next_byname(inf.regname);
    } else {
        while (fidx < reg.nfields()) {
            CVDBField f = reg.field(fidx++);
            if (strncmp(f.name(), fieldname.c_str(),
                    fieldname.length()) == 0) {
                match = strdup((std::string(s, field_so) + f.name()).c_str());
                break;
            }
        }
    }
    return match;
}

static char *
cli_command_generator(const char *s, int state)
{
    static int idx;

    if (state == 0) {
        idx = 0;
    }
    size_t len = strlen(s);
    while (idx < cli_ncmds) {
        if (strncmp (cli_cmtab[idx].name, s, len) == 0) {
            return strdup(cli_cmtab[idx++].name);
        }
        ++idx;
    }
    return NULL;
}

static char **
cli_generator(const char *text, int start, int end)
{
    char **matches = NULL;

    cli_comp_start = start;
    cli_comp_len = end - start;

    if (start == 0) {
        matches = rl_completion_matches(text, cli_command_generator);
        rl_completion_display_matches_hook = NULL;
        rl_completion_append_character = ' ';
    } else {
        matches = rl_completion_matches(text, cli_register_generator);
        rl_completion_append_character = '\0';
    }
    rl_attempted_completion_over = 1;
    return matches;
}

void
cli_generator_init(void)
{
    rl_attempted_completion_function = cli_generator;
}
