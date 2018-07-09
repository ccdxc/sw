// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package monitoringApiServer is a auto generated package.
Input file: telemetry.proto
*/
package monitoring

import (
	"reflect"

	"github.com/pensando/sw/venice/utils/runtime"
)

var typesMapTelemetry = map[string]*runtime.Struct{

	"monitoring.FlowExportPolicy": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FlowExportPolicy{}) },
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"O": runtime.Field{Name: "O", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": runtime.Field{Name: "Spec", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "monitoring.FlowExportSpec"},

			"Status": runtime.Field{Name: "Status", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "monitoring.FlowExportStatus"},
		},

		CLITags: map[string]runtime.CLIInfo{
			"format":   runtime.CLIInfo{Path: "Spec.Targets[].Format", Skip: false, Insert: "", Help: ""},
			"interval": runtime.CLIInfo{Path: "Spec.Targets[].Interval", Skip: false, Insert: "", Help: ""},
		},
	},
	"monitoring.FlowExportSpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FlowExportSpec{}) },
		Fields: map[string]runtime.Field{
			"Targets": runtime.Field{Name: "Targets", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "targets", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "monitoring.FlowExportTarget"},
		},
	},
	"monitoring.FlowExportStatus": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FlowExportStatus{}) },
		Fields:    map[string]runtime.Field{},
	},
	"monitoring.FlowExportTarget": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FlowExportTarget{}) },
		Fields: map[string]runtime.Field{
			"Interval": runtime.Field{Name: "Interval", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "interval", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Format": runtime.Field{Name: "Format", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "format", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Exports": runtime.Field{Name: "Exports", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "exports", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "api.ExportConfig"},
		},
	},
	"monitoring.FwlogExport": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FwlogExport{}) },
		Fields: map[string]runtime.Field{
			"Targets": runtime.Field{Name: "Targets", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "targets", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "api.ExportConfig"},

			"Format": runtime.Field{Name: "Format", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "format", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Filter": runtime.Field{Name: "Filter", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "export-filter", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"SyslogConfig": runtime.Field{Name: "SyslogConfig", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "syslog-config", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.SyslogExportConfig"},
		},
	},
	"monitoring.FwlogPolicy": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FwlogPolicy{}) },
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"O": runtime.Field{Name: "O", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": runtime.Field{Name: "Spec", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "monitoring.FwlogSpec"},

			"Status": runtime.Field{Name: "Status", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "monitoring.FwlogStatus"},
		},

		CLITags: map[string]runtime.CLIInfo{
			"export-filter":  runtime.CLIInfo{Path: "Spec.Exports[].Filter", Skip: false, Insert: "", Help: ""},
			"filter":         runtime.CLIInfo{Path: "Spec.Filter", Skip: false, Insert: "", Help: ""},
			"format":         runtime.CLIInfo{Path: "Spec.Exports[].Format", Skip: false, Insert: "", Help: ""},
			"retention-time": runtime.CLIInfo{Path: "Spec.RetentionTime", Skip: false, Insert: "", Help: ""},
		},
	},
	"monitoring.FwlogSpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FwlogSpec{}) },
		Fields: map[string]runtime.Field{
			"RetentionTime": runtime.Field{Name: "RetentionTime", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "retention-time", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Filter": runtime.Field{Name: "Filter", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "filter", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"Exports": runtime.Field{Name: "Exports", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "exports", Pointer: false, Slice: true, Map: false, KeyType: "", Type: "monitoring.FwlogExport"},
		},
	},
	"monitoring.FwlogStatus": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(FwlogStatus{}) },
		Fields:    map[string]runtime.Field{},
	},
	"monitoring.StatsPolicy": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(StatsPolicy{}) },
		Fields: map[string]runtime.Field{
			"T": runtime.Field{Name: "T", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.TypeMeta"},

			"O": runtime.Field{Name: "O", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "meta", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "api.ObjectMeta"},

			"Spec": runtime.Field{Name: "Spec", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "spec", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "monitoring.StatsSpec"},

			"Status": runtime.Field{Name: "Status", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "status", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "monitoring.StatsStatus"},
		},

		CLITags: map[string]runtime.CLIInfo{
			"compaction-interval ":      runtime.CLIInfo{Path: "Spec.CompactionInterval", Skip: false, Insert: "", Help: ""},
			"downsample-retention-time": runtime.CLIInfo{Path: "Spec.DownSampleRetentionTime", Skip: false, Insert: "", Help: ""},
			"retention-time":            runtime.CLIInfo{Path: "Spec.RetentionTime", Skip: false, Insert: "", Help: ""},
		},
	},
	"monitoring.StatsSpec": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(StatsSpec{}) },
		Fields: map[string]runtime.Field{
			"CompactionInterval": runtime.Field{Name: "CompactionInterval", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "compaction-interval ", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"RetentionTime": runtime.Field{Name: "RetentionTime", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "retention-time", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},

			"DownSampleRetentionTime": runtime.Field{Name: "DownSampleRetentionTime", CLITag: runtime.CLIInfo{Path: "", Skip: false, Insert: "", Help: ""}, JSONTag: "downsample-retention-time", Pointer: true, Slice: false, Map: false, KeyType: "", Type: "TYPE_STRING"},
		},
	},
	"monitoring.StatsStatus": &runtime.Struct{
		GetTypeFn: func() reflect.Type { return reflect.TypeOf(StatsStatus{}) },
		Fields:    map[string]runtime.Field{},
	},
}

func init() {
	schema := runtime.GetDefaultScheme()
	schema.AddSchema(typesMapTelemetry)
}
