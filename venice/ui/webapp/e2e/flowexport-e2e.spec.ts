import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { FlowExport } from './page-objects/flowexport.po';
import { AppPage } from './page-objects/app.po';
import { E2EuiTools } from './page-objects/E2EuiTools';
import { IMonitoringFlowExportPolicy, MonitoringFlowExportPolicy } from '@sdk/v1/models/generated/monitoring';


describe('venice-ui flowexport', () => {
  let flowExportPage: FlowExport;
  let appPage: AppPage;
  let loginPage: LoginPage;


  beforeEach(async (done) => {
    appPage = new AppPage();
    flowExportPage = new FlowExport();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await flowExportPage.navigateTo();
    await flowExportPage.verifyPage();
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

  const genFlowExport = () => {
    const myMonitoringFlowExportPolicy: MonitoringFlowExportPolicy = new MonitoringFlowExportPolicy({
      'api-version': 'v1',
      'meta': {
        'name': 'flowExport-' + E2EuiTools.s4(),
        'tenant': 'default',
        'namespace': 'default'
      },
      'spec': {
        'interval': '10s',
        'format': 'Ipfix',
        'match-rules': [
          {
            'source': {
              'endpoints': [
                '10.1.1.1'
              ],
              'ip-addresses': [
                '10.1.1.1'
              ],
              'mac-addresses': [
                'aa:bb:cc:dd:ee:ff'
              ]
            },
            'destination': {
              'endpoints': [
                '10.2.2.2'
              ],
              'ip-addresses': [
                '10.2.2.2'
              ],
              'mac-addresses': [
                'ff:ee:dd:cc:bb:aa'
              ]
            },
            'app-protocol-selectors': {
              'ports': [
                'TCP/9000'
              ],
              'applications': [
                'TCP'
              ]
            }
          }
        ],
        'exports': [
          {
            'destination': '10.3.3.3',
            'transport': 'TCP/90',
            'credentials': {
              'auth-type': 'AUTHTYPE_NONE'
            }
          }
        ]
      }
    });
    return myMonitoringFlowExportPolicy;
  };

  const monitoringFlowExportPolicy = genFlowExport();

  it('should add flow export record', async () => {
    await flowExportPage.createFlowExportPolicy(monitoringFlowExportPolicy);
    let recordCreated = false;
    await browser.sleep(5000); // wait for web-socket to refresh data.
    // loop through all records to find a new tech-support record whose name matches expected value
    await appPage.verifyTableHasContents(
      (rowIdx: number, columnIdx: number, rowValues: any[]) => {
        if (columnIdx === 0) {
          const celValue = rowValues[columnIdx];
          if (celValue === monitoringFlowExportPolicy.meta.name) {
            recordCreated = true;
          }
        }
      },
      () => {
        expect(recordCreated).toBeTruthy(monitoringFlowExportPolicy.meta.name + ' created');
      }
    );

  });

  it('should delete flow export record', async () => {
    await browser.sleep(5000); // wait for data load up.
    await flowExportPage.deletelowExportPolicy(monitoringFlowExportPolicy);
    await browser.sleep(5000);
    let recordDeleted = true;
    // loop through all records to find a new tech-support record whose name matches expected value
    await appPage.verifyTableHasContents(
      // add onCell and onComplete functions as parameters.
      (rowIdx: number, columnIdx: number, rowValues: any[]) => {
        if (columnIdx === 0) {
          const celValue = rowValues[columnIdx];
          if (celValue === monitoringFlowExportPolicy.meta.name) {
            recordDeleted = false;
          }
        }
      },
      () => {
        expect(recordDeleted).toBeTruthy(monitoringFlowExportPolicy.meta.name + ' deleted');
      }
    );
  });

});
