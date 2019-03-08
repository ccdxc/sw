import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { FirewallLogs } from './page-objects/firewallogs.po';
import { AppPage } from './page-objects/app.po';
import { E2EuiTools } from './page-objects/E2EuiTools';

describe('venice-ui fwlogs', () => {
  let firewallLogsPage: FirewallLogs;
  let appPage: AppPage;
  let loginPage: LoginPage;
  let sourcePorts = '8000';
  let destPorts = '9000';

  beforeEach(async (done) => {
    appPage = new AppPage();
    firewallLogsPage = new FirewallLogs();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await firewallLogsPage.navigateTo();
    await firewallLogsPage.verifyPage();
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


  /**
   * This test loops throught all fwlog records and randomly picks a row and get the row srcPort and dstPort
   */
  it('should have firewall log records in the table', async () => {
    appPage.getTableRowLength().then(rowLen => {
      if (rowLen > 0) {
        appPage.verifyTableHasContents((rowIdx: number, columnIdx: number, rowValues: any[]) => {
          const randomIndex = Math.floor(Math.random() * (10)); // randomly pick a row between 0-10
          if (rowIdx === randomIndex) {
            if (columnIdx === 3) { // sourcePort column
              const celValueSrcPorts = rowValues[columnIdx];
              if (celValueSrcPorts !== null && celValueSrcPorts.trim().length > 0) {
                sourcePorts = celValueSrcPorts;
              }
            }
            if (columnIdx === 4) { // destinationPort column
              const celValueDstPorts = rowValues[columnIdx];
              if (celValueDstPorts !== null && celValueDstPorts.trim().length > 0) {
                destPorts = celValueDstPorts;
              }
            }
          }
        });
      } else {
        expect(rowLen).toBe(0);
        sourcePorts = '';
        destPorts = '';
      }
    });
    await browser.sleep(5000);
  });

  it('should search fwlogs ', async () => {
    browser.sleep(2000);
    await E2EuiTools.setInputBoxValue('.fwlogs .fwlogs-search .fwlogs-inputport.ui-inputtext[formcontrolname="source-ports"]', sourcePorts);
    await E2EuiTools.setInputBoxValue('.fwlogs .fwlogs-search .fwlogs-inputport.ui-inputtext[formcontrolname="dest-ports"]', destPorts);
    browser.sleep(1000);
    await E2EuiTools.clickElement('.fwlogs .fwlogs-search .global-button-primary.fwlogs-button-search');
    browser.sleep(2000);
    let sourcePortMatch = true;
    let destPortMatch = true;
    await appPage.verifyTableHasContents(
      (rowIdx: number, columnIdx: number, rowValues: any[]) => {
        if (columnIdx === 3) { // sourcePort column
          const celValueSrcPorts = rowValues[columnIdx];
          if (celValueSrcPorts !== sourcePorts) {
            sourcePortMatch = false;
          }
        }
        if (columnIdx === 4) { // destinationPort column
          const celValueDstPorts = rowValues[columnIdx];
          if (celValueDstPorts !== destPorts) {
            destPortMatch = false;
          }
        }
      },
      () => {
        expect(sourcePortMatch && destPortMatch).toBeTruthy('FwLog table');
      }
    );
    await browser.sleep(5000);
  });

});
