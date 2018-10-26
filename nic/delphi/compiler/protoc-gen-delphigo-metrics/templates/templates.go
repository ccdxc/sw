package templates

// templates for delphi code generation
var (
	GometricsTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

{{$fileName := .GetName}}
{{$pkgName := .Package}}
{{$msgs := .Messages}}
{{range $msgs}}
  {{if (HasSuffix .GetName "Metrics")}}
type {{.GetName}} struct {
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
    {{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
	Key       {{.GetGolangTypeName}}
	{{else}}
	Key       {{.GetGolangTypeName}}
	{{end}}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	{{.GetName}} gometrics.Counter
    {{else if (eq .GetTypeName ".delphi.Gauge") }}
	{{.GetName}}    gometrics.Gauge
	{{end}} {{end}}

	// private state
	metrics gometrics.Metrics
}

// Size returns the size of the metrics object
func (mtr *{{.GetName}}) Size() int {
	sz := 0
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
    {{if (eq .GetName "Key") }}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	sz += mtr.{{.GetName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	sz += mtr.{{.GetName}}.Size()
	{{end}} {{end}}
	return sz
}

// Unmarshall unmarshall the raw counters from shared memory
func (mtr *{{.GetName}}) Unmarshall() error {
	var offset int
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
    {{if (eq .GetName "Key") }}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	mtr.{{.GetName}} = mtr.metrics.GetCounter(offset)
	offset += mtr.{{.GetName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	mtr.{{.GetName}} = mtr.metrics.GetGauge(offset)
	offset += mtr.{{.GetName}}.Size()
	{{end}} {{end}}
	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *{{.GetName}}) getOffset(fldName string) int {
	var offset int

	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
	{{if (eq .GetName "Key") }}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	if fldName == "{{.GetName}}" {
		return offset
	}
	offset += mtr.{{.GetName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	if fldName == "{{.GetName}}" {
		return offset
	}
	offset += mtr.{{.GetName}}.Size()
	{{end}} {{end}}
	return offset
}

{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
{{else if (eq .GetTypeName ".delphi.Counter") }}
// Set{{.GetName}} sets cunter in shared memory
func (mtr *{{$msgName}}) Set{{.GetName}}(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("{{.GetName}}"))
	return nil
}
{{else if (eq .GetTypeName ".delphi.Gauge") }}
// Set{{.GetName}} sets gauge in shared memory
func (mtr *{{$msgName}}) Set{{.GetName}}(val gometrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("{{.GetName}}"))
	return nil
}
{{end}} {{end}}

// {{.GetName}}Iterator is the iterator object
type {{.GetName}}Iterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *{{.GetName}}Iterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *{{.GetName}}Iterator) Next() *{{.GetName}} {
	mtr := it.iter.Next()
	tmtr := &{{.GetName}}{metrics: mtr}
	tmtr.Unmarshall()
	return tmtr
}

// Find finds the metrics object by key
func (it *{{.GetName}}Iterator) Find(key uint32) (*{{.GetName}}, error) {
	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(key))))
	if err != nil {
		return nil, err
	}
	tmtr := &{{.GetName}}{metrics: mtr}
	tmtr.Unmarshall()
	return tmtr, nil
}

// Create creates the object in shared memory
func (it *{{.GetName}}Iterator) Create(key uint32) (*{{.GetName}}, error) {
	tmtr := &{{.GetName}}{}
	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(key))), tmtr.Size())
	tmtr = &{{.GetName}}{metrics: mtr}
	tmtr.Unmarshall()
	return tmtr, nil
}

// Delete deletes the object from shared memory
func (it *{{.GetName}}Iterator) Delete(key uint32) error {
	return it.iter.Delete(string(proto.EncodeVarint(uint64(key))))
}

// New{{.GetName}}Iterator returns an iterator
func New{{.GetName}}Iterator() (*{{.GetName}}Iterator, error) {
	iter, err := gometrics.NewMetricsIterator("{{.GetName}}")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &{{.GetName}}Iterator{iter: iter}, nil
}
{{end}}
{{end}}
`
)
