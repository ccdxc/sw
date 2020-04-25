from "registry.test.pensando.io:5000/pensando/nic:1.42"

inside "/etc" do
    run "rm localtime"
    run "ln -s /usr/share/zoneinfo/US/Pacific localtime"
end

copy "entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
