# Protobuf compiler plugin for delphi

This plugin is based on grpc-gateway protobuf plugin(https://github.com/grpc-ecosystem/grpc-gateway/tree/master/protoc-gen-grpc-gateway).

All the unnecessary parts of the grpc-gateway plugin has been trimmed down.

- `descriptor` package contains wrapper for protobuf descriptor (see https://github.com/golang/protobuf/tree/master/protoc-gen-go/descriptor)
- `generator` package contains the code generator which uses `templates` package to generate the code.
- `templates` contains delphi specific go template for code generation

`descriptor` and `generator` package can be used as a generic protobuf plugin framework to generate any code.
by feeding different templates to the code generator, different language binding for delphi can be generated.
