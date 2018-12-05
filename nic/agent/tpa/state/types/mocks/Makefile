types_mock.go: ../types.go
	mockgen -source=../types.go --package=mocks  -destination types_mock.go
	goimports -local "github.com/pensando/sw" -w *.go

clean:
	rm -f types_mock.go
