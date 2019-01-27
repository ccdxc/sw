package main

import (
	"bufio"
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

	"github.com/pensando/test-infra-tracker/types"
)

// minCoverage is the minimum expected coverage for a package
const (
	goTestCmd   = "GOPATH=%s VENICE_DEV=1 CGO_LDFLAGS_ALLOW=-I/usr/local/share/libtool go test -cover -tags test -p 1 %s"
	minCoverage = 75.0
	failPrefix  = "--- FAIL:"
	// report should not enforce coverage when there are no test files.
	covIgnorePrefix = "[no test files]"
	// only handle pensando/sw repo
	swPkgPrefix = "github.com/pensando/sw/"
	swBaseRepo  = "pensando/sw"
	testbed     = "jobd-ci"
)

var (
	// ErrTestFailed errors out when there is a compilation failure or test assertions.
	ErrTestFailed = errors.New("test execution failed")

	// ErrTestCovFailed errors out when the coverage percent is < 75.0%
	ErrTestCovFailed = errors.New("coverage tests failed")

	errFailSend    = errors.New("failed to send test tracker report")
	errNotBaseRepo = errors.New("not pensando/sw repo")

	retryTrackerInterval = 5 * time.Second
	retryTrackerCount    = 5

	covIgnoreFilePath = fmt.Sprintf("%s/src/github.com/pensando/sw/scripts/report/.coverignore", os.Getenv("GOPATH"))
)

// TestReport summarizes all the targets. We capture only the failed tests.
type TestReport struct {
	RunFailed       bool
	Results         []*Target
	IgnoredPackages []string
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

	// Read the .covignore file
	f, _ := os.Open(covIgnoreFilePath)
	defer f.Close()

	s := bufio.NewScanner(f)
	for s.Scan() {
		t.IgnoredPackages = append(t.IgnoredPackages, s.Text())
	}

	for _, tgt := range os.Args[1:] {
		p := Target{
			Name: tgt,
		}
		t.Results = append(t.Results, &p)
	}
	t.runCoverage()
	j, _ := t.reportToJSON()

	t.testCoveragePass()

	// send result to testtracker only in jobd CI environment
	if isJobdCI() {
		trackerURL := os.Getenv("TRACKER_URL")
		if trackerURL != "" {
			if err := t.sendToTestTracker(trackerURL); err != nil {
				fmt.Println(err)
			}
		} else {
			fmt.Println("TRACKER_URL environment variable is not defined")
		}
	}

	if t.RunFailed {
		t = t.filterFailedTests()
		failedTests, _ := t.reportToJSON()
		log.Fatalf("Tests failed. %s", fmt.Sprintf(string(failedTests)))
	}

	fmt.Println(string(j))
}

func isJobdCI() bool {
	fmt.Printf("JOB_REPOSITORY: %s, JOB_FORK_REPOSITORY: %s, JOB_BASE_REPOSITORY: %s, JOB_ID: %s, TARGET_ID: %s\n",
		os.Getenv("JOB_REPOSITORY"), os.Getenv("JOB_FORK_REPOSITORY"), os.Getenv("JOB_BASE_REPOSITORY"),
		os.Getenv("JOB_ID"), os.Getenv("TARGET_ID"))
	return os.Getenv("JOB_ID") != "" &&
		os.Getenv("TARGET_ID") != "" &&
		os.Getenv("JOB_FORK_REPOSITORY") != ""
}

func (t *TestReport) filterFailedTests() TestReport {
	var retval TestReport
	retval.RunFailed = t.RunFailed
	for _, tgt := range t.Results {
		if tgt.Error != "" {
			retval.Results = append(retval.Results, tgt)
		}
	}
	return retval
}

func (t *TestReport) runCoverage() {
	for _, tgt := range t.Results {
		err := tgt.test(t.IgnoredPackages)
		if err != nil || tgt.Error != "" {
			t.RunFailed = true
		}
	}
}

