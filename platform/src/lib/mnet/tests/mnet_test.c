#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>

#include "mnet.h"

#define NUM_MNETS_TEST  (4)

struct mnet_dev_create_req_t req[NUM_MNETS_TEST];

int main(int argc, char *argv[])
{
    int ret;
    const char* iface_names[NUM_MNETS_TEST] = {"oob_mnic0-", "inb_mnic0-", "inb_mnic1-", "int_mnic0-"};

    //create mnic-oob
    req[0].devcmd_pa = 0x1380db000;
    req[0].devcmd_db_pa = 0x1380dc000;
    req[0].drvcfg_pa = 0x06040000;
    req[0].msixcfg_pa = 0x06010000;
    req[0].doorbell_pa =0x8560880;
    snprintf(req[0].iface_name, sizeof(req[0].iface_name), iface_names[0]);

    ret = create_mnet(&req[0]);

    if(ret)
    {
        printf("mnet iface: %s : create_mnet failed with err: %d\n", req[0].iface_name, ret);
        exit(-1);
    }

    //create mnic-inb0
    req[1].devcmd_pa = 0x1380dd000;
    req[1].devcmd_db_pa = 0x1380de000;
    req[1].drvcfg_pa = 0x06040080;
    req[1].msixcfg_pa = 0x06010040;
    req[1].doorbell_pa = 0x85608c0;
    snprintf(req[1].iface_name, sizeof(req[1].iface_name), iface_names[1]);

    ret = create_mnet(&req[1]);

    if(ret)
    {
        printf("mnet iface: %s : create_mnet failed with err: %d\n", req[1].iface_name, ret);
        exit(-1);
    }

    //create mnic-inb1
    req[2].devcmd_pa = 0x1380df000;
    req[2].devcmd_db_pa = 0x1380e0000;
    req[2].drvcfg_pa = 0x06040100;
    req[2].msixcfg_pa = 0x06010080;
    req[2].doorbell_pa = 0x8560900;
    snprintf(req[2].iface_name, sizeof(req[2].iface_name), iface_names[2]);

    ret = create_mnet(&req[2]);

    if(ret)
    {
        printf("mnet iface: %s : create_mnet failed with err: %d\n", req[2].iface_name, ret);
        exit(-1);
    }

    printf("Exiting now   !!! --- !!!\n");

#if 0
    //create mnic-internal
    req[3].devcmd_pa = 0x1380e1000;
    req[3].devcmd_db_pa = 0x1380e2000;
    req[3].drvcfg_pa = 0x6040180;
    req[3].msixcfg_pa = 0x60100c0;
    req[3].doorbell_pa = 0x8560940;
    snprintf(req[3].iface_name, sizeof(req[2].iface_name), iface_names[3]);

    ret = create_mnet(&req[3]);

    if(ret)
    {
        printf("mnet iface: %s : create_mnet failed with err: %d\n", req[3].iface_name, ret);
        exit(-1);
    }
#endif

#if 0
    //removing mnet devices
    remove_mnet((req[0].iface_name));
    remove_mnet((req[1].iface_name));
    remove_mnet((req[2].iface_name));
    remove_mnet((req[3].iface_name));
#endif

    return 0;
}

