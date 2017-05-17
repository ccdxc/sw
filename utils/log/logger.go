package log

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"runtime"
	"syscall"

	"io"

	kitlog "github.com/go-kit/kit/log"
	"github.com/go-stack/stack"
)

type Logger interface {
	Fatal(args ...interface{})
	Fatalf(format string, args ...interface{})
	Fatalln(args ...interface{})
	Error(args ...interface{})
	Errorf(format string, args ...interface{})
	ErrorLog(keyvals ...interface{})
	Errorln(args ...interface{})
	Info(args ...interface{})
	Infof(format string, args ...interface{})
	InfoLog(keyvals ...interface{})
	Infoln(args ...interface{})
	Print(args ...interface{})
	Printf(format string, args ...interface{})
	Println(args ...interface{})
	DebugLog(keyvals ...interface{})
	Log(keyvals ...interface{}) error
	Audit(ctx context.Context, keyvals ...interface{}) error
	WithContext(pairs ...string) Logger
	SetOutput(w io.Writer) Logger
}

type kitLogger struct {
	logger kitlog.Logger
	debug  bool
}

var caller = kitlog.Caller(4)

func stackTrace() kitlog.Valuer {
	return func() interface{} {
		v := stack.Trace().TrimRuntime().TrimBelow(stack.Caller(4))
		var r string = "["
		var fmtstr = "[%s:%d %n()]"
		for _, c := range v {
			r = r + " " + fmt.Sprintf(fmtstr, c, c, c)
		}
		return r + "]"
	}
}

func GetNewLogger(debug bool) Logger {
	l := kitlog.NewLogfmtLogger(os.Stdout)
	if debug {
		l = kitlog.With(l, "ts", kitlog.DefaultTimestampUTC, "caller", stackTrace())
	} else {
		l = kitlog.With(l, "ts", kitlog.DefaultTimestampUTC, "caller", caller)
	}
	return &kitLogger{logger: l, debug: debug}
}

func (l *kitLogger) WithContext(pairs ...string) Logger {
	if (len(pairs) % 2) != 0 {
		panic("invalid argument")
	}
	r := kitLogger{logger: l.logger, debug: l.debug}
	for i := 0; i < len(pairs); i = i + 2 {
		r.logger = kitlog.With(r.logger, pairs[i], pairs[i+1])
	}
	return &r
}

func (l *kitLogger) SetOutput(w io.Writer) Logger {
	wr := kitlog.NewSyncWriter(w)
	l.logger = kitlog.NewLogfmtLogger(wr)
	if l.debug {
		l.logger = kitlog.With(l.logger, "ts", kitlog.DefaultTimestampUTC, "caller", stackTrace())
	} else {
		l.logger = kitlog.With(l.logger, "ts", kitlog.DefaultTimestampUTC, "caller", caller)
	}
	return l
}

func (l *kitLogger) Fatal(args ...interface{}) {
	l.logger.Log("level", "fatal", "msg", fmt.Sprint(args...))
	panic(fmt.Sprint(args...))
}

func (l *kitLogger) Fatalf(format string, args ...interface{}) {
	l.logger.Log("level", "fatal", "msg", fmt.Sprintf(format, args...))
	panic(fmt.Sprintf(format, args...))
}

func (l *kitLogger) Fatalln(args ...interface{}) {
	l.logger.Log("level", "fatal", "msg", fmt.Sprint(args...))
	panic(fmt.Sprint(args...))
}

func (l *kitLogger) Error(args ...interface{}) {
	l.logger.Log("level", "error", "msg", fmt.Sprint(args...))
}

func (l *kitLogger) Errorf(format string, args ...interface{}) {
	l.logger.Log("level", "error", "msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) ErrorLog(keyvals ...interface{}) {
	keyvals = append(keyvals, "level", "error")
	l.logger.Log(keyvals...)
}

func (l *kitLogger) Errorln(args ...interface{}) {
	l.logger.Log("level", "error", "msg", fmt.Sprint(args...))
}

func (l *kitLogger) Info(args ...interface{}) {
	l.logger.Log("level", "info", "msg", fmt.Sprint(args...))
}

func (l *kitLogger) Infof(format string, args ...interface{}) {
	l.logger.Log("level", "info", "msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) InfoLog(keyvals ...interface{}) {
	keyvals = append(keyvals, "level", "info")
	l.logger.Log(keyvals...)
}

func (l *kitLogger) Infoln(args ...interface{}) {
	l.logger.Log("level", "info", "msg", fmt.Sprint(args...))
}

func (l *kitLogger) Print(args ...interface{}) {
	l.logger.Log("level", "debug", "msg", fmt.Sprint(args...))
}

func (l *kitLogger) Printf(format string, args ...interface{}) {
	l.logger.Log("level", "debug", "msg", fmt.Sprintf(format, args...))
}

func (l *kitLogger) DebugLog(keyvals ...interface{}) {
	keyvals = append(keyvals, "level", "debug")
	l.logger.Log(keyvals...)
}

func (l *kitLogger) Println(args ...interface{}) {
	l.logger.Log("level", "debug", "msg", fmt.Sprint(args...))
}

func (l *kitLogger) Log(keyvals ...interface{}) error {
	return l.logger.Log(keyvals...)
}

func (l *kitLogger) Audit(ctx context.Context, keyvals ...interface{}) error {
	if ctx == nil {
		return nil
	}
	v := ctx.Value("pensando-tenant")
	if v != nil {
		s := v.(string)
		keyvals = append(keyvals, "tenant", s)
	}

	if v, ok := ctx.Value("pensando-user-id").(string); ok {
		keyvals = append(keyvals, "user", v)
	}
	if v, ok := ctx.Value("pensando-txnid").(string); ok {
		keyvals = append(keyvals, "txnId", v)
	}
	// XXX-TBD add Span id from open tracing to the audit log.
	keyvals = append(keyvals, "level", "audit")
	return l.logger.Log(keyvals...)
}

// SetTraceDebug
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
