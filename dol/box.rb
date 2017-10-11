from "registry.test.pensando.io:5000/pensando/nic:1.1"

run "pip3 install mock zmq grpcio"

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", ".", ignore_list: %w[.git]
end

copy "entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"
workdir "/sw"

entrypoint "/entrypoint.sh"

tag "pensando/dol"
