package systemd

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/coreos/go-systemd/dbus"
)

func setupConn(t *testing.T) *dbus.Conn {
	conn, err := dbus.New()
	if err != nil {
		if strings.Contains(err.Error(), "no such file or directory") {
			t.Skip("Requires systemd. Skipping tests")
			return nil
		}
		t.Fatal(err)
	}

	return conn
}

func findFixture(target string, t *testing.T) string {
	abs, err := filepath.Abs("test/" + target)
	if err != nil {
		t.Fatal(err)
	}
	return abs
}

func teardownUnit(target string, conn *dbus.Conn, t *testing.T) {
	// Blindly stop the unit in case it is running
	conn.StopUnit(target, "replace", nil)

	// Blindly remove the symlink in case it exists
	targetRun := filepath.Join("/run/systemd/system/", target)
	os.Remove(targetRun)
}

func setupUnit(target string, conn *dbus.Conn, t *testing.T) {
	// First teardown any existing unit
	teardownUnit(target, conn, t)

	abs := findFixture(target, t)
	fixture := []string{abs}

	changes, err := conn.LinkUnitFiles(fixture, true, true)
	if err != nil {
		t.Fatal(err)
	}

	if len(changes) < 1 {
		t.Fatalf("Expected one change, got %v", changes)
	}

	runPath := filepath.Join("/run/systemd/system/", target)
	if changes[0].Filename != runPath {
		t.Fatal("Unexpected target filename")
	}
}

func enableUnit(target string, conn *dbus.Conn, t *testing.T) {
	abs := findFixture(target, t)
	_, _, err := conn.EnableUnitFiles([]string{abs}, true, true)
	if err != nil {
		t.Fatal(err)
	}
}

func disableUnit(target string, conn *dbus.Conn, t *testing.T) {
	_, err := conn.DisableUnitFiles([]string{target}, true)
	if err != nil {
		t.Fatal(err)
	}
}

func getUnitStatus(units []dbus.UnitStatus, name string) *dbus.UnitStatus {
	for _, u := range units {
		if u.Name == name {
			return &u
		}
	}
	return nil
}

// Ensure that basic unit starting and stopping works.
func TestStartStopUnit(t *testing.T) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. systemd needs this permission")
	}

	target := "start-stop.service"
	conn := setupConn(t)
	defer conn.Close()

	setupUnit(target, conn, t)
	defer teardownUnit(target, conn, t)

	s := New()
	err := s.StartTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	units, err := conn.ListUnits()
	if err != nil {
		t.Fatal(err)
	}

	unit := getUnitStatus(units, target)

	if unit == nil {
		t.Fatalf("Test unit not found in list")
	} else if unit.ActiveState != "active" {
		t.Fatalf("Test unit not active")
	}

	err = s.StopTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	units, err = conn.ListUnits()
	if err != nil {
		t.Fatal(err)
	}

	unit = getUnitStatus(units, target)

	if unit != nil {
		t.Fatalf("Test unit found in list, should be stopped")
	}
}

// Ensure that basic unit restarting works.
func TestRestartUnitAfterStart(t *testing.T) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. systemd needs this permission")
	}

	target := "start-stop.service"
	conn := setupConn(t)
	defer conn.Close()

	setupUnit(target, conn, t)
	defer teardownUnit(target, conn, t)

	s := New()
	err := s.StartTarget(target)
	if err != nil {
		t.Fatal(err)
	}
	units, err := conn.ListUnits()
	if err != nil {
		t.Fatal(err)
	}

	unit := getUnitStatus(units, target)
	if unit == nil {
		t.Fatalf("Test unit not found in list")
	} else if unit.ActiveState != "active" {
		t.Fatalf("Test unit not active")
	}

	err = s.RestartTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	units, err = conn.ListUnits()
	if err != nil {
		t.Fatal(err)
	}

	unit = getUnitStatus(units, target)
	if unit == nil {
		t.Fatalf("Test unit not found in list")
	} else if unit.ActiveState != "active" {
		t.Fatalf("Test unit not active")
	}
}

// Ensure that unit restarting works when Unit is stopped
func TestRestartUnitAfterStop(t *testing.T) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. systemd needs this permission")
	}

	target := "start-stop.service"
	conn := setupConn(t)
	defer conn.Close()

	setupUnit(target, conn, t)
	defer teardownUnit(target, conn, t)
	// service is stopped by now

	s := New()
	err := s.RestartTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	units, err := conn.ListUnits()
	if err != nil {
		t.Fatal(err)
	}

	unit := getUnitStatus(units, target)
	if unit == nil {
		t.Fatalf("Test unit not found in list")
	} else if unit.ActiveState != "active" {
		t.Fatalf("Test unit not active")
	}
}

var (
	target = "pensando.target"
	srv1   = "service1.service"
	srv2   = "service2.service"
)

func setupThreeUnitTest(t *testing.T, conn *dbus.Conn) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. systemd needs this permission")
	}
	setupUnit(target, conn, t)
	setupUnit(srv1, conn, t)
	enableUnit(srv1, conn, t)
	setupUnit(srv2, conn, t)
	enableUnit(srv2, conn, t)
}
func teardownThreeUnitTest(t *testing.T, conn *dbus.Conn) {
	if os.Getuid() != 0 {
		t.Skip("Must run the tests as root. systemd needs this permission")
	}
	disableUnit(srv2, conn, t)
	disableUnit(srv1, conn, t)
	teardownUnit(srv2, conn, t)
	teardownUnit(srv1, conn, t)
	teardownUnit(target, conn, t)
}

