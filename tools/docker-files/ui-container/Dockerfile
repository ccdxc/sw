FROM node:10.15.3-stretch
MAINTAINER "Ravi Gadde" <ravi@pensando.io>


# Install npm packages
RUN mkdir -p /usr/local/lib/web-app-framework/dist
COPY web-app-framework/package.json /usr/local/lib/web-app-framework
COPY web-app-framework/npm-shrinkwrap.json /usr/local/lib/web-app-framework
COPY web-app-framework/dist/web-app-framework-0.0.0.tgz /usr/local/lib/web-app-framework/dist/web-app-framework-0.0.0.tgz
RUN cd /usr/local/lib/web-app-framework && npm install && npm run replaceShrinkwrap
RUN cd /usr/local/lib/web-app-framework && tar zcf node_modules.tgz node_modules

RUN mkdir -p /usr/local/lib/webapp
COPY webapp/package.json /usr/local/lib/webapp
COPY webapp/npm-shrinkwrap.json /usr/local/lib/webapp
RUN cd /usr/local/lib/webapp
RUN cd /usr/local/lib/webapp && npm install  && npm run replaceShrinkwrap && tar zcf node_modules.tgz node_modules

RUN mkdir -p /usr/local/lib/venice-sdk
COPY venice-sdk/package.json /usr/local/lib/venice-sdk
COPY venice-sdk/npm-shrinkwrap.json /usr/local/lib/venice-sdk
COPY venice-sdk/pensando-swagger-ts-generator-1.1.29.tgz /usr/local/lib/venice-sdk
RUN cd /usr/local/lib/venice-sdk && npm install && npm run replaceShrinkwrap && tar zcf node_modules.tgz node_modules

# Install ng
USER node
RUN mkdir /home/node/.npm-global
ENV PATH=/home/node/.npm-global/bin:$PATH
ENV NPM_CONFIG_PREFIX=/home/node/.npm-global
RUN npm install -g @angular/cli@7.0.4
# Install spectacle-docs for venice docs generation
RUN npm install -g spectacle-docs
RUN ng -v

USER root
RUN chmod -R 777 /root

FROM node:10.15.3-stretch
ENV PATH=/home/node/.npm-global/bin:$PATH

RUN mkdir -p /usr/local/lib/web-app-framework /usr/local/lib/webapp /usr/local/lib/venice-sdk /home/node/.npm-global /root/.npm && chmod -R 777 /root
RUN echo 'deb http://dl.google.com/linux/chrome/deb/ stable main' >> /etc/apt/sources.list
RUN wget https://dl-ssl.google.com/linux/linux_signing_key.pub
RUN apt-key add linux_signing_key.pub
RUN apt-get update
RUN apt-get install curl sudo google-chrome-stable  -y
RUN ln /usr/bin/google-chrome-stable /usr/bin/chrome
RUN chmod 755 /usr/bin/chrome

COPY --from=0 /usr/local/lib/web-app-framework/node_modules.tgz /usr/local/lib/web-app-framework/node_modules.tgz
COPY --from=0 /usr/local/lib/webapp/node_modules.tgz /usr/local/lib/webapp/node_modules.tgz
COPY --from=0 /usr/local/lib/venice-sdk/node_modules.tgz /usr/local/lib/venice-sdk/node_modules.tgz
COPY --from=0 /home/node/.npm-global /home/node/.npm-global
WORKDIR /import/src/github.com/pensando/sw
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/home/node/.npm-global/bin
ENV HOME=/root
ENV NPM_CONFIG_PREFIX=/root/.npm
USER root
RUN cd /usr/local/lib && npm install phantomjs-prebuilt@2.1.16
RUN chmod -R 777 /root
CMD make ui
