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
	{{.GetName}} gometrics.Counter
    {{else if (eq .GetTypeName ".delphi.Gauge") }}
	{{.GetName}}    gometrics.Gauge
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
	sz += mtr.{{.GetName}}.Size()
	{{else if (eq .GetTypeName ".delphi.Gauge") }}
	sz += mtr.{{.GetName}}.Size()
	{{end}} {{end}}
	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *{{.GetName}}) Unmarshal() error {
	var offset int
	{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
	{{if (eq .GetName "Key") }}
	{{ if .TypeIsMessage }}
	json.Unmarshal([]byte(mtr.metrics.GetKey()), &mtr.key)
	{{else}}
	val, _ := proto.DecodeVarint([]byte(mtr.metrics.GetKey()))
	mtr.key = {{.GetGolangTypeName}}(val)
	{{end}}
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
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key
{{if (.HasExtOption "delphi.singleton")}}
func (it *{{.GetName}}Iterator) Find() (*{{.GetName}}, error) {
	var key int
	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(0))))
{{else}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
func (it *{{$msgName}}Iterator) Find(key {{.GetGolangTypeName}}) (*{{$msgName}}, error) {
	{{ if .TypeIsMessage }}
	buf, _ := json.Marshal(key)
	mtr, err := it.iter.Find(string(buf))
	{{else}}
	mtr, err := it.iter.Find(string(proto.EncodeVarint(uint64(key))))
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
	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(0))), tmtr.Size())
{{else}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
func (it *{{$msgName}}Iterator) Create(key {{.GetGolangTypeName}}) (*{{$msgName}}, error) {
	tmtr := &{{$msgName}}{}
	{{ if .TypeIsMessage }}
	buf, _ := json.Marshal(key)
	mtr := it.iter.Create(string(buf), tmtr.Size())
	{{else}}
	mtr := it.iter.Create(string(proto.EncodeVarint(uint64(key))), tmtr.Size())
	{{end}}
{{end}}{{end}}{{end}}
	tmtr = &{{.GetName}}{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory
{{if (.HasExtOption "delphi.singleton")}}
func (it *{{.GetName}}Iterator) Delete() error {
	return it.iter.Delete(string(proto.EncodeVarint(uint64(0))))
{{else}}
{{$msgName := .GetName}} {{$fields := .Fields}}{{range $fields}}
{{if (eq .GetName "Key") }}
func (it *{{$msgName}}Iterator) Delete(key {{.GetGolangTypeName}}) error {
	{{ if .TypeIsMessage }}
	buf, _ := json.Marshal(key)
	return it.iter.Delete(string(buf))
	{{else}}
	return it.iter.Delete(string(proto.EncodeVarint(uint64(key))))
	{{end}}
{{end}}{{end}}{{end}}
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
	{{.GetName}}	{{.GetGolangTypeName}}
    {{end}}
}
  {{end}}
{{end}}
{{end}}
`
)
