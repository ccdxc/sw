Steps:

1. cd nic/tools/hal/protoc-gen-mtool/
2. go install (this will create this binary into $GOPATH/bin/
   (if you see an error about missing gogo proto, just do 'go get github.com/gogo/protobuf/proto')
3. ls -F ../../../proto/hal/*.proto | xargs echo | xargs protoc -I$GOPATH/src/github.com/gogo/protobuf/gogoproto -I../../../proto/hal -I../../../../vendor/github.com/gogo/protobuf/gogoproto -I../../../hal/third-party/google/include --mtool_out=/tmp/
