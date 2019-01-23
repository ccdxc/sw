SERVICE_PROTOS = $(wildcard *.proto)
include ../../../../Makefile.proto

metric_mock.go: metric.pb.go
	mockgen -source=metric.pb.go --package=mock  -destination mock/metric_mock.go
	goimports -local "github.com/pensando/sw" -w *.go