func TestTargetDepsAndPID(t *testing.T) {
	conn := setupConn(t)
	defer conn.Close()

	setupThreeUnitTest(t, conn)
	defer teardownThreeUnitTest(t, conn)

	s := New()
	w, _, _ := s.NewWatcher()
	defer w.Close()

	// Test TargetDeps
	l, err := w.TargetDeps(target)
	if err != nil {
		t.Error(err)
	}
	if len(l) != 2 {
		t.Fatalf("Expected 2 dependents. Got %d for target: %s", len(l), target)
	}
	if l[0] != srv1 && l[0] != srv2 {
		t.Fatalf("unExpected dependents %s for target: %s", l[0], target)
	}
	if l[1] != srv1 && l[1] != srv2 {
		t.Fatalf("unExpected dependents %s for target: %s", l[1], target)
	}

	// For targets there is no PID and hence expect error
	_, err = w.GetPID(target)
	if err == nil {
		t.Fatalf("Expected no PID for systemd target. Got one")
	}

	// GetPID should succeed for units though
	_, err = w.GetPID(srv1)
	if err != nil {
		t.Fatalf("unable to determine PID for unit: %s", srv1)
	}

}
func checkExpectedEvents(t *testing.T, m *map[UnitEvent]struct{}, evChan <-chan *UnitEvent, errChan <-chan error) {
loop:
	for {
		select {
		case e, _ := <-evChan:
			delete(*m, *e)
		case e, _ := <-errChan:
			t.Fatalf("unExpected events in ErrorChannel %+v", e)

			// we wait even after receiving every expectedEvents to make sure we dont receive unexpected stuff
		case <-time.After(2 * time.Second):
			if len(*m) != 0 {
				t.Fatalf("Didnt receive expected events %+v in channel . ", *m)
			}
			break loop
		}
	}
}

func TestWatcherStartTarget(t *testing.T) {
	conn := setupConn(t)
	defer conn.Close()

	setupThreeUnitTest(t, conn)
	defer teardownThreeUnitTest(t, conn)

	s := New()
	w, evChan, errChan := s.NewWatcher()
	defer w.Close()

	// Test Subscribe
	w.Subscribe(target)

	// Check for expected events after StartTarget
	err := s.StartTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	var expectedEvents = map[UnitEvent]struct{}{
		{target, Active, "desc_target", "loaded", "active", "active"}:  {},
		{srv1, Active, "desc_service1", "loaded", "active", "running"}: {},
		{srv2, Active, "desc_service2", "loaded", "active", "running"}: {},
	}

	checkExpectedEvents(t, &expectedEvents, evChan, errChan)
}

func TestWatcherReStartTargetAfterStart(t *testing.T) {
	conn := setupConn(t)
	defer conn.Close()

	setupThreeUnitTest(t, conn)
	defer teardownThreeUnitTest(t, conn)

	s := New()
	w, evChan, errChan := s.NewWatcher()
	defer w.Close()
	s.StartTarget(target)

	// Test Subscribe
	w.Subscribe(target)

	// Since the units are not running, Restart will only give the start events
	var expectedEvents = map[UnitEvent]struct{}{
		{target, Active, "desc_target", "loaded", "active", "active"}:  {},
		{srv1, Active, "desc_service1", "loaded", "active", "running"}: {},
		{srv2, Active, "desc_service2", "loaded", "active", "running"}: {},
		{target, Dead, "", "", "", ""}:                                 {},
		{srv1, Dead, "", "", "", ""}:                                   {},
		{srv2, Dead, "", "", "", ""}:                                   {},
	}

	err := s.RestartTarget(target)
	if err != nil {
		t.Fatal(err)
	}
	checkExpectedEvents(t, &expectedEvents, evChan, errChan)
}

func TestWatcherReStartTargetAfterStop(t *testing.T) {
	conn := setupConn(t)
	defer conn.Close()

	setupThreeUnitTest(t, conn)
	defer teardownThreeUnitTest(t, conn)

	s := New()
	w, evChan, errChan := s.NewWatcher()
	defer w.Close()

	// Test Subscribe
	w.Subscribe(target)

	// Since the units are not running, Restart will only give the start events
	var expectedEvents = map[UnitEvent]struct{}{
		{target, Active, "desc_target", "loaded", "active", "active"}:  {},
		{srv1, Active, "desc_service1", "loaded", "active", "running"}: {},
		{srv2, Active, "desc_service2", "loaded", "active", "running"}: {},
	}

	err := s.RestartTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	checkExpectedEvents(t, &expectedEvents, evChan, errChan)
}

func TestWatcherStopTarget(t *testing.T) {
	conn := setupConn(t)
	defer conn.Close()

	setupThreeUnitTest(t, conn)
	defer teardownThreeUnitTest(t, conn)

	s := New()
	s.StartTarget(target)

	w, evChan, errChan := s.NewWatcher()
	defer w.Close()

	// Test Subscribe
	w.Subscribe(target)

	err := s.StopTarget(target)
	if err != nil {
		t.Fatal(err)
	}

	var expectedEvents = map[UnitEvent]struct{}{
		{target, Dead, "", "", "", ""}: {},
		{srv1, Dead, "", "", "", ""}:   {},
		{srv2, Dead, "", "", "", ""}:   {},
	}

	checkExpectedEvents(t, &expectedEvents, evChan, errChan)
}
