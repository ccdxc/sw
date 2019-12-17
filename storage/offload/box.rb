from "registry.test.pensando.io:5000/pensando/nic:1.39"

PACKAGES = %w[
  numactl-devel libuuid-devel libaio-devel CUnit-devel 
]

env GOPATH: "/usr"
run "yum install -y #{PACKAGES.join(" ")}"

workdir "/sw"
copy "storage/offload/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
