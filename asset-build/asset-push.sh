#!/bin/bash
if [ -z $RELEASE ]
then
  echo "RELEASE is not set"
else
  find . -type f -print0 | while IFS= read -r -d $'\0' file;
    do asset-push builds hourly $RELEASE "$file" ;
  done
fi
