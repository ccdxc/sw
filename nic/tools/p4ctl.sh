#!/bin/sh

export NIC_DIR='/nic'
export PLATFORM_DIR='/platform'
export HAL_CONFIG_PATH='/nic/conf'

if ! [ -x "$(command -v locale)" ]; then
  echo 'Error: locale is not installed. Please follow the p4ctl README ' >&2
  exit 1
fi

export LD_LIBRARY_PATH=$NIC_DIR/lib:$PLATFORM_DIR/lib
$NIC_DIR/bin/p4ctl repl

