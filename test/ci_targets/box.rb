from "ubuntu:18.04"

env PATH: "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/usr/local/go/bin:/go/bin",
    GOPATH: "/go"

run %Q[sed -i -e 's|security.ubuntu.com|package-mirror.test.pensando.io|g' -e 's|archive.ubuntu.com|package-mirror.test.pensando.io|g' /etc/apt/sources.list]
run "apt update"
run "apt install -y curl libltdl-dev"
run "curl -sSL http://package-mirror.test.pensando.io/go1.10.3.linux-amd64.tar.gz | tar xz -C /usr/local"

workdir "/go"
