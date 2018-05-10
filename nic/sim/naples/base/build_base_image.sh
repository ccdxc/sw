#! /bin/bash
set -e
docker build -t registry.test.pensando.io:5000/naples:base .
docker push registry.test.pensando.io:5000/naples:base
