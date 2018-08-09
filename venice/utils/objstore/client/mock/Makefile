mock:
	mockgen -source=../objstore.go --package=mock  -destination mock.go objStoreBackend
	goimports -local "github.com/pensando/sw" -w *.go

