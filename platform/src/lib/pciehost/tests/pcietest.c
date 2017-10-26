#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pciehost.h"

int
main(int argc, char *argv[])
{
#if 0
    pcieh = pcieh_open(openargs);
    pdev = pciehdev_alloc(pcieh);
    pcfg = pciehcfg_alloc(pdev);
    pbar = pciehbar_alloc(pdev);
    pciehbar_free(pbar);
    pciehcfg_free(pcfg);
    pciehdev_free(pdev);
    pcieh_close(pcieh);
#endif

#if 0
    pcieh_t pcieh;
    pciehdev_t pdev;
    pciehcfg_t pcfg;
    pciehbar_t pbar;
    pciehreg_t preg;
#define PCIEH_NONE (-1) // XXX

    pcieh = pcieh_open(openargs);
    pcieh_dev_alloc(pcieh, &pdev);
    pcieh_cfg_alloc(pdev, &pcfg);

    pcieh_cfg_enet_template(pcfg); // XXX
    pcieh_cfg_set_enet_template(pcfg); // XXX
    pcieh_cfg_enetvf_template(pcfg); // XXX
    pcieh_cfg_nvme_template(pcfg); // XXX
    pcieh_cfg_nvmevf_template(pcfg); // XXX
    pcieh_cfg_vdev_template(pcfg); // XXX

    pcieh_bar_alloc(pdev, &pbar);
    pcieh_bar_addreg(pbar, preg);

    pcieh_dev_add(parent, pdev); // XXX

    pcieh_dev_finalize(pdev); // XXX
    pcieh_finalize(pcieh); // XXX

    pcieh_cfg_free(pdev);
    pcieh_bar_free(pdev);
    pcieh_dev_free(pdev);
    pcieh_close(pcieh);
#endif

#if 0
    pcieh = pcieh_open(openargs);
    pcieh_dev_alloc(pcieh, &pdev);

    pcieh_dev_set_enet_template(pdev, &params);
    pcieh_dev_config_enet_template(pdev, &params);
    pcieh_dev_config_enet(pdev, &params);

    pcieh_dev_set_enetvf_template(pdev, &params);

    pcieh_dev_add(parent, pdev); // XXX

    pcieh_dev_finalize(pdev); // XXX
    pcieh_finalize(pcieh); // XXX

    while (1) {
        pcieh_initialize(pcieh); // XXX
        ...
        pcieh_finalize(pcieh); // XXX
    }

    pcieh_dev_free(pdev);
    pcieh_close(pcieh);
#endif

#if 0
    pcieh = pcieh_open(openargs);
    pcieh_initialize(pcieh);

    pcieh_dev_alloc(pcieh, &pdev);

    pcieh_dev_set_enet_template(pdev, &params);
    pcieh_dev_config_enet_template(pdev, &params);
    pcieh_dev_config_enet(pdev, &params);

    pcieh_dev_set_enetvf_template(pdev, &params);

    pcieh_dev_add(parent, pdev); // XXX
    pcieh_dev_finalize(pdev); // XXX
    pcieh_dev_free(pdev);
    ...
    pcieh_finalize(pcieh); // XXX

    while (1) {
        pcieh_initialize(pcieh); // XXX
        ...
        pcieh_finalize(pcieh); // XXX
    }

    pcieh_dev_free(pdev);
    pcieh_close(pcieh);
#endif

#if 0
    pcieh = pcieh_open();
    while (1) {
        pcieh_initialize(pcieh);
        pcieh_bus_topology(pcieh);
        foreach_dev(dev) {
            getparams(dev, &params);
            pcieh_dev_enet(pcieh, &pdev);
            pcieh_dev_add(parent, pdev);
            pcieh_dev_finalize(pdev, &params);
        }
        pcieh_finalize();
    }
    pcieh_close();
#endif

#if 1
    // INJECT EVENTS DOWN:
    // bus_reset();
    // dev_reset();
    //     send reset events up to client (need registration)
    // initialize(); // reset hw
    // finalize(); // load to hw
    //
    // GENERATE EVENTS UP:
    // linkup(&negotiated_params);
    // 

    // top-level management
    // os-specific sys interface
    pciehmgr() {
        // SOFT
        // lay out bus topology
        // manage bridges,devices
        pciehdev() {
            // initialize device config space
            pciehcfg() {
            }
            // initialize device bar space, rombars
            pciehbar() {
                // manage bar regions
                pciehbarreg() {
                }
            }
        }

        // bind soft topology to hw
        // plug in to pciehmgr() layer?
        // store info in share mem
        //     no stored pointers, only "handles" (indexes)
        //     so different contexts (user/kernel/threadx?) can follow links
        // separate lib?
        pciehdevhw() {
            // manage config space strides in memory
            // set registers PMT, PRT, BDFT, etc
            // handle processing of indirect events in
            // send noteworthy events up (resets, linkup/dn)
        }
    }

    pcieh = pciehmgr_open(init, sysops, def_vendorid, def_subids) {
        pciehhw_open(init, sysops);
    }
    while (1) {
        pciehmgr_initialize(pcieh) {
            pciehhw_initialize(); // CRS=1
        }
        foreach_dev(dev) {
            getparams(dev, &devparams);
            pdev = pciehdev_construct("enet", &devparams) {
                ops = pciehdev_find("enet");
                pdev = pciehdev_new(&devparams, ops, privsz);
                ops->dev_initialize(pdev) {
                    pcfg = enetcfg_construct() {
                        pcfg = pciehcfg_new(&cfgparams);
                        pciehcfg_sethdr_type0(pcfg);
                        pciehcfg_setcap_pm(pcfg);
                        pciehcfg_setcap_msi(pcfg);
                        pciehcfg_setcap_msix(pcfg);
                        pciehcfg_setextcap_pcie(pcfg);
                        return pcfg;
                    }
                    pciehdev_add_cfg(pdev, pcfg);

                    pbars = enetbars_construct() {
                        pbars = pciehbars_new(&barparams);
                        if (is_boot) {
                            oprom_bar = pciehbar_new();
                            pciehbars_add_bar(pbars, oprom_bar);
                        }
                    }
                    pciehdev_add_bars(pdev, pbars) {
                        if (pdev->pcfg) {
                            pciehcfg_set_bars(pcfg, pbars);
                        }
                    }
                }
                return pdev;
            }
            pciehdev_construct("enet", params);
            pciehdev_construct("enetvf", params);
            pciehdev_construct("bridgeup", params);
            pciehdev_construct("bridgedn", params);

            pciehdev_add(pdev); // add PF to next default location
            pciehdev_add_fn(pdev, pfn); // add multifunction to base
            pciehdev_add_vf(pdev, pvf); // add VFs to PF
            pciehdev_add_child(parentdev, pdev);
            pciehdev_add_root(pbridge); // XXX??? need this?
        }
        pciehmgr_finalize() { // CRS=0
            foreach_dev(root) {
                pciehdev_finalize(pdev);
            }
            pciehhw_finalize(proot);
        }
    }
    pciehmgr_close(pcieh) {
        pciehhw_close();
    }
#endif

    exit(0);
}
