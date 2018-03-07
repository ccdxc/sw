SRC := ${GOPATH}/src/github.com/pensando/sw/venice/utils/kvstore/watch.go
MOCKOBJ := mock_watcher.go

${MOCKOBJ}: ${SRC} Makefile
	mockgen -destination=$@ -package=mock --source ${SRC} Watcher
	#  package name
	sed -i 's/\bWatchEvent/kvstore.WatchEvent/g' $@
	# imports
	sed -i '/import (/a kvstore "github.com/pensando/sw/venice/utils/kvstore"' $@
	goimports -local "github.com/pensando/sw" -w $@

.PHONY : clean
clean:
	rm -f ${MOCKOBJ}

