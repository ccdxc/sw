FROM alpine:3.6
MAINTAINER "Vijayan Ramakrishnan" <vijayan@pensando.io>
RUN apk update
COPY nmd /
CMD /nmd -cmd pen-master:9002 -hostif s0
