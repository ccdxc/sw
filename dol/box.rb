from "srv1.pensando.io:5000/pensando/nic:1.1"

run "pip3 install mock zmq grpcio"

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", ".", ignore_list: %w[.git]
end

workdir "/sw"

tag "pensando/dol"
