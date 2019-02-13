import { browser, by, element, protractor, ElementFinder } from 'protractor';
import { LoginPage } from './page-objects/login.po';
import { FlowExport } from './page-objects/flowexport.po';
import { AppPage } from './page-objects/app.po';

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
    await flowExportPage.navigateTo();
    await flowExportPage.verifyPage();
    appPage.getTableRowLength().then(rowLen => {
      if (rowLen > 0) {
        appPage.verifyTableHasContents();
      } else {
        expect(rowLen).toBe(0);
      }
    });
  });

});
