from "registry.test.pensando.io:5000/pen-test-build:v0.4"

PACKAGE = "github.com/pensando/sw"
DIR     = "/go/src/#{PACKAGE}"

env "VENICE_DEV" => "1"
env "CGO_LDFLAGS_ALLOW" => "-I/usr/local/share/libtool"
run "env"

run "mkdir -p '#{DIR}'"
workdir DIR

entrypoint []
cmd %w[/bin/bash]
