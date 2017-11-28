from "golang:1.9"

PACKAGE = "github.com/pensando/sw"
DIR     = "/go/src/#{PACKAGE}"

#enable jessie-backports
run 'echo "deb http://http.debian.net/debian jessie-backports main" > /etc/apt/sources.list.d/jessie-backports.list'
run "apt-get update && apt-get install rsync libpcap-dev -y"
run "apt-get install net-tools"
run "apt-get install -y libltdl-dev softhsm2"
run "go get golang.org/x/tools/cmd/goimports"

run "mkdir -p '#{DIR}'"
workdir DIR

entrypoint []
cmd %w[/bin/bash]
