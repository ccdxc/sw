FROM     registry.test.pensando.io:5000/naples:base

LABEL maintainer="Pensando Systems Inc"

COPY bashrc_sim /root/.bashrc
COPY SF-Kingdom1.postman_collection.json SF-Kingdom2.postman_collection.json \
     SF-Kingdom1-DEL.postman_collection.json postman_env.json  README /naples/nic/data/examples/
COPY bootstrap.sh /naples/nic/tools/
ADD nic.tgz /naples/
HEALTHCHECK --interval=2m --timeout=5s --start-period=5m --retries=3 CMD /naples/nic/tools/health-check.sh || exit 1

CMD ["/naples/nic/tools/start-naples.sh"]
