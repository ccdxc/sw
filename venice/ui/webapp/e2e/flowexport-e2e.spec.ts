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
                'aabb.ccdd.eeff'
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
                'ffee.ddcc.bbaa'
              ]
            },
            'app-protocol-selectors': {
              'proto-ports': [
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
            'transport': 'UDP/90',
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
    await browser.sleep(5000);
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringFlowExportPolicy.meta.name, true);
    await browser.sleep(5000);

  });

  it('should delete flow export record', async () => {
    await browser.sleep(5000); // wait for data load up.
    await flowExportPage.deletelowExportPolicy(monitoringFlowExportPolicy);
    await browser.sleep(5000);
    await E2EuiTools.verifyRecordAddRemoveInTable(monitoringFlowExportPolicy.meta.name, false);
    await browser.sleep(5000);
  });

});
