#!/bin/bash

set -e
set -x

sudo yum install -y wget curl git

cd /tmp
wget https://storage.googleapis.com/golang/go1.8.1.linux-amd64.tar.gz
sudo tar -C /usr/local -xzf go1.8.1.linux-amd64.tar.gz
rm go1.8.1.linux-amd64.tar.gz

# Install Swagger
sudo curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64
sudo chmod +x /usr/local/bin/swagger

# install commonly used go tools
tee -a /tmp/goinstall.bash <<ENDF
export GOBIN=/usr/local/go/bin
export PATH=/usr/local/go/bin:$PATH
go get  "github.com/tools/godep"
go get  "github.com/nsf/gocode"
go get  "github.com/alecthomas/gometalinter"
go get  "golang.org/x/tools/cmd/goimports"
go get  "golang.org/x/tools/cmd/guru"
go get  "golang.org/x/tools/cmd/gorename"
go get  "github.com/golang/lint/golint"
go get  "github.com/rogpeppe/godef"
go get  "github.com/kisielk/errcheck"
go get  "github.com/jstemmer/gotags"
go get  "github.com/klauspost/asmfmt/cmd/asmfmt"
go get  "github.com/zmb3/gogetdoc"
go get  "github.com/josharian/impl"
gometalinter -i
ENDF

chmod +x /tmp/goinstall.bash
sudo bash -c /tmp/goinstall.bash

exit 0
