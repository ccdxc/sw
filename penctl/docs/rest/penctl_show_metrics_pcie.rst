.. _penctl_show_metrics_pcie:

penctl show metrics pcie
------------------------

Metrics for pciemgr

Synopsis
~~~~~~~~



---------------------------------
 Metrics for pciemgr
---------------------------------


Options
~~~~~~~

::

  -h, --help   help for pcie

Options inherited from parent commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

  -a, --authtoken string   path to file containing authorization token
  -j, --json               display in json format (default true)
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format

SEE ALSO
~~~~~~~~

* `penctl show metrics <penctl_show_metrics.rst>`_ 	 - Show metrics from Naples
* `penctl show metrics pcie pciemgr <penctl_show_metrics_pcie_pciemgr.rst>`_ 	 - Pcie Mgr Metrics information:
not_intr : notify total intrs.
not_spurious : notify spurious intrs.
not_cnt : notify total txns.
not_max : notify max txns per intr.
not_cfgrd : notify config reads.
not_cfgwr : notify config writes.
not_memrd : notify memory reads.
not_memwr : notify memory writes.
not_iord : notify io reads.
not_iowr : notify io writes.
not_unknown : notify unknown type.
not_rsrv0 : notify rsrv0.
not_rsrv1 : notify rsrv1.
not_msg : notify pcie message.
not_unsupported : notify unsupported.
not_pmv : notify pgm model violation.
not_dbpmv : notify doorbell pmv.
not_atomic : notify atomic trans.
not_pmtmiss : notify PMT miss.
not_pmrmiss : notify PMR miss.
not_prtmiss : notify PRT miss.
not_bdf2vfidmiss : notify bdf2vfid table miss.
not_prtoor : notify PRT out-of-range.
not_vfidoor : notify vfid out-of-range.
not_bdfoor : notify bdf out-of-range.
not_pmrind : notify PMR force indirect.
not_prtind : notify PRT force indirect.
not_pmrecc : notify PMR ECC error.
not_prtecc : notify PRT ECC error.
ind_intr : indirect total intrs.
ind_spurious : indirect spurious intrs.
ind_cfgrd : indirect config reads.
ind_cfgwr : indirect config writes.
ind_memrd : indirect memory reads.
ind_memwr : indirect memory writes.
ind_iord : indirect io reads.
ind_iowr : indirect io writes.
ind_unknown : indirect unknown type.


* `penctl show metrics pcie port <penctl_show_metrics_pcie_port.rst>`_ 	 - Pcie Port Metrics information:


Value Description:

intr_total : total port intrs.
intr_ltssmst_early : link train before linkup.
intr_ltssmst : link train after  linkup.
intr_linkup2dn : link down.
intr_linkdn2up : link up.
intr_rstup2dn : mac up.
intr_rstdn2up : mac down.
intr_secbus : secondary bus set.
linkup : link is up.
hostup : host is up (secbus).
phypolllast : phy poll count (last).
phypollmax : phy poll count (max).
phypollperstn : phy poll lost perstn.
phypollfail : phy poll failed.
gatepolllast : gate poll count (last).
gatepollmax : gate poll count (max).
markerpolllast : marker poll count (last).
markerpollmax : marker poll count (max).
axipendpolllast : axipend poll count (last).
axipendpollmax : axipend poll count (max).
faults : link faults.



