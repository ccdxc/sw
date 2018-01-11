#!/bin/bash

set -euo pipefail

term() {
	killall dockerd
	wait
}

dockerd -s vfs &

trap term INT TERM

exec sh -c "$@"
