from "golang:1.9"

PACKAGE = "github.com/pensando/sw"
DIR     = "/go/src/#{PACKAGE}"

#enable jessie-backports
run 'echo "deb http://http.debian.net/debian jessie-backports main" > /etc/apt/sources.list.d/jessie-backports.list'
run "apt-get update && apt-get install rsync libpcap-dev -y"
run "apt-get install net-tools"
run "apt-get install -y libltdl-dev softhsm2 unzip"
run "go get golang.org/x/tools/cmd/goimports"
run "curl -LO https://github.com/google/protobuf/releases/download/v3.4.0/protoc-3.4.0-linux-x86_64.zip"
run "unzip protoc-3.4.0-linux-x86_64.zip"
run "mv bin/protoc /usr/local/bin"
run "mv include/google /usr/local/include/"
env "VENICE_DEV" => "1"
run "env"

run "mkdir -p '#{DIR}'"
workdir DIR

# install docker
run "curl -sSL https://get.docker.com | CHANNEL=stable bash"
copy "tools/test-build/daemon.json", "/etc/docker/daemon.json"

entrypoint []
cmd %w[/bin/bash]
