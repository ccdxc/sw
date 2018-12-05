broker_mock.go: ../broker.go
	mockgen -source=../broker.go --package=mock  -destination broker_mock.go
	goimports -local "github.com/pensando/sw" -w *.go

gen-clean:
	rm -f broker_mock.go
