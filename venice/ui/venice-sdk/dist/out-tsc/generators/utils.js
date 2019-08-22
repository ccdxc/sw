"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var fs = require("fs");
var path = require("path");
var generator = require("@pensando/swagger-ts-generator");
var Handlebars = require("handlebars");
function getSwagggerFiles() {
    // Generating models and services
    // Path to generated api files
    // Expecting format to be /api/generated/*/swagger/svc_*.swagger.json
    var configs = [];
    var generatedApi = path.join(process.cwd(), '../../../api/generated');
    var generatedApiFiles = fs.readdirSync(generatedApi);
    generatedApiFiles.forEach(function (generatedApiFile) {
        if (fs.statSync(generatedApi + '/' + generatedApiFile).isDirectory() &&
            fs.existsSync(generatedApi + '/' + generatedApiFile + '/swagger/internal') &&
            fs.statSync(generatedApi + '/' + generatedApiFile + '/swagger/internal').isDirectory()) {
            var folderName_1 = generatedApiFile;
            var swaggerFiles = fs.readdirSync(generatedApi + '/' + generatedApiFile + '/swagger/internal').filter(function (file) {
                return file.startsWith('svc_') && file.endsWith('.swagger.json');
            });
            swaggerFiles.forEach(function (swaggerFile) {
                var config;
                // Search models has special generation to add more helpful UI typing
                if (generatedApiFile === 'search') {
                    config = genConfig('v1', folderName_1, generatedApi + '/' + generatedApiFile + '/swagger/internal/' + swaggerFile, true);
                }
                else {
                    config = genConfig('v1', folderName_1, generatedApi + '/' + generatedApiFile + '/swagger/internal/' + swaggerFile);
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
exports.getSwagggerFiles = getSwagggerFiles;
function getSwagger() {
    var swaggerConfig = getSwagggerFiles();
    var options = {
        files: [],
        apiSwaggerMap: {},
        swaggerHookmap: {
            'events': eventsHook
        }
    };
    swaggerConfig.forEach(function (config) {
        options.apiSwaggerMap[config.apiGroup] = config.swagger.swaggerFile;
    });
    return options;
}
exports.getSwagger = getSwagger;
function genConfig(version, folderName, swaggerFile, isSearch) {
    if (isSearch === void 0) { isSearch = false; }
    var srcAppFolder = './' + version + '/';
    var folders = {
        srcWebapiFolder: srcAppFolder + 'models/generated/' + folderName,
        serviceFolder: srcAppFolder + 'services/generated/',
        serviceBaseImport: './abstract.service',
        controllerImport: '../../../../webapp/src/app/services/controller.service',
        modelImport: '../../models/generated/' + folderName,
    };
    var files = {
        swaggerJson: 'swagger.json',
    };
    var swagger = {
        swaggerFile: swaggerFile,
        generatorOptions: {
            isSearch: isSearch,
            // Service options
            modelImport: folders.modelImport,
            serviceFolder: folders.serviceFolder,
            serviceBaseImport: folders.serviceBaseImport,
            serviceBaseName: 'AbstractService',
            implicitParams: ['O_Tenant'],
            controllerImport: folders.controllerImport,
            // model options
            modelFolder: folders.srcWebapiFolder,
            enumTSFile: folders.srcWebapiFolder + '/enums.ts',
            generateClasses: true,
            modelModuleName: 'webapi.models',
            enumModuleName: 'webapi.enums',
            enumRef: './enums',
            namespacePrefixesToRemove: [],
            typeNameSuffixesToRemove: [],
            typesToFilter: [
                'ModelAndView',
                'View',
            ]
        }
    };
    var config = {
        files: files,
        swagger: swagger,
        apiGroup: folderName
    };
    return config;
}
function readJSON(file) {
    return JSON.parse(fs.readFileSync(file, 'utf8').trim());
}
exports.readJSON = readJSON;
/**
 * Recursively and synchronously walks from the base directory
 * and returns files for which the filter function returns true
 * @param dir base directory
 * @param filter Filter function to determine whether to keep a file
 */
function walk(dir, filter) {
    var results = [];
    var list = fs.readdirSync(dir);
    list.forEach(function (file) {
        file = dir + '/' + file;
        var stat = fs.statSync(file);
        if (stat && stat.isDirectory()) {
            /* Recurse into a subdirectory */
            results = results.concat(walk(file, filter));
        }
        else {
            /* Is a file */
            if (filter(file)) {
                results.push(file);
            }
        }
    });
    return results;
}
exports.walk = walk;
function readAndCompileTemplateFile(templateFileName) {
    var templateSource = fs.readFileSync(path.resolve(process.cwd(), "templates", templateFileName), 'utf8');
    var template = Handlebars.compile(templateSource);
    return template;
}
exports.readAndCompileTemplateFile = readAndCompileTemplateFile;
function writeTemplate(templateFile, input, outputFile) {
    var template = readAndCompileTemplateFile(templateFile);
    var result = template(input);
    ensureFile(outputFile, result);
}
exports.writeTemplate = writeTemplate;
function ensureFile(outputFileName, content) {
    generator.utils.ensureFile(outputFileName, content);
    generator.utils.log("generated " + outputFileName);
}
exports.ensureFile = ensureFile;
function regexQuote(str) {
    return str.replace(/([.?*+^$[\]\\(){}|-])/g, "\\$1");
}
exports.regexQuote = regexQuote;
function eventsHook(swagger) {
    var eventTypes = [];
    var eventPath = path.join(process.cwd(), '../../../events/generated/eventtypes/eventtypes.json');
    var events = readJSON(eventPath);
    Object.keys(events).forEach(function (cat) {
        events[cat].forEach(function (entry) {
            eventTypes.push(entry.Name);
        });
    });
    swagger.definitions.eventsEvent.properties.type.enum = eventTypes;
}
function getManifest() {
    return readJSON(path.join(process.cwd(), '../../../api/generated/apiclient/svcmanifest.json'));
}
exports.getManifest = getManifest;
