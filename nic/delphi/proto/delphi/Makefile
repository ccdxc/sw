
default:
	LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GOPATH}/src/github.com/pensando/sw/nic/hal/third-party/google/x86_64/lib ${GOPATH}/src/github.com/pensando/sw/nic/hal/third-party/google/x86_64/bin/protoc -I ./ -I /usr/local/include/ -I ../../../hal/third-party/google/include/ --gofast_out=. delphi.proto

gen-clean:
