from "registry.test.pensando.io:5000/pensando/nic:1.40"

env GOPATH: "/usr"
run "pip3 install mock zmq grpcio"

workdir "/sw"

copy "dol/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
