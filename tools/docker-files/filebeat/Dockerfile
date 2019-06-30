FROM docker.elastic.co/beats/filebeat:6.3.0
USER root
COPY docker-entrypoint.sh /
RUN ["chmod", "+x", "/docker-entrypoint.sh"]
CMD ["/docker-entrypoint.sh"]