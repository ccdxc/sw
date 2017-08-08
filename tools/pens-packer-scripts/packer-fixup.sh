#!/bin/bash

rm -rf packer-templates/.git
rm -rf packer-templates/.gitignore
rm -rf packer-templates/.travis.yml

cp pens-packer-scripts/pens.sh packer-templates/scripts/centos/
cp pens-packer-scripts/repo.sh packer-templates/scripts/centos-7.3/repo.sh
