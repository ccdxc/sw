#!/bin/bash

echo "Setting permissions for log volume..."
chown -R 1000:1000 /var/log/pensando/filebeat

/usr/local/bin/docker-entrypoint --path.logs=/var/log/pensando/filebeat