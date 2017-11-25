from "registry.test.pensando.io:5000/pensando/nic:1.1"

PACKAGES = %w[
  patch
]

env GOPATH: "/usr"
workdir "/sw"
run "yum install -y #{PACKAGES.join(" ")}"
