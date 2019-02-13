import { browser, by, element, protractor, ElementFinder  } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { Auditevents } from './page-objects/auditevents.po';
import { AppPage } from './page-objects/app.po';

describe('venice-ui auditevents', () => {
  let auditeventsPage: Auditevents;
  let appPage: AppPage;
  let loginPage: LoginPage;

  beforeEach(async (done) => {
    appPage = new AppPage();
    auditeventsPage = new Auditevents();
    loginPage = new LoginPage();
    await loginPage.navigateTo();
    await loginPage.login();
    browser.waitForAngularEnabled(false);
    const until = protractor.ExpectedConditions;
    await browser.wait(until.presenceOf(element(by.css('.app-shell-container'))), 10000, 'Element taking too long to appear in the DOM');
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

  it('should have events in the table', async () => {
    await auditeventsPage.navigateTo();
    await auditeventsPage.verifyPage();
    await appPage.verifyTableHasContents((rowIdx: number, columnIdx: number, rowValues: any[])  => {
        const actionText = rowValues[2]; // action column index is 2
        if (columnIdx === 8 && actionText === 'login') {
            const clientIpValue = rowValues[8]; // client-ips column index is 2
            expect(clientIpValue).toBe('');
        } else {
            const celValue = rowValues[columnIdx];
            expect(celValue).not.toBe('');
        }
    });
  });

});
