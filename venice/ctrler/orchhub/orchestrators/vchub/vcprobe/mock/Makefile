vcprobe_mock.go: ../vcprobe.go
	mockgen -source=../vcprobe.go --package=mock  -destination vcprobe_mock.go
	goimports -local "github.com/pensando/sw" -w *.go

gen-clean:
	rm -f vcprobe_mock.go
