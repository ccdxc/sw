package log

import (
	"bytes"
	"context"
	"strings"
	"testing"
)

func TestLoggerToFile(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	l := GetNewLogger(false).SetOutput(buf)
	l.Log("msg", "testmsg")
	if !strings.Contains(buf.String(), "msg=testmsg") {
		t.Errorf("Expecting [%s] got:[%s]", "msg=testmsg", buf.String())
	}
}

func TestLevels(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	// Debug
	l := GetNewLogger(false).SetOutput(buf)
	l.Printf("testmsg")
	if !strings.Contains(buf.String(), "msg=testmsg") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Print("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.DebugLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=debug]", buf.String())
	}

	buf.Reset()
	l.Println("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=debug") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=debug]", buf.String())
	}

	// info
	buf.Reset()
	l.Infof("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=info]", buf.String())
	}

	buf.Reset()
	l.Infoln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=info]", buf.String())
	}

	buf.Reset()
	l.InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "error=TestError") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=info]", buf.String())
	}

	// Error
	buf.Reset()
	l.Errorf("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=error]", buf.String())
	}

	buf.Reset()
	l.Errorln("testmsg1")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=error]", buf.String())
	}

	buf.Reset()
	l.ErrorLog("msg", "testmsg1", "error", "testerror")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=error") ||
		!strings.Contains(buf.String(), "error=testerror") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=error]", buf.String())
	}
}

func TestDebugMode(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	l := GetNewLogger(true).SetOutput(buf)
	l.InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "error=TestError") || !strings.Contains(buf.String(), "caller=\"[ [") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=info]", buf.String())
	}
}

func TestWithContext(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}

	l := GetNewLogger(true).SetOutput(buf).WithContext("Tag1", "ONE", "tag2", "TWO")
	l.InfoLog("msg", "testmsg1", "error", "TestError")
	if !strings.Contains(buf.String(), "msg=testmsg1") || !strings.Contains(buf.String(), "level=info") ||
		!strings.Contains(buf.String(), "Tag1=ONE") || !strings.Contains(buf.String(), "tag2=TWO") {
		t.Errorf("Expecting [%s] got:[%s]", "[msg=testmsg[] and [level=info]", buf.String())
	}
}

func TestFatalf(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}
	l := GetNewLogger(false).SetOutput(buf)
	defer func() {
		if r := recover(); r == nil {
			t.Errorf("Expecting panic but did not")
		}
		if !strings.Contains(buf.String(), "Testing Panic") {
			t.Errorf("Did not see expected log expected: [%s] got:[%s]", "Testing Panic", buf.String())
		}
	}()
	l.Fatalf("Testing Panic")
}

func TestFatal(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}
	l := GetNewLogger(false).SetOutput(buf)
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
	l := GetNewLogger(false).SetOutput(buf)
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

func TestAuditLog(t *testing.T) {
	t.Parallel()
	buf := &bytes.Buffer{}
	l := GetNewLogger(false).SetOutput(buf)
	var ctx context.Context
	ctx = context.WithValue(ctx, "pensando-tenant", "TestTenant")
	ctx = context.WithValue(ctx, "pensando-user-id", "TestUser")
	ctx = context.WithValue(ctx, "pensando-txnid", "TestTxnId")

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
	ctx1 := context.WithValue(ctx, "pensando-tenant", "TestTenant")
	l.Audit(ctx1, "msg", "TestMsg")
	if strings.Contains(buf.String(), "user=") || !strings.Contains(buf.String(), "tenant=TestTenant") ||
		strings.Contains(buf.String(), "txnId=") || !strings.Contains(buf.String(), "msg=TestMsg") ||
		!strings.Contains(buf.String(), "level=audit") {
		t.Errorf("Expecting [%s] got:[%s]", "[tenant=TestTenant][msg=TestMsg][level=audit]",
			buf.String())
	}
}
