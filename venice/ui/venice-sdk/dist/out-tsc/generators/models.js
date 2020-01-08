"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var fs = require("fs");
var rmdir = require("rimraf");
var utils = require("./utils");
var generator = require("@pensando/swagger-ts-generator");
var _ = require("lodash");
function genModelAndServices(config) {
    if (fs.existsSync(config.genModelFolder)) {
        // Delete all contents
        rmdir.sync(config.genModelFolder);
    }
    fs.mkdirSync(config.genModelFolder);
    if (fs.existsSync(config.genServiceFolder)) {
        // Delete all contents
        rmdir.sync(config.genServiceFolder);
    }
    fs.mkdirSync(config.genServiceFolder);
    // Generating models and services
    var swaggerConfig = utils.getSwagggerFiles();
    swaggerConfig.forEach(function (swaggerConfig) {
        generator.generateTSFiles(swaggerConfig.swagger.swaggerFile, swaggerConfig.swagger.generatorOptions);
    });
    generateCatKind();
}
exports.genModelAndServices = genModelAndServices;
function generateCatKind() {
    var data = {};
    var manifest = utils.getManifest();
    Object.keys(manifest).forEach(function (category) {
        if (category === "bookstore") {
            // Skipping the bookstore exampe
            return;
        }
        var internalServices = manifest[category]["Svcs"];
        // We assume only one internal service
        var serviceData = internalServices[Object.keys(internalServices)[0]];
        var version = serviceData.Version;
        if (data[version] == null) {
            data[version] = {};
        }
        if (data[version][_.upperFirst(category)] == null) {
            data[version][_.upperFirst(category)] = {};
        }
        var catObj = data[version][_.upperFirst(category)];
        serviceData.Messages.forEach(function (kind) {
            if (kind === "TroubleshootingSession") {
                return;
            }
            catObj[kind] = {
                // File will be placed in {Version}/models/generated/
                "importPath": "./" + category,
                "importName": _.upperFirst(category) + kind,
                scopes: serviceData.Properties[kind].Scopes,
                actions: serviceData.Properties[kind].Actions
            };
        });
        if (category === "monitoring") {
            catObj["Event"] = {
                "importPath": "./events",
                "importName": "EventsEvent",
            };
            catObj["AuditEvent"] = {
                "importPath": "./audit",
                "importName": "AuditAuditEvent",
            };
        }
    });
    var outputFile = 'v1/models/generated/category-mapping.model.ts';
    utils.writeTemplate('generate-cat-kind-ts.hbs', data['v1'], outputFile);
}
