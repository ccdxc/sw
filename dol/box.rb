from "srv1.pensando.io:5000/pensando/nic:dependencies"

run "pip3 install mock zmq grpcio"

workdir "/sw/dol"

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", ".", ignore_list: %w[.git]
end

tag "pensando/dol"
