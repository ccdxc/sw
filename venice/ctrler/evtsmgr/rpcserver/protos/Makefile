EXTRA_INCLUDES =  -I${GOPATH}/src/github.com/pensando/sw/api/protos
EXTRA_GOFAST_OUT = Mgithub.com/pensando/sw/api/protos/events.proto=github.com/pensando/sw/api/generated/events,Mgithub.com/pensando/sw/api/protos/eventpolicy.proto=github.com/pensando/sw/api/generated/monitoring,
SERVICE_PROTOS := $(shell ls *.proto)

all:  $(SERVICE_PROTOS:.proto=.pb.go)
include ../../../../Makefile.proto