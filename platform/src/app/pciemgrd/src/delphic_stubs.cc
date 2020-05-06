/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>

#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"

int delphi_client_start(void)
{
    return 0;
}

void update_pcie_port_status(const int port,
                             const pciemgr_port_status_t status,
                             const int gen,
                             const int width,
                             const int reversed,
                             const char *faultstr)
{
}

void update_pcie_metrics(const int port)
{
}
