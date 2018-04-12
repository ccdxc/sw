// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//
// This file contains the logger API for dynamically instantiated loggers

package log

import (
	"context"
	"fmt"
	"io"

	kitlog "github.com/go-kit/kit/log"
	kitlevel "github.com/go-kit/kit/log/level"
)

// GetNewLogger returns a new Logger object
// This should be called by application that don't
// want to use the default singleton logger.
func GetNewLogger(config *Config) Logger {
	return newLogger(config)
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

// SetFilter configures the log filter based on filter type
func (l *kitLogger) SetFilter(filter FilterType) Logger {
	l.logger = kitlevel.NewFilter(l.logger, getFilterOption(filter))
	return l
}

// The following logger APIs do not use the default singleton logger
// and works on the new instantiated logger.

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

func (l *kitLogger) V(m int) bool {
	return true
}
