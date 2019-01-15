.. _penctl_update_port:

penctl update port
------------------

update port object

Synopsis
~~~~~~~~


update port object

::

  penctl update port [flags]

Options
~~~~~~~

::

      --admin-state string   Set port admin state - none, up, down (default "up")
      --auto-neg string      Enable or disable auto-neg using enable | disable (default "disable")
      --fec-type string      Specify fec-type - rs, fc, none (default "none")
  -h, --help                 help for port
      --mtu uint32           Specify port MTU
      --pause string         Specify pause - link, pfc, none (default "none")
      --port uint32          Specify port number
      --speed string         Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g

Options inherited from parent commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

  -i, --interface string   ethernet device of naples
  -j, --json               display in json format
  -t, --tabular            display in tabular format (default true)
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format

SEE ALSO
~~~~~~~~

* `penctl update <penctl_update.rst>`_ 	 - Update Object

