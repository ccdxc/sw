#!/bin/bash

LIMIT=10
for a in $(seq 1 $LIMIT)
do
  # check if all nodes are running
  numRunning=`vagrant ssh -c "kubectl get nodes" node1 | grep Ready | wc -l`
  echo $numRunning nodes are running
  if [ "$numRunning" -ge 2 ]
  then
    echo "All nodes are running"
    break
  fi

  # Check if we exceeded the loop
  if [ "$a" -ge $LIMIT ]
  then
    die 1 "All nodes are not running. Failing test"
  fi

  sleep 10
done

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
