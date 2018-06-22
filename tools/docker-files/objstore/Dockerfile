FROM minio/minio:RELEASE.2018-05-16T23-35-33Z
LABEL maintainer="ranjith@pensando.io"
COPY entrypoint.sh /
RUN ["chmod", "+x", "/entrypoint.sh"]
ENV HTTP_ENABLE=true
ENV MINIO_ACCESS_KEY=miniokey
ENV MINIO_SECRET_KEY=minio0523
ENTRYPOINT ["/entrypoint.sh"]
