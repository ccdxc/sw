import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { FirewallLogs } from './page-objects/firewallogs.po';
import { AppPage } from './page-objects/app.po';
import { E2EuiTools } from './page-objects/E2EuiTools';

describe('venice-ui fwlogs', () => {
  let firewallLogsPage: FirewallLogs;
  let appPage: AppPage;
  let loginPage: LoginPage;
  let sourcePorts = 0;
  let destPorts = 0;

  const sourceColumn = 4;
  const destinationColumn = 5;

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
    const tableData = await appPage.getTableContent();
    expect(tableData.length > 0).toBeTruthy('firewall log should always have records');
    if (tableData.length > 0) {
      // if Venice just starts up, there might be no fw-log record at all
      const randomIndex = Math.floor(Math.random() * (tableData.length - 1)); // randomly pick a row between 0-10
      const rowValues = tableData[randomIndex];
      sourcePorts = rowValues[sourceColumn];
      destPorts = rowValues[destinationColumn];
      console.log('Get fwLog table search ports. ' + sourcePorts + ' ' + destPorts);
      await browser.sleep(5000);
    }
  });

  it('should search fwlogs ', async () => {
    console.log('FwLog table search. Port criteria: ' + sourcePorts + ' ' + destPorts);
    browser.sleep(3000);
    const tableLenght = await appPage.getTableRowLength();
    if (tableLenght === 0) {
      console.log('fwlog table has no data. We can not search record');
      expect(tableLenght).toBe(0, 'firewall log has no record');
    } else {
      await E2EuiTools.setInputBoxValue('.fwlogs .fwlogs-search .fwlogs-inputport.ui-inputtext[formcontrolname="source-ports"]', sourcePorts);
      await E2EuiTools.setInputBoxValue('.fwlogs .fwlogs-search .fwlogs-inputport.ui-inputtext[formcontrolname="dest-ports"]', destPorts);
      browser.sleep(1000);
      await E2EuiTools.clickElement('.fwlogs .fwlogs-search .global-button-primary.fwlogs-button-search');
      browser.sleep(5000);
      let sourcePortMatch = true;
      let destPortMatch = true;


      const tableData = await appPage.getTableContent();
      // debug: console.log('should search fwlogs ', tableData.length, tableData);
      expect(tableData.length > 0).toBeTruthy('firewall log should search records');
      for (let i = 0; i < tableData.length; i++) {
        const rowValues = tableData[i];
        for (let j = 0; j < rowValues.length; j++) {
          const columnIdx = j;
          if (columnIdx === sourceColumn) { // sourcePort column
            const celValueSrcPorts = rowValues[columnIdx];
            if (celValueSrcPorts !== sourcePorts) {
              sourcePortMatch = false;
              break;
            }
          }
          if (columnIdx === destinationColumn) { // destinationPort column
            const celValueDstPorts = rowValues[columnIdx];
            if (celValueDstPorts !== destPorts) {
              destPortMatch = false;
              break;
            }
          }
        }
      }
      expect(sourcePortMatch && destPortMatch).toBeTruthy('FwLog table search success. ' + sourcePorts + ' ' + destPorts);
    }
    await browser.sleep(5000);  // must put a timer here to avoid weired browser behavior.
  });

});
