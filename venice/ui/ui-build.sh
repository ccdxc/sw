#!/bin/bash
node -v
cd ~/src/github.com/pensando/sw/venice/ui
cd web-app-framework 
npm install 
npm run packagr 
cd dist
npm pack .
cd ..
cd ../webapp 
npm install  ../web-app-framework/dist/web-app-framework-0.0.0.tgz 
npm install 
ng build 
cd dist
zip -r -X "../veniceui.zip" *
cd ..
mv veniceui.zip ../../
cd ~/src/github.com/pensando/sw/venice/
echo 'venice-buid is done - see sw/venice/veniceui.zip'