// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cni

import (
	"flag"
	"fmt"

	v1 "k8s.io/api/core/v1"
	meta_v1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	clientset "k8s.io/client-go/kubernetes"
	"k8s.io/client-go/rest"
	"k8s.io/client-go/tools/clientcmd"
)

// KubeletConfPath is the location of kubelet config file.
// We use kubelet config to get Auth certificates to talk to API server
const KubeletConfPath = "/etc/kubernetes/kubelet.conf"

// KubeClient is the kubernetes api server client
type KubeClient struct {
	config    *rest.Config        // kubeconfig or kubelet config for auth
	clientset clientset.Interface // apiserver client
	isMock    bool                // mock kubeclient API
}

// NewKubeclient returns a kubeclient instance
func NewKubeclient() (*KubeClient, error) {
	if flag.Lookup("test.v") != nil {
		return &KubeClient{nil, nil, true}, nil
	}

	// uses the current context in kubeconfig
	config, err := clientcmd.BuildConfigFromFlags("", KubeletConfPath)
	if err != nil {
		return nil, fmt.Errorf("Error creating kubeclient config: %v", err)
	}

	// creates the clientset
	clientset, err := clientset.NewForConfig(config)
	if err != nil {
		return nil, fmt.Errorf("Error creating kubeclient clientset: %v", err)
	}

	// create a new kube client instance
	kubeclient := KubeClient{
		config:    config,
		clientset: clientset,
	}

	return &kubeclient, nil
}

// GetPod returns a k8s Pod info from namespace and pod name
func (kc *KubeClient) GetPod(namespace, podName string) (*v1.Pod, error) {
	// mock the response if required
	if kc.isMock {
		return &v1.Pod{}, nil
	}

	// get info from API server
	return kc.clientset.CoreV1().Pods(namespace).Get(podName, meta_v1.GetOptions{})
}

// ListPods returns a list of all pods in a namespace
func (kc *KubeClient) ListPods(namespace string) (*v1.PodList, error) {
	// mock the response if required
	if kc.isMock {
		return &v1.PodList{}, nil
	}

	// get info from API server
	return kc.clientset.CoreV1().Pods(namespace).List(meta_v1.ListOptions{})
}
