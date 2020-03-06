package templates

// templates for delphi code generation
var (
	GometricsTemplate = `// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/golang/protobuf/proto"
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

{{$fileName := .GetName}}
{{$pkgName := .Package}}
{{$msgs := .Messages}}
{{range $msgs}}
  {{if (HasSuffix .GetName "Metrics")}}
type {{.GetName}} struct {
	ObjectMeta      api.ObjectMeta
	{{if (.HasExtOption "delphi.singleton")}}
	key 	        int
	{{end}}
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
    {{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
           {{if (HasSuffix .GetGolangTypeName "Key")}}
	key       {{.GetGolangTypeName}}
           {{else}} {{ ThrowError "Key field type doesnt have Key suffix" $fileName $msgName }}
           {{end}}
	{{else}}
	key       {{.GetGolangTypeName}}
	{{end}}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	{{if (eq .GetJsonName "-") }}
		{{.GetCamelCaseName}} metrics.Counter {{Quote}}structs:"{{.GetJsonName}}"{{Quote}}
	{{else}}
		{{.GetCamelCaseName}} metrics.Counter
	{{end}}
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	{{if (eq .GetJsonName "-") }}
		{{.GetCamelCaseName}} metrics.Gauge {{Quote}}structs:"{{.GetJsonName}}"{{Quote}}
	{{else}}
		{{.GetCamelCaseName}} metrics.Gauge
	{{end}}
	{{end}} {{end}}

	// private state
	metrics gometrics.Metrics
}
	
{{if (.HasExtOption "delphi.singleton")}}
func (mtr *{{$msgName}}) GetKey() int {
        return 0
}
{{end}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
  {{if (eq .GetName "Key") }}
func (mtr *{{$msgName}}) GetKey() {{.GetGolangTypeName}} {
        return mtr.key
}
{{end}}{{end}}

// Size returns the size of the metrics object
func (mtr *{{.GetName}}) Size() int {
	sz := 0
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
    {{if (eq .GetName "Key") }}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	sz += mtr.{{.GetCamelCaseName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	sz += mtr.{{.GetCamelCaseName}}.Size()
	{{end}} {{end}}
	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *{{.GetName}}) Unmarshal() error {
	var offset int
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
	{{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
	proto.Unmarshal(mtr.metrics.GetKey(), &mtr.key)
	{{else}}
        gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())
	{{end}}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	mtr.{{.GetCamelCaseName}} = mtr.metrics.GetCounter(offset)
	offset += mtr.{{.GetCamelCaseName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	mtr.{{.GetCamelCaseName}} = mtr.metrics.GetGauge(offset)
	offset += mtr.{{.GetCamelCaseName}}.Size()
	{{end}} {{end}}
	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *{{.GetName}}) getOffset(fldName string) int {
	var offset int

	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
	{{if (eq .GetName "Key") }}
	{{else if (eq .GetTypeName ".delphi.Counter") }}
	if fldName == "{{.GetCamelCaseName}}" {
		return offset
	}
	offset += mtr.{{.GetCamelCaseName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	if fldName == "{{.GetCamelCaseName}}" {
		return offset
	}
	offset += mtr.{{.GetCamelCaseName}}.Size()
	{{end}} {{end}}
	return offset
}

{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
{{else if (eq .GetTypeName ".delphi.Counter") }}
// Set{{.GetCamelCaseName}} sets cunter in shared memory
func (mtr *{{$msgName}}) Set{{.GetCamelCaseName}}(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("{{.GetCamelCaseName}}"))
	return nil
}
{{else if (eq .GetTypeName ".delphi.Gauge") }}
// Set{{.GetCamelCaseName}} sets gauge in shared memory
func (mtr *{{$msgName}}) Set{{.GetCamelCaseName}}(val metrics.Gauge) error {
	mtr.metrics.SetGauge(val, mtr.getOffset("{{.GetCamelCaseName}}"))
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
	if mtr == nil {
		return nil
	}

	tmtr := &{{.GetName}}{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key
{{if (.HasExtOption "delphi.singleton")}}
func (it *{{.GetName}}Iterator) Find() (*{{.GetName}}, error) {
	var key int
	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(uint32(0)))
{{else}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
func (it *{{$msgName}}Iterator) Find(key {{.GetGolangTypeName}}) (*{{$msgName}}, error) {
	{{ if .TypeIsMessage }}
	buf, _ := proto.Marshal(&key)
	mtr, err := it.iter.Find(buf)
	{{else}}
	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))
	{{end}}
{{end}}{{end}}{{end}}
	if err != nil {
		return nil, err
	}
	tmtr := &{{$msgName}}{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory
{{if (.HasExtOption "delphi.singleton")}}
func (it *{{.GetName}}Iterator) Create() (*{{.GetName}}, error) {
	var key int
	tmtr := &{{.GetName}}{}
	mtr := it.iter.Create(gometrics.EncodeScalarKey(uint32(0)), tmtr.Size())
{{else}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
func (it *{{$msgName}}Iterator) Create(key {{.GetGolangTypeName}}) (*{{$msgName}}, error) {
	tmtr := &{{$msgName}}{}
	{{ if .TypeIsMessage }}
	buf, _ := proto.Marshal(&key)
	mtr := it.iter.Create(buf, tmtr.Size())
	{{else}}
	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())
	{{end}}
{{end}}{{end}}{{end}}
	tmtr = &{{.GetName}}{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory
{{if (.HasExtOption "delphi.singleton")}}
func (it *{{.GetName}}Iterator) Delete() error {
	return it.iter.Delete(gometrics.EncodeScalarKey(uint32(0)))
{{else}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
func (it *{{$msgName}}Iterator) Delete(key {{.GetGolangTypeName}}) error {
	{{ if .TypeIsMessage }}
	buf, _ := proto.Marshal(&key)
	return it.iter.Delete(buf)
	{{else}}
	return it.iter.Delete(gometrics.EncodeScalarKey(key))
	{{end}}
{{end}}{{end}}{{end}}
}

// Free frees the iterator memory
func (it *{{$msgName}}Iterator) Free() {
	it.iter.Free()
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
{{else}}
  {{if (HasSuffix .GetName "Key")}}
type {{.GetName}} struct {
    {{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
	{{.GetName}}	{{.GetGolangTypeName}} {{Quote}}protobuf:"{{.GetWireTypeName}},{{.GetNumber}},opt,name={{.GetName}},json={{.GetName}}" json:"{{.GetName}},omitempty"{{Quote}}
    {{end}}
}
func (m *{{$msgName}}) Reset()                    { *m = {{$msgName}}{} }
func (m *{{$msgName}}) String() string            { return proto.CompactTextString(m) }
func (*{{$msgName}}) ProtoMessage()               {}

  {{end}}
{{end}}
{{end}}
`
)
