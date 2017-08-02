from "srv1.pensando.io:5000/pensando/nic:dependencies"

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", ".", ignore_list: %w[.git]
end

tag "pensando/nic"
