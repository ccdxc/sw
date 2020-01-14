#include <ev.h>
#include <inttypes.h>
#include <stdint.h>

#include "log.hpp"
#include "vmstats.hpp"

ev_timer timer;
uint64_t g_pgmajfault = 0;

static void
callback (EV_P_ ev_timer *w, int revents)
{
    FILE *f;
    char *line = NULL;
    size_t len;
    ssize_t n;
    uint64_t pgmajfault = 0;

    f = fopen("/proc/vmstat", "r");
    if (f == NULL) {
        g_log->err("Failed to open vmstats");
        return;
    }

    // getline actually allocates a some memory for us and stores it in line
    while ((n = getline(&line, &len, f)) != -1) {
        if (sscanf(line, "pgmajfault %" SCNu64, &pgmajfault) == 1) {
            if (pgmajfault != g_pgmajfault) {
                g_pgmajfault = pgmajfault;
                g_log->warn("New pgmajfault value: %lu", g_pgmajfault);
            }
        }
    }

    if (line) {
        // get allocated first time we call getline
        free(line);
    }

    fclose(f);
}

void
vmstats_init (struct ev_loop *loop)
{
    g_log->debug("vmstats_init");
    ev_timer_init(&timer, callback, 5.0, 5.0);
    ev_timer_start(loop, &timer);
}
