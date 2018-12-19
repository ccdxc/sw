#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include "nic/sdk/platform/pal/include/pal.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/evutils/include/evutils.h"

static void
my_isr(void *arg)
{
    printf("in my_isr\n");
}

int
main(int argc, char *argv[])
{
    struct pal_int pal_int;
    uint64_t msgaddr;
    uint32_t msgdata;
    int intrres, opt, r;

    intrres = 0;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
        case 'i':
            intrres = strtoul(optarg, NULL, 0);
            break;
        case '?':
            exit(1);
        }
    }

    r = pal_int_open_msi(&pal_int, &msgaddr, &msgdata);
    if (r < 0) {
        fprintf(stderr, "pal_int_open_msi failed %d\n", r);
        exit(1);
    }

    printf("intrres %d msgaddr 0x%lx msgdata 0x%x\n",
           intrres, msgaddr, msgdata);

    intr_config_local_msi(intrres, msgaddr, msgdata);
    evutil_add_pal_int(&pal_int, my_isr, NULL);

    printf("waiting for intr %d\n", intrres);
    evutil_run();

    evutil_remove_pal_int(&pal_int);
    pal_int_close(&pal_int);
    exit(0);
}
