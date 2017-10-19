from "registry.test.pensando.io:5000/pensando/nic:1.2"

env GOPATH: "/usr"

inside "/etc" do
  run "rm localtime"
  run "ln -s /usr/share/zoneinfo/US/Pacific localtime"
end 

workdir "/sw"

# in the CI this block is triggered; in the Makefiles it is not.
if getenv("NO_COPY") == ""
  copy ".", "/sw", ignore_list: %w[.git]
end

copy "nic/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"

tag "pensando/nic"