func (t *TestReport) reportToJSON() ([]byte, error) {
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

func (tgt *Target) test(ignoredPackages []string) error {
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
	if tgt.Error == "" {
		isCovIgnored := tgt.checkCoverageIgnore(ignoredPackages)
		if tgt.Coverage < minCoverage && !isCovIgnored {
			tgt.Error = ErrTestCovFailed.Error()
		}
		return err
	}
	return nil
}

func (tgt *Target) checkCoverageIgnore(ignoredPackages []string) (mustIgnore bool) {
	for _, i := range ignoredPackages {
		if tgt.Name == i {
			mustIgnore = true
			return
		}
	}
	return
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
	out := strings.TrimSpace(string(b))
	lines := strings.Split(out, "\n")
	for _, line := range lines {
		// We need this for packages which doesn't have any test files.
		// Common cases include binary only packages, generated code, type definitions.
		if strings.Contains(line, covIgnorePrefix) {
			tgt.Coverage = 100.0
			continue
		}
		// The coverage percentage parsing should be > 0.0%
		// Cases include packages which has a *test.go file but doesn't test the main binary.
		// This will also ignore parsing coverage details for the integration tests themselves
		re := regexp.MustCompile("[1-9]+[0-9]*.[0-9]*%")
		v := re.FindString(line)
		if len(v) <= 0 {
			tgt.Coverage = 100.0
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

func (t *TestReport) sendToTestTracker(trackerURL string) error {
	if os.Getenv("JOB_FORK_REPOSITORY") != swBaseRepo {
		return errNotBaseRepo
	}

	targetID, err := strconv.Atoi(os.Getenv("TARGET_ID"))
	if err != nil {
		return fmt.Errorf("while getting target ID: %v", err)
	}

	sha := "unknown"
	title := "unknown"
	content, err := exec.Command("git", "rev-parse", "--short", "HEAD").Output()
	if err != nil {
		return fmt.Errorf("while extracting sha: %v", err)
	}
	sha = string(content)

	content, err = exec.Command("sh", "-c", `git log --format=%s -n 1 `+sha).CombinedOutput()
	if err != nil {
		return fmt.Errorf("while extracting log of sha: %v", err)
	}
	title = strings.TrimSpace(string(content))

	cli, err := types.NewClient(trackerURL)
	if err != nil {
		return err
	}

	reports := types.Reports{
		Repository: swBaseRepo,
		Testbed:    testbed,
		SHA:        sha,
		SHATitle:   title,
		TargetID:   int32(targetID),
		Testcases:  []*types.Report{},
	}
	for _, tgt := range t.Results {
		n := strings.TrimPrefix(tgt.Name, swPkgPrefix)
		report := &types.Report{
			Name:        n,
			Description: n,
			FinishTime:  time.Now(),
			Coverage:    int32(tgt.Coverage),
			LogURL:      fmt.Sprintf("http://jobd/logs/%d", targetID),
		}
		if tgt.FailedTests == nil && tgt.Error == "" {
			report.Result = 1
		} else {
			report.Result = -1
			report.Detail = tgt.Error
		}
		d, err := time.ParseDuration(tgt.Duration)
		if err != nil {
			// if error, set duration to 0
			fmt.Printf("while converting %s's test duration to second: %v\n", tgt.Duration, err)
			report.Duration = 0
		} else {
			report.Duration = int32(d.Seconds())
		}

		parts := strings.SplitN(strings.TrimPrefix(tgt.Name, swPkgPrefix), "/", 2)
		report.Area = parts[0]
		if len(parts) > 1 {
			report.Subarea = parts[1]
		}
		reports.Testcases = append(reports.Testcases, report)
	}

	// send reports with 5 retry
	for retry := 0; retry < retryTrackerCount; retry++ {
		if err := cli.Report(&reports); err == nil {
			return nil
		} else if !strings.Contains(err.Error(), "connection timed out") &&
			!strings.Contains(err.Error(), "connection refused") {
			// retry only if connection failure, for example potential network issue, server restart
			return err
		} else {
			fmt.Printf("while sending report: %v\n", err)
		}

		fmt.Printf("sleep and retry %d time\n", retry+1)
		time.Sleep(retryTrackerInterval)
	}
	return errFailSend
}
