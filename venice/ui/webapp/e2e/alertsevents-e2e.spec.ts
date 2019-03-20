import { browser, by, element, protractor } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { E2EuiTools } from './page-objects/E2EuiTools';
import { IMonitoringAlertDestination, IMonitoringAlertPolicy, IApiStatus, MonitoringAlertPolicy, MonitoringAlertDestination } from '@sdk/v1/models/generated/monitoring';

import { Alertsevents } from './page-objects/alertsevents.po';
import { AppPage } from './page-objects/app.po';

describe('venice-ui alertsevents', () => {
  let alertseventsPage: Alertsevents;
  let appPage: AppPage;
  let loginPage: LoginPage;

  const genAlertPolicy = () => {
    const alertPolicy = new MonitoringAlertPolicy({
      'kind': 'AlertPolicy',
      'api-version': 'v1',
      'meta': {
        'name': 'alertpolicy-' + E2EuiTools.s4(),
        'tenant': 'default',
        'namespace': 'default'
      },
      'spec': {
        'resource': 'Event',
        'severity': 'Critical', // must match the event.severity dropdown option text
        'message': '',
        'requirements': [
          {
            'key': 'severity',  // severity will bring up multi-select in field-selector.value part.
            'operator': 'equals',
            'values': [
              'Critical'  // has to match multi-select option text
            ]
          }
        ],
        'persistence-duration': '',
        'clear-duration': '',
        'enable': true,
        'auto-resolve': false,
        'destinations': [
          'dest1'
        ]
      }
    }

    );
    return alertPolicy;
  };

  const genDestination = () => {
    const destination = new MonitoringAlertDestination({
      'meta': {
        'name': 'alertDestination-' + E2EuiTools.s4(),
        'tenant': 'default',
        'namespace': 'default'
      },
      'spec': {
        'selector': {
          'requirements': [
            {
              'key': 'status.severity',
              'operator': 'in',
              'values': [
                'CRITICAL'
              ]
            }
          ]
        },
        'email-export': {
          'email-list': null
        },
        'snmp-export': {
          'snmp-trap-servers': null
        },
        'syslog-export': {
          'format': 'SYSLOG_BSD',
          'targets': [
            {
              'destination': '10.1.1.1',
              'transport': 'TCP/9000',
              'credentials': {
                'auth-type': 'AUTHTYPE_NONE'
              }
            }
          ],
          'config': {
            'facility-override': 'LOG_USER'
          }
        }
      },
      'kind': 'AlertDestination'
    });
    return destination;
  };

  const monitoringAlertDestination = genDestination();
  const monitoringAlertPolicy = genAlertPolicy();

  beforeEach(async (done) => {
    appPage = new AppPage();
    alertseventsPage = new Alertsevents();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await alertseventsPage.navigateTo('alertsevents');
    await alertseventsPage.verifyPage('alertsevents');
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

  it('should have Alert & Events records in the table', async () => {
    await alertseventsPage.verifyTableHasContents();
  });

  it('should add Event-Alert Policy record', async () => {
    await alertseventsPage.createEventAlertPolicy(monitoringAlertPolicy);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringAlertPolicy.meta.name, true);
    await browser.sleep(5000);
  });

  it('should update Event-Alert Policy record', async () => {
    await alertseventsPage.updateventAlertPolicy(monitoringAlertPolicy);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    const severityValueCSS = 'app-eventalertpolicies  app-neweventalertpolicy .neweventalertpolicy-container p-dropdown[formControlName="severity"] label';
   const  targetValue =  'Critical';
    await alertseventsPage.verifyEventAlertPolicy(monitoringAlertPolicy, severityValueCSS, targetValue);
    await browser.sleep(5000);
  });

  it('should delete Event-Alert Policy record', async () => {
    await alertseventsPage.deleteEventAlertPolicy(monitoringAlertPolicy);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringAlertPolicy.meta.name, false);
    await browser.sleep(5000);
  });

  /// -------------- ///

  it('should add Alert Destination record', async () => {
    await alertseventsPage.createAlertDestination(monitoringAlertDestination);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringAlertDestination.meta.name, true);
    await browser.sleep(5000);

  });

  it('should update Alert Destination record', async () => {
    // for debug: monitoringAlertDestination.meta.name = 'alertDestination-a1e6';
    const newTransportValue = 'TCP/8000';
    const elementCSS = 'app-newdestination .syslog-input.ui-inputtext[formcontrolname="transport"]' ;
    monitoringAlertDestination.spec['syslog-export'].targets[0].transport = newTransportValue;
    await alertseventsPage.updateAlertDestination(monitoringAlertDestination, elementCSS, newTransportValue);
    await browser.sleep(2000); // wait for web-socket to refresh data.

    await alertseventsPage.verifyAlertDestination(monitoringAlertDestination, elementCSS, newTransportValue);
    await browser.sleep(5000);

  });

  it('should delete Alert Destination record', async () => {
    await alertseventsPage.deleteAlertDestination(monitoringAlertDestination);
    await browser.sleep(2000); // wait for web-socket to refresh data.
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringAlertDestination.meta.name, false);
    await browser.sleep(5000);

  });

});
