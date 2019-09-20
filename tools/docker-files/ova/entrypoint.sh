#!/bin/bash

set -x
set -e
echo | ./VMware-ovftool-4.3.0-13981069-lin.x86_64.bundle --eulas-agreed
cat venice-livecd.json | sed "s/d9e731670bd9dcb0597c565888d974ba8ae0de37/$(sha1sum pen-install.iso | cut -f1 -d ' ')/" > /tmp/venice-livecd.json
/t/packer build /tmp/venice-livecd.json

rm -fr packer_cache
chmod -R 777 output-venice-livecd
