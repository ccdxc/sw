from "golang:1.8.3"

PACKAGE = "github.com/pensando/sw"
DIR     = "/go/src/#{PACKAGE}"

run "apt-get update && apt-get install rsync libpcap-dev -y"
run "apt-get install net-tools"

if getenv("NO_COPY") == ""
  copy ".", DIR, ignore_list: [ ".git", "box.rb" ]
end

run "mkdir -p '#{DIR}'"
workdir DIR

entrypoint []
cmd %w[/bin/bash]
tag "pensando/sw:dependencies"
