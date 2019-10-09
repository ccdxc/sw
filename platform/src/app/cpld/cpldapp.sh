#!/bin/sh

export LD_LIBRARY_PATH=$FWUPDATE_RESOURCES/lib
exec $FWUPDATE_RESOURCES/bin/cpldapp-real $*
