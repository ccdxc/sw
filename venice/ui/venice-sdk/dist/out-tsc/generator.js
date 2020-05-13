"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var path = require("path");
var metrics_1 = require("./generators/metrics");
var helpdocs_1 = require("./generators/helpdocs");
var events_1 = require("./generators/events");
var permissions_1 = require("./generators/permissions");
var utils_1 = require("./generators/utils");
var handlebars = require("handlebars");
var models_1 = require("./generators/models");
// Expects to run from root of venice-sdk
// Generate folders for generated files, and remove any files if there are any
var version = 'v1';
// const genModelFolder = 
// const genServiceFolder =
handlebars.registerHelper('toJSON', function (obj) {
    return JSON.stringify(obj, null, 2);
});
handlebars.registerHelper('ifeq', function (a, b, options) {
    if (a == b) {
        return options.fn(this);
    }
    return options.inverse(this);
});
handlebars.registerHelper('ifnoteq', function (a, b, options) {
    if (a != b) {
        return options.fn(this);
    }
    return options.inverse(this);
});
// gen models
var modelConfig = {
    genModelFolder: path.join(process.cwd(), 'v1/', 'models/generated/'),
    genServiceFolder: path.join(process.cwd(), 'v1/', 'services/generated/'),
};
models_1.genModelAndServices(modelConfig);
// Event types
var eventPath = path.join(process.cwd(), '../../../events/generated/eventtypes/eventtypes.json');
var outputFile = version + '/models/generated/eventtypes.ts';
events_1.generateEventTypesFile(eventPath, outputFile);
// permissions
var manifest = utils_1.getManifest();
permissions_1.generateUIPermissionsFile(manifest);
// Help docs
var helpConfig = {
    helpDir: "../docs/help",
    tempDir: "./tmp",
    htmlDir: "../webapp/src/assets/generated/docs/help/",
    htmlLinkBase: "/assets/generated/docs/help"
};
helpdocs_1.generateHelpDocs(helpConfig);
// Metrics doc
var metricsOutput = path.join(process.cwd(), 'metrics/', 'generated/');
var pipelines = [
    {
        pipeline: "Iris",
        basePath: '../../../metrics/generated/',
    },
    {
        pipeline: "Apulu",
        basePath: '../../../metrics/apulu/generated',
    }
];
metrics_1.generateMetricMetadata(pipelines, metricsOutput);
