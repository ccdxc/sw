FROM golang:1.9.3-alpine3.7
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN apk update
RUN apk add curl bash git make build-base linux-headers gcc libpcap-dev libtool protobuf-dev
RUN curl -SsLO https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin ;

# Install Swagger
RUN curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64 ; chmod +x /usr/local/bin/swagger

COPY goinstall.bash /tmp/goinstall.bash
# extra go packages
RUN chmod +x /tmp/goinstall.bash && /tmp/goinstall.bash

COPY env.sh /etc/profile.d/env.sh

WORKDIR /import/src/github.com/pensando/sw
ENV GOPATH=/import
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
CMD make build
