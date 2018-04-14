#!/bin/bash

#if [ "$1" = "" ]; then
    #echo "usage: $0 <installed-directory-path>"
    #exit;
#fi

echo "Stopping NAPLES container ..."
docker rm -f naples-v1
echo "Stopped NAPLES container ..."
