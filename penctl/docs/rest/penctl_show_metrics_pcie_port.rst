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
powerdown: powerdown count .
link_dn2up_int : link_dn2up_int.
link_up2dn_int : link_up2dn_int.
sec_bus_rst_int : sec_bus_rst_int.
rst_up2dn_int : rst_up2dn_int.
rst_dn2up_int : rst_dn2up_int.
portgate_open2close_int : portgate_open2close_int.
ltssm_st_changed_int : ltssm_st_changed_int.
sec_busnum_changed_int : sec_busnum_changed_int.
rc_pme_int : rc_pme_int.
rc_aerr_int : rc_aerr_int.
rc_serr_int : rc_serr_int. 
rc_hpe_int : rc_hpe_int.
rc_eq_req_int : rc_eq_req_int.
rc_dpc_int : rc_dpc_int.
pm_turnoff_int : pm_turnoff_int.
txbfr_overflow_int : txbfr_overflow_int.
rxtlp_err_int : rxtlp_err_int.
tl_flr_req_int : tl_flr_req_int.
rc_legacy_intpin_changed_int : rc_legacy_intpin_changed_int.

IntrTotal	: total port intrs
IntrPolled	: total port intrs polled
IntrPerstn	: pcie out of reset
IntrLtssmstEarly	: link train before linkup
IntrLtssmst	: link train after  linkup
IntrLinkup2Dn	: link down
IntrLinkdn2Up	: link up
IntrRstup2Dn	: mac up
IntrRstdn2Up	: mac down
IntrSecbus	: secondary bus set
Linkup	: link is up
Hostup	: host is up (secbus)
Phypolllast	: phy poll count (last)
Phypollmax	: phy poll count (max)
Phypollperstn	: phy poll lost perstn
Phypollfail	: phy poll failed
Gatepolllast	: gate poll count (last)
Gatepollmax	: gate poll count (max)
Markerpolllast	: marker poll count (last)
Markerpollmax	: marker poll count (max)
Axipendpolllast	: axipend poll count (last)
Axipendpollmax	: axipend poll count (max)
Faults	: link faults
Powerdown	: powerdown count
LinkDn2UpInt	: link_dn2up_int
LinkUp2DnInt	: link_up2dn_int
SecBusRstInt	: sec_bus_rst_int
RstUp2DnInt	: rst_up2dn_int
RstDn2UpInt	: rst_dn2up_int
PortgateOpen2CloseInt	: portgate_open2close_int
LtssmStChangedInt	: ltssm_st_changed_int
SecBusnumChangedInt	: sec_busnum_changed_int
RcPmeInt	: rc_pme_int
RcAerrInt	: rc_aerr_int
RcSerrInt	: rc_serr_int
RcHpeInt	: rc_hpe_int
RcEqReqInt	: rc_eq_req_int
RcDpcInt	: rc_dpc_int
PmTurnoffInt	: pm_turnoff_int
TxbfrOverflowInt	: txbfr_overflow_int
RxtlpErrInt	: rxtlp_err_int
TlFlrReqInt	: tl_flr_req_int
RcLegacyIntpinChangedInt	: rc_legacy_intpin_changed_int

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
      --compat-1.1         run in 1.1 firmware compatibility mode
      --dsc-url string     set url for Distributed Service Card
  -j, --json               display in json format (default true)
  -t, --tabular            display in table format
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format

SEE ALSO
~~~~~~~~

* `penctl show metrics pcie <penctl_show_metrics_pcie.rst>`_ 	 - Metrics for Pcie Subsystem

