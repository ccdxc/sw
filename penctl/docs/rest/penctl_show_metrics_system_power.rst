.. _penctl_show_metrics_system_power:

penctl show metrics system power
--------------------------------

System power information

Synopsis
~~~~~~~~



---------------------------------
 System power information:


Value Description:

pin: Input power to the system.
pout1: Core output power.
pout2: Arm output power.
The power is milli Watt

Pin	: Input power to the system
Pout1	: Core output power
Pout2	: ARM output power

---------------------------------


::

  penctl show metrics system power [flags]

Options
~~~~~~~

::

  -h, --help   help for power

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

* `penctl show metrics system <penctl_show_metrics_system.rst>`_ 	 - Metrics for system monitors

