.. _penctl_logs:

penctl logs
-----------

Show logs from Naples

Synopsis
~~~~~~~~



------------------------------
 Show Module Logs From Naples 
------------------------------


::

  penctl logs [flags]

Options
~~~~~~~

::

  -h, --help            help for logs
  -m, --module string   Module to show logs for
			Valid modules are:
				apigw
				apiserver
				cmd
				ckm
				etcd
				filebeat
				ntp
				vchub
				npm
				influx
				evtsmgr
				spyglass
				elastic
				evtsproxy
				kube-apiserver
				kube-scheduler
				kube-controller-manager
				kubelet
				tpm
				tsm
				nmd
				netagent
				k8sagent
				vos
				aggregator
				citadel


Options inherited from parent commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

  -j, --json      display in json format
  -t, --tabular   display in tabular format (default true)
  -v, --verbose   verbose output
  -y, --yaml      display in yaml json

SEE ALSO
~~~~~~~~

* `penctl <penctl.rst>`_ 	 - Pensando CLIs

