#!/bin/bash

# Wait up to 5 mins
for i in {1..30}; do
  output=`grep "Eject the installation media" /var/log/messages | wc -l`
  if [ $output -eq "1" ]; then
    echo "venice installation completed!!!"
    sleep 10
    break
  fi
  echo "($i) venice installation not done yet, sleeping for 10 secs"
  sleep 10
done

