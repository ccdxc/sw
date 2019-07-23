import * as fs from 'fs';
import * as path from 'path'
import * as generator from '@pensando/swagger-ts-generator'
import * as Handlebars from 'handlebars'

export function getSwagggerFiles() {
  // Generating models and services
  // Path to generated api files
  // Expecting format to be /api/generated/*/swagger/svc_*.swagger.json
  let configs = [];
  const generatedApi = path.join(process.cwd(), '../../../api/generated');
  const generatedApiFiles = fs.readdirSync(generatedApi);
  generatedApiFiles.forEach( (generatedApiFile) => {
    if (fs.statSync(generatedApi+'/'+generatedApiFile).isDirectory() && 
        fs.existsSync(generatedApi+'/'+generatedApiFile+'/swagger') &&
        fs.statSync(generatedApi+'/'+generatedApiFile+'/swagger').isDirectory()) {
      const folderName = generatedApiFile;
      const swaggerFiles = fs.readdirSync(generatedApi+'/'+generatedApiFile+'/swagger').filter(function (file) {
        return file.startsWith('svc_') && file.endsWith('.swagger.json');
      });
      swaggerFiles.forEach( (swaggerFile) => {
        let config;
        // Search models has special generation to add more helpful UI typing
        if (generatedApiFile === 'search') {
          config = genConfig('v1', folderName, generatedApi+'/'+generatedApiFile+'/swagger/'+ swaggerFile, true);
        } else {
          config = genConfig('v1', folderName, generatedApi+'/'+generatedApiFile+'/swagger/'+ swaggerFile);
        }
        if (generatedApiFile === 'events') {
          config.swagger.generatorOptions.swaggerFileHook = eventsHook;
        }
        configs = configs.concat(config);
      });
    }
  });
  return configs;
}

export function getSwagger() {
  const swaggerConfig = getSwagggerFiles();
  const options = {
    files: [],
    apiSwaggerMap: {},
    swaggerHookmap: {
      'events': eventsHook
    }   
  }
  swaggerConfig.forEach( config => {
    options.apiSwaggerMap[config.apiGroup] = config.swagger.swaggerFile;
  });
  return options;
}

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
      generatorOptions: {
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
      apiGroup: folderName
  }
  return config;
}

// Config object should hold all swagger files to process
// list of configs.

export interface SwaggerConfigs {
  fileConfigs: SwaggerConfig[],
}

interface SwaggerConfig {
  swaggerFile: string,

}

export function readJSON(file) {
  return JSON.parse(fs.readFileSync(file, 'utf8').trim());
}


/**
 * Recursively and synchronously walks from the base directory
 * and returns files for which the filter function returns true
 * @param dir base directory
 * @param filter Filter function to determine whether to keep a file
 */
export function walk(dir, filter) {
  var results = [];
  var list = fs.readdirSync(dir);
  list.forEach(function(file) {
      file = dir + '/' + file;
      var stat = fs.statSync(file);
      if (stat && stat.isDirectory()) { 
          /* Recurse into a subdirectory */
          results = results.concat(walk(file, filter));
      } else { 
          /* Is a file */
          if (filter(file)) {
            results.push(file);
          }
      }
  });
  return results;
}

export function readAndCompileTemplateFile(templateFileName) {
  let templateSource = fs.readFileSync(path.resolve(process.cwd(), "templates", templateFileName), 'utf8');
  let template = Handlebars.compile(templateSource);
  return template;
}

export function writeTemplate(templateFile, input, outputFile) {
  const template = readAndCompileTemplateFile(templateFile);
  const result = template(input);
  ensureFile(outputFile, result);

}

export function ensureFile(outputFileName, content) {
  generator.utils.ensureFile(outputFileName, content);
  generator.utils.log(`generated ${outputFileName}`);
}

function eventsHook(swagger) {
  const eventTypes = [];
  const eventPath = path.join(process.cwd(), '../../../events/generated/eventtypes/eventtypes.json');
  const events = readJSON(eventPath);
  Object.keys(events).forEach((cat) => {
    events[cat].forEach( (entry) => {
      eventTypes.push(entry.Name);
    });
  });
  swagger.definitions.eventsEvent.properties.type.enum =  eventTypes;
}


export function getManifest() {
  return readJSON(path.join(process.cwd(), '../../../api/generated/apiclient/svcmanifest.json'))
}