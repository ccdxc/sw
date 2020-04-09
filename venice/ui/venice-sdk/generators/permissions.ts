import * as path from 'path';
import * as fs from 'fs';
import { writeTemplate } from './utils';
import * as _ from 'lodash';

export function generateUIPermissionsFile(manifestData) {
  const delimiter = '_';

  // Get swagger config function
  // Get manifest data

  const authPath = path.join(process.cwd(), '../../../api/generated/auth/swagger/internal/svc_auth.swagger.json');
  const authSwagger = JSON.parse(fs.readFileSync(authPath, 'utf8').trim());
  const permActions = authSwagger.definitions.authPermission.properties.actions.enum
  const permEnum = [];
  Object.keys(manifestData).forEach( (category) => {
    if (category === "bookstore") {
      // Skipping the bookstore exampe
      return;
    }
    const internalServices = manifestData[category]["Svcs"];
    // We assume only one internal service
    const serviceData = internalServices[Object.keys(internalServices)[0]];

    serviceData.Messages.forEach( (kind) => {
      permActions.forEach( (action) => {
        permEnum.push( _.toLower(category) + _.toLower(kind) + delimiter + _.toLower(action));
      });
    });
  });
  const addCustomKind = (kind) => {
    permActions.forEach( (action) => {
      permEnum.push( _.toLower(kind) + delimiter + _.toLower(action));
    });
  }
  permEnum.push('auditevent' + delimiter + 'read');
  permEnum.push('eventsevent' + delimiter + 'read');
  permEnum.push('fwlog' + delimiter + 'read');

  permEnum.push('metrics' + delimiter + 'read');
  permEnum.push('adminrole');

  const outputFile = 'v1' + '/models/generated/UI-permissions-enum.ts';
  writeTemplate('generate-permissions-enum-ts.hbs', permEnum, outputFile)
}
