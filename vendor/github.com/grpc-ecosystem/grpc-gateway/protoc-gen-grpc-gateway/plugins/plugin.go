package plugin

import (
	"errors"
	"text/template"

	"github.com/gogo/protobuf/proto"
	"github.com/golang/glog"
	"github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway/descriptor"
)

// ParserFunc is a registered function to parse options. val has the raw option.
type ParserFunc func(val interface{}) (interface{}, error)

var parserMap map[string]ParserFunc
var FuncMap = template.FuncMap{}

var (
	errNotFound     = errors.New("extension Not found")
	errInvalidInput = errors.New("invalid Input")
	errParseFailed  = errors.New("could not parse option")
)

func getext(pb proto.Message, extname string) (interface{}, error) {
	glog.V(1).Infof("Got pb %+v", pb)
	desc, err := proto.ExtensionDescs(pb)
	if err != nil {
		return nil, err
	}
	for _, d := range desc {
		if d.Name == extname {
			opt, err := proto.GetExtension(pb, d)
			if err != nil {
				return nil, err
			}
			if parser, ok := parserMap[extname]; ok {
				return parser(opt)
			} else {
				return nil, errParseFailed
			}
		}
	}
	return nil, errNotFound
}

func GetExtension(name string, in interface{}) (interface{}, error) {
	switch in.(type) {
	case *descriptor.File:
		i := in.(*descriptor.File)
		p := i.FileDescriptorProto
		opts := p.GetOptions()
		if opts == nil {
			return nil, errNotFound
		}
		return getext(opts, name)
	case *descriptor.Service:
		i := in.(*descriptor.Service)
		p := i.ServiceDescriptorProto
		opts := p.GetOptions()
		if opts == nil {
			return nil, errNotFound
		}
		return getext(opts, name)
	case *descriptor.Method:
		i := in.(*descriptor.Method)
		p := i.MethodDescriptorProto
		opts := p.GetOptions()
		if opts == nil {
			return nil, errNotFound
		}
		return getext(opts, name)
	case *descriptor.Message:
		i := in.(*descriptor.Message)
		p := i.DescriptorProto
		opts := p.GetOptions()
		if opts == nil {
			return nil, errNotFound
		}
		return getext(opts, name)
	case *descriptor.Field:
		i := in.(*descriptor.Field)
		p := i.FieldDescriptorProto
		opts := p.GetOptions()
		if opts == nil {
			return nil, errNotFound
		}
		return getext(opts, name)
	default:
		glog.V(1).Infof("Got Type %t", in)
		return nil, errInvalidInput
	}
}

func RegisterOptionParser(name string, fn ParserFunc) {
	glog.V(1).Infof("registering parser for option %v", name)
	if parserMap == nil {
		parserMap = make(map[string]ParserFunc)
	}
	parserMap[name] = fn
}

func RegisterFunc(name string, fn interface{}) {
	glog.V(1).Infof("registering function %v", name)
	FuncMap[name] = fn
}
