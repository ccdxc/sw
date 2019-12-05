"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var fs = require("fs");
var rmdir = require("rimraf");
var utils_1 = require("./utils");
var path = require("path");
var generator = require("@pensando/swagger-ts-generator");
var showdown = require("showdown");
function generateHelpDocs(config) {
    if (fs.existsSync(config.tempDir)) {
        // Delete all contents
        rmdir.sync(config.tempDir);
    }
    if (fs.existsSync(config.htmlDir)) {
        // Delete all contents
        rmdir.sync(config.htmlDir);
    }
    fs.mkdirSync(config.htmlDir, { recursive: true });
    // generate swagger map
    var options = utils_1.getSwagger();
    // Recursively search all files for md
    var results = utils_1.walk(config.helpDir, function (file) { return file.endsWith('.md') && !file.includes("README"); });
    results = results.map(function (x) {
        return path.relative(path.join(process.cwd(), config.helpDir), x);
    });
    // Generate doc options
    results.forEach(function (file) {
        options.files.push({
            input: path.join(config.helpDir, file),
            output: path.join(config.tempDir, file),
            filePath: file,
        });
    });
    generator.generateDocs(options);
    // Generate link ref
    var converter = new showdown.Converter({ tables: true, metadata: true });
    var refMap = {};
    options.files.forEach(function (entry) {
        var md = fs.readFileSync(entry.output, 'utf8');
        converter.makeHtml(md);
        var metadata = converter.getMetadata();
        var id = metadata.id;
        if (id == null) {
            console.error("id was null for file " + entry.output);
        }
        id = metadata.id.replace(/\s/g, '');
        id = id.replace(/\r?\n|\r/g, '');
        refMap[id] = path.join(config.htmlLinkBase, entry.filePath.split('.').slice(0, -1).join('.')) + '.html';
    });
    // Write link manifest file
    var outputFile = path.join(config.htmlDir, 'linkMap.ts');
    utils_1.writeTemplate('generate-help-link-ts.hbs', refMap, outputFile);
    // Replace reference links in md files
    var linkReplacer = {
        type: 'lang',
        filter: function (text, converter, options) {
            Object.keys(refMap).forEach(function (key) {
                text = text.replace(new RegExp(utils_1.regexQuote('(%' + key + ')'), 'g'), '(' + refMap[key] + ')');
            });
            text = text.replace('/images/', path.join(config.htmlLinkBase, '/images/'));
            return text;
        }
    };
    converter.addExtension(linkReplacer, 'LinkReplacer');
    options.files.forEach(function (entry) {
        var md = fs.readFileSync(entry.output, 'utf8');
        var html = converter.makeHtml(md);
        var output = path.join(config.htmlDir, entry.filePath.split('.').slice(0, -1).join('.')) + '.html';
        utils_1.ensureFile(output, html);
    });
    // Copy all other files over to webapp
    results = utils_1.walk(config.helpDir, function (file) { return !file.endsWith('.md'); });
    results = results.map(function (x) {
        return path.relative(path.join(process.cwd(), config.helpDir), x);
    });
    results.forEach(function (file) {
        var output = path.join(config.htmlDir, file);
        fs.mkdirSync(path.dirname(output), { recursive: true });
        fs.copyFileSync(path.join(config.helpDir, file), output);
    });
    // clean up tmp
    if (fs.existsSync(config.tempDir)) {
        // Delete all contents
        rmdir(config.tempDir, { maxBusyTries: 20 }, function () { });
    }
}
exports.generateHelpDocs = generateHelpDocs;
