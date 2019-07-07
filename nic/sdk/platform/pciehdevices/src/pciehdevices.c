/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

typedef struct pciehdevice_info_s {
    pciehdevice_t *devtab;
    unsigned int ndevs;
} pciehdevice_info_t;
static pciehdevice_info_t pciehdevice_info;

/*
 * This is called early by constructors.
 */
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
pciehdevice_lookup(const pciehdevice_type_t type)
{
    pciehdevice_info_t *pdi = &pciehdevice_info;
    pciehdevice_t *pdevice;
    int i;

    for (pdevice = pdi->devtab, i = 0; i < pdi->ndevs; i++, pdevice++) {
        if (pdevice->type == type) {
            return pdevice;
        }
    }
    return NULL;
}

static int
verify_common_res(const pciehdevice_resources_t *pres)
{
    const pciehdev_res_t *pfres = &pres->pfres;
    const pciehdev_res_t *vfres = &pres->vfres;

#define V(expr) \
    if (!(expr)) { \
        pciesys_logdebug("verify_common_res: %s expect %s\n", \
                         pres->pfres.name, #expr);            \
    }

    /* Verify the common PF resources. */
    V(pfres->is_vf == 0);
    V(pfres->cmb_stride == 0);
    V(pfres->dsn_stride == 0);

    /* Verify the common VF resources. */
    if (pfres->totalvfs) {
        V(vfres->is_vf);
        V(vfres->totalvfs == 0);
        V(vfres->port == pfres->port);
        V(vfres->romsz == 0);

        /* strides only used for vfidx > 1 */
        if (pfres->totalvfs > 1) {
            V(vfres->cmbsz == 0 || vfres->cmb_stride != 0);
            V(vfres->dsn == 0 || vfres->dsn_stride != 0);
        }
    }
    return 0;
}

pciehdev_t *
pciehdevice_new(const pciehdevice_resources_t *pres)
{
    pciehdevice_t *pdevice = pciehdevice_lookup(pres->type);
    const pciehdev_res_t *pfres = &pres->pfres;
    const pciehdev_res_t *vfres = &pres->vfres;
    int totalvfs = pfres->totalvfs;
    pciehdev_t *pfdev, *vfdev;
    int vfidx;

    pfdev = vfdev = NULL;

    /* unknown device type? */
    if (pdevice == NULL) {
        pciesys_logerror("pciehdevice_new: bad type %d\n", pres->type);
        goto error_out;
    }

    /* verify the common resourses. */
    if (verify_common_res(pres) < 0) {
        goto error_out;
    }

    /* VFs requested for a device that doesn't provide VFs? */
    if (totalvfs && pdevice->initvf == NULL) {
        pciesys_logwarn("pciehdevice_new: %s sriov requested (%d) "
                        "but unsupported for device type %d\n",
                        pres->pfres.name, totalvfs, pres->type);
        totalvfs = 0;
    }

    /*
     * Set the common PF device resources.
     */
    pfdev = pciehdev_new();
    pciehdev_set_name(pfdev, pfres->name);
    pciehdev_set_port(pfdev, pfres->port);
    pciehdev_set_lifb(pfdev, pfres->lifb);
    pciehdev_set_lifc(pfdev, pfres->lifc);
    pciehdev_set_intrb(pfdev, pfres->intrb);
    pciehdev_set_intrc(pfdev, pfres->intrc);
    pciehdev_set_intrm(pfdev, pfres->intrdmask);

    if (totalvfs) {
        pciehdev_set_pf(pfdev, 1);
        pciehdev_set_totalvfs(pfdev, totalvfs);
    }

    /*
     * Init VF first, so PF init can derive some info from VF
     * e.g. vendor/device id, vf bars.
     */
    for (vfidx = 0; vfidx < totalvfs; vfidx++) {
        char vfname[32];

        vfdev = pciehdev_new();

        /*
         * Set the common VF device resources.
         */
        snprintf(vfname, sizeof(vfname), "%s-vf%d", pfres->name, vfidx);
        pciehdev_set_name(vfdev, vfname);
        pciehdev_set_vf(vfdev, 1);
        pciehdev_set_vfidx(vfdev, vfidx);
        pciehdev_set_port(vfdev, vfres->port);

        if (pdevice->initvf(vfdev, vfres) < 0) {
            goto error_out;
        }

        /*
         * Add this new VF to the PF device list.
         */
        pciehdev_addvf(pfdev, vfdev);
        vfdev = NULL; /* now belongs to pfdev */
    }

    /*
     * Now initialize PF, with optional VFs attached.
     */
    if (pdevice->initpf(pfdev, pfres) < 0) {
        goto error_out;
    }

    return pfdev;

 error_out:
    if (vfdev) pciehdev_delete(vfdev);
    if (pfdev) pciehdev_delete(pfdev);
    return NULL;
}

void
pciehdevice_delete(pciehdev_t *pdev)
{
    pciehdev_delete(pdev);
}
