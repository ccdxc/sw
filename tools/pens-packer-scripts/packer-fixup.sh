#!/bin/bash

rm -rf packer-templates/.git
rm -rf packer-templates/.gitignore
rm -rf packer-templates/.travis.yml

if [ "$TURIN" == "" ]; then
  cp pens-packer-scripts/pens.sh packer-templates/scripts/centos/
else
  cp pens-packer-scripts/pens_turin.sh packer-templates/scripts/centos/pens.sh
fi

cp pens-packer-scripts/repo.sh packer-templates/scripts/centos-7.4/repo.sh
