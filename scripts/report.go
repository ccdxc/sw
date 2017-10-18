package main

import (
	"fmt"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
)

type CoverRec struct {
	percent float64
	passed  int
	failed  int
}

var coverage = map[string]CoverRec{}

func main() {

	targets := os.Args[1:]
	getCoverage(targets)
	getNumTests(targets)
	for target, coverRec := range coverage {
		fmt.Printf("target: %s, coverage = %2.2f%%, passed = %d, failed %d\n", target, coverRec.percent, coverRec.passed, coverRec.failed)
	}
}

func getNumTests(targets []string) {
	for _, target := range targets {
		cmd := fmt.Sprintf("GOPATH=%s go test -v -p 1 %s", os.Getenv("GOPATH"), target)
		out, err := exec.Command("sh", "-c", cmd).CombinedOutput()
		if err != nil {
			fmt.Printf("Output:\n%s\n", out)
			fmt.Printf("Error executing coverage test for target %s: %s\n", cmd, err)
			return
		}

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
		rec := CoverRec{}
		if orec, ok := coverage[target]; ok {
			rec = orec
		}
		rec.passed = passed
		rec.failed = failed
		coverage[target] = rec
	}
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

		totalPercent := 0.0
		totalCount := 0

		lines := strings.Split(string(out), "\n")
		for _, line := range lines {
			re := regexp.MustCompile("[0-9]+.[0-9]%")
			v := re.FindString(line)
			if len(v) <= 0 {
				continue
			}
			v = strings.TrimSuffix(v, "%")
			// fmt.Printf("%s: percentage: %s\n", line, v)
			f, err := strconv.ParseFloat(v, 64)
			if err != nil {
				fmt.Printf("Error: %s parsing float\n", err)
			} else {
				totalPercent += f
				totalCount++
			}
		}
		if totalCount == 0 {
			totalCount++
		}
		rec := CoverRec{}
		if orec, ok := coverage[target]; ok {
			rec = orec
		}
		rec.percent = totalPercent / float64(totalCount)
		coverage[target] = rec
	}
}
