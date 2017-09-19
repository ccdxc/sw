package log

import (
	"bytes"
	"context"
	"io/ioutil"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestLoggerToFile(t *testing.T) {
	t.Parallel()
	file := "/tmp/test.log"
	config := &Config{
		Module:      "LogTester",
		Format:      LogFmt,
		Debug:       false,
		CtxSelector: ContextCaller,
		LogToStdout: false,
		LogToFile:   true,
		FileCfg: FileConfig{
			Filename:   file,
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}

	l := GetNewLogger(config)
	l.Log("msg", "testmsg")

	data, err := ioutil.ReadFile(file)
	if err != nil {
		t.Errorf("Error opening file: %s err: %v", file, err)
	}

	str := string(data)
	_, path, _, ok := runtime.Caller(0)
	if !ok {
		t.Errorf("Failed to get runtime caller info")
	}

	fileName := filepath.Base(path)

	// Test message and caller filename is part for log message
	if !strings.Contains(str, "msg=testmsg") ||
		!strings.Contains(str, "caller="+fileName) {
		t.Errorf("Expecting [%s] got:[%s]", "msg=testmsg", str)
	}

	err = os.Remove(file)
	if err != nil {
		t.Errorf("Error removing file: %s err: %v", file, err)
	}
}

func TestLevels(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	config.Filter = AllowAllFilter
	l := GetNewLogger(config).SetOutput(buf)

	// Print
	l.Printf("%s", "testmsg")
	if !strings.Contains(buf.String(), "msg=testmsg") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Print("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Println("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	// Debug
	buf.Reset()
	l.DebugLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Debug("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Debugf("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Debugln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	// Info
	buf.Reset()
	l.Info("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	buf.Reset()
	l.Infof("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	buf.Reset()
	l.Infoln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	buf.Reset()
	l.InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	// Error
	buf.Reset()
	l.Error("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	buf.Reset()
	l.Errorf("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	buf.Reset()
	l.Errorln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	buf.Reset()
	l.ErrorLog("msg", "testmsg1", "error", "testerror")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") ||
		!strings.Contains(buf.String(), "error=testerror") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	// Warn
	buf.Reset()
	l.Warn("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	buf.Reset()
	l.Warnf("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	buf.Reset()
	l.WarnLn("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	buf.Reset()
	l.WarnLog("msg", "testmsg1", "error", "testerror")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") ||
		!strings.Contains(buf.String(), "error=testerror") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}
}

func TestLevelsWithDefaultLogger(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	config.Filter = AllowAllFilter
	SetConfig(config)
	getDefaultInstance().SetOutput(buf)

	// Log
	Log("msg", "testmsg")
	if !strings.Contains(buf.String(), "msg=testmsg") ||
		!strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	// Print
	Printf("%s", "testmsg")
	if !strings.Contains(buf.String(), "msg=testmsg") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	Print("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	Println("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	// Debug
	buf.Reset()
	Debug("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	Debugf("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	Debugln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	buf.Reset()
	DebugLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=debug]", buf.String())
	}

	// Info
	buf.Reset()
	Info("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	buf.Reset()
	Infof("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	buf.Reset()
	Infoln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	buf.Reset()
	InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	// Error
	buf.Reset()
	Error("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	buf.Reset()
	Errorf("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=error]", buf.String())
	}

	buf.Reset()
	Errorln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	buf.Reset()
	ErrorLog("msg", "testmsg1", "error", "testerror")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") ||
		!strings.Contains(buf.String(), "error=testerror") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=error]", buf.String())
	}

	// Warn
	buf.Reset()
	Warn("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	buf.Reset()
	Warnf("%s", "testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	buf.Reset()
	WarnLn("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	buf.Reset()
	WarnLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=warn") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=warn]", buf.String())
	}

	// Context
	buf.Reset()
	WithContext("Tag1", "ONE", "tag2", "TWO").InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "Tag1=ONE") || !strings.Contains(buf.String(), "tag2=TWO") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}

	// Fatal
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
		if !strings.Contains(buf.String(), "Testing Panic") {
			t.Errorf("Did not see expected log expected: [%s] got:[%s]", "Testing Panic", buf.String())
		}
	}()

	buf.Reset()
	Fatalf("%s", "Testing Panic")
}

func TestDebugMode(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	config.Debug = true
	l := GetNewLogger(config).SetOutput(buf)
	l.InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "error=TestError") || !strings.Contains(buf.String(), "caller=\"[ [") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=info]", buf.String())
	}
}

func TestWithContext(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	config.Debug = true
	l := GetNewLogger(config).SetOutput(buf).WithContext("Tag1", "ONE", "tag2", "TWO")
	l.InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "Tag1=ONE") || !strings.Contains(buf.String(), "tag2=TWO") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg] and [level=info]", buf.String())
	}
}

func TestFatalf(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	l := GetNewLogger(config).SetOutput(buf)
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
		if !strings.Contains(buf.String(), "Testing Panic") {
			t.Errorf("Did not see expected log expected: [%s] got:[%s]", "Testing Panic", buf.String())
		}
	}()
	l.Fatalf("%s", "Testing Panic")
}

func TestFatal(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	l := GetNewLogger(config).SetOutput(buf)
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
		if !strings.Contains(buf.String(), "Testing Panic") {
			t.Errorf("Did not see expected log expected: [%s] got:[%s]", "Testing Panic", buf.String())
		}
	}()
	l.Fatal("Testing Panic")
}

