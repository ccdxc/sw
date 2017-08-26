package plugin

import (
	protoplugin "github.com/gogo/protobuf/protoc-gen-gogo/plugin"
	"github.com/golang/glog"
)

// RequestMutatorFunc is the registered function that mutates the CodeGen request req
type RequestMutatorFunc func(req *protoplugin.CodeGeneratorRequest)

// MutatorMap is a map of registered Mutator functions.
var MutatorMap map[string]RequestMutatorFunc

// RegisterReqMutator registers a mutator function that will be called during initialization
func RegisterReqMutator(name string, fn RequestMutatorFunc) {
	glog.V(1).Infof("registering function %v", name)
	if MutatorMap == nil {
		MutatorMap = make(map[string]RequestMutatorFunc)
	}
	MutatorMap[name] = fn
}
