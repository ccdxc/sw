mock.go: ../objstore.go
	mockgen -source=../objstore.go --package=mock  -destination mock.go objStoreBackend
	goimports -local "github.com/pensando/sw" -w *.go

gen-clean:
	rm -f mock.go

