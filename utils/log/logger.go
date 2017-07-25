package log

import (
	"context"
	"fmt"
	kitlog "github.com/go-kit/kit/log"
	kitlevel "github.com/go-kit/kit/log/level"
	"github.com/go-stack/stack"
	"gopkg.in/natefinch/lumberjack.v2"
	"io"
	"os"
	"os/signal"
	"runtime"
	"strconv"
	"syscall"
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

	// Enable context data annotation (timestamp, module, pid)
	Context bool

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

var caller = kitlog.Caller(4)

// CtxKey is context key type
type CtxKey int

// context keys
const (
	PensandoTenant CtxKey = 1
	PensandoUserID CtxKey = 2
	PensandoTxnID  CtxKey = 3
)

func stackTrace() kitlog.Valuer {
	return func() interface{} {
		v := stack.Trace().TrimRuntime().TrimBelow(stack.Caller(4))
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
		Filter:      AllowAllFilter,
		Debug:       false,
		Context:     true,
		LogToStdout: true,
		LogToFile:   false,
		FileCfg:     FileConfig{},
	}
}

// GetNewLogger returns a new logger instance
func GetNewLogger(config *Config) Logger {

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

	// Add context data: Timestamp, Module, Pid
	if config.Context == true {
		l = kitlog.With(l,
			"ts", kitlog.DefaultTimestampUTC,
			"module", config.Module,
			"pid", strconv.Itoa(os.Getpid()),
		)
	}

	// Add debug trace if enabled
	if config.Debug {
		l = kitlog.With(l, "caller", stackTrace())
	} else {
		if config.Context == true {
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
	return l
}

// getFilterOption returns the filter function based on filter type.
// The filter function returned is used internally by kit logger to
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
	return l.logger.Log(keyvals...)
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
