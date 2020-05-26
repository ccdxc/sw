FROM registry.test.pensando.io:5000/pens-base-2:v0.6
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add docker
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/community >> /etc/apk/repositories
COPY cmd  /
ENTRYPOINT ["/cmd"]
