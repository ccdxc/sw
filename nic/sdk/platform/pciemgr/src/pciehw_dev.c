/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/pciemgr/include/pciehw.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/pciemgrutils/include/pciehdev_impl.h"
#include "pciehw_impl.h"

#define NPORTS 8

typedef struct pciehdev_portinfo_s {
    u_int32_t initialized:1;
    u_int32_t finalized:1;
    u_int32_t memtun_br:1;
    pciehdev_t *root;
} pciehdev_portinfo_t;

typedef struct pciehdev_data_s {
    pciemgr_params_t params;
    pciehdev_evhandler_t evhandler;
    pciehdev_portinfo_t portinfo[NPORTS];
} pciehdev_data_t;

static pciehdev_data_t pciehdev_data;

static pciehdev_data_t *
pciehdev_get_data(void)
{
    return &pciehdev_data;
}

/******************************************************************/

int
pciehdev_open(pciemgr_params_t *params)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();

    if (params) pdevdata->params = *params;
    pciehcfg_set_params(params);
    return pciehw_open(params);
}

void
pciehdev_close(void)
{
    pciehw_close();
}

/******************************************************************/

int
pciehdev_initialize(const int port)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];

    assert(port >= 0 && port < NPORTS);

    if (pi->finalized) {
        pi->finalized = 0;
    }
    pciehw_initialize_topology(port);
    pciehdev_delete(pi->root);
    pi->root = NULL;
    pi->memtun_br = 0;
    pi->initialized = 1;
    return 0;
}

static int
pciehdev_finalize_dev(pciehdev_t *pdev, u_int16_t bdf, u_int8_t *nextbus)
{
    if (pdev) {
        pdev->bdf = bdf;

#if 0
        printf("finalize_dev: %-8s bdf %s par %s peer %s child %s\n",
               pciehdev_get_name(pdev), bdf_to_str(pdev->bdf),
               pdev->parent ? pciehdev_get_name(pdev->parent) : "(none)",
               pdev->peer ? pciehdev_get_name(pdev->peer) : "(none)",
               pdev->child ? pciehdev_get_name(pdev->child) : "(none)");
#endif

        /*
         * Process child link first, then peer, so we match
         * the path of the BIOS scan.  Our assigned bdf's
         * will match the actual bdf's when the BIOS assigns
         * bus numbers to bridges.
         */
        if (pdev->child) {
            int childbdf;
            if (pdev->pf || pdev->fn0) {
                childbdf = bdf_make(bdf_to_bus(bdf),
                                    bdf_to_dev(bdf),
                                    1);
            } else {
                childbdf = bdf_make((*nextbus)++, 0, 0);
            }
            pciehdev_finalize_dev(pdev->child, childbdf, nextbus);
        }

        if (pdev->peer) {
            int peerbdf;
            if (pdev->peer->vf || pdev->peer->fnn) {
                /* ARI math, rolls over into dev/bus if needed */
                peerbdf = bdf + 1;
            } else {
                peerbdf = bdf_make(bdf_to_bus(bdf), bdf_to_dev(bdf) + 1, 0);
            }
            pciehdev_finalize_dev(pdev->peer, peerbdf, nextbus);
        }
    }
    return 0;
}

int
pciehdev_finalize(const int port)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    u_int8_t nextbus = pdevdata->params.first_bus;

    assert(port >= 0 && port < NPORTS);

    if (!pi->initialized) {
        return -EBADF;
    }
    if (pi->finalized) {
        return -EBUSY;
    }

    /* add a bridge to complete this topology */
    if (pi->root == NULL) {
        pciehdev_t *pbrdn;
        const int memtun_en = pi->memtun_br == 0;
        pi->root = pciehdev_bridgeup_new();
        pi->root->port = port;
        pbrdn = pciehdev_bridgedn_new(port, memtun_en);
        pbrdn->port = port;
        pciehdev_addchild(pi->root, pbrdn);
        if (memtun_en) pi->memtun_br = 1;
    }

    pciehdev_finalize_dev(pi->root, bdf_make(0, 0, 0), &nextbus);
    pciehw_finalize_topology(pi->root);
    pi->finalized = 1;
    return 0;
}

int
pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    const u_int8_t port = pdev ? pdev->port : 0;
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    pciehdev_t **ppdev;

    if (pi->finalized) {
        return -EBUSY;
    }
    ppdev = pdev ? &pdev->child : &pi->root;
    for (; *ppdev; ppdev = &(*ppdev)->peer) {
        continue;
    }
    *ppdev = pchild;
    if (pchild) {
        pchild->parent = pdev;
    }
    return 0;
}

static int
pciehdev_add_sriov(pciehdev_t *pbrdn, pciehdev_t *pdev)
{
    assert(pdev->pf);
    assert(!pdev->vf);
    assert(pdev->totalvfs);
    assert(pdev->child);
    assert(!pdev->child->pf);
    assert(pdev->child->vf);

    pciehdev_addchild(pbrdn, pdev);
    //pciehdev_addchild(pdev, pdev->child);
    return 0;
}

