.PHONY: all test clean

all: test

compiler_test.pb.go: compiler_test.proto
	protoc --delphigo_out=Mnic/delphi/proto/delphi/delphi.proto=github.com/pensando/sw/nic/delphi/proto/delphi:. \
		./compiler_test.proto -I /usr/src/github.com/pensando/sw -I .

test: compiler_test.pb.go
	go test -cover 

clean:
	rm -f compiler_test.pb.go
