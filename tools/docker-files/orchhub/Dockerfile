FROM registry.test.pensando.io:5000/pens-base-2:v0.5
MAINTAINER "Barun Halder"
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add curl bash git make linux-headers gcc openssh
RUN apk add --no-cache git make musl-dev go
RUN go get -u github.com/vmware/govmomi/govc
COPY orchhub /
ENTRYPOINT ["/orchhub"]
