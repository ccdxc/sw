.. _penctl_system_factory-default:

penctl system factory-default
-----------------------------

Perform "erase-config" plus remove all Distributed Service Card internal databases and diagnostic failure logs (reboot required)

Synopsis
~~~~~~~~



------------------------------------------------------------------------------------------------------------------
 Perform "erase-config" plus remove all Distributed Service Card internal databases and diagnostic failure logs (reboot required) 
------------------------------------------------------------------------------------------------------------------


::

  penctl system factory-default [flags]

Options
~~~~~~~

::

  -h, --help   help for factory-default

Options inherited from parent commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

  -a, --authtoken string   path to file containing authorization token
      --compat-1.1         run in 1.1 firmware compatibility mode
      --dsc-url string     set url for Distributed Service Card
  -j, --json               display in json format (default true)
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format

SEE ALSO
~~~~~~~~

* `penctl system <penctl_system.rst>`_ 	 - System Operations

