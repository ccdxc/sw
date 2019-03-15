import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { Auditevents } from './page-objects/auditevents.po';
import { AppPage } from './page-objects/app.po';
import { FieldSelectorCriteria } from './page-objects';

describe('venice-ui auditevents', () => {
  let auditeventsPage: Auditevents;
  let appPage: AppPage;
  let loginPage: LoginPage;

  let username = 'test';

  beforeEach(async (done) => {
    appPage = new AppPage();
    appPage.setContainerCSS('app-auditevents');
    auditeventsPage = new Auditevents();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
    await auditeventsPage.navigateTo();
    await auditeventsPage.verifyPage();
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
   * Test to verify there are audit events
   */
  it('should have events in the table', async () => {
    const randomIndex = Math.floor(Math.random() * (10 - 1)); // randomly pick a row between 0-10
    await appPage.verifyTableHasContents((rowIdx: number, columnIdx: number, rowValues: any[]) => {
      const actionText = rowValues[2]; // action column index is 2
      if (columnIdx === 8 && actionText === 'login') {
        const clientIpValue = rowValues[8]; // client-ips column index is 2
        expect(clientIpValue).toBe('');
      } else {
        const celValue = rowValues[columnIdx];
        expect(celValue).not.toBe('');
      }
      if (randomIndex === rowIdx ) {
        username = rowValues[0]; // the username
      }
    });
  });

  /**
   * Test using filtering audit events.
   * 1. configure criteria
   * 2. click search buttons and verify returned data
   */
  it('should filter table', async () => {
    const values: FieldSelectorCriteria[] = [
        {
          key: 'action',
          operator: 'not equals',
          value: 'login'
        },
        {
          key: 'user.name',
          operator: 'equals',
          value: 'test'
        }
    ];
    await appPage.setFieldSelectorValues(values);

    // click search button
    const searchButton = await element(by.css('.global-button-primary.auditevents-searchbutton.auditevents-searchbutton-save'));
    await searchButton.click();
    await browser.sleep(5000);

    let isAllactionLogin: boolean = true;
    let isAlluserTest: boolean = true;
    const tableData = await appPage.getTableContent();
    for (let i = 0; i < tableData.length; i++) {
      const rowValues = tableData[i];
      for (let j = 0; j < rowValues.length; j++) {
        const columnIdx = j;
        if (columnIdx === 0) { // user column
          const userText = rowValues[0];
          if (userText !== username) {
            isAllactionLogin = false;
          }
        }
        if (columnIdx === 2) { // action column
          const actionText = rowValues[columnIdx];
          if (actionText === 'login') {
            isAlluserTest = false;
            break;
          }
        }
      }
    }
    expect(isAllactionLogin).toBeTruthy('action should not be "login"');
    expect(isAlluserTest).toBeTruthy('login user should be ' + username );
  });

});
