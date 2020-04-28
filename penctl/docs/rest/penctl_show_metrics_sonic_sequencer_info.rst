.. _penctl_show_metrics_sonic_sequencer_info:

penctl show metrics sonic sequencer_info
----------------------------------------

Sequencer queues information

Synopsis
~~~~~~~~



---------------------------------
 Sequencer queues information:
 Key indices - LifId: logical interface ID, QId: queue ID


Value Description:


QStateAddr	: queue state memory address
QGroup	: queue group
           : 0 - compress/decompress
           : 1 - compress/decompress status
           : 2 - crypto
           : 3 - crypto status

CoreId	: CPU core ID (not available currently

---------------------------------


::

  penctl show metrics sonic sequencer_info [flags]

Options
~~~~~~~

::

  -h, --help   help for sequencer_info

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

* `penctl show metrics sonic <penctl_show_metrics_sonic.rst>`_ 	 - Metrics for Storage Offload DSC

