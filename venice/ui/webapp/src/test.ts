// This file is required by karma.conf.js and loads recursively all the .spec and framework files

import 'zone.js/dist/long-stack-trace-zone';
import 'zone.js/dist/proxy.js';
import 'zone.js/dist/sync-test';
import 'zone.js/dist/jasmine-patch';
import 'zone.js/dist/async-test';
import 'zone.js/dist/fake-async-test';
import { getTestBed } from '@angular/core/testing';
import {
  BrowserDynamicTestingModule,
  platformBrowserDynamicTesting
} from '@angular/platform-browser-dynamic/testing';
import { environment } from '@env/environment';
import 'hammerjs';

// Unfortunately there's no typing for the `__karma__` variable. Just declare it as any.
declare const __karma__: any;
declare const require: any;

jasmine.DEFAULT_TIMEOUT_INTERVAL = 120000;

// Prevent Karma from running prematurely.
__karma__.loaded = function() { };

// First, initialize the Angular testing environment.
getTestBed().initTestEnvironment(
  BrowserDynamicTestingModule,
  platformBrowserDynamicTesting()
);
// Then we find all the tests.
const context = require.context('./', true, /\.spec\.ts$/);

if (environment.disableFocusTests) {
  // Disabling focus tests
  const localGlobal = global as any;
  const fdescribe = localGlobal.fdescribe;
  const fit = localGlobal.fit;
  localGlobal.fdescribe = (description, testFn) => {
    fdescribe(description, () => {
      it('', () => {
        fail('fdescribe and fit tests are not allowed to be checked in');
      });
    });
  };
  localGlobal.fit = (description, testFn) => {
    fit(description, () => {
      fail('fdescribe and fit tests are not allowed to be checked in');
    });
  };
}

// And load the modules.
context.keys().map(context);
// Finally, start Karma to run the tests.
__karma__.start();
