from "registry.test.pensando.io:5000/centos:7.3.1611"

PACKAGES = %w[
  qemu-img
]

run "yum install -y #{PACKAGES.join(" ")}"

cmd "bash"

run "yum clean all"

flatten
