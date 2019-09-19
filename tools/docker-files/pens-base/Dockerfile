# base container with kubelet binary included on top of systemd
FROM centos/systemd
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>

RUN mkdir -p /target/usr/bin
RUN curl -fL --retry 3 --keepalive-time 2  -o /tmp/kubernetes-server-linux-amd64.tar.gz  https://storage.googleapis.com/kubernetes-release/release/v1.15.3/kubernetes-server-linux-amd64.tar.gz
RUN tar Oxvf /tmp/kubernetes-server-linux-amd64.tar.gz  kubernetes/server/bin/kubelet > /target/usr/bin/kubelet
RUN strip   /target/usr/bin/kubelet
RUN chmod 755 /target//usr/bin/kubelet
RUN tar Oxvf /tmp/kubernetes-server-linux-amd64.tar.gz  kubernetes/server/bin/kubectl > /target/usr/bin/kubectl
RUN strip   /target/usr/bin/kubectl
RUN chmod 755 /target//usr/bin/kubectl

FROM alpine:3.10.2
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
COPY --from=0 /target/usr/bin/kubelet /target/usr/bin/kubelet
COPY --from=0 /target/usr/bin/kubectl /target/usr/bin/kubectl
COPY systemd-docker.linux.x86-64 /target/usr/bin/systemd-docker
ENTRYPOINT [ "/sbin/init" ]

# user is expected to run something like : docker run -v /sys/fs/cgroup:/sys/fs/cgroup:ro --privileged -d  base-container
