#!/bin/bash

# Run two pods
if ! vagrant ssh -c "kubectl run nginx1 --image=nginx" node1; then
  echo "Failed to start nginx container"
  exit 1
fi

if ! vagrant ssh -c "kubectl run nginx2 --image=nginx" node1; then
  echo "Failed to start nginx container"
  exit 1
fi

# Verify that both pods came up
LIMIT=10
for a in $(seq 1 $LIMIT)
do
  # Sleep for a while
  sleep 10

  # check if both the pods are running
  numRunning=`vagrant ssh -c "kubectl get pods" node1 | grep nginx | grep Running | wc -l`
  echo $numRunning nginx pods are running
  if [ "$numRunning" -ge 2 ]
  then
    echo "Test SUCCESS: all nginx pods are running"
    break
  fi

  # Check if we exceeded the loop
  if [ "$a" -ge $LIMIT ]
  then
    echo "Nginx pods are not running. Failing test"
    exit 1
  fi
done

if ! vagrant ssh -c "kubectl delete deploy nginx1 nginx2" node1; then
  echo "Failed to stop nginx container"
  exit 1
fi
