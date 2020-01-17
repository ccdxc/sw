FROM golang:1.13.4-alpine3.10
MAINTAINER "Barun Halder" <barun@pensando.io>
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add curl bash git make linux-headers gcc openssh
RUN go get -u github.com/vmware/govmomi/govc
RUN go get -u github.com/vmware/govmomi/vcsim
ENTRYPOINT ["/go/bin/vcsim","-l",":8989", "-trace=true"]
