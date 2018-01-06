#!/bin/bash

VENICE_DIR=$GOPATH/src/github.com/pensando/sw/venice/cli
cd $VENICE_DIR/gen

# generate objects that templates need to work on first
cp pregen/.gen.go pregen/genobjs.go
go run gen.go objs

# then generate objects, cli, cmds, etc.
go run gen.go

goimports -local "github.com/pensando/sw" -l -w $VENICE_DIR/gen/pregen/genobjs.go
goimports -local "github.com/pensando/sw" -l -w $VENICE_DIR/genobjs.go
goimports -local "github.com/pensando/sw" -l -w $VENICE_DIR/gencmds.go
goimports -local "github.com/pensando/sw" -l -w $VENICE_DIR/testserver/tserver/genobjs.go
goimports -local "github.com/pensando/sw" -l -w $VENICE_DIR/testserver/tserver/tserver.go

exit 0
