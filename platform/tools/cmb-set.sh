#!/bin/bash

if "${1:-true}" ; then
	echo 31 > /sys/module/ionic_rdma/parameters/sqcmb_order
	echo N > /sys/module/ionic_rdma/parameters/sqcmb_inline
	echo 31 > /sys/module/ionic_rdma/parameters/rqcmb_order
	echo N > /sys/module/ionic_rdma/parameters/rqcmb_sqcmb
else
	echo 0 > /sys/module/ionic_rdma/parameters/sqcmb_order
	echo 0 > /sys/module/ionic_rdma/parameters/rqcmb_order
fi
