#!/bin/bash

for i in /etc/passwd /etc/shadow /etc/hostname /etc/host.conf /etc/nsswitch.conf /etc/hosts
do
    [[ -f $i ]] && cp -a --parents $i /config/copy-fs/
done
