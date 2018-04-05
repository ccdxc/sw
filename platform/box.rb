from "registry.test.pensando.io:5000/pensando/nic:1.21"

PACKAGES = %w[
  patch readline-devel
]

env GOPATH: "/usr"
workdir "/sw"
run "yum install -y #{PACKAGES.join(" ")}"
