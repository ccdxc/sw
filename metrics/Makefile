# Generate metrics manifest.
# Each path where protos for metrics exist is listed below as a tuple of prefix and the path seperated by a :
METRICS_PATHS =\
	venicepapi:api/protos \
	nicmgr:nic/proto/nicmgr \
	dropstats:nic/proto/dropstats \
	ftestats:nic/proto/ftestats \
	rulestats:nic/proto/rulestats \
	flowstats:nic/proto/flowstats \
	nmdmetrics:nic/agent/protos/nmd/nmdmetrics

# all generated files are in the generated/<prefix> path

all:
	@- $(foreach PATH,$(METRICS_PATHS), \
        $(eval PREFIX = $(word 1,$(subst :, ,$(PATH)))) \
        $(eval PROTOPATH = $(word 2,$(subst :, ,$(PATH)))) \
   		\
   		${GOPATH}/src/github.com/pensando/sw/tools/scripts/genmetrics.sh ${PREFIX} ${PROTOPATH}; \
		)

clean:
	@rm -rf generated/*

gen-clean: clean
