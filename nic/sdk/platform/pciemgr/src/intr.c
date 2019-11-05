/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"
#include "intr.h"

void
pciehw_intr_config(pciehwdev_t *phwdev, const int legacy, const int fmask)
{
    const u_int32_t intrb = phwdev->intrb;
    const u_int32_t intrc = phwdev->intrc;
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_fwcfg_mode(intr, legacy, fmask);
    }
}

/*
 * The legacy intx interrupt gets swizzled as it progresses
 * up the pcie hierarchy according to the "device" number.
 *
 * In config space, the legacy IntA,IntB,IntC,IntD interrupts
 * are numbered 1,2,3,4 respectively.  We are doing this calculation
 * in 0-based 0,1,2,3 to make the mod math work out.  The hardware
 * int_pin register expects 0-based numbers so this matches the hw
 * register value we need to compute.
 *
 * See PCIe Base Spec Rev 4.0v1 Table 2-19 for more info.
 */
static int
swizzle_intx_pin(pciehwdev_t *phwdev)
{
    const u_int16_t d = bdf_to_dev(phwdev->bdf);
    pciehwdev_t *parent_hwdev = pciehwdev_get(phwdev->parenth);
    if (parent_hwdev == NULL) return d % 4;
    return (swizzle_intx_pin(parent_hwdev) + d) % 4;
}

void
pciehw_intr_init(pciehwdev_t *phwdev)
{
    const int port = phwdev->port;
    const u_int32_t lif = phwdev->lifb;
    const u_int32_t intrb = phwdev->intrb;
    const u_int32_t intrc = phwdev->intrc;
    u_int8_t intpin;
    u_int32_t intr;

    /*
     * Initialize all device interrupts.
     */

    /* intpin */
    phwdev->intpin = swizzle_intx_pin(phwdev);

    /* configure pba */
    intr_pba_cfg(lif, intrb, intrc);

    /* intr in legacy mode (for now) */
    intpin = phwdev->intpin;
    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_fwcfg_legacy(intr, lif, port, intpin);
    }

    /* return intr resources to default reset state */
    intr_reset_pci(intrb, intrc, phwdev->intrdmask);
}

void
pciehw_intrhw_init(void)
{
    intr_hwinit();
}
