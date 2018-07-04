from "registry.test.pensando.io:5000/pensando/linux-src:4.14.0"

copy "./entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"
entrypoint "/entrypoint.sh"

