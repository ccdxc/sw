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
	"path/filepath"
	"runtime"
	"strconv"
	"sync"
	"syscall"

	kitlog "github.com/go-kit/kit/log"
	kitlevel "github.com/go-kit/kit/log/level"
	"github.com/go-stack/stack"
	"gopkg.in/natefinch/lumberjack.v2"

	"github.com/pensando/sw/venice/utils/log/jsonlogger"
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
	l := newKitLogLogger(config)

	if config.ThrottleCfg != nil {
		l = newThrottledLogger(config.ThrottleCfg, l)
	}

	// Some iota test code assumes that logfile is created as soon as the process starts.
	// Hence create it
	if config.LogToFile {
		if err := os.MkdirAll(filepath.Dir(config.FileCfg.Filename), os.ModePerm); err != nil {
			fmt.Printf("Failed to create directory %s for logfile %s err: %v", filepath.Dir(config.FileCfg.Filename), config.FileCfg.Filename, err)
		} else {
			logFile, err := os.OpenFile(config.FileCfg.Filename, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0644)
			if err != nil {
				fmt.Printf("Failed to open logfile: %s err: %v", config.FileCfg.Filename, err)
			} else {
				logFile.Close()
			}
		}
	}
	return &kitLogger{logger: l, config: *config}
}

// newKitLogLogger is an internal utility function that creates kitlog.Logger given the config
func newKitLogLogger(config *Config) kitlog.Logger {
	// Init stdout io writer if enabled
	var stdoutWr io.Writer
	if config.LogToStdout {
		stdoutWr = os.Stdout
	}

	// Init File io writer if enabled
	if config.LogToFile {
		if config.fileWriter != nil {
			config.fileWriter.Close()
			config.fileWriter = nil
		}

		config.fileWriter = &lumberjack.Logger{
			Filename:   config.FileCfg.Filename,
			MaxSize:    config.FileCfg.MaxSize,
			MaxBackups: config.FileCfg.MaxBackups,
			MaxAge:     config.FileCfg.MaxAge,
		}
	}

	// Choose io writers based on config
	var wr io.Writer
	if config.LogToStdout && config.LogToFile {
		mw := io.MultiWriter(stdoutWr, config.fileWriter)
		wr = kitlog.NewSyncWriter(mw)
	} else if config.LogToStdout {
		wr = kitlog.NewSyncWriter(stdoutWr)
	} else if config.LogToFile {
		wr = kitlog.NewSyncWriter(config.fileWriter)
	} else {
		wr = kitlog.NewSyncWriter(ioutil.Discard)
	}

	// Instantiate logger based on format
	var l kitlog.Logger
	switch config.Format {
	case LogFmt:
		l = kitlog.NewLogfmtLogger(wr)
	case JSONFmt:
		l = jsonlogger.NewJSONLogger(wr)
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

	return l
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

// set SetTraceDebug once. if called again, its a no-op
var traceOnce sync.Once

// SetTraceDebug enables trace debug
// Enable tracedumping with SIGQUIT or ^\
// Will dump stacktrace for all go routines and continue execution
func SetTraceDebug() {
	traceOnce.Do(func() {
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
	})
}
