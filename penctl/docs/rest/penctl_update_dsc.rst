.. _penctl_update_dsc:

penctl update dsc
-----------------

Set Distributed Service Card Modes and Profiles

Synopsis
~~~~~~~~



----------------------------
 Set Distributed Service Card configuration 
----------------------------


::

  penctl update dsc [flags]

Options
~~~~~~~

::

  -c, --controllers stringSlice     List of controller IP addresses or ids
  -g, --default-gw string           Default GW for mgmt
  -d, --dns-servers stringSlice     List of DNS servers
  -s, --dsc-profile string          Active Distributed Service Card Profile (default "FEATURE_PROFILE_BASE")
  -h, --help                        help for dsc
  -i, --id string                   DSC ID
  -o, --managed-by string           Distributed Service Card Management. host or network (default "host")
  -k, --management-network string   Management Network. inband or oob
  -m, --mgmt-ip string              Management IP in CIDR format
  -f, --naples-profile string       Active Distributed Service Card Profile (default "FEATURE_PROFILE_BASE")
  -p, --primary-mac string          Primary mac

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

