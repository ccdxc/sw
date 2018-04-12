// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//
// This file contains the logger API for the default/singleton logger

package log

import (
	"fmt"
	"sync"

	kitlevel "github.com/go-kit/kit/log/level"
)

// singleton is singleton Instance
var singleton *kitLogger
var once sync.Once

// getDefaultInstance returns the default logger instance
func getDefaultInstance() *kitLogger {
	once.Do(func() {
		createSingleton()
	})
	return singleton
}

// Create the default singleton logger
func createSingleton() {
	config := GetDefaultConfig("Default")
	singleton = newLogger(config)
}

// GetDefaultConfig returns default log config object
func GetDefaultConfig(module string) *Config {

	return &Config{
		Module:      module,
		Format:      LogFmt,
		Filter:      AllowInfoFilter,
		Debug:       false,
		CtxSelector: ContextAll,
		LogToStdout: true,
		LogToFile:   false,
		FileCfg:     FileConfig{},
	}
}

// The following logger APIs uses the underlying
// default singleton logger instance

// WithContext uses default logger with added context pairs
func WithContext(pairs ...string) Logger {
	return getDefaultInstance().WithContext(pairs...)
}

// SetConfig overrides the default logger with the
// new logger allocated for the config passed
func SetConfig(config *Config) Logger {
	once.Do(func() {
		createSingleton()
	})
	singleton = newLogger(config)

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

	return singleton
}

// SetFilter configures the log filter for the default logger
func SetFilter(filter FilterType) Logger {
	singleton.logger = kitlevel.NewFilter(singleton.logger, getFilterOption(filter))
	return singleton
}

// Fatal logs error messages with panic for non-recoverable cases
func Fatal(args ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
	panic(fmt.Sprint(args...))
}

// Fatalf logs error messages with panic for non-recoverable cases
func Fatalf(format string, args ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log("msg", fmt.Sprintf(format, args...))
	panic(fmt.Sprintf(format, args...))
}

// Fatalln logs error messages with panic for non-recoverable cases
func Fatalln(args ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
	panic(fmt.Sprint(args...))
}

// Error logs error messages
func Error(args ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Errorf logs error messages
func Errorf(format string, args ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log("msg", fmt.Sprintf(format, args...))
}

// ErrorLog logs error messages
func ErrorLog(keyvals ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log(keyvals...)
}

// Errorln logs error messages
func Errorln(args ...interface{}) {
	kitlevel.Error(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Warn logs warning messages
func Warn(args ...interface{}) {
	kitlevel.Warn(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Warnf logs warning messages
func Warnf(format string, args ...interface{}) {
	kitlevel.Warn(getDefaultInstance().logger).Log("msg", fmt.Sprintf(format, args...))
}

// WarnLog logs warning messages
func WarnLog(keyvals ...interface{}) {
	kitlevel.Warn(getDefaultInstance().logger).Log(keyvals...)
}

// WarnLn logs warning messages
func WarnLn(args ...interface{}) {
	kitlevel.Warn(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Info logs informational messages
func Info(args ...interface{}) {
	kitlevel.Info(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Infof logs informational messages
func Infof(format string, args ...interface{}) {
	kitlevel.Info(getDefaultInstance().logger).Log("msg", fmt.Sprintf(format, args...))
}

// InfoLog logs informational messages
func InfoLog(keyvals ...interface{}) {
	kitlevel.Info(getDefaultInstance().logger).Log(keyvals...)
}

// Infoln logs informational messages
func Infoln(args ...interface{}) {
	kitlevel.Info(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Print logs debug messages
func Print(args ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Printf logs debug messages
func Printf(format string, args ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log("msg", fmt.Sprintf(format, args...))
}

// Println logs debug messages
func Println(args ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Debug logs debug messages
func Debug(args ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Debugf logs debug messages
func Debugf(format string, args ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log("msg", fmt.Sprintf(format, args...))
}

// DebugLog logs debug messages
func DebugLog(keyvals ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log(keyvals...)
}

// Debugln logs debug messages
func Debugln(args ...interface{}) {
	kitlevel.Debug(getDefaultInstance().logger).Log("msg", fmt.Sprint(args...))
}

// Log messages
func Log(keyvals ...interface{}) error {
	return kitlevel.Info(getDefaultInstance().logger).Log(keyvals...)
}
