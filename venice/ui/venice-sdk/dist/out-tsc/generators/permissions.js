"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var path = require("path");
var fs = require("fs");
var utils_1 = require("./utils");
var _ = require("lodash");
function generateUIPermissionsFile(manifestData) {
    var delimiter = '_';
    // Get swagger config function
    // Get manifest data
    var authPath = path.join(process.cwd(), '../../../api/generated/auth/swagger/internal/svc_auth.swagger.json');
    var authSwagger = JSON.parse(fs.readFileSync(authPath, 'utf8').trim());
    var permActions = authSwagger.definitions.authPermission.properties.actions.enum;
    var permEnum = [];
    Object.keys(manifestData).forEach(function (category) {
        if (category === "bookstore") {
            // Skipping the bookstore exampe
            return;
        }
        var internalServices = manifestData[category]["Svcs"];
        // We assume only one internal service
        var serviceData = internalServices[Object.keys(internalServices)[0]];
        serviceData.Messages.forEach(function (kind) {
            permActions.forEach(function (action) {
                permEnum.push(_.toLower(category) + _.toLower(kind) + delimiter + _.toLower(action));
            });
        });
    });
    var addCustomKind = function (kind) {
        permActions.forEach(function (action) {
            permEnum.push(_.toLower(kind) + delimiter + _.toLower(action));
        });
    };
    permEnum.push('auditevent' + delimiter + 'read');
    permEnum.push('eventsevent' + delimiter + 'read');
    permEnum.push('fwlog' + delimiter + 'read');
    permEnum.push('metrics' + delimiter + 'read');
    permEnum.push('adminrole');
    var outputFile = 'v1' + '/models/generated/UI-permissions-enum.ts';
    utils_1.writeTemplate('generate-permissions-enum-ts.hbs', permEnum, outputFile);
}
exports.generateUIPermissionsFile = generateUIPermissionsFile;
