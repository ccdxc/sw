# this is the base image for Venice components:
# ApiServer, ApiGw, OrchHub, NPM, n4sagent
# includes libtool which is needed by keymgr for PKCS#11 backend
FROM alpine:3.10
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add libtool blkid
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/community >> /etc/apk/repositories
ENV GODEBUG=madvdontneed=1
