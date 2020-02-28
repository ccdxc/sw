from python:3.7-alpine3.7

ENV CURATOR_VERSION 5.8

RUN apk --update add python py-setuptools py-pip && \
  pip install elasticsearch-curator==5.8 && \
  apk del py-pip && \
  rm -rf /var/cache/apk/*

COPY actions.yml /actions.yml