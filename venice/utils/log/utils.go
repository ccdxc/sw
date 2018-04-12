// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//
// This file contains the utility functions for logger library

package log

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/signal"
	"runtime"
	"strconv"
	"syscall"

	kitlog "github.com/go-kit/kit/log"
	kitlevel "github.com/go-kit/kit/log/level"
	"github.com/go-stack/stack"
	"gopkg.in/natefinch/lumberjack.v2"
)

func stackTrace() kitlog.Valuer {
	return func() interface{} {
		v := stack.Trace().TrimRuntime().TrimBelow(stack.Caller(stackDepth))

		var r = "["
		var fmtstr = "[%s:%d %n()]"
		for _, c := range v {
			r = r + " " + fmt.Sprintf(fmtstr, c, c, c)
		}
		return r + "]"
	}
}

// newLogger is a internal utility function that
// allocates a new logger object
func newLogger(config *Config) *kitLogger {

	// Init stdout io writer if enabled
	var stdoutWr io.Writer
	if config.LogToStdout {
		stdoutWr = os.Stdout
	}

	// Init File io writer if enabled
	var fileWr io.Writer
	if config.LogToFile {
		fileWr = &lumberjack.Logger{
			Filename:   config.FileCfg.Filename,
			MaxSize:    config.FileCfg.MaxSize,
			MaxBackups: config.FileCfg.MaxBackups,
			MaxAge:     config.FileCfg.MaxAge,
		}
	}

	// Choose io writers based on config
	var wr io.Writer
	if config.LogToStdout && config.LogToFile {
		mw := io.MultiWriter(stdoutWr, fileWr)
		wr = kitlog.NewSyncWriter(mw)
	} else if config.LogToStdout {
		wr = kitlog.NewSyncWriter(stdoutWr)
	} else if config.LogToFile {
		wr = kitlog.NewSyncWriter(fileWr)
	} else {
		wr = kitlog.NewSyncWriter(ioutil.Discard)
	}

	// Instantiate logger based on format
	var l kitlog.Logger
	switch config.Format {
	case LogFmt:
		l = kitlog.NewLogfmtLogger(wr)
	case JSONFmt:
		l = kitlog.NewJSONLogger(wr)
	default:
		// By default, choose log-fmt
		l = kitlog.NewLogfmtLogger(wr)
	}

	// Add context data based on selectors : Timestamp, Module, Pid, Caller
	if config.CtxSelector&ContextTimestamp != 0 {
		l = kitlog.With(l, "ts", kitlog.DefaultTimestampUTC)
	}

	if config.CtxSelector&ContextModule != 0 {
		l = kitlog.With(l, "module", config.Module)
	}

	if config.CtxSelector&ContextPid != 0 {
		l = kitlog.With(l, "pid", strconv.Itoa(os.Getpid()))
	}

	// Add debug trace if enabled
	if config.Debug {
		l = kitlog.With(l, "caller", stackTrace())
	} else {
		if config.CtxSelector&ContextCaller != 0 {
			l = kitlog.With(l, "caller", caller)
		}
	}

	// Configure log filter
	l = kitlevel.NewFilter(l, getFilterOption(config.Filter))

	if config.LogToFile == true {
		err := config.LogtoFileHandler()
		if err != nil {
			panic(fmt.Sprintf("Failed to open logfile: %s err: %v", config.FileCfg.Filename, err))
		}
	} else if config.LogToStdout == true {
		err := config.LogtoStdoutHandler()
		if err != nil {
			panic(fmt.Sprintf("Failed to write to stdout: %s err", err))
		}
	}

	return &kitLogger{logger: l, config: *config}
}

// getFilterOption returns the filter function based on filter type.
// The filter function returned is used internally by kit logger to
// implement filtering of log levels.
// implement filtering of log levels.
func getFilterOption(filter FilterType) kitlevel.Option {

	switch filter {
	case AllowAllFilter, AllowDebugFilter:
		return kitlevel.AllowDebug()
	case AllowInfoFilter:
		return kitlevel.AllowInfo()
	case AllowWarnFilter:
		return kitlevel.AllowWarn()
	case AllowErrorFilter:
		return kitlevel.AllowError()
	case AllowNoneFilter:
		return kitlevel.AllowNone()
	}

	return kitlevel.AllowInfo()
}

// SetTraceDebug enables trace debug
// Enable tracedumping with SIGQUIT or ^\
// Will dump stacktrace for all go routines and continue execution
func SetTraceDebug() {
	go func() {
		sigs := make(chan os.Signal, 1)
		signal.Notify(sigs, syscall.SIGQUIT)
		buf := make([]byte, 1<<20)
		for {
			<-sigs
			stacklen := runtime.Stack(buf, true)
			fmt.Printf("=== received SIGQUIT ===\n*** goroutine dump *** \n%s\n*** end\n", buf[:stacklen])
		}
	}()
}
