from "registry.test.pensando.io:5000/pensando/nic:1.39"

PACKAGES = %w[
  patch libedit2 libedit-devel
]

env GOPATH: "/usr"

workdir "/sw"

run "yum install -y #{PACKAGES.join(" ")}"

copy "platform/entrypoint.sh", "/entrypoint.sh"
run "chmod +x /entrypoint.sh"

entrypoint "/entrypoint.sh"
