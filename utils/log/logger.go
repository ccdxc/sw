package log

import (
	"context"
	"fmt"
	"io"
	"os"
	"os/signal"
	"runtime"
	"strconv"
	"sync"
	"syscall"

	kitlog "github.com/go-kit/kit/log"
	kitlevel "github.com/go-kit/kit/log/level"
	"github.com/go-stack/stack"
	"gopkg.in/natefinch/lumberjack.v2"
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

// GetNewLogger returns a new Logger object
// This should be called by application that don't
// want to use the default singleton logger.
func GetNewLogger(config *Config) Logger {
	return newLogger(config)
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

	return &kitLogger{logger: l, config: *config}
}

// WithContext adds context data specified as KV pairs
func (l *kitLogger) WithContext(pairs ...string) Logger {
	if (len(pairs) % 2) != 0 {
		panic("invalid argument")
	}
	r := kitLogger{logger: l.logger, config: l.config}
	for i := 0; i < len(pairs); i = i + 2 {
		r.logger = kitlog.With(r.logger, pairs[i], pairs[i+1])
	}
	return &r
}

// SetOutput configures the io.Writer
func (l *kitLogger) SetOutput(w io.Writer) Logger {
	wr := kitlog.NewSyncWriter(w)
	l.logger = kitlog.NewLogfmtLogger(wr)

	if l.config.Debug {
		l.logger = kitlog.With(l.logger, "ts", kitlog.DefaultTimestampUTC, "caller", stackTrace())
	}

	// Configure levelled logging
	l.logger = kitlevel.NewFilter(l.logger, getFilterOption(l.config.Filter))
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

// SetFilter configures the log filter based on filter type
func (l *kitLogger) SetFilter(filter FilterType) Logger {
	l.logger = kitlevel.NewFilter(l.logger, getFilterOption(filter))
	return l
}

func (l *kitLogger) Fatal(args ...interface{}) {
	kitlevel.Error(l.logger).Log("msg", fmt.Sprint(args...))
	panic(fmt.Sprint(args...))
}

func (l *kitLogger) Fatalf(format string, args ...interface{}) {
	kitlevel.Error(l.logger).Log("msg", fmt.Sprintf(format, args...))
	panic(fmt.Sprintf(format, args...))
}

func (l *kitLogger) Fatalln(args ...interface{}) {
	kitlevel.Error(l.logger).Log("msg", fmt.Sprint(args...))
	panic(fmt.Sprint(args...))
}

func (l *kitLogger) Error(args ...interface{}) {
	kitlevel.Error(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Errorf(format string, args ...interface{}) {
	kitlevel.Error(l.logger).Log("msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) ErrorLog(keyvals ...interface{}) {
	kitlevel.Error(l.logger).Log(keyvals...)
}

func (l *kitLogger) Errorln(args ...interface{}) {
	kitlevel.Error(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Warn(args ...interface{}) {
	kitlevel.Warn(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Warnf(format string, args ...interface{}) {
	kitlevel.Warn(l.logger).Log("msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) WarnLog(keyvals ...interface{}) {
	kitlevel.Warn(l.logger).Log(keyvals...)
}

func (l *kitLogger) WarnLn(args ...interface{}) {
	kitlevel.Warn(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Info(args ...interface{}) {
	kitlevel.Info(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Infof(format string, args ...interface{}) {
	kitlevel.Info(l.logger).Log("msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) InfoLog(keyvals ...interface{}) {
	kitlevel.Info(l.logger).Log(keyvals...)
}

func (l *kitLogger) Infoln(args ...interface{}) {
	kitlevel.Info(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Print(args ...interface{}) {
	kitlevel.Debug(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Printf(format string, args ...interface{}) {
	kitlevel.Debug(l.logger).Log("msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) Println(args ...interface{}) {
	kitlevel.Debug(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Debug(args ...interface{}) {
	kitlevel.Debug(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Debugf(format string, args ...interface{}) {
	kitlevel.Debug(l.logger).Log("msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) DebugLog(keyvals ...interface{}) {
	kitlevel.Debug(l.logger).Log(keyvals...)
}

func (l *kitLogger) Debugln(args ...interface{}) {
	kitlevel.Debug(l.logger).Log("msg", fmt.Sprint(args...))
}

func (l *kitLogger) Log(keyvals ...interface{}) error {
	return kitlevel.Info(l.logger).Log(keyvals...)
}

func (l *kitLogger) Audit(ctx context.Context, keyvals ...interface{}) error {
	if ctx == nil {
		return nil
	}
	v := ctx.Value(PensandoTenant)
	if v != nil {
		s := v.(string)
		keyvals = append(keyvals, "tenant", s)
	}

	if v, ok := ctx.Value(PensandoUserID).(string); ok {
		keyvals = append(keyvals, "user", v)
	}
	if v, ok := ctx.Value(PensandoTxnID).(string); ok {
		keyvals = append(keyvals, "txnId", v)
	}
	// XXX-TBD add Span id from open tracing to the audit log.
	keyvals = append(keyvals, "level", "audit")
	return l.logger.Log(keyvals...)
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
