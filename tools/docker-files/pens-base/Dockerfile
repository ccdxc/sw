# base container with kubelet binary included on top of systemd
FROM centos/systemd
MAINTAINER "Suresh Nalluru" <nasuku@yahoo.com>

RUN mkdir -p /target/usr/bin
RUN curl -fL --retry 3 --keepalive-time 2  -o /tmp/kubernetes-server-linux-amd64.tar.gz  https://storage.googleapis.com/kubernetes-release/release/v1.7.12/kubernetes-server-linux-amd64.tar.gz
RUN tar Oxvf /tmp/kubernetes-server-linux-amd64.tar.gz  kubernetes/server/bin/kubelet > /target/usr/bin/kubelet
RUN strip   /target/usr/bin/kubelet
RUN chmod 755 /target//usr/bin/kubelet


from golang:1.8.3
RUN mkdir -p /target/usr/bin
RUN curl -LO https://github.com/ibuildthecloud/systemd-docker/archive/v0.2.1.tar.gz
RUN tar zxvf v0.2.1.tar.gz
RUN ( cd systemd-docker-0.2.1 ; CGO_ENABLED=0 GOPATH=$(pwd)/Godeps/_workspace go build -o /target/usr/bin/systemd-docker . )
RUN strip /target/usr/bin/systemd-docker
RUN chmod 755 /target/usr/bin/systemd-docker


FROM alpine:3.6
MAINTAINER "Suresh Nalluru" <nasuku@yahoo.com>
COPY --from=0 /target/usr/bin/kubelet /target/usr/bin/kubelet
COPY --from=1 /target/usr/bin/systemd-docker /target/usr/bin/systemd-docker
ENTRYPOINT [ "/sbin/init" ]

# user is expected to run something like : docker run -v /sys/fs/cgroup:/sys/fs/cgroup:ro --privileged -d  base-container
