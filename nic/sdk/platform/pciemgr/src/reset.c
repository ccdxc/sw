/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"
#include "platform/misc/include/bdf.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

static void
pciehw_reset_lifs_event(pciehwdev_t *phwdev,
                        const int lifb, const int lifc,
                        const pciehdev_rsttype_t rsttype)
{
    pciehdev_eventdata_t evd;
    pciehdev_reset_t *reset;

    memset(&evd, 0, sizeof(evd));
    evd.evtype = PCIEHDEV_EV_RESET;
    evd.port = phwdev->port;
    evd.lif = phwdev->lifb;
    reset = &evd.reset;
    reset->rsttype = rsttype;
    reset->lifb = lifb;
    reset->lifc = lifc;
    pciehw_event(phwdev, &evd);
}

static void
pciehw_reset_event(pciehwdev_t *phwdev, const pciehdev_rsttype_t rsttype)
{
    pciehw_reset_lifs_event(phwdev, phwdev->lifb, phwdev->lifc, rsttype);
}

static void
pciehw_reset_device(pciehwdev_t *phwdev, const pciehdev_rsttype_t rsttype)
{
    const u_int32_t intrb = phwdev->intrb;
    const u_int32_t intrc = phwdev->intrc;
    const int dmask = phwdev->intrdmask;

#ifdef PCIEMGR_DEBUG
    pciesys_logdebug("%s: dev reset\n", pciehwdev_get_name(phwdev));
#endif

    if (rsttype != PCIEHDEV_RSTTYPE_NONE) {
        pciehw_reset_event(phwdev, rsttype);
    }
    intr_reset_pci(intrb, intrc, dmask);
    pciehw_cfg_reset(phwdev, rsttype);
}

static void
pciehw_reset_descendents(pciehwdev_t *phwdev, const pciehdev_rsttype_t rsttype)
{
    pciehwdev_t *pchild;

    for (pchild = pciehwdev_get(phwdev->childh);
         pchild != NULL;
         pchild = pciehwdev_get(pchild->peerh)) {

        pciehw_reset_device(pchild, rsttype);

        /*
         * If we are a PF then resetting our cfg space will disable and
         * reset all active VFs so no need to reset them again.  If this
         * is a bridge with child devices, go reset those children here.
         */
        if (!pchild->pf) {
            pciehw_reset_descendents(pchild, rsttype);
        }
    }
}

/*
 * A "bus" reset originates on a bridge device with a request
 * for a secondary bus reset.  We're called with the pwhdev of
 * the bridge, but the bridge doesn't get reset.  We reset all
 * the descendents of the bridge device.
 */
void
pciehw_reset_bus(pciehwdev_t *phwdev, const u_int8_t bus)
{
    pciesys_loginfo("%s: bus reset 0x%02x\n", pciehwdev_get_name(phwdev), bus);
    pciehw_reset_descendents(phwdev, PCIEHDEV_RSTTYPE_BUS);
}

/*
 * Function Level Reset (FLR) is issued on a device endpoint to reset
 * the device.  If issued on a PF then all the VFs get reset too.
 */
void
pciehw_reset_flr(pciehwdev_t *phwdev)
{
    pciesys_loginfo("%s: flr reset\n", pciehwdev_get_name(phwdev));
    pciehw_reset_device(phwdev, PCIEHDEV_RSTTYPE_FLR);
}

/*
 * A PF controls enabling of VFs.  If some enabled VFs get disabled
 * by the PF then we want to reset the VFs.
 *
 * In order to reduce the number of msgs generated for this reset event
 * we compress all the VF reset msgs into a single reset msg spanning
 * all the lifs that were affected.
 */
void
pciehw_reset_vfs(pciehwdev_t *phwdev, const int vfb, const int vfc)
{
    pciehwdev_t *vfhwdev;
    int vfidx;

    pciesys_loginfo("%s: vfs reset %d-%d\n",
                    pciehwdev_get_name(phwdev), vfb, vfb + vfc - 1);
    for (vfidx = vfb; vfidx < vfb + vfc; vfidx++) {
        vfhwdev = pciehwdev_getvf(phwdev, vfidx);
        pciehw_reset_device(vfhwdev, PCIEHDEV_RSTTYPE_NONE);
    }
    vfhwdev = pciehwdev_getvf(phwdev, vfb);
    pciehw_reset_lifs_event(phwdev, vfhwdev->lifb, vfhwdev->lifc * vfc,
                            PCIEHDEV_RSTTYPE_VF);
}
