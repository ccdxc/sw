from "registry.test.pensando.io:5000/pensando/nic:1.41"

inside "/etc" do
  run "rm localtime"
  run "ln -s /usr/share/zoneinfo/US/Pacific localtime"
end

copy "nic/apollo/tools/athena/customer-docker/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
