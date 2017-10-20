#!/bin/bash

export GOBIN=/usr/local/go/bin
export PATH=/usr/local/go/bin:$PATH

go get  "github.com/tools/godep"
go get  "github.com/nsf/gocode"
go get  "github.com/alecthomas/gometalinter"
go get  "golang.org/x/tools/cmd/goimports"
go get  "golang.org/x/tools/cmd/guru"
go get  "golang.org/x/tools/cmd/gorename"
go get  "github.com/golang/lint/golint"
go get  "golang.org/x/tools/cmd/goimports"
go get  "github.com/rogpeppe/godef"
go get  "github.com/kisielk/errcheck"
go get  "github.com/jstemmer/gotags"
go get  "github.com/klauspost/asmfmt/cmd/asmfmt"
go get  "github.com/zmb3/gogetdoc"
go get  "github.com/josharian/impl"
gometalinter -i
