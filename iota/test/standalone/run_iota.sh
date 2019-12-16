#!/bin/bash

set -ex

cd /sw/iota
cat /warmd.json

./iota.py --testbed /warmd.json $@
