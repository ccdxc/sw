import * as fs from 'fs';
import * as path from 'path'
import * as rmdir from 'rimraf';
import * as utils from './utils';
import * as generator from '@pensando/swagger-ts-generator'
import * as _ from 'lodash';

export interface Config {
  genModelFolder: string,
  genServiceFolder: string
}

export function genModelAndServices(config: Config) {

  if (fs.existsSync(config.genModelFolder)){
    // Delete all contents
    rmdir.sync(config.genModelFolder);
  }
  fs.mkdirSync(config.genModelFolder);
  
  if (fs.existsSync(config.genServiceFolder)){
    // Delete all contents
    rmdir.sync(config.genServiceFolder);
  }
  fs.mkdirSync(config.genServiceFolder);

  // Generating models and services
  const swaggerConfig = utils.getSwagggerFiles();
  swaggerConfig.forEach( swaggerConfig => {
    generator.generateTSFiles(swaggerConfig.swagger.swaggerFile, swaggerConfig.swagger.generatorOptions);
  })

  generateCatKind();
}

function generateCatKind() {
  const data = {};
  const manifest = utils.getManifest();
  Object.keys(manifest).forEach( (category) => {
    if (category === "bookstore") {
      // Skipping the bookstore exampe
      return;
    }
    const internalServices = manifest[category]["Svcs"];
    // We assume only one internal service
    const serviceData = internalServices[Object.keys(internalServices)[0]];
    const version = serviceData.Version;
    if (data[version] == null) {
      data[version] = {};
    }
    if (data[version][_.upperFirst(category)] == null) {
      data[version][_.upperFirst(category)] = {};
    }
    const catObj = data[version][_.upperFirst(category)];
    serviceData.Messages.forEach( (kind) => {
      if (kind === "TroubleshootingSession") {
        return;
      }
      catObj[kind] = {
        // File will be placed in {Version}/models/generated/
        "importPath": "./" + category,
        "importName": _.upperFirst(category) + kind,
        scopes: serviceData.Properties[kind].Scopes,
        actions: serviceData.Properties[kind].Actions
      }
    });
    if (category === "monitoring") {
      catObj["Event"] = {
        "importPath": "./events",
        "importName": "EventsEvent",
      }
      catObj["AuditEvent"] = {
        "importPath": "./audit",
        "importName": "AuditAuditEvent",
      }
    }
  });

  const outputFile = 'v1/models/generated/category-mapping.model.ts';
  utils.writeTemplate('generate-cat-kind-ts.hbs', data['v1'], outputFile);
  
}