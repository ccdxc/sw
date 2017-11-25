from "registry.test.pensando.io:5000/pensando/nic:1.1"

env GOPATH: "/usr"
run "yum install tcpdump -y"
run "pip3 install mock zmq grpcio"

workdir "/sw"

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", "/sw", ignore_list: %w[.git]
end

copy "dol/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
