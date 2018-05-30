// Protractor configuration file, see link for more information
// https://github.com/angular/protractor/blob/master/lib/config.ts

const { SpecReporter } = require('jasmine-spec-reporter');

const E2E_BASE_URL = process.env['E2E_BASE_URL'] || 'http://192.168.69.189:10001/';
exports.config = {
  allScriptsTimeout: 11000,
  specs: [
    './e2e/**/*.spec.ts'
  ],
  capabilities: {
    browserName: 'chrome',

    chromeOptions: {
      args: ["--headless", "--disable-gpu", "--window-size=800,600"]
    }
  },
  directConnect: true,
  baseUrl: E2E_BASE_URL,
  framework: 'jasmine',
  jasmineNodeOpts: {
    showColors: true,
    defaultTimeoutInterval: 30000,
    print: function() { }
  },
  onPrepare() {
    require('ts-node').register({
      project: 'e2e/tsconfig.e2e.json'
    });
    require('child_process').exec(
      'echo "Creating Auth policy and user" && curl -d "@e2e/data/auth-policy.json" -X POST '
      + E2E_BASE_URL + '/v1/auth/authn-policy && curl -d "@e2e/data/create-user.json" -X POST '
      + E2E_BASE_URL + '/v1/auth/default/users',
      (error, stdout, stderr) => {
        console.log(`${stdout}`);
        console.log(`${stderr}`);
        if (error !== null) {
          console.log(`exec error: ${error}`);
        }
      });
    jasmine.getEnv().addReporter(new SpecReporter({ spec: { displayStacktrace: true } }));
  }
};
