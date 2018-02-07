package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"time"
)

// minCoverage is the minimum expected coverage for a package
const (
	goTestCmd   = "GOPATH=%s VENICE_DEV=1 go test -cover -tags test -p 1 %s"
	minCoverage = 75.0
	failPrefix  = "--- FAIL:"
)

var ErrTestFailed = errors.New("test execution failed")
var ErrTestCovFailed = errors.New("coverage tests failed")

// TestReport summarizes all the targets. We capture only the failed tests.
type TestReport struct {
	RunFailed bool
	Results   []*Target
}

// Target holds test execution details.
type Target struct {
	Name        string
	Coverage    float64
	Duration    string
	FailedTests []string
	Error       string
}

func main() {
	if len(os.Args) <= 1 {
		log.Fatalf("report needs at-least one target to run tests.")
	}

	var t TestReport
	for _, tgt := range os.Args[1:] {
		p := Target{
			Name: tgt,
		}
		t.Results = append(t.Results, &p)
	}
	t.RunCoverage()
	j, _ := t.reportToJson()

	t.testCoveragePass()

	if t.RunFailed {
		log.Fatalf("Tests failed. %s", fmt.Sprintf(string(j)))
	}

	fmt.Println(string(j))
}

func (t *TestReport) RunCoverage() {
	for _, tgt := range t.Results {
		err := tgt.test()
		if err != nil || tgt.Error != "" {
			t.RunFailed = true
		}
	}
}

func (t *TestReport) reportToJson() ([]byte, error) {
	return json.MarshalIndent(t, "", "  ")
}

func (t *TestReport) testCoveragePass() {
	for _, tgt := range t.Results {
		if tgt.Error == ErrTestCovFailed.Error() {
			log.Println(fmt.Sprintf("\033[31m%s\033[39m", "Insufficient code coverage for the following packages:"))
			log.Println(fmt.Sprintf("\033[31m%s\033[39m", tgt.Name))
			t.RunFailed = true
		}
	}
}

func (tgt *Target) test() error {
	cmd := fmt.Sprintf(goTestCmd, os.Getenv("GOPATH"), tgt.Name)
	start := time.Now()
	defer func(tgt *Target) {
		tgt.Duration = time.Since(start).String()
	}(tgt)
	out, err := exec.Command("sh", "-c", cmd).CombinedOutput()
	if err != nil {
		fmt.Println(string(out))
		tgt.Error = ErrTestFailed.Error()
	}

	err = tgt.parseCmdOutput(out)
	if tgt.Coverage < minCoverage && tgt.Error == "" {
		tgt.Error = ErrTestCovFailed.Error()
	}
	return err
}

func (tgt *Target) parseCmdOutput(b []byte) error {
	err := tgt.getFailedTests(b)
	if err != nil {
		log.Printf("could not get failed tests: %v\n", err)
		return err
	}
	return tgt.getCoveragePercent(b)
}

func (tgt *Target) getFailedTests(b []byte) error {
	lines := strings.Split(string(b), "\n")
	for _, line := range lines {
		if strings.HasPrefix(line, failPrefix) {
			t := strings.Split(line, " ")[2]
			tgt.FailedTests = append(tgt.FailedTests, t)
		}
	}
	if len(tgt.FailedTests) > 0 {
		log.Printf("Test Failure: %v\n", tgt.Name)
		return ErrTestFailed
	}
	return nil
}

func (tgt *Target) getCoveragePercent(b []byte) error {
	lines := strings.Split(string(b), "\n")
	for _, line := range lines {
		re := regexp.MustCompile("[0-9]+.[0-9]%")
		v := re.FindString(line)
		if len(v) <= 0 {
			continue
		}
		v = strings.TrimSuffix(v, "%")
		f, err := strconv.ParseFloat(v, 64)
		if err != nil {
			fmt.Printf("error: %s parsing float\n", err)
		} else {
			tgt.Coverage = f
		}
	}
	return nil
}
