FROM golang:1.13.8-alpine3.10
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add curl bash git make build-base linux-headers gcc libpcap-dev libtool protobuf-dev
RUN curl -SsLO https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin ;
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/community >> /etc/apk/repositories

# Install Swagger
RUN curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64 ; chmod +x /usr/local/bin/swagger

COPY env.sh /etc/profile.d/env.sh

WORKDIR /import/src/github.com/pensando/sw
ENV GOPATH=/import
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
CMD make build
