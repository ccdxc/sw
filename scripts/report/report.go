package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"time"
)

type CoverageReport struct {
	Kind    string
	Records map[string]CoverRec
}

type Error struct {
	Kind        string
	Description string
}

type CoverRec struct {
	CoveragePercent float64
	TestDuration    time.Duration
	Passed          int
	Failed          int
}

var report = CoverageReport{Kind: "coverage", Records: map[string]CoverRec{}}

func printError(err error) {
	fmt.Printf(`{\n"Kind": "error", "Description": "%s"}`, err)
}

func main() {
	if len(os.Args) <= 1 {
		fmt.Printf("invalid arguments\n")
	}
	fileName := os.Args[1]
	targets := os.Args[2:]
	getCoverage(targets)
	getNumTests(targets)

	j, err := json.MarshalIndent(report, "", "  ")
	if err != nil {
		printError(err)
		return
	}
	fmt.Printf("%s\n", string(j))
	ioutil.WriteFile(fileName, j, 0644)
}

func findPassFail(out []byte, rec *CoverRec) CoverRec {
	passed := 0
	failed := 0

	lines := strings.Split(string(out), "\n")
	for _, line := range lines {
		if strings.HasPrefix(line, "--- PASS") {
			passed++
		} else if strings.HasPrefix(line, "--- FAIL") {
			failed++
		}
	}

	rec.Passed = passed
	rec.Failed = failed

	return *rec
}

func getNumTests(targets []string) {
	for _, target := range targets {
		cmd := fmt.Sprintf("GOPATH=%s go test -v -p 1 %s", os.Getenv("GOPATH"), target)
		t0 := time.Now()
		out, err := exec.Command("sh", "-c", cmd).CombinedOutput()
		if err != nil {
			fmt.Printf("Output:\n%s\n", out)
			fmt.Printf("Error executing coverage test for target %s: %s\n", cmd, err)
			return
		}
		t1 := time.Now()

		rec := CoverRec{}
		if orec, ok := report.Records[target]; ok {
			rec = orec
		}
		rec.TestDuration = t1.Sub(t0)

		report.Records[target] = findPassFail(out, &rec)
	}
}

func findCoveragePercent(out []byte, rec *CoverRec) CoverRec {
	totalCoveragePercent := 0.0
	totalCount := 0

	lines := strings.Split(string(out), "\n")
	for _, line := range lines {
		re := regexp.MustCompile("[0-9]+.[0-9]%")
		v := re.FindString(line)
		if len(v) <= 0 {
			continue
		}
		v = strings.TrimSuffix(v, "%")
		f, err := strconv.ParseFloat(v, 64)
		if err != nil {
			fmt.Printf("Error: %s parsing float\n", err)
		} else {
			totalCoveragePercent += f
			totalCount++
		}
	}
	if totalCount == 0 {
		totalCount++
	}
	rec.CoveragePercent = totalCoveragePercent / float64(totalCount)

	return *rec
}

func getCoverage(targets []string) {
	for _, target := range targets {
		cmd := fmt.Sprintf("GOPATH=%s go test -cover -tags test -p 1 %s", os.Getenv("GOPATH"), target)
		out, err := exec.Command("sh", "-c", cmd).CombinedOutput()
		if err != nil {
			fmt.Printf("Error executing coverage test for target %s: %s\n", cmd, err)
			fmt.Printf("Output:\n%s\n", out)
			return
		}

		rec := CoverRec{}
		if orec, ok := report.Records[target]; ok {
			rec = orec
		}
		report.Records[target] = findCoveragePercent(out, &rec)
	}
}
