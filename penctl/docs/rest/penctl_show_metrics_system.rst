.. _penctl_show_metrics_system:

penctl show metrics system
--------------------------

Metrics for system monitors

Synopsis
~~~~~~~~



---------------------------------
 Metrics for system monitors
---------------------------------


Options
~~~~~~~

::

  -h, --help   help for system

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
* `penctl show metrics system frequency <penctl_show_metrics_system_frequency.rst>`_ 	 - System frequency information:


Value Description:

frequency: Frequency of the system.
The frequency is MHz


* `penctl show metrics system memory <penctl_show_metrics_system_memory.rst>`_ 	 - System memory information:


Value Description:

Total Memory: Total Memory of the system.
Available Memory: Available Memory of the system.
Free Memory: Free Memory of the system.
The memory is KB


* `penctl show metrics system power <penctl_show_metrics_system_power.rst>`_ 	 - System power information:


Value Description:

pin: Input power to the system.
pout1: Core output power.
pout2: Arm output power.
The power is milli Watt


* `penctl show metrics system temp <penctl_show_metrics_system_temp.rst>`_ 	 - System temperature information:


Value Description:

local_temperature: Temperature of the board.
die_temperature: Temperature of the die.
hbm_temperature: Temperature of the hbm.
The temperature is degree Celcius



