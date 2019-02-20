.. _penctl_update_naples:

penctl update naples
--------------------

Set NAPLES Modes and Feature Profiles

Synopsis
~~~~~~~~



----------------------------
 Set NAPLES configuration 
----------------------------


::

  penctl update naples [flags]

Options
~~~~~~~

::

  -c, --controllers stringSlice   List of controller IP addresses or hostnames
  -g, --default-gw string         Default GW for mgmt
  -d, --dns-servers stringSlice   List of DNS servers
  -f, --feature-profile string    Active NAPLES Profile
  -h, --help                      help for naples
  -n, --hostname string           Host name
  -o, --management-mode string    host or network managed (default "host")
  -m, --mgmt-ip string            Management IP in CIDR format
  -k, --network-mode string       oob or inband
  -p, --primary-mac string        Primary mac

Options inherited from parent commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

  -j, --json      display in json format (default true)
      --verbose   display penctl debug log
  -v, --version   display version of penctl
  -y, --yaml      display in yaml format

SEE ALSO
~~~~~~~~

* `penctl update <penctl_update.rst>`_ 	 - Update Object

