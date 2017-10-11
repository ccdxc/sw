#!/bin/sh

set -euo pipefail

cp -R ../vendor/github.com/gogo/protobuf/protobuf/* /usr/local/include/

exec "$@"
