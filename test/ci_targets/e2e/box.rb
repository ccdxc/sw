from "ubuntu:18.04"

env PATH: "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/usr/local/go/bin:/go/bin",
    GOPATH: "/go"

run "apt update"
run "apt install -y curl libltdl-dev"
run "curl -sSL http://package-mirror.test.pensando.io/go1.9.5.linux-amd64.tar.gz | tar xz -C /usr/local"

workdir "/go"