static int
pciehdev_add_multifunc(pciehdev_t *pbrdn, pciehdev_t *pdev)
{
    /* XXX unimplemented */
    assert(0);
    return 0;
}

static int
pciehdev_add_dev(pciehdev_t *pbrdn, pciehdev_t *pdev)
{
    return pciehdev_addchild(pbrdn, pdev);
}

int
pciehdev_add(pciehdev_t *pdev)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    const u_int8_t port = pdev->port;
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    pciehdev_t *pbrdn;
    int memtun_en = 0;
    int r = 0;

    if (pdev->port >= NPORTS) {
        return -EINVAL;
    }
    if (pdev->name[0] == '\0') {
        return -ENXIO;
    }
    if (!pi->initialized) {
        return -EBADF;
    }
    if (pi->finalized) {
        return -EBUSY;
    }

    /*
     * Make sure we have a downstream port to connect this device.
     */
    if (pi->root == NULL) {
        pi->root = pciehdev_bridgeup_new();
        pi->root->port = port;
    }
    memtun_en = pi->memtun_br == 0;
    pbrdn = pciehdev_bridgedn_new(port, memtun_en);
    pbrdn->port = port;
    pciehdev_addchild(pi->root, pbrdn);
    if (memtun_en) pi->memtun_br = 1;

    /*
     * Now connect dev to bridge.
     */
    if (pdev->pf) {
        r = pciehdev_add_sriov(pbrdn, pdev);
    } else if (pdev->fn0) {
        r = pciehdev_add_multifunc(pbrdn, pdev);
    } else {
        r = pciehdev_add_dev(pbrdn, pdev);
    }
    return r;
}

int
pciehdev_addfn(pciehdev_t *pdev, pciehdev_t *pfn, const int fnc)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    const u_int8_t port = pdev->port;
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];

    if (pdev->name[0] == '\0') {
        return -ENXIO;
    }
    if (pi->finalized) {
        return -EBUSY;
    }
    pciehdev_make_fn0(pdev);
    pciehdev_make_fnn(pfn, fnc);
    return pciehdev_addchild(pdev, pfn);
}

pciehdev_t *
pciehdev_get_root(const u_int8_t port)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehdev_portinfo_t *pi = &pdevdata->portinfo[port];
    return pi->root;
}

pciehdev_t *
pciehdev_get_parent(pciehdev_t *pdev)
{
    return pdev->parent;
}

pciehdev_t *
pciehdev_get_child(pciehdev_t *pdev)
{
    return pdev->child;
}

pciehdev_t *
pciehdev_get_peer(pciehdev_t *pdev)
{
    return pdev->peer;
}

static pciehdev_t *
pciehdev_find_by_name(pciehdev_t *pdev, const char *name)
{
    pciehdev_t *pdev_found = NULL;

    if (pdev == NULL) {
        return NULL;
    }
    if (strcmp(pdev->name, name) == 0) {
        pdev_found = pdev;
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_name(pdev->peer, name);
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_name(pdev->child, name);
    }
    return pdev_found;
}

pciehdev_t *
pciehdev_get_by_name(const char *name)
{
    pciehdev_t *pdev_found = NULL;
    int port;

    for (port = 0; port < NPORTS; port++) {
        pciehdev_t *root = pciehdev_get_root(port);

        pdev_found = pciehdev_find_by_name(root, name);
        if (pdev_found) break;
    }
    return pdev_found;
}

static pciehdev_t *
pciehdev_find_by_bdf(pciehdev_t *pdev, const u_int16_t bdf)
{
    pciehdev_t *pdev_found = NULL;

    if (pdev == NULL) {
        return NULL;
    }
    if (pdev->bdf == bdf) {
        pdev_found = pdev;
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_bdf(pdev->peer, bdf);
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_bdf(pdev->child, bdf);
    }
    return pdev_found;
}

pciehdev_t *
pciehdev_get_by_bdf(const u_int8_t port, const u_int16_t bdf)
{
    pciehdev_t *root = pciehdev_get_root(port);
    return pciehdev_find_by_bdf(root, bdf);
}

void *
pciehdev_get_hwdev(pciehdev_t *pdev)
{
    return pdev->phwdev;
}

void
pciehdev_set_hwdev(pciehdev_t *pdev, void *phwdev)
{
    pdev->phwdev = phwdev;
}

u_int16_t
pciehdev_get_bdf(pciehdev_t *pdev)
{
    return pdev->bdf;
}

int
pciehdev_register_event_handler(pciehdev_evhandler_t evhandler)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pdevdata->evhandler = evhandler;
    return 0;
}

void
pciehdev_event(const pciehdev_eventdata_t *evd)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();

    if (pdevdata->evhandler) {
        pdevdata->evhandler(evd);
    }
}
