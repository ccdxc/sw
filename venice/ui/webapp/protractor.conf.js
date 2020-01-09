// Protractor configuration file, see link for more information
// https://github.com/angular/protractor/blob/master/lib/config.ts

const { SpecReporter } = require('jasmine-spec-reporter');
const tsConfig = require("./e2e/tsconfig.e2e.json");

const E2E_BASE_URL = process.env['E2E_BASE_URL'] || 'https://localhost:4200/';
const DISABLE_FOCUS_TESTS = process.env['DISABLE_FOCUS_TESTS'] || 'false';
exports.config = {
  allScriptsTimeout: 22000,
  specs: [
    './e2e/**/*.spec.ts'
  ],
  suites: {
    // how to run:
    // ng e2e  --base-url="https://server:port" --suite=suitename (suitename listed below)
    // e.g  ng e2e  --base-url="https://localhost:4200" --suite=login 

    all: './e2e/**/*e2e.spec.ts',
    alertsevents: 'e2e/alertsevents-e2e.spec.ts',
    auditevents: 'e2e/auditevents-e2e.spec.ts',
    // purposelly comment it out. Test it manually.->  authpolicy: 'e2e/authpolicy-e2e.spec.ts',
    cluster: 'e2e/cluster-e2e.spec.ts',
    flowexport: 'e2e/flowexport-e2e.spec.ts',
    fwlogpolicies: 'e2e/fwlogpolicies-e2e.spec.ts',
    fwlogs: 'e2e/fwlogs-e2e.spec.ts',
    login: 'e2e/login-e2e.spec.ts',
    naples: 'e2e/naples-e2e.spec.ts',
    routing: 'e2e/routing-e2e.spec.ts',
    search: 'e2e/search-e2e.spec.ts',
    securityapps: 'e2e/securityapps-e2e.spec.ts',
    techsupport: 'e2e/techsupport-e2e.spec.ts',
    users: 'e2e/users-e2e.spec.ts',
  },
  capabilities: {
    browserName: 'chrome',
    unexpectedAlertBehaviour: 'accept',
    //binary: process.env.CHROME_BIN,
    chromeOptions: {
        args: ["--headless",  "--no-sandbox", "--disable-gpu", "--window-size=1200,900"]  
      // The following allowes watching the e2e test run and is helpful for debugging
      //  args: ["--window-size=1400,1000",]
    },
    //  loggingPrefs: {driver: 'ALL', server: 'ALL', browser: 'ALL'}
  },
  // Disabling so that we can use native async/await
  // Using async/await causes control flow to become unstable
  // See https://www.protractortest.org/#/async-await
  SELENIUM_PROMISE_MANAGER: false,
  directConnect: true,
  baseUrl: E2E_BASE_URL,
  framework: 'jasmine',
  jasmineNodeOpts: {
    showColors: true,
    defaultTimeoutInterval: 120000,
    print: function () { }
  },
  onPrepare() {
    require('ts-node').register({
      project: 'e2e/tsconfig.e2e.json'
    });
    require("tsconfig-paths").register({
      project: 'e2e/tsconfig.e2e.json',
      baseUrl: 'e2e/',
      paths: tsConfig.compilerOptions.paths
    });
    jasmine.getEnv().addReporter(new SpecReporter({ spec: { displayStacktrace: true } }));
    if (DISABLE_FOCUS_TESTS === 'true') {
      // Disabling focus tests
      const localGlobal = global;
      const fdescribe = localGlobal.fdescribe;
      const fit = localGlobal.fit;
      localGlobal.fdescribe = (description, testFn) => {
        fdescribe(description, () => {
          it('', () => {
            fail("fdescribe and fit tests are not allowed to be checked in");
          })
        })
      };
      localGlobal.fit = (description, testFn) => {
        fit(description, () => {
          fail("fdescribe and fit tests are not allowed to be checked in");
        })
      }
    }
    // When promises fail, Node only prints them to console.
    // This forces unhandled failed promises to throw an error
    process.on('unhandledRejection', error => { throw error; });
    let currentCommand = Promise.resolve();
    // Serialise all webdriver commands to prevent EPIPE errors
    const webdriverSchedule = browser.driver.schedule;
    browser.driver.schedule = (command, description) => {
      currentCommand = currentCommand.then(() =>
        webdriverSchedule.call(browser.driver, command, description)
      );
      return currentCommand;
    }
  }
};
