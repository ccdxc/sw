all: geneventtypes

geneventtypes:
	${GOPATH}/src/github.com/pensando/sw/tools/scripts/geneventtypes.sh protos

clean:
	@rm -rf generated/eventtypes generated/eventattrs/*.pb.go

gen-clean: clean