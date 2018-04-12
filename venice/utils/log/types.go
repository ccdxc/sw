// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//
// This file contains the Logger interface definition and associated
// type defintions (for format, filters, context-selectors etc)

package log

import (
	"context"
	"io"

	kitlog "github.com/go-kit/kit/log"
)

// Logger interface definition
type Logger interface {
	Fatal(args ...interface{})
	Fatalf(format string, args ...interface{})
	Fatalln(args ...interface{})

	Error(args ...interface{})
	Errorf(format string, args ...interface{})
	ErrorLog(keyvals ...interface{})
	Errorln(args ...interface{})

	Warn(args ...interface{})
	Warnf(format string, args ...interface{})
	WarnLog(keyvals ...interface{})
	WarnLn(args ...interface{})

	Info(args ...interface{})
	Infof(format string, args ...interface{})
	InfoLog(keyvals ...interface{})
	Infoln(args ...interface{})

	Debug(args ...interface{})
	Debugf(format string, args ...interface{})
	DebugLog(keyvals ...interface{})
	Debugln(args ...interface{})

	Print(args ...interface{})
	Printf(format string, args ...interface{})
	Println(args ...interface{})

	Log(keyvals ...interface{}) error
	Audit(ctx context.Context, keyvals ...interface{}) error

	WithContext(pairs ...string) Logger
	SetOutput(w io.Writer) Logger
	SetFilter(f FilterType) Logger

	V(l int) bool
}

// FormatType identifies logging format type
type FormatType int

// Log format types
const (
	LogFmt FormatType = iota
	JSONFmt
)

// FilterType defines the log levels allowed
type FilterType byte

const (

	// AllowAllFilter allows all levels
	AllowAllFilter FilterType = 1 << iota

	// AllowDebugFilter allows debug, info, warn, error levels
	AllowDebugFilter

	// AllowInfoFilter allows info, warn, error levels
	AllowInfoFilter

	// AllowWarnFilter allows warn, error levels
	AllowWarnFilter

	// AllowErrorFilter allows only error levels
	AllowErrorFilter

	// AllowNoneFilter none
	AllowNoneFilter
)

// ContextSelector is a bitmask type representing the various context
// attributes that can be selected for log annotation.
type ContextSelector uint16

const (

	// ContextNone disables adding any context data in logging
	ContextNone ContextSelector = 0

	// ContextTimestamp adds timestamp to log context
	ContextTimestamp ContextSelector = 1 << iota

	// ContextModule adds module name to log context
	ContextModule

	// ContextPid adds process-id to log context
	ContextPid

	// ContextCaller adds caller filename & line number to log context
	ContextCaller

	// ContextAll adds all of the following attributes
	ContextAll ContextSelector = ContextTimestamp | ContextModule | ContextPid | ContextCaller
)

// FileConfig contains config params for logging to file
type FileConfig struct {
	// Filename of the Log file
	Filename string

	// MaxSize of a Logfile in MBs
	MaxSize int

	// MaxBackups indicates #of backup files in rotation
	MaxBackups int

	// MaxAge indicates log retention period in days
	MaxAge int
}

// Config contains config params for the logger
type Config struct {
	// Module Name
	Module string

	// Format Type
	Format FormatType

	// Filter sets the allowed log levels
	Filter FilterType

	// Debug flag enables logging with stack-trace
	Debug bool

	// CtxSelector specifies the context attributes to include.
	CtxSelector ContextSelector

	// LogToStdout enable logging to stdout
	LogToStdout bool

	// LogtoFile enabled logging to file
	LogToFile bool

	// FileCfg contains config params for
	// logging to file with log rotation
	FileCfg FileConfig
}

type kitLogger struct {
	logger kitlog.Logger
	config Config
}

// Context is pairs of strings used for adding
// context data to log message
type Context []string

// stackDepth for displaying caller filename
const (
	stackDepth = 6
)

var caller = kitlog.Caller(stackDepth)

// CtxKey is context key type
type CtxKey int

// context keys
const (
	PensandoTenant CtxKey = 1
	PensandoUserID CtxKey = 2
	PensandoTxnID  CtxKey = 3
)
