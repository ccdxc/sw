#!/bin/bash

make penctl-version
OUT_DIR=output NAPLES_FW_NAME=naples_fw.tar FW_PACKAGE_DIR=capri make firmware-normal
if [ "$PIPELINE" = "iris" ]; then
OUT_DIR=output NAPLES_FW_NAME=naples_fw.tar FW_PACKAGE_DIR=capri make firmware-upgrade
fi
