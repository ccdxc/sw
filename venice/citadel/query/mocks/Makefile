broker_mock.go: ../query.go
	mockgen -source=../query.go --package=mocks  -destination broker_mock.go
	goimports -local "github.com/pensando/sw" -w *.go

gen-clean:
	rm -f broker_mock.go
