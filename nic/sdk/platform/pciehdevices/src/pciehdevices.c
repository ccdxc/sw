/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

typedef struct pciehdevice_info_s {
    pciehdevice_t *devtab;
    unsigned int ndevs;
} pciehdevice_info_t;
static pciehdevice_info_t pciehdevice_info;

void
pciehdevice_register(pciehdevice_t *pdevice)
{
    pciehdevice_info_t *pdi = &pciehdevice_info;

    pdi->devtab = realloc(pdi->devtab,
                          (pdi->ndevs + 1) * sizeof(pciehdevice_t));
    assert(pdi->devtab != NULL);
    pdi->devtab[pdi->ndevs++] = *pdevice;
}

static pciehdevice_t *
pciehdevice_lookup(const char *name)
{
    pciehdevice_info_t *pdi = &pciehdevice_info;
    pciehdevice_t *pdevice;
    int i;

    for (pdevice = pdi->devtab, i = 0; i < pdi->ndevs; i++, pdevice++) {
        if (strcmp(pdevice->name, name) == 0) {
            return pdevice;
        }
    }
    return NULL;
}

pciehdev_t *
pciehdevice_new(const char *type,
                const char *name,
                const pciehdevice_resources_t *pres)
{
    pciehdevice_t *pdevice = pciehdevice_lookup(type);
    const pciehdev_res_t *pfres = &pres->pfres;
    const pciehdev_res_t *vfres = &pres->vfres;
    pciehdev_t *pfdev, *vfdev;

    /* unknown device type? */
    if (pdevice == NULL) {
        return NULL;
    }

    assert(pfres->is_vf == 0);
    assert(pfres->devcmd_stride == 0);
    assert(pfres->devcmddb_stride == 0);
    assert(pfres->cmb_stride == 0);
    assert(pfres->dsn_stride == 0);

    if (pfres->totalvfs) {
        assert(vfres->is_vf);
        assert(vfres->totalvfs == 0);
        assert(vfres->port == pfres->port);
        assert(vfres->romsz == 0);

        if (pfres->totalvfs > 1) {
            assert(vfres->devcmdpa == 0 || vfres->devcmd_stride != 0);
            assert(vfres->devcmddbpa == 0 || vfres->devcmddb_stride != 0);
            assert(vfres->cmbsz == 0 || vfres->cmb_stride != 0);
            assert(vfres->dsn == 0 || vfres->dsn_stride != 0);
        }
    }

    pfdev = pciehdev_new();
    pciehdev_set_name(pfdev, name);
    pciehdev_set_port(pfdev, pfres->port);
    pciehdev_set_lifb(pfdev, pfres->lifb);
    pciehdev_set_lifc(pfdev, pfres->lifc);
    pciehdev_set_intrb(pfdev, pfres->intrb);
    pciehdev_set_intrc(pfdev, pfres->intrc);

    /*
     * Init VF first, so PF init can derive some info from VF
     * e.g. vendor/device id, vf bars.
     */
    if (pfres->totalvfs > 0) {
        char vfname[32];

        vfdev = pciehdev_new();

        snprintf(vfname, sizeof(vfname), "%s-vf", name);
        pciehdev_set_name(vfdev, vfname);
        pciehdev_set_port(vfdev, vfres->port);
        pciehdev_set_lifb(vfdev, vfres->lifb);
        pciehdev_set_lifc(vfdev, vfres->lifc);
        pciehdev_set_intrb(vfdev, vfres->intrb);
        pciehdev_set_intrc(vfdev, vfres->intrc);

        if (pdevice->init_vfbars) pdevice->init_vfbars(vfdev, vfres);
        if (pdevice->init_vfcfg)  pdevice->init_vfcfg (vfdev, vfres);

        pciehdev_linkvf(pfdev, vfdev, pfres->totalvfs);
    }

    pdevice->init_bars(pfdev, pfres);
    pdevice->init_cfg(pfdev, pfres);

    return pfdev;
}

void
pciehdevice_delete(pciehdev_t *pdev)
{
    pciehdev_delete(pdev);
}
