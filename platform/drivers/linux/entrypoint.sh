#!/bin/sh

make -C /sw pull-assets
exec "$@"
