FROM alpine:3.6
RUN apk update
RUN apk add curl
RUN curl -fL --retry 3 --keepalive-time 2  -o /tmp/kubernetes-server-linux-amd64.tar.gz  https://storage.googleapis.com/kubernetes-release/release/v1.7.12/kubernetes-server-linux-amd64.tar.gz
RUN tar xvf /tmp/kubernetes-server-linux-amd64.tar.gz  kubernetes/server/bin/kubectl
RUN chmod 755 kubernetes/server/bin/kubectl

FROM golang:1.10.0-alpine3.7
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN apk update
RUN apk add curl bash git make build-base linux-headers gcc libpcap-dev libtool
RUN curl -SsLO https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin ;
COPY --from=0 kubernetes/server/bin/kubectl /usr/local/bin/kubectl


WORKDIR /import/src/github.com/pensando/sw
ENV GOPATH=/import
ENV GOBIN=/usr/local/go/bin
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
CMD sh -c 'while :;do sleep 100; done'
