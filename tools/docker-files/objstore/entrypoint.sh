#!/bin/sh
echo "starting with args $@"
minio server --address :19001 /data
