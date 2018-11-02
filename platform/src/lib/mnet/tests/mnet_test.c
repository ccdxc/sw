#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>

#include "mnet.h"

#define NUM_MNETS_TEST  (3)

struct mnet_req_resp_t mnet_info[NUM_MNETS_TEST];

int main(int argc, char *argv[])
{
    int ret;
    const char* iface_names[NUM_MNETS_TEST] = {"mnic-oob-", "mnic-inb0-", "mnic-inb1-"};

    //create mnic-oob
    mnet_info[0].req.devcmd_pa = 0x1380db000;
    mnet_info[0].req.devcmd_db_pa = 0x1380dc000;
    mnet_info[0].req.drvcfg_pa = 0x06040000;
    mnet_info[0].req.msixcfg_pa = 0x06010000;
    mnet_info[0].req.iface_type = MNIC_TYPE_ETH;
    snprintf(mnet_info[0].req.iface_name, sizeof(mnet_info[0].req.iface_name), iface_names[0]);

    ret = create_mnet(&mnet_info[0]);

    if(ret)
    {
        printf("create_mnet failed with err: %d\n", ret);
        exit(-1);
    }

    printf("iface_name returned by mnet for mnic-oob: %s\n", mnet_info[0].resp.eth_iface_name);

    //create mnic-inb0
    mnet_info[1].req.devcmd_pa = 0x1380dd000;
    mnet_info[1].req.devcmd_db_pa = 0x1380de000;
    mnet_info[1].req.drvcfg_pa = 0x06040020;
    mnet_info[1].req.msixcfg_pa = 0x06010010;
    mnet_info[1].req.iface_type = MNIC_TYPE_ETH;
    snprintf(mnet_info[1].req.iface_name, sizeof(mnet_info[1].req.iface_name), iface_names[1]);

    ret = create_mnet(&mnet_info[1]);

    if(ret)
    {
        printf("mnet iface: %s : create_mnet failed with err: %d\n", mnet_info[1].req.iface_name, ret);
        exit(-1);
    }

    printf("iface_name returned by mnet for mnic-inb0: %s\n", mnet_info[1].resp.eth_iface_name);

    //create mnic-inb1
    mnet_info[2].req.devcmd_pa = 0x1380df000;
    mnet_info[2].req.devcmd_db_pa = 0x1380e0000;
    mnet_info[2].req.drvcfg_pa = 0x06040040;
    mnet_info[2].req.msixcfg_pa = 0x06010020;
    mnet_info[2].req.iface_type = MNIC_TYPE_ETH;
    snprintf(mnet_info[2].req.iface_name, sizeof(mnet_info[2].req.iface_name), iface_names[2]);

    ret = create_mnet(&mnet_info[2]);

    if(ret)
    {
        printf("mnet iface: %s : create_mnet failed with err: %d\n", mnet_info[1].req.iface_name, ret);
        exit(-1);
    }

    printf("iface_name returned by mnet for mnic-inb1: %s\n", mnet_info[2].resp.eth_iface_name);

    //removing mnet devices
    remove_mnet((mnet_info[0].resp.eth_iface_name));
    remove_mnet((mnet_info[1].resp.eth_iface_name));
    remove_mnet((mnet_info[2].resp.eth_iface_name));

    return 0;
}

