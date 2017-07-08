from "golang:1.8.3"

PACKAGE = "github.com/pensando/sw"
DIR     = "/go/src/#{PACKAGE}"

run "apt-get update && apt-get install rsync -y"

copy ".", DIR, ignore_list: [ ".git", "box.rb" ]

workdir DIR

run "make ws-tools"

entrypoint []
cmd %w[/bin/bash]
tag "pensando/sw:dependencies"
