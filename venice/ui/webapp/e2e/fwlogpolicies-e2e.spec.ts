import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { FwlogPolicies } from './page-objects/fwlogpolicies.po';
import { AppPage } from './page-objects/app.po';
import { E2EuiTools } from './page-objects/E2EuiTools';
import { MonitoringFwlogPolicy, IMonitoringFwlogPolicy } from '@sdk/v1/models/generated/monitoring';


describe('venice-ui fwlogs-policies', () => {
  let fwlogsPoliciesPage: FwlogPolicies;
  let appPage: AppPage;
  let loginPage: LoginPage;

  beforeEach(async (done) => {
    appPage = new AppPage();
    fwlogsPoliciesPage = new FwlogPolicies();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await fwlogsPoliciesPage.navigateTo();
    await fwlogsPoliciesPage.verifyPage();
    done();
  });

  afterEach(async (done) => {
    try {
      await appPage.reset();
      done();
    } catch (error) {
      fail('Failed during cleanup: ' + error);
    }
  });

  const genFirewallLogPolicy = () => {
    const myMonitoringFwlogPolicy: MonitoringFwlogPolicy = new MonitoringFwlogPolicy({
      'kind': 'FwlogPolicy',
      'api-version': 'v1',
      'meta': {
        'name': 'fwlogpolicy-' + E2EuiTools.s4(),
        'tenant': 'default',
        'namespace': 'default'
      },
      'spec': {
        'format': 'SYSLOG_BSD',
        'targets': [
          {
            'destination': '10.1.1.2',
            'transport': 'tcp/8000',
            'credentials': {
              'auth-type': 'AUTHTYPE_NONE'
            }
          }
        ],
        'filter': [
          'FIREWALL_ACTION_ALL'
        ],
        'config': {
          'facility-override': 'LOG_USER',
          'prefix': 'firewall-log-policy-1'
        }
      }
    });
    return myMonitoringFwlogPolicy;
  };

  const monitoringFwlogPolicy = genFirewallLogPolicy();

  it('should have firewall-log-policy records in the table', async () => {
    appPage.getTableRowLength().then(rowLen => {
      if (rowLen > 0) {
        appPage.verifyTableHasContents((rowIdx: number, columnIdx: number, rowValues: any[]) => {
          // columnIdx is '3, the column is "action"
          if (columnIdx !== 3) {
            const celValue = rowValues[columnIdx];
            expect(celValue).not.toBe('');
          }
        });
      } else {
        console.log('There is no record found.');
        expect(rowLen).toBe(0);
      }
    });
  });

  it('should add firewall-log-policy record', async () => {
    await fwlogsPoliciesPage.createNewFwlogPolicy(monitoringFwlogPolicy);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringFwlogPolicy.meta.name, true);
    await browser.sleep(2000);
  });

  it('should delete firewall-log-policy record', async () => {
    await browser.sleep(5000); // wait for data load up.
    await fwlogsPoliciesPage.deleteFwlogPolicy(monitoringFwlogPolicy);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringFwlogPolicy.meta.name, false);
    await browser.sleep(2000);
  });


});
