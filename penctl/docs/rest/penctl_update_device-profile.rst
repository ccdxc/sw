.. _penctl_update_device-profile:

penctl update device-profile
----------------------------

Distributed Service Card profile object

Synopsis
~~~~~~~~



----------------------------
 Update Distributed Service Card Profiles 
----------------------------


::

  penctl update device-profile [flags]

Options
~~~~~~~

::

  -h, --help                  help for device-profile
  -n, --name string           Name of the Device Profile to be updated
  -p, --port-default string   Set default port admin state for next reboot. (enable | disable) (default "enable")

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

* `penctl update <penctl_update.rst>`_ 	 - Update Object

