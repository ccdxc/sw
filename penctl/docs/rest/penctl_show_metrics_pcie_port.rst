.. _penctl_show_metrics_pcie_port:

penctl show metrics pcie port
-----------------------------

Pcie Port Metrics information

Synopsis
~~~~~~~~



---------------------------------
 Pcie Port Metrics information:
Value Description:
intr_total : total port intrs.
intr_polled : total port intrs polled.
intr_perstn : pcie out of reset.
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


---------------------------------


::

  penctl show metrics pcie port [flags]

Options
~~~~~~~

::

  -h, --help   help for port

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

* `penctl show metrics pcie <penctl_show_metrics_pcie.rst>`_ 	 - Metrics for Pcie Subsystem