func TestFatalln(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	l := GetNewLogger(config).SetOutput(buf)
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
		if !strings.Contains(buf.String(), "Testing Panic") {
			t.Errorf("Did not see expected log expected: [%s] got:[%s]", "Testing Panic", buf.String())
		}
	}()
	l.Fatalln("Testing Panic")
}

func TestLogFilter(t *testing.T) {
	testCases := []struct {
		name     string
		filter   FilterType
		expected string
	}{
		{
			"TestAllowAll",
			AllowAllFilter,
			strings.Join([]string{
				`level=debug msg="debug log"`,
				`level=info msg="info log"`,
				`level=warn msg="warn log"`,
				`level=error msg="error log"`,
			}, "\n"),
		},
		{
			"TestAllowDebug",
			AllowDebugFilter,
			strings.Join([]string{
				`level=debug msg="debug log"`,
				`level=info msg="info log"`,
				`level=warn msg="warn log"`,
				`level=error msg="error log"`,
			}, "\n"),
		},
		{
			"TestAllowInfo",
			AllowInfoFilter,
			strings.Join([]string{
				`level=info msg="info log"`,
				`level=warn msg="warn log"`,
				`level=error msg="error log"`,
			}, "\n"),
		},
		{
			"TestAllowWarn",
			AllowWarnFilter,
			strings.Join([]string{
				`level=warn msg="warn log"`,
				`level=error msg="error log"`,
			}, "\n"),
		},
		{
			"TestAllowError",
			AllowErrorFilter,
			strings.Join([]string{
				`level=error msg="error log"`,
			}, "\n"),
		},
		{
			"TestAllowNone",
			AllowNoneFilter,
			``,
		},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {

			buf := &bytes.Buffer{}
			config := GetDefaultConfig("TestLogger")
			config.CtxSelector = ContextNone
			config.Filter = tc.filter
			l := GetNewLogger(config).SetOutput(buf).SetFilter(tc.filter)

			l.Debug("debug log")
			l.Info("info log")
			l.Warn("warn log")
			l.Error("error log")

			if expected, obtained := tc.expected, strings.TrimSpace(buf.String()); expected != obtained {
				t.Errorf("\nexpected:\n%s\nobtained:\n%s", expected, obtained)
			}
		})
	}
}

func TestAuditLog(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	config := GetDefaultConfig("TestLogger")
	l := GetNewLogger(config).SetOutput(buf)
	var ctx = context.Background()
	ctx = context.WithValue(ctx, PensandoTenant, "TestTenant")
	ctx = context.WithValue(ctx, PensandoUserID, "TestUser")
	ctx = context.WithValue(ctx, PensandoTxnID, "TestTxnId")

	l.Audit(ctx, "msg", "TestMsg")
	if !strings.Contains(buf.String(), "user=TestUser") || !strings.Contains(buf.String(), "tenant=TestTenant") ||
		!strings.Contains(buf.String(), "txnId=TestTxnId") || !strings.Contains(buf.String(), "msg=TestMsg") ||
		!strings.Contains(buf.String(), "level=audit") {
		t.Errorf("Expecting [%s] got:[%s]", "[user=TestUser][tenant=TestTenant][txnId=TestTxnIdr][msg=TestMsg][level=audit]",
			buf.String())
	}

	// This is to test when all auditlog fields are not available in the context
	buf.Reset()
	ctx = context.Background()
	ctx1 := context.WithValue(ctx, PensandoTenant, "TestTenant")
	l.Audit(ctx1, "msg", "TestMsg")
	if strings.Contains(buf.String(), "user=") || !strings.Contains(buf.String(), "tenant=TestTenant") ||
		strings.Contains(buf.String(), "txnId=") || !strings.Contains(buf.String(), "msg=TestMsg") ||
		!strings.Contains(buf.String(), "level=audit") {
		t.Errorf("Expecting [%s] got:[%s]", "[tenant=TestTenant][msg=TestMsg][level=audit]",
			buf.String())
	}
}
