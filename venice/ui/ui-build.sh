#!/bin/bash
node -v
cd ~/src/github.com/pensando/sw/venice/ui
cd web-app-framework 
npm install --prefer-offline
npm run pack
cd ../webapp 
npm install  ../web-app-framework/dist/web-app-framework-0.0.0.tgz --prefer-offline
npm install --prefer-offline
ng build 
cd dist
zip -r -X "../veniceui.zip" *
cd ..
mv veniceui.zip ../../
cd ~/src/github.com/pensando/sw/venice/
echo 'venice-buid is done - see sw/venice/veniceui.zip'
