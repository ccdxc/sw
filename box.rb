import "box-base.rb"

run "make build"

cmd %w[make unit-test-verbose]

tag "pensando/sw"
