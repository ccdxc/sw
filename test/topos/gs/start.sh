#!/bin/sh
cd /home/vm/venice
docker load -i /home/vm/venice/pen-dind.tar
docker load -i /home/vm/venice/pen-e2e.tar
docker load -i /home/vm/venice/venice/tars/pen-netagent.tar

./dind/do.py -disableTmpfs -configFile /home/vm/venice/testbed.json -custom_config_file /home/vm/venice/venice-conf.json -venice_image_dir /home/vm/venice/venice
echo "Waiting to create auth policy and user......"
sleep 120
newman run /home/vm/venice/authbootstrap_postman_collection.json
/home/vm/venice/naples_admit.py
