// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.
/*
 * Package cmd is a auto generated package.
 * Input file: flowstats.proto
 */

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var ipv4flowbehavioralmetricsShowCmd = &cobra.Command{

	Use:   "ipv4flowbehavioral",
	Short: "Show IPv4FlowBehavioralMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv4FlowBehavioralMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv4flowbehavioralmetricsShowCmdHandler,
}

func ipv4flowbehavioralmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv4flowbehavioralmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv4flowbehavioral object(s) found")
	}
	return nil
}

var ipv4flowdropmetricsShowCmd = &cobra.Command{

	Use:   "ipv4flowdrop",
	Short: "Show IPv4FlowDropMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv4FlowDropMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv4flowdropmetricsShowCmdHandler,
}

func ipv4flowdropmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv4flowdropmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv4flowdrop object(s) found")
	}
	return nil
}

var ipv4flowlatencymetricsShowCmd = &cobra.Command{

	Use:   "ipv4flowlatency",
	Short: "Show IPv4FlowLatencyMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv4FlowLatencyMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv4flowlatencymetricsShowCmdHandler,
}

func ipv4flowlatencymetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv4flowlatencymetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv4flowlatency object(s) found")
	}
	return nil
}

var ipv4flowperformancemetricsShowCmd = &cobra.Command{

	Use:   "ipv4flowperformance",
	Short: "Show IPv4FlowPerformanceMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv4FlowPerformanceMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv4flowperformancemetricsShowCmdHandler,
}

func ipv4flowperformancemetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv4flowperformancemetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv4flowperformance object(s) found")
	}
	return nil
}

var ipv4flowrawmetricsShowCmd = &cobra.Command{

	Use:   "ipv4flowraw",
	Short: "Show IPv4FlowRawMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv4FlowRawMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv4flowrawmetricsShowCmdHandler,
}

func ipv4flowrawmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv4flowrawmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv4flowraw object(s) found")
	}
	return nil
}

var ipv6flowbehavioralmetricsShowCmd = &cobra.Command{

	Use:   "ipv6flowbehavioral",
	Short: "Show IPv6FlowBehavioralMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv6FlowBehavioralMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv6flowbehavioralmetricsShowCmdHandler,
}

func ipv6flowbehavioralmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv6flowbehavioralmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv6flowbehavioral object(s) found")
	}
	return nil
}

var ipv6flowdropmetricsShowCmd = &cobra.Command{

	Use:   "ipv6flowdrop",
	Short: "Show IPv6FlowDropMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv6FlowDropMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv6flowdropmetricsShowCmdHandler,
}

func ipv6flowdropmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv6flowdropmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv6flowdrop object(s) found")
	}
	return nil
}

var ipv6flowlatencymetricsShowCmd = &cobra.Command{

	Use:   "ipv6flowlatency",
	Short: "Show IPv6FlowLatencyMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv6FlowLatencyMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv6flowlatencymetricsShowCmdHandler,
}

func ipv6flowlatencymetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv6flowlatencymetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv6flowlatency object(s) found")
	}
	return nil
}

var ipv6flowperformancemetricsShowCmd = &cobra.Command{

	Use:   "ipv6flowperformance",
	Short: "Show IPv6FlowPerformanceMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv6FlowPerformanceMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv6flowperformancemetricsShowCmdHandler,
}

func ipv6flowperformancemetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv6flowperformancemetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv6flowperformance object(s) found")
	}
	return nil
}

var ipv6flowrawmetricsShowCmd = &cobra.Command{

	Use:   "ipv6flowraw",
	Short: "Show IPv6FlowRawMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show IPv6FlowRawMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  ipv6flowrawmetricsShowCmdHandler,
}

func ipv6flowrawmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/ipv6flowrawmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No ipv6flowraw object(s) found")
	}
	return nil
}

var l2flowbehavioralmetricsShowCmd = &cobra.Command{

	Use:   "l2flowbehavioral",
	Short: "Show L2FlowBehavioralMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show L2FlowBehavioralMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  l2flowbehavioralmetricsShowCmdHandler,
}

func l2flowbehavioralmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/l2flowbehavioralmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No l2flowbehavioral object(s) found")
	}
	return nil
}

var l2flowdropmetricsShowCmd = &cobra.Command{

	Use:   "l2flowdrop",
	Short: "Show L2FlowDropMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show L2FlowDropMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  l2flowdropmetricsShowCmdHandler,
}

func l2flowdropmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/l2flowdropmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No l2flowdrop object(s) found")
	}
	return nil
}

var l2flowperformancemetricsShowCmd = &cobra.Command{

	Use:   "l2flowperformance",
	Short: "Show L2FlowPerformanceMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show L2FlowPerformanceMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  l2flowperformancemetricsShowCmdHandler,
}

func l2flowperformancemetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/l2flowperformancemetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No l2flowperformance object(s) found")
	}
	return nil
}

var l2flowrawmetricsShowCmd = &cobra.Command{

	Use:   "l2flowraw",
	Short: "Show L2FlowRawMetrics from Distributed Service Card",
	Long:  "\n---------------------------------\n Show L2FlowRawMetrics From Distributed Service Card \n---------------------------------\n",
	RunE:  l2flowrawmetricsShowCmdHandler,
}

func l2flowrawmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = true
	bodyBytes, err := restGet("telemetry/v1/metrics/l2flowrawmetrics/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if bodyBytes == nil {
		fmt.Println("No l2flowraw object(s) found")
	}
	return nil
}

func init() {

	metricsShowCmd.AddCommand(ipv4flowbehavioralmetricsShowCmd)

	metricsShowCmd.AddCommand(ipv4flowdropmetricsShowCmd)

	metricsShowCmd.AddCommand(ipv4flowlatencymetricsShowCmd)

	metricsShowCmd.AddCommand(ipv4flowperformancemetricsShowCmd)

	metricsShowCmd.AddCommand(ipv4flowrawmetricsShowCmd)

	metricsShowCmd.AddCommand(ipv6flowbehavioralmetricsShowCmd)

	metricsShowCmd.AddCommand(ipv6flowdropmetricsShowCmd)

	metricsShowCmd.AddCommand(ipv6flowlatencymetricsShowCmd)

	metricsShowCmd.AddCommand(ipv6flowperformancemetricsShowCmd)

	metricsShowCmd.AddCommand(ipv6flowrawmetricsShowCmd)

	metricsShowCmd.AddCommand(l2flowbehavioralmetricsShowCmd)

	metricsShowCmd.AddCommand(l2flowdropmetricsShowCmd)

	metricsShowCmd.AddCommand(l2flowperformancemetricsShowCmd)

	metricsShowCmd.AddCommand(l2flowrawmetricsShowCmd)

}