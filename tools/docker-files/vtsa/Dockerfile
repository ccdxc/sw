FROM registry.test.pensando.io:5000/pens-base-2:v0.4
MAINTAINER "Barun Halder"
COPY vtsa /bin
COPY vtsa.json /bin
ADD https://storage.googleapis.com/kubernetes-release/release/v1.7.14/bin/linux/amd64/kubectl /bin/kubectl
RUN chmod +x /bin/kubectl
ENV KUBECONFIG="/etc/pensando/kubernetes/pen-kubelet/kubeconfig"
ENTRYPOINT ["/bin/vtsa"]
