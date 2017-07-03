FROM alpine:3.6
RUN apk update
RUN apk add openntpd
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
ENTRYPOINT ["ntpd"]
CMD ["-d" ,"-f", "/etc/pensando/ntpd.conf", "-s"]