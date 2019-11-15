from "registry.test.pensando.io:5000/pensando/nic:1.37"

copy "venice/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
