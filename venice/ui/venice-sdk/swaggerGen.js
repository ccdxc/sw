var fs = require( 'fs' );
var path = require( 'path' );
var rmdir = require('rimraf');

var swaggerTSGenerator = require('@pensando/swagger-ts-generator/');

// Expects to run from root of venice-sdk

// Generate folders for generated files, and remove any files if there are any
const version = 'v1';
const genModelFolder = path.join(__dirname, 'v1/', 'models/generated/');
const genServiceFolder = path.join(__dirname, 'v1/', 'services/generated/');

if (fs.existsSync(genModelFolder)){
  // Delete all contents
  rmdir.sync(genModelFolder);
}
fs.mkdirSync(genModelFolder);

if (fs.existsSync(genServiceFolder)){
  // Delete all contents
  rmdir.sync(genServiceFolder);
}
fs.mkdirSync(genServiceFolder);

// Generating models and services
// Path to generated api files
// Expecting format to be /api/generated/*/swagger/svc_*.swagger.json
var generatedApi = path.join(__dirname, '../../../api/generated');
var generatedApiFiles = fs.readdirSync(generatedApi);
generatedApiFiles.forEach( (generatedApiFile) => {
  if (fs.statSync(generatedApi+'/'+generatedApiFile).isDirectory() && 
      fs.existsSync(generatedApi+'/'+generatedApiFile+'/swagger') &&
      fs.statSync(generatedApi+'/'+generatedApiFile+'/swagger').isDirectory()) {
    folderName = generatedApiFile;
    swaggerFiles = fs.readdirSync(generatedApi+'/'+generatedApiFile+'/swagger').filter(function (file) {
      return file.startsWith('svc_') && file.endsWith('.swagger.json');
    });
    swaggerFiles.forEach( (swaggerFile) => {
      let config;
      // Search models has special generation to add more helpful UI typing
      if (generatedApiFile === 'search') {
        config = genConfig(version, folderName, generatedApi+'/'+generatedApiFile+'/swagger/'+ swaggerFile, true);
      } else {
        config = genConfig(version, folderName, generatedApi+'/'+generatedApiFile+'/swagger/'+ swaggerFile);
      }
      swaggerTSGenerator.generateTSFiles(config.swagger.swaggerFile, config.swagger.swaggerTSGeneratorOptions);
    })
  }
});

function genConfig(version, folderName, swaggerFile, isSearch = false) {
  var srcAppFolder = './' + version + '/';
  var folders = {
      srcWebapiFolder: srcAppFolder + 'models/generated/' + folderName,
      serviceFolder: srcAppFolder + 'services/generated/',
      serviceBaseImport: './abstract.service',
      controllerImport: '../../../../webapp/src/app/services/controller.service',
      modelImport: '../../models/generated/' + folderName,
  }
  var files = {
      swaggerJson: 'swagger.json',
  }

  var swagger = {
      swaggerFile: swaggerFile,
      swaggerTSGeneratorOptions: {
          isSearch: isSearch,
          // Service options
          modelImport: folders.modelImport,
          serviceFolder: folders.serviceFolder,
          serviceBaseImport: folders.serviceBaseImport,
          serviceBaseName:  'AbstractService',
          implicitParams: ['O_Tenant'],
          controllerImport: folders.controllerImport,
          // model options
          modelFolder: folders.srcWebapiFolder,
          enumTSFile: folders.srcWebapiFolder + '/enums.ts',
          generateClasses: true,
          modelModuleName: 'webapi.models',
          enumModuleName: 'webapi.enums',
          enumRef: './enums',
          namespacePrefixesToRemove: [
          ],
          typeNameSuffixesToRemove: [
          ],
          typesToFilter: [
              'ModelAndView', // Springfox artifact
              'View', // Springfox artifact
          ]
      }
  }

  var config = {
      files: files,
      swagger: swagger,
  }
  return config;
}

